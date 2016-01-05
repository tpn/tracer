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
RefreshTraceStoreMemoryMapFileInfo(PTRACE_STORE_MEMORY_MAP TraceStoreMemoryMap)
{
    if (!TraceStoreMemoryMap) {
        return FALSE;
    }

    return GetFileInformationByHandleEx(
        TraceStoreMemoryMap->FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        &TraceStoreMemoryMap->FileInfo,
        sizeof(TraceStoreMemoryMap->FileInfo)
    );
}


BOOL
InitializeStore(
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _In_opt_    DWORD InitialSize
)
{
    BOOL Success;
    BOOL IsMetadataStore = FALSE;
    LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

    if (!Path || !TraceStore) {
        return FALSE;
    }

    if (!TraceStore->FileHandle) {
        // We're a metadata store.
        IsMetadataStore = TRUE;
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

    TraceStore->InitialSize.HighPart = 0;
    TraceStore->InitialSize.LowPart = InitialSize;
    TraceStore->ExtensionSize.HighPart = 0;
    TraceStore->ExtensionSize.LowPart = InitialSize;
    TraceStore->MaximumSize.QuadPart = 0LL;

    TraceStore->MappingSize.HighPart = 0;
    TraceStore->MappingSize.LowPart = InitialSize;

    // Cap the mapping size to the maximum if necessary.
    if (TraceStore->MappingSize.QuadPart > MaximumMappingSize.QuadPart) {
        TraceStore->MappingSize.QuadPart = MaximumMappingSize.QuadPart;
    }

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
        // Update the file info.
        if (!Success || !RefreshTraceStoreFileInfo(TraceStore)) {
            goto error;
        }
    }
    // Alternatively, if we're not a metadata trace store (in which case, TraceStore->MetadataStore
    // would be NULL), if the existing file has a size greater than the initial size, use a larger
    // mapping size, provided it's under our maximum mapping size.
    else if (TraceStore->MetadataStore &&
             TraceStore->FileInfo.AllocationSize.QuadPart > TraceStore->MappingSize.QuadPart) {

        TraceStore->MappingSize.QuadPart = TraceStore->FileInfo.AllocationSize.QuadPart;

        if (TraceStore->MappingSize.QuadPart > MaximumMappingSize.QuadPart) {
            TraceStore->MappingSize.QuadPart = MaximumMappingSize.QuadPart;
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
    TraceStore->EndAddress = (PVOID)RtlOffsetToPointer(TraceStore->BaseAddress,
                                                       TraceStore->MappingSize.LowPart);

    TraceStore->AllocateRecords = AllocateRecords;

    InitializeCriticalSectionAndSpinCount(
        &TraceStore->CriticalSection,
        DefaultTraceStoreCriticalSectionSpinCount
    );

    if (IsMetadataStore) {
        return TRUE;
    }

    InitializeCriticalSectionAndSpinCount(
        &TraceStore->NextTraceStoreMemoryMap.CriticalSection,
        DefaultTraceStoreCriticalSectionSpinCount
    );

    InitializeCriticalSectionAndSpinCount(
        &TraceStore->LastTraceStoreMemoryMap.CriticalSection,
        DefaultTraceStoreCriticalSectionSpinCount
    );

    //
    // We submit a file extension threadpool work when we're "InitialSize"-bytes away
    // from the end address.  This will probably need tuning.
    //
    TraceStore->ExtendAtAddress = (PVOID)RtlOffsetFromPointer(TraceStore->EndAddress,
                                                              TraceStore->InitialSize.LowPart);

    //
    // For now, we only use a single file handle (i.e. single file) with
    // multiple memory mappings.  Down the track we may add support for
    // multiple files to multiple memory mappings.
    //
    TraceStore->NextTraceStoreMemoryMap.FileHandle = TraceStore->FileHandle;

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
        OPEN_ALWAYS,
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
    DWORD Index, StoreIndex;
    DWORD LastError;
    WCHAR Path[_OUR_MAX_PATH];
    LPWSTR FileNameDest;
    DWORD LongestFilename = GetLongestTraceStoreFileName();
    DWORD TraceStoresAllocationSize = GetTraceStoresAllocationSize(NumberOfTraceStores);
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

    for (Index = 0, StoreIndex = 0; Index < NumberOfTraceStores; Index++, StoreIndex += 2) {
        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex];
        PTRACE_STORE MetadataStore = TraceStore+1;
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

    TraceStores->NumberOfTraceStores = NumberOfTraceStores;

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

    EnterCriticalSection(&TraceStore->CriticalSection);

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

    if (TraceStore->FileExtendedEvent) {
        CloseHandle(TraceStore->FileExtendedEvent);
        TraceStore->FileExtendedEvent = NULL;
    }

    if (TraceStore->ExtendFileWork) {
        CloseThreadpoolWork(TraceStore->ExtendFileWork);
        TraceStore->ExtendFileWork = NULL;
    }

    if (TraceStore->PrefaultFuturePageWork) {
        CloseThreadpoolWork(TraceStore->PrefaultFuturePageWork);
        TraceStore->PrefaultFuturePageWork = NULL;
    }

    LeaveCriticalSection(&TraceStore->CriticalSection);
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
    USHORT Index;

    if (!TraceStores) {
        return;
    }

    for (Index = 0; Index < NumberOfTraceStores * 2; Index += 2) {
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

    EnterCriticalSection(&TraceStore->CriticalSection);

    BytesWritten->QuadPart = (
        (DWORD_PTR)TraceStore->NextAddress -
        (DWORD_PTR)TraceStore->BaseAddress
    );

    LeaveCriticalSection(&TraceStore->CriticalSection);

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

    EnterCriticalSection(&TraceStore->CriticalSection);

    NumberOfRecords->QuadPart = TraceStore->pMetadata->NumberOfRecords.QuadPart;

    LeaveCriticalSection(&TraceStore->CriticalSection);

    return TRUE;
}

DWORD
GetTraceStoresAllocationSize(const USHORT NumberOfTraceStores)
{
    // For each trace store, we also store a metadata trace store, hence the
    // "NumberOfTraceStores * 2" part.  The subsequent "minus size-of-1-trace-store"
    // accounts for the fact that the TRACE_STORES structure includes a single trace
    // store structure, e.g.:
    //      typedef struct _TRACE_STORES {
    //          ...
    //          TRACE_STORE Stores[1];
    //
    // So if NumberOfTraceStores is 6, there will actually be space for
    // 12 TRACE_STORE structures.
    return (
        sizeof(TRACE_STORES) + (
            (sizeof(TRACE_STORE) * NumberOfTraceStores * 2) -
            sizeof(TRACE_STORE)
        )
    );
}

VOID
CALLBACK
PrefaultFuturePageCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
)
{
    PTRACE_STORE TraceStore = (PTRACE_STORE)Context;
    PULONG FaultAddress;
    EnterCriticalSection(&TraceStore->CriticalSection);
    RtlCopyMemory(&FaultAddress, TraceStore->PrefaultAddress, sizeof(PULONG));
    LeaveCriticalSection(&TraceStore->CriticalSection);
    *FaultAddress = 0;
}

BOOL
ExtendTraceStoreFile(_Inout_ PTRACE_STORE TraceStore)
{
    BOOL Success;
    PTRACE_CONTEXT TraceContext = TraceStore->TraceContext;
    PTRACE_STORE_MEMORY_MAP NextMemoryMap = &TraceStore->NextTraceStoreMemoryMap;
    LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

    if (!NextMemoryMap->FileHandle) {
        return FALSE;
    }

    // We don't support extension of metadata stores at the moment.
    if (!TraceStore->MetadataStore) {
        return FALSE;
    }

    EnterCriticalSection(&NextMemoryMap->CriticalSection);

    if (!RefreshTraceStoreMemoryMapFileInfo(NextMemoryMap)) {
        goto error;
    }

    NextMemoryMap->MappingSize.HighPart = 0;
    NextMemoryMap->MappingSize.LowPart = TraceStore->InitialSize.LowPart;

    // Cap the mapping size to the maximum if necessary.
    if (NextMemoryMap->MappingSize.QuadPart > MaximumMappingSize.QuadPart) {
        NextMemoryMap->MappingSize.QuadPart = MaximumMappingSize.QuadPart;
    }

    //
    // If the allocated file size is less than the mapping size, extend the file.
    //
    if (NextMemoryMap->FileInfo.AllocationSize.QuadPart < NextMemoryMap->MappingSize.QuadPart) {
        Success = SetFilePointerEx(
            NextMemoryMap->FileHandle,
            NextMemoryMap->MappingSize,
            NULL,
            FILE_BEGIN
        );
        if (!Success) {
            goto error;
        }
        // Extend the file.
        if (!SetEndOfFile(NextMemoryMap->FileHandle)) {
            goto error;
        }
    }
    // Alternatively, if we're not a metadata trace store (in which case, TraceStore->MetadataStore
    // would be NULL), if the existing file has a size greater than the initial size, use a larger
    // mapping size, provided it's under our maximum mapping size.
    else if (NextMemoryMap->FileInfo.AllocationSize.QuadPart > NextMemoryMap->MappingSize.QuadPart) {

        NextMemoryMap->MappingSize.QuadPart = NextMemoryMap->FileInfo.AllocationSize.QuadPart;

        if (NextMemoryMap->MappingSize.QuadPart > MaximumMappingSize.QuadPart) {
            NextMemoryMap->MappingSize.QuadPart = MaximumMappingSize.QuadPart;
        }
    }

    NextMemoryMap->MappingHandle = CreateFileMapping(
        NextMemoryMap->FileHandle,
        NULL,
        PAGE_READWRITE,
        NextMemoryMap->MappingSize.HighPart,
        NextMemoryMap->MappingSize.LowPart,
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
    TraceStore->EndAddress = (PVOID)RtlOffsetToPointer(TraceStore->BaseAddress,
                                                       TraceStore->MappingSize.LowPart);

    TraceStore->AllocateRecords = AllocateRecords;

    InitializeCriticalSectionAndSpinCount(
        &TraceStore->CriticalSection,
        DefaultTraceStoreCriticalSectionSpinCount
    );

    if (IsMetadataStore) {
        return TRUE;
    }

    InitializeCriticalSectionAndSpinCount(
        &TraceStore->NextTraceStoreMemoryMap.CriticalSection,
        DefaultTraceStoreCriticalSectionSpinCount
    );

    InitializeCriticalSectionAndSpinCount(
        &TraceStore->LastTraceStoreMemoryMap.CriticalSection,
        DefaultTraceStoreCriticalSectionSpinCount
    );

    //
    // We submit a file extension threadpool work when we're "InitialSize"-bytes away
    // from the end address.  This will probably need tuning.
    //
    TraceStore->ExtendAtAddress = (PVOID)RtlOffsetFromPointer(TraceStore->EndAddress,
                                                              TraceStore->InitialSize.LowPart);

    //
    // For now, we only use a single file handle (i.e. single file) with
    // multiple memory mappings.  Down the track we may add support for
    // multiple files to multiple memory mappings.
    //
    TraceStore->NextTraceStoreMemoryMap.FileHandle = TraceStore->FileHandle;

end:
    
error:
    LeaveCriticalSection(&TraceStore->NextTraceStoreMemoryMap.CriticalSection);
}

VOID
CALLBACK
ExtendTraceStoreFileCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
)
{

    BOOL Success = ExtendTraceStoreFile((PTRACE_STORE)Context);
    if (Success) {
        SetEvent()

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
    PVOID ReturnAddress = NULL, NextAddress = NULL;

    if (!TraceStore) {
        return NULL;
    }

#ifdef _M_X64
    AllocationSize = (DWORD_PTR)(RecordSize.QuadPart * NumberOfRecords.QuadPart);
#else
    {
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
#endif

    EnterCriticalSection(&TraceStore->CriticalSection);

    if (!TraceStore->pMetadata->RecordSize.QuadPart) {
        TraceStore->pMetadata->RecordSize.QuadPart = RecordSize.QuadPart;
    }

    if (TraceStore->pMetadata->RecordSize.QuadPart != RecordSize.QuadPart) {
        goto end;
    }

    NextAddress = (PVOID)((ULONG_PTR)TraceStore->NextAddress + AllocationSize);

    if (NextAddress >= TraceStore->EndAddress) {
        //TRACE_STORE_MEMORY_MAP NextMemoryMap;
        // Need to copy over the NextMemoryMap details if created.
        ++TraceStore->DroppedRecords;
        goto end;
    }
    else if (NextAddress >= TraceStore->PrefaultAddress) {
        

    } else if (NextAddress >= TraceStore->ExtendAtAddress) {

    }

    TraceStore->pMetadata->NumberOfRecords.QuadPart += NumberOfRecords.QuadPart;

    TraceStore->PrevAddress = TraceStore->NextAddress;
    TraceStore->NextAddress = NextAddress;
    ReturnAddress = TraceStore->PrevAddress;

end:
    LeaveCriticalSection(&TraceStore->CriticalSection);

    return ReturnAddress;
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
    _Inout_bytecap_(*SizeOfTraceContext)    PTRACE_CONTEXT          TraceContext,
    _In_                                    PDWORD                  SizeOfTraceContext,
    _In_                                    PTRACE_SESSION          TraceSession,
    _In_                                    PTRACE_STORES           TraceStores,
    _In_                                    PTP_CALLBACK_ENVIRON    ThreadpoolCallbackEnvironment,
    _In_opt_                                PVOID                   UserData
)
{
    USHORT Index;
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

    if (!ThreadpoolCallbackEnvironment) {
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
    TraceContext->ThreadpoolCallbackEnvironment = ThreadpoolCallbackEnvironment;
    TraceContext->UserData = UserData;

    for (Index = 0; Index < TraceStores->NumberOfTraceStores * 2; Index += 2) {
        PTRACE_STORE TraceStore = &TraceStores->Stores[Index];
        TraceStore->FileExtendedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!TraceStore->FileExtendedEvent) {
            return FALSE;
        }

        TraceStore->ExtendFileWork = CreateThreadpoolWork(
            &ExtendTraceStoreFileCallback,
            TraceStore,
            ThreadpoolCallbackEnvironment
        );

        if (!TraceStore->ExtendFileWork) {
            return FALSE;
        }

        TraceStore->PrefaultFuturePageWork = CreateThreadpoolWork(
            &PrefaultFuturePageCallback,
            TraceStore,
            ThreadpoolCallbackEnvironment
        );

        if (!TraceStore->PrefaultFuturePageWork) {
            return FALSE;
        }

        TraceStore->TraceContext = TraceContext;
    }

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

// vim: set ts=8 sw=4 sts=4 expandtab si ai:
