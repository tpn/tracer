// Copyright Trent Nelson <trent@trent.me>

#include <Windows.h>
#include <Strsafe.h>
#include "Tracing.h"

INIT_ONCE InitOnceFindLongestTraceStoreFileName = INIT_ONCE_STATIC_INIT;

INIT_ONCE InitOnceSystemTimerFunction = INIT_ONCE_STATIC_INIT;

static DWORD DefaultTraceStoreCriticalSectionSpinCount = 4000;

#define MAX_UNICODE_STRING 255
#define _OUR_MAX_PATH MAX_UNICODE_STRING

BOOL
CALLBACK
FindLongestTraceStoreFileNameCallback(
    PINIT_ONCE InitOnce,
    PVOID Parameter,
    PVOID *lpContext
)
{
    DWORD Index;
    ULARGE_INTEGER Longest = { 0 };
    ULARGE_INTEGER Length = { 0 };
    DWORD MaxPath = (
        _OUR_MAX_PATH   -
        3               - /* C:\ */
        1               - // NUL
        TraceStoreMetadataSuffixLength
    );

    for (Index = 0; Index < NumberOfTraceStores; Index++) {
        LPCWSTR FileName = TraceStoreFileNames[Index];
        if (FAILED(StringCchLengthW(FileName, MaxPath, &Length.QuadPart))) {
            return FALSE;
        }
        if (Length.QuadPart > Longest.QuadPart) {
            Longest.QuadPart = Length.QuadPart;
        }
    }
    Longest.QuadPart += TraceStoreMetadataSuffixLength;

    *((PDWORD64)lpContext) = Longest.QuadPart;

    return TRUE;
}

DWORD
GetLongestTraceStoreFileName()
{
    BOOL  Status;
    ULARGE_INTEGER Longest;

    Status = InitOnceExecuteOnce(
        &InitOnceFindLongestTraceStoreFileName,
        FindLongestTraceStoreFileNameCallback,
        NULL,
        (LPVOID *)&Longest.QuadPart
    );

    if (!Status || Longest.HighPart != 0) {
        return 0;
    } else {
        return Longest.LowPart;
    }
}

BOOL
CALLBACK
GetSystemTimerFunctionCallback(
    _Inout_     PINIT_ONCE  InitOnce,
    _Inout_     PVOID       Parameter,
    _Inout_opt_ PVOID       *lpContext
)
{
    HMODULE Module;
    FARPROC Proc;
    static SYSTEM_TIMER_FUNCTION SystemTimerFunction = { 0 };

    if (!lpContext) {
        return FALSE;
    }

    Module = GetModuleHandle(TEXT("kernel32"));
    if (Module == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "GetSystemTimePreciseAsFileTime");
    if (Proc) {
        SystemTimerFunction.GetSystemTimePreciseAsFileTime = (PGETSYSTEMTIMEPRECISEASFILETIME)Proc;
    } else {
        Module = LoadLibrary(TEXT("ntdll"));
        if (!Module) {
            return FALSE;
        }
        Proc = GetProcAddress(Module, "NtQuerySystemTime");
        if (!Proc) {
            return FALSE;
        }
        SystemTimerFunction.NtQuerySystemTime = (PNTQUERYSYSTEMTIME)Proc;
    }

    *((PPSYSTEM_TIMER_FUNCTION)lpContext) = &SystemTimerFunction;
    return TRUE;
}

PSYSTEM_TIMER_FUNCTION
GetSystemTimerFunction()
{
    BOOL Status;
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction;

    Status = InitOnceExecuteOnce(
        &InitOnceSystemTimerFunction,
        GetSystemTimerFunctionCallback,
        NULL,
        (LPVOID *)&SystemTimerFunction
    );

    if (!Status) {
        return NULL;
    } else {
        return SystemTimerFunction;
    }
}

_Check_return_
BOOL
CallSystemTimer(
    _Out_       PFILETIME   SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
)
{
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction = NULL;

    if (ppSystemTimerFunction) {
        if (*ppSystemTimerFunction) {
            SystemTimerFunction = *ppSystemTimerFunction;
        } else {
            SystemTimerFunction = GetSystemTimerFunction();
            *ppSystemTimerFunction = SystemTimerFunction;
        }
    } else {
        SystemTimerFunction = GetSystemTimerFunction();
    }

    if (!SystemTimerFunction) {
        return FALSE;
    }

    if (SystemTimerFunction->GetSystemTimePreciseAsFileTime) {
        SystemTimerFunction->GetSystemTimePreciseAsFileTime(SystemTime);
    } else if (SystemTimerFunction->NtQuerySystemTime) {
        if (!SystemTimerFunction->NtQuerySystemTime((PLARGE_INTEGER)SystemTime)) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    return TRUE;
}

BOOL
RefreshTraceStoreFileInfo(PTRACE_STORE TraceStore)
{
    if (!TraceStore) {
        return FALSE;
    }

    return GetFileInformationByHandleEx(
        TraceStore->FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        &TraceStore->FileInfo,
        sizeof(TraceStore->FileInfo)
    );
}

BOOL
SetTraceStoreCriticalSection(
    _Inout_     PTRACE_STORE        TraceStore,
    _In_        PCRITICAL_SECTION   CriticalSection
)
{
    if (!TraceStore || !CriticalSection) {
        return FALSE;
    }

    TraceStore->CriticalSection = CriticalSection;

    return TRUE;
}

BOOL
InitializeStore(
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _In_opt_    DWORD InitialSize
)
{
    BOOL Success;

    if (!Path || !TraceStore) {
        return FALSE;
    }

    if (!TraceStore->FileHandle) {
        TraceStore->FileHandle = CreateFileW(
            Path,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_FLAG_OVERLAPPED,
            NULL
        );
    }

    if (TraceStore->FileHandle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    if (!RefreshTraceStoreFileInfo(TraceStore)) {
        goto error;
    }

    TraceStore->MappingSize.HighPart = 0;
    TraceStore->MappingSize.LowPart = InitialSize;
    //TraceStore->MappingSize.LowPart = (2 << 30);

    // If the allocated size of the underlying file is less than our desired
    // mapping size (which is primed by the InitialSize parameter), extend the
    // file to that length (via SetFilePointerEx(), then SetEndOfFile()).
    if (TraceStore->FileInfo.AllocationSize.QuadPart < TraceStore->MappingSize.QuadPart) {
        LARGE_INTEGER StartOfFile = { 0 };
        Success = SetFilePointerEx(
            TraceStore->FileHandle,
            TraceStore->MappingSize,
            NULL,
            FILE_BEGIN
        );
        if (!Success) {
            goto error;
        }
        // Extend the file.
        if (!SetEndOfFile(TraceStore->FileHandle)) {
            goto error;
        }
        // Reset the file pointer back to the start.
        Success = SetFilePointerEx(
            TraceStore->FileHandle,
            StartOfFile,
            NULL,
            FILE_BEGIN
        );
        if (!Success) {
            goto error;
        }
    }

    TraceStore->MappingHandle = CreateFileMapping(
        TraceStore->FileHandle,
        NULL,
        PAGE_READWRITE,
        0,
        TraceStore->MappingSize.LowPart,
        NULL
    );

    if (TraceStore->MappingHandle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    TraceStore->BaseAddress = MapViewOfFile(
        TraceStore->MappingHandle,
        FILE_MAP_READ | FILE_MAP_WRITE,
        0,
        0,
        TraceStore->MappingSize.LowPart
    );

    if (!TraceStore->BaseAddress) {
        goto error;
    }

    TraceStore->PrevAddress = NULL;
    TraceStore->NextAddress = TraceStore->BaseAddress;

    TraceStore->AllocateRecords = AllocateRecords;

    return TRUE;
error:
    CloseTraceStore(TraceStore);
    return FALSE;
}

BOOL
InitializeTraceStore(
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _Inout_     PTRACE_STORE MetadataStore,
    _In_opt_    DWORD InitialSize
)
{
    BOOL Success;
    HRESULT Result;
    WCHAR MetadataPath[_OUR_MAX_PATH];

    if (!Path || !TraceStore) {
        return FALSE;
    }

    SecureZeroMemory(&MetadataPath, sizeof(MetadataPath));
    Result = StringCchCopyW(
        &MetadataPath[0],
        _OUR_MAX_PATH,
        Path
    );
    if (FAILED(Result)) {
        return FALSE;
    }

    Result = StringCchCatW(
        &MetadataPath[0],
        _OUR_MAX_PATH,
        TraceStoreMetadataSuffix
    );
    if (FAILED(Result)) {
        return FALSE;
    }

    // Create the data file first, before the :metadata stream.
    TraceStore->FileHandle = CreateFileW(
        Path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (TraceStore->FileHandle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    Success = InitializeStore(
        &MetadataPath[0],
        MetadataStore,
        sizeof(TRACE_STORE_METADATA)
    );

    if (!Success) {
        return FALSE;
    }

    MetadataStore->NumberOfRecords.QuadPart = 1;
    MetadataStore->RecordSize.QuadPart = sizeof(TRACE_STORE_METADATA);

    TraceStore->MetadataStore = MetadataStore;
    TraceStore->pMetadata = (PTRACE_STORE_METADATA)MetadataStore->BaseAddress;

    if (!InitializeStore(Path, TraceStore, InitialSize)) {
        goto error;
    }

    return TRUE;
error:
    CloseTraceStore(TraceStore);
    return FALSE;
}

PTRACE_STORES
GetMetadataStoresFromTracesStores(
    _In_    PTRACE_STORES   TraceStores
)
{
    return (PTRACE_STORES)(
        (DWORD_PTR)&TraceStores->Stores[0] + 
        (sizeof(TRACE_STORE) * TraceStores->NumberOfTraceStores)
    );
}

BOOL
InitializeTraceStores(
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PDWORD          SizeOfTraceStores,
    _In_opt_    PDWORD          InitialFileSizes
)
{
    BOOL Success;
    HRESULT Result;
    DWORD Index;
    DWORD LastError;
    WCHAR Path[_OUR_MAX_PATH];
    LPWSTR FileNameDest;
    PTRACE_STORES MetadataStores;
    DWORD LongestFilename = GetLongestTraceStoreFileName();
    DWORD TraceStoresAllocationSize = GetTraceStoresAllocationSize();
    DWORD LongestPossibleDirectoryLength = (
        _OUR_MAX_PATH   -
        1               - // '\'
        1               - // final NUL
        LongestFilename
    );
    LARGE_INTEGER DirectoryLength;
    LARGE_INTEGER RemainingChars;
    LPDWORD Sizes = InitialFileSizes;

    if (!SizeOfTraceStores) {
        return FALSE;
    }

    if (!TraceStores || *SizeOfTraceStores < TraceStoresAllocationSize) {
        *SizeOfTraceStores = TraceStoresAllocationSize;
        return FALSE;
    }

    if (!BaseDirectory) {
        return FALSE;
    }

    MetadataStores = GetMetadataStoresFromTracesStores(TraceStores);

    if (!Sizes) {
        Sizes = (LPDWORD)&InitialTraceStoreFileSizes[0];
    }

    Result = StringCchLengthW(
        BaseDirectory,
        LongestPossibleDirectoryLength,
        &DirectoryLength.QuadPart
    );

    if (FAILED(Result)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (DirectoryLength.HighPart != 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SecureZeroMemory(&Path, sizeof(Path));

    Result = StringCchCopyW(
        &Path[0],
        LongestPossibleDirectoryLength,
        BaseDirectory
    );
    if (FAILED(Result)) {
        return ERROR_INVALID_PARAMETER;
    }

    Path[DirectoryLength.LowPart] = L'\\';
    FileNameDest = &Path[DirectoryLength.LowPart+1];
    RemainingChars.QuadPart = (
        _OUR_MAX_PATH -
        DirectoryLength.LowPart -
        2
    );

    SecureZeroMemory(TraceStores, sizeof(TraceStores));

    Success = CreateDirectory(BaseDirectory, NULL);
    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            SetLastError(LastError);
            return FALSE;
        }
    }

    for (Index = 0; Index < NumberOfTraceStores; Index++) {
        PTRACE_STORE TraceStore = &TraceStores->Stores[Index];
        PTRACE_STORE MetadataStore = &MetadataStores->Stores[Index];
        LPCWSTR FileName = TraceStoreFileNames[Index];
        DWORD InitialSize = Sizes[Index];
        Result = StringCchCopyW(FileNameDest, RemainingChars.QuadPart, FileName);
        if (FAILED(Result)) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        Success = InitializeTraceStore(
            Path,
            TraceStore,
            MetadataStore,
            InitialSize
        );
        if (!Success) {
            return FALSE;
        }
    }

    return TRUE;
}

VOID
CloseStore(
    _In_ PTRACE_STORE TraceStore
)
{
    if (!TraceStore) {
        return;
    }

    if (TraceStore->CriticalSection) {
        EnterCriticalSection(TraceStore->CriticalSection);
    }

    if (TraceStore->BaseAddress) {
        FlushViewOfFile(TraceStore->BaseAddress, 0);
        UnmapViewOfFile(TraceStore->BaseAddress);
        TraceStore->BaseAddress = NULL;
    }

    if (TraceStore->MappingHandle) {
        CloseHandle(TraceStore->MappingHandle);
        TraceStore->MappingHandle = NULL;
    }

    if (TraceStore->FileHandle) {
        FlushFileBuffers(TraceStore->FileHandle);
        CloseHandle(TraceStore->FileHandle);
        TraceStore->FileHandle = NULL;
    }

    if (TraceStore->CriticalSection) {
        LeaveCriticalSection(TraceStore->CriticalSection);
    }
}

VOID
CloseTraceStore(
    _In_ PTRACE_STORE TraceStore
)
{
    if (!TraceStore) {
        return;
    }

    if (TraceStore->MetadataStore) {
        CloseStore(TraceStore->MetadataStore);
        TraceStore->MetadataStore = NULL;
    }

    CloseStore(TraceStore);
}

void
CloseTraceStores(PTRACE_STORES TraceStores)
{
    DWORD Index;

    if (!TraceStores) {
        return;
    }

    for (Index = 0; Index < NumberOfTraceStores; Index++) {
        CloseTraceStore(&TraceStores->Stores[Index]);
    }
}

BOOL
GetTraceStoreBytesWritten(
    PTRACE_STORE TraceStore,
    PULARGE_INTEGER BytesWritten
)
{
    if (!TraceStore || !TraceStore->BaseAddress || !BytesWritten) {
        return FALSE;
    }

    if (TraceStore->CriticalSection) {
        EnterCriticalSection(TraceStore->CriticalSection);
    }

    BytesWritten->QuadPart = (
        (DWORD_PTR)TraceStore->NextAddress -
        (DWORD_PTR)TraceStore->BaseAddress
    );

    if (TraceStore->CriticalSection) {
        LeaveCriticalSection(TraceStore->CriticalSection);
    }

    return TRUE;
}

BOOL
GetTraceStoreNumberOfRecords(
    PTRACE_STORE TraceStore,
    PULARGE_INTEGER NumberOfRecords
)
{
    if (!TraceStore) {
        return FALSE;
    }

    if (TraceStore->CriticalSection) {
        EnterCriticalSection(TraceStore->CriticalSection);
    }

    NumberOfRecords->QuadPart = TraceStore->pMetadata->NumberOfRecords.QuadPart;

    if (TraceStore->CriticalSection) {
        LeaveCriticalSection(TraceStore->CriticalSection);
    }
    return TRUE;
}

DWORD
GetTraceStoresAllocationSize(void)
{
    // Account for the metadata stores, which are located after
    // the trace stores.
    return sizeof(TRACE_STORES) * 2;
}

_Check_return_
LPVOID
AllocateRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    ULARGE_INTEGER  RecordSize,
    _In_    ULARGE_INTEGER  NumberOfRecords
)
{
    DWORD_PTR AllocationSize;

    if (!TraceStore) {
        return NULL;
    }

    if (sizeof(TraceStore->NextAddress) == sizeof(RecordSize.QuadPart)) {
        AllocationSize = (DWORD_PTR)(
            RecordSize.QuadPart *
            NumberOfRecords.QuadPart
        );
    } else {
        ULARGE_INTEGER Size = { 0 };
        // Ignore allocation attempts over 2GB on 32-bit.
        if (RecordSize.HighPart != 0 || NumberOfRecords.HighPart != 0) {
            return NULL;
        }
        Size.QuadPart = UInt32x32To64(RecordSize.LowPart, NumberOfRecords.LowPart);
        if (Size.HighPart != 0) {
            return NULL;
        }
        AllocationSize = (DWORD_PTR)Size.LowPart;
    }

    if (TraceStore->CriticalSection) {
        EnterCriticalSection(TraceStore->CriticalSection);
    }

    if (!TraceStore->pMetadata->RecordSize.QuadPart) {
        TraceStore->pMetadata->RecordSize.QuadPart = RecordSize.QuadPart;
    }

    if (TraceStore->pMetadata->RecordSize.QuadPart != RecordSize.QuadPart) {
        if (TraceStore->CriticalSection) {
            LeaveCriticalSection(TraceStore->CriticalSection);
        }
        return NULL;
    }

    TraceStore->PrevAddress = TraceStore->NextAddress;

    TraceStore->NextAddress = (LPVOID)((ULONG_PTR)TraceStore->PrevAddress + AllocationSize);

    TraceStore->pMetadata->NumberOfRecords.QuadPart += NumberOfRecords.QuadPart;

    if (TraceStore->CriticalSection) {
        LeaveCriticalSection(TraceStore->CriticalSection);
    }

    return TraceStore->PrevAddress;
}

LPVOID
GetNextRecord(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    ULARGE_INTEGER RecordSize
)
{
    ULARGE_INTEGER RecordCount = { 1 };
    return AllocateRecords(TraceContext, TraceStore, RecordSize, RecordCount);
}

BOOL
InitializeTraceSession(
    _Inout_bytecap_(*SizeOfTraceSession) PTRACE_SESSION TraceSession,
    _In_                                 PDWORD         SizeOfTraceSession
)
{
    if (!TraceSession) {
        if (SizeOfTraceSession) {
            *SizeOfTraceSession = sizeof(*TraceSession);
        }
        return FALSE;
    }

    if (!SizeOfTraceSession) {
        return FALSE;
    }

    if (*SizeOfTraceSession < sizeof(*TraceSession)) {
        return FALSE;
    } else if (*SizeOfTraceSession == 0) {
        *SizeOfTraceSession = sizeof(*TraceSession);
    }

    SecureZeroMemory(TraceSession, sizeof(*TraceSession));

    GetSystemTimeAsFileTime(&TraceSession->SystemTime);
    return TRUE;

}

BOOL
InitializeTraceContext(
    _Inout_bytecap_(*SizeOfTraceContext)    PTRACE_CONTEXT  TraceContext,
    _In_                                    PDWORD          SizeOfTraceContext,
    _In_                                    PTRACE_SESSION  TraceSession,
    _In_                                    PTRACE_STORES   TraceStores,
    _In_opt_                                PVOID           UserData
)
{
    if (!TraceContext) {
        if (SizeOfTraceContext) {
            *SizeOfTraceContext = sizeof(*TraceContext);
        }
        return FALSE;
    }

    if (!SizeOfTraceContext) {
        return FALSE;
    }

    if (*SizeOfTraceContext < sizeof(*TraceContext)) {
        return FALSE;
    }

    if (!TraceSession) {
        return FALSE;
    }

    if (!TraceStores) {
        return FALSE;
    }

    TraceContext->SystemTimerFunction = GetSystemTimerFunction();
    if (!TraceContext->SystemTimerFunction) {
        return FALSE;
    }

    TraceContext->Size = *SizeOfTraceContext;
    TraceContext->TraceSession = TraceSession;
    TraceContext->TraceStores = TraceStores;
    TraceContext->SequenceId = 1;
    TraceContext->UserData = UserData;

    return TRUE;
}

VOID
RegisterName(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       NameToken,
    _In_    PCWSTR          Name
)
{

}

VOID
RegisterFunction(
    _Inout_     PTRACE_CONTEXT  TraceContext,
    _In_        DWORD_PTR       FunctionToken,
    _In_        PCWSTR          FunctionName,
    _In_        DWORD           LineNumber,
    _In_opt_    DWORD_PTR       ModuleToken,
    _In_opt_    PCWSTR          ModuleName,
    _In_opt_    PCWSTR          ModuleFilename
)
{

}

#ifdef __cpp
} // extern "C"
#endif
