// Copyright Trent Nelson <trent@trent.me>

#include <Windows.h>
#include <Strsafe.h>
#include "Tracing.h"

INIT_ONCE InitOnceFindLongestTraceStoreFileName = INIT_ONCE_STATIC_INIT;

INIT_ONCE InitOnceSystemTimerFunction = INIT_ONCE_STATIC_INIT;

static const LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

static const SIZE_T InitialTraceContextHeapSize = (1 << 21); // 2MB
static const SIZE_T MaximumTraceContextHeapSize = (1 << 26); // 64MB

static const USHORT InitialFreeMemoryMaps = 32;

static const USHORT TraceStoreAllocationStructSize = (
    sizeof(TRACE_STORE_ALLOCATION)
);
static const USHORT TraceStoreAddressStructSize = sizeof(TRACE_STORE_ADDRESS);
static const USHORT TraceStoreInfoStructSize = sizeof(TRACE_STORE_INFO);

static const ULONG DefaultTraceStoreMappingSize             = (1 << 21); // 2MB
static const ULONG DefaultTraceStoreEventMappingSize        = (1 << 23); // 8MB

static const ULONG DefaultAllocationTraceStoreSize          = (1 << 21); // 2MB
static const ULONG DefaultAllocationTraceStoreMappingSize   = (1 << 16); // 64KB

static const ULONG DefaultAddressTraceStoreSize             = (1 << 21); // 2MB
static const ULONG DefaultAddressTraceStoreMappingSize      = (1 << 16); // 64KB

static const ULONG DefaultInfoTraceStoreSize                = (1 << 16); // 64KB
static const ULONG DefaultInfoTraceStoreMappingSize         = (1 << 16); // 64KB

TRACER_API INITIALIZE_TRACE_STORES InitializeTraceStores;

_Check_return_
BOOL
CallSystemTimer(
    _Out_       PFILETIME               SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION SystemTimerFunctionPointer
    );

_Check_return_
_Success_(return != 0)
BOOL
InitializeTraceStoreTime(
    _In_    PRTL                Rtl,
    _In_    PTRACE_STORE_TIME   Time
    )
{
    BOOL Success;

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Time)) {
        return FALSE;
    }

    QueryPerformanceFrequency(&Time->Frequency);
    Time->Multiplicand.QuadPart = TIMESTAMP_TO_SECONDS;

    QueryPerformanceCounter(&Time->StartTime.PerformanceCounter);
    GetSystemTimeAsFileTime(&Time->StartTime.FileTimeUtc);
    GetSystemTime(&Time->StartTime.SystemTimeUtc);
    GetLocalTime(&Time->StartTime.SystemTimeLocal);

    Success = FileTimeToLocalFileTime(
        &Time->StartTime.FileTimeUtc,
        &Time->StartTime.FileTimeLocal
    );

    if (!Success) {
        return FALSE;
    }

    Time->StartTime.SecondsSince1970.HighPart = 0;

    Success = Rtl->RtlTimeToSecondsSince1970(
        (PLARGE_INTEGER)&Time->StartTime.FileTimeLocal,
        &Time->StartTime.SecondsSince1970.LowPart
    );

    if (!Success) {
        return FALSE;
    }

    Time->StartTime.MicrosecondsSince1970.QuadPart = (
        UInt32x32To64(
            Time->StartTime.SecondsSince1970.LowPart,
            SECONDS_TO_MICROSECONDS
        )
    );

    return TRUE;
}

BOOL
LoadNextTraceStoreAddress(
    _In_    PTRACE_STORE TraceStore,
    _Out_   PPTRACE_STORE_ADDRESS AddressPointer
    );

FORCEINLINE
BOOL
IsMetadataTraceStore(_In_ PTRACE_STORE TraceStore)
{
    return TraceStore->IsMetadata;
}

BOOL
FindLongestTraceStoreFileName(_Out_ PUSHORT LengthPointer)
{
    DWORD Index;
    NTSTATUS Result;
    ULARGE_INTEGER Longest = { 0 };
    ULARGE_INTEGER Length = { 0 };
    DWORD MaxPath = (
        _OUR_MAX_PATH   -
        3               - /* C:\ */
        1               - // NUL
        LongestTraceStoreSuffixLength
    );

    for (Index = 0; Index < NumberOfTraceStores; Index++) {
        LPCWSTR FileName = TraceStoreFileNames[Index];
        Result = StringCchLengthW(
            FileName,
            MaxPath,
            (PSIZE_T)&Length.QuadPart
        );
        if (FAILED(Result)) {
            return FALSE;
        }
        if (Length.QuadPart > Longest.QuadPart) {
            Longest.QuadPart = Length.QuadPart;
        }
    }
    Longest.QuadPart += LongestTraceStoreSuffixLength;

    if (Longest.QuadPart > MAX_STRING) {
        return FALSE;
    }

    *LengthPointer = (USHORT)Longest.LowPart;

    return TRUE;
}


BOOL
CALLBACK
FindLongestTraceStoreFileNameCallback(
    PINIT_ONCE InitOnce,
    PVOID Parameter,
    PVOID *lpContext
    )
{
    DWORD Index;
    NTSTATUS Result;
    ULARGE_INTEGER Longest = { 0 };
    ULARGE_INTEGER Length = { 0 };
    DWORD MaxPath = (
        _OUR_MAX_PATH   -
        3               - /* C:\ */
        1               - // NUL
        TraceStoreAllocationSuffixLength
    );

    for (Index = 0; Index < NumberOfTraceStores; Index++) {
        LPCWSTR FileName = TraceStoreFileNames[Index];
        Result = StringCchLengthW(
            FileName,
            MaxPath,
            (PSIZE_T)&Length.QuadPart
        );
        if (FAILED(Result)) {
            return FALSE;
        }
        if (Length.QuadPart > Longest.QuadPart) {
            Longest.QuadPart = Length.QuadPart;
        }
    }
    Longest.QuadPart += TraceStoreAllocationSuffixLength;

    *((PDWORD64)lpContext) = Longest.QuadPart;

    return TRUE;
}

DWORD
GetLongestTraceStoreFileNameOnce()
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

DWORD
GetLongestTraceStoreFileName()
{
    BOOL Success;
    USHORT Length;

    Success = FindLongestTraceStoreFileName(&Length);

    if (!Success) {
        return 0;
    }

    return Length;
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
        SystemTimerFunction.GetSystemTimePreciseAsFileTime = (
            (PGETSYSTEMTIMEPRECISEASFILETIME)Proc
        );
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

FORCEINLINE
VOID
ReturnFreeTraceStoreMemoryMap(
    _Inout_ PTRACE_STORE TraceStore,
    _Inout_ PTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    SecureZeroMemory(MemoryMap, sizeof(*MemoryMap));

    InterlockedPushEntrySList(
        &TraceStore->FreeMemoryMaps,
        &MemoryMap->ListEntry
    );

    if (!InterlockedDecrement(&TraceStore->NumberOfActiveMemoryMaps)) {
        SetEvent(TraceStore->AllMemoryMapsAreFreeEvent);
    }
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
        BOOL Success;

        Success = (
            SystemTimerFunction->NtQuerySystemTime(
                (PLARGE_INTEGER)SystemTime
            )
        );

        if (!Success) {
            return FALSE;
        }

    } else {
        return FALSE;
    }

    return TRUE;
}

FORCEINLINE
BOOL
PopTraceStoreMemoryMap(
    _In_ PSLIST_HEADER ListHead,
    _In_ PPTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    PSLIST_ENTRY ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *MemoryMap = CONTAINING_RECORD(ListEntry,
                                   TRACE_STORE_MEMORY_MAP,
                                   ListEntry);

    return TRUE;
}

FORCEINLINE
BOOL
PopFreeTraceStoreMemoryMap(
    _In_ PTRACE_STORE TraceStore,
    _In_ PPTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    PSLIST_HEADER ListHead = &TraceStore->FreeMemoryMaps;

    PSLIST_ENTRY ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *MemoryMap = CONTAINING_RECORD(ListEntry,
                                   TRACE_STORE_MEMORY_MAP,
                                   ListEntry);

    InterlockedIncrement(&TraceStore->NumberOfActiveMemoryMaps);

    return TRUE;
}

FORCEINLINE
VOID
PushTraceStoreMemoryMap(
    _Inout_ PSLIST_HEADER ListHead,
    _Inout_ PTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    InterlockedPushEntrySList(ListHead, &MemoryMap->ListEntry);
}

FORCEINLINE
BOOL
GetTraceStoreMemoryMapFileInfo(
    _In_    PTRACE_STORE_MEMORY_MAP MemoryMap,
    _Inout_ PFILE_STANDARD_INFO FileInfo
    )
{
    return GetFileInformationByHandleEx(
        MemoryMap->FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        FileInfo,
        sizeof(*FileInfo)
    );
}

_Success_(return != 0)
BOOL
InitializeStore(
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
    )
{
    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!TraceStore->FileHandle) {
        // We're a metadata store.
        TraceStore->FileHandle = CreateFileW(
            Path,
            TraceStore->CreateFileDesiredAccess,
            FILE_SHARE_READ,
            NULL,
            OPEN_ALWAYS,
            FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
            NULL
        );
    }

    if (TraceStore->FileHandle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    TraceStore->InitialSize.HighPart = 0;
    TraceStore->InitialSize.LowPart = InitialSize;
    TraceStore->ExtensionSize.HighPart = 0;
    TraceStore->ExtensionSize.LowPart = InitialSize;

    if (!MappingSize) {
        MappingSize = DefaultTraceStoreMappingSize;
    }

    TraceStore->MappingSize.HighPart = 0;
    TraceStore->MappingSize.LowPart = MappingSize;

    //
    // Cap the mapping size to the maximum if necessary.
    //

    if (TraceStore->MappingSize.QuadPart > MaximumMappingSize.QuadPart) {
        TraceStore->MappingSize.QuadPart = MaximumMappingSize.QuadPart;
    }

    TraceStore->AllocateRecords = AllocateRecords;
    TraceStore->FreeRecords = FreeRecords;

    TraceStore->NumberOfAllocations.QuadPart = 0;
    TraceStore->TotalAllocationSize.QuadPart = 0;

    return TRUE;
error:
    CloseTraceStore(TraceStore);
    return FALSE;
}

BOOL
InitializeTraceStorePath(
    _In_    PCWSTR          Path,
    _In_    PTRACE_STORE    TraceStore
    )
{
    HRESULT Result;
    ULARGE_INTEGER Length;
    ULARGE_INTEGER MaximumLength;

    MaximumLength.HighPart = 0;
    MaximumLength.LowPart = sizeof(TraceStore->PathBuffer);

    Result = StringCbLengthW(Path, MaximumLength.QuadPart, &Length.QuadPart);

    if (FAILED(Result)) {
        return FALSE;
    }

    TraceStore->Path.Length = (USHORT)Length.LowPart;
    TraceStore->Path.MaximumLength = (USHORT)MaximumLength.LowPart;
    TraceStore->Path.Buffer = &TraceStore->PathBuffer[0];

    __movsw((PWORD)TraceStore->Path.Buffer,
            (PWORD)Path,
            (TraceStore->Path.Length >> 1));

    TraceStore->PathBuffer[TraceStore->Path.Length >> 1] = L'\0';

    return TRUE;
}

BOOL
InitializeTraceStore(
    _In_        PRTL Rtl,
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _Inout_     PTRACE_STORE AllocationStore,
    _Inout_     PTRACE_STORE AddressStore,
    _Inout_     PTRACE_STORE InfoStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
    )
{
    BOOL Success;
    HRESULT Result;
    WCHAR AllocationPath[_OUR_MAX_PATH];
    WCHAR AddressPath[_OUR_MAX_PATH];
    WCHAR InfoPath[_OUR_MAX_PATH];
    PCWSTR AllocationSuffix = TraceStoreAllocationSuffix;
    PCWSTR AddressSuffix = TraceStoreAddressSuffix;
    PCWSTR InfoSuffix = TraceStoreInfoSuffix;

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

#define INIT_METADATA_PATH(Name)                                      \
    SecureZeroMemory(&##Name##Path, sizeof(##Name##Path));            \
    Result = StringCchCopyW(                                          \
        &##Name##Path[0],                                             \
        _OUR_MAX_PATH,                                                \
        Path                                                          \
    );                                                                \
    if (FAILED(Result)) {                                             \
        return FALSE;                                                 \
    }                                                                 \
                                                                      \
    Result = StringCchCatW(                                           \
        &##Name##Path[0],                                             \
        _OUR_MAX_PATH,                                                \
        TraceStore##Name##Suffix                                      \
    );                                                                \
    if (FAILED(Result)) {                                             \
        return FALSE;                                                 \
    }                                                                 \
                                                                      \
    ##Name##Store->Rtl = Rtl;                                         \
    if (!InitializeTraceStorePath(&##Name##Path[0], ##Name##Store)) { \
        return FALSE;                                                 \
    }

    INIT_METADATA_PATH(Allocation);
    INIT_METADATA_PATH(Address);
    INIT_METADATA_PATH(Info);

    TraceStore->Rtl = Rtl;

    if (!InitializeTraceStorePath(Path, TraceStore)) {
        return FALSE;
    }

    //
    // Create the data file first before the streams.
    //

    TraceStore->FileHandle = CreateFileW(
        Path,
        TraceStore->CreateFileDesiredAccess,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (TraceStore->FileHandle == INVALID_HANDLE_VALUE) {
        DWORD LastError = GetLastError();
        goto Error;
    }

#define INIT_METADATA(Name)                                            \
    Name##Store->IsMetadata = TRUE;                                    \
    Name##Store->IsReadonly = TraceStore->IsReadonly;                  \
    Name##Store->SequenceId = TraceStore->SequenceId;                  \
    Name##Store->CreateFileDesiredAccess = (                           \
        TraceStore->CreateFileDesiredAccess                            \
    );                                                                 \
    Name##Store->CreateFileMappingProtectionFlags = (                  \
        TraceStore->CreateFileMappingProtectionFlags                   \
    );                                                                 \
    Name##Store->MapViewOfFileDesiredAccess = (                        \
        TraceStore->MapViewOfFileDesiredAccess                         \
    );                                                                 \
    Name##Store->Frequency.QuadPart = (                                \
        TraceStore->Frequency.QuadPart                                 \
    );                                                                 \
                                                                       \
    Success = InitializeStore(                                         \
        &##Name##Path[0],                                              \
        ##Name##Store,                                                 \
        Default##Name##TraceStoreSize,                                 \
        Default##Name##TraceStoreMappingSize                           \
    );                                                                 \
                                                                       \
    if (!Success) {                                                    \
        goto Error;                                                    \
    }                                                                  \
                                                                       \
    ##Name##Store->NumberOfRecords.QuadPart = 1;                       \
    ##Name##Store->RecordSize.QuadPart = TraceStore##Name##StructSize; \
                                                                       \
    TraceStore->##Name##Store = ##Name##Store;

    INIT_METADATA(Allocation);
    INIT_METADATA(Address);
    INIT_METADATA(Info);

    TraceStore->IsMetadata = FALSE;

    if (!InitializeStore(Path, TraceStore, InitialSize, MappingSize)) {
        goto Error;
    }

    return TRUE;
Error:
    CloseTraceStore(TraceStore);
    return FALSE;
}

BOOL
InitializeTraceStores(
    _In_        PRTL            Rtl,
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PULONG          SizeOfTraceStores,
    _In_opt_    PULONG          InitialFileSizes,
    _In_        BOOL            Readonly,
    _In_        BOOL            Compress
    )
{
    BOOL Success;
    HRESULT Result;
    DWORD Index;
    DWORD StoreIndex;
    DWORD LastError;
    DWORD CreateFileDesiredAccess;
    DWORD CreateFileMappingProtectionFlags;
    DWORD MapViewOfFileDesiredAccess;
    WCHAR Path[_OUR_MAX_PATH];
    LPWSTR FileNameDest;
    DWORD LongestFilename = GetLongestTraceStoreFileName();
    DWORD TraceStoresAllocationSize = (
        GetTraceStoresAllocationSize(
            NumberOfTraceStores *
            ElementsPerTraceStore
        )
    );
    DWORD LongestPossibleDirectoryLength = (
        _OUR_MAX_PATH   -
        1               - // '\'
        1               - // final NUL
        LongestFilename
    );
    LARGE_INTEGER DirectoryLength;
    LARGE_INTEGER RemainingChars;
    PULONG Sizes = InitialFileSizes;

    if (!SizeOfTraceStores) {
        return FALSE;
    }

    if (!TraceStores || *SizeOfTraceStores < TraceStoresAllocationSize) {
        *SizeOfTraceStores = TraceStoresAllocationSize;
        return FALSE;
    }

    if (!Rtl) {
        return FALSE;
    }

    if (!BaseDirectory) {
        return FALSE;
    }

    if (!Sizes) {
        Sizes = (PULONG)&InitialTraceStoreFileSizes[0];
    }

    Result = StringCchLengthW(
        BaseDirectory,
        LongestPossibleDirectoryLength,
        (PSIZE_T)&DirectoryLength.QuadPart
    );

    if (FAILED(Result)) {
        return FALSE;
    }

    if (DirectoryLength.HighPart != 0) {
        return FALSE;
    }

    SecureZeroMemory(&Path, sizeof(Path));

    Result = StringCchCopyW(
        &Path[0],
        LongestPossibleDirectoryLength,
        BaseDirectory
    );
    if (FAILED(Result)) {
        return FALSE;
    }

    Path[DirectoryLength.LowPart] = L'\\';
    FileNameDest = &Path[DirectoryLength.LowPart+1];
    RemainingChars.QuadPart = (
        _OUR_MAX_PATH -
        DirectoryLength.LowPart -
        2
    );

    SecureZeroMemory(TraceStores, TraceStoresAllocationSize);
    TraceStores->Size = (USHORT)TraceStoresAllocationSize;

    if (Readonly) {
        CreateFileDesiredAccess = GENERIC_READ;
        CreateFileMappingProtectionFlags = PAGE_READONLY;
        MapViewOfFileDesiredAccess = FILE_MAP_READ;
    }
    else {
        CreateFileDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        CreateFileMappingProtectionFlags = PAGE_READWRITE;
        MapViewOfFileDesiredAccess = FILE_MAP_READ | FILE_MAP_WRITE;
    }

    Success = CreateDirectory(BaseDirectory, NULL);
    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            return FALSE;
        }
    }

    if (!Readonly && Compress) {
        HANDLE DirectoryHandle;
        USHORT CompressionFormat = COMPRESSION_FORMAT_DEFAULT;
        DWORD BytesReturned = 0;

        DirectoryHandle = CreateFileW(
            BaseDirectory,
            CreateFileDesiredAccess,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_ALWAYS,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );

        if (DirectoryHandle == INVALID_HANDLE_VALUE) {
            LastError = GetLastError();
            return FALSE;
        }

        Success = DeviceIoControl(
            DirectoryHandle,                // hDevice
            FSCTL_SET_COMPRESSION,          // dwIoControlCode
            &CompressionFormat,             // lpInBuffer
            sizeof(CompressionFormat),      // nInBufferSize
            NULL,                           // lpOutBuffer
            0,                              // nOutBufferSize
            &BytesReturned,                 // lpBytesReturned
            NULL                            // lpOverlapped
        );

        if (!Success) {
            OutputDebugStringA("Failed to enable compression.\n");
        }

        CloseHandle(DirectoryHandle);
    }

    TraceStores->Rtl = Rtl;

    TraceStores->NumberOfTraceStores = NumberOfTraceStores;
    TraceStores->ElementsPerTraceStore = ElementsPerTraceStore;

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {

        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex];
        PTRACE_STORE AllocationStore = TraceStore + 1;
        PTRACE_STORE AddressStore = TraceStore + 2;
        PTRACE_STORE InfoStore = TraceStore + 3;

        LPCWSTR FileName = TraceStoreFileNames[Index];
        DWORD InitialSize = Sizes[Index];
        ULONG MappingSize = DefaultTraceStoreMappingSize;

        if (StoreIndex == TraceStoreEventIndex) {
            MappingSize = DefaultTraceStoreEventMappingSize;
        }

        Result = StringCchCopyW(
            FileNameDest,
            (SIZE_T)RemainingChars.QuadPart,
            FileName
        );

        if (FAILED(Result)) {
            return FALSE;
        }

        TraceStore->IsReadonly = Readonly;
        TraceStore->SequenceId = Index;
        TraceStore->CreateFileDesiredAccess = CreateFileDesiredAccess;
        TraceStore->CreateFileMappingProtectionFlags = (
            CreateFileMappingProtectionFlags
        );
        TraceStore->MapViewOfFileDesiredAccess = (
            MapViewOfFileDesiredAccess
        );

        Success = InitializeTraceStore(
            Rtl,
            Path,
            TraceStore,
            AllocationStore,
            AddressStore,
            InfoStore,
            InitialSize,
            MappingSize
        );

        if (!Success) {
            return FALSE;
        }
    }

    return TRUE;
}

VOID
CloseMemoryMap(_In_ PTRACE_STORE_MEMORY_MAP MemoryMap)
{
    if (!MemoryMap) {
        return;
    }

    if (MemoryMap->BaseAddress) {
        FlushViewOfFile(MemoryMap->BaseAddress, 0);
        UnmapViewOfFile(MemoryMap->BaseAddress);
        MemoryMap->BaseAddress = NULL;
    }

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }
}

VOID
SubmitCloseMemoryMapThreadpoolWork(
    _In_ PTRACE_STORE TraceStore,
    _Inout_ PPTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    if (!ARGUMENT_PRESENT(MemoryMap) || !*MemoryMap) {
        return;
    }

    PushTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, *MemoryMap);
    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);
    *MemoryMap = NULL;
}

BOOL
TruncateStore(
    _In_ PTRACE_STORE TraceStore
    )
{
    BOOL Success;
    BOOL IsMetadata;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER TotalAllocationSize;
    FILE_STANDARD_INFO FileInfo;

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    IsMetadata = IsMetadataTraceStore(TraceStore);

    TotalAllocationSize.QuadPart = TraceStore->TotalAllocationSize.QuadPart;

    if (!IsMetadata) {

        EndOfFile.QuadPart = TraceStore->pEof->EndOfFile.QuadPart;

        if (EndOfFile.QuadPart != TotalAllocationSize.QuadPart) {
            __debugbreak();
        }

    } else {

        if (TotalAllocationSize.QuadPart == 0) {

            if (TraceStore->NumberOfAllocations.QuadPart != 1) {
                __debugbreak();
            }

            TotalAllocationSize.QuadPart = TraceStore->RecordSize.QuadPart;
        }

        EndOfFile.QuadPart = TotalAllocationSize.QuadPart;
    }

    //
    // Get the file's current end of file info.
    //

    Success = GetFileInformationByHandleEx(
        TraceStore->FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        &FileInfo,
        sizeof(FileInfo)
    );

    if (!Success) {
        return FALSE;
    }

    if (FileInfo.EndOfFile.QuadPart == EndOfFile.QuadPart) {

        //
        // Nothing more to do.
        //

        return TRUE;
    }

    //
    // Adjust the file pointer to the desired position.
    //

    Success = SetFilePointerEx(TraceStore->FileHandle,
                               EndOfFile,
                               NULL,
                               FILE_BEGIN);

    if (!Success) {
        return FALSE;
    }

    //
    // And set the end of file.
    //

    Success = SetEndOfFile(TraceStore->FileHandle);

    return Success;
}

VOID
CloseStore(
    _In_ PTRACE_STORE TraceStore
    )
{
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->PrevMemoryMap);
    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->MemoryMap);

    while (PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {
        SubmitCloseMemoryMapThreadpoolWork(TraceStore, &MemoryMap);
    }

    WaitForSingleObject(TraceStore->AllMemoryMapsAreFreeEvent, INFINITE);

    if (TraceStore->FileHandle) {
        TruncateStore(TraceStore);
        FlushFileBuffers(TraceStore->FileHandle);
        CloseHandle(TraceStore->FileHandle);
        TraceStore->FileHandle = NULL;
    }

    if (TraceStore->NextMemoryMapAvailableEvent) {
        CloseHandle(TraceStore->NextMemoryMapAvailableEvent);
        TraceStore->NextMemoryMapAvailableEvent = NULL;
    }

    if (TraceStore->AllMemoryMapsAreFreeEvent) {
        CloseHandle(TraceStore->AllMemoryMapsAreFreeEvent);
        TraceStore->AllMemoryMapsAreFreeEvent = NULL;
    }

    if (TraceStore->CloseMemoryMapWork) {
        CloseThreadpoolWork(TraceStore->CloseMemoryMapWork);
        TraceStore->CloseMemoryMapWork = NULL;
    }

    if (TraceStore->PrepareNextMemoryMapWork) {
        CloseThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);
        TraceStore->PrepareNextMemoryMapWork = NULL;
    }

    if (TraceStore->PrefaultFuturePageWork) {
        CloseThreadpoolWork(TraceStore->PrefaultFuturePageWork);
        TraceStore->PrefaultFuturePageWork = NULL;
    }

}

VOID
CloseTraceStore(
    _In_ PTRACE_STORE TraceStore
    )
{
    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    //
    // Close the trace store first before the metadata stores.  This is
    // important because closing the trace store will retire any active memory
    // maps, which will update the underlying MemoryMap->pAddress structures,
    // which will be backed by the AddressStore, which we can only access as
    // long as we haven't closed it.
    //

    CloseStore(TraceStore);

    if (TraceStore->AllocationStore) {
        CloseStore(TraceStore->AllocationStore);
        TraceStore->AllocationStore = NULL;
    }

    if (TraceStore->AddressStore) {
        CloseStore(TraceStore->AddressStore);
        TraceStore->AddressStore = NULL;
    }

    if (TraceStore->InfoStore) {
        CloseStore(TraceStore->InfoStore);
        TraceStore->InfoStore = NULL;
    }

}

VOID
CloseTraceStores(
    _In_ PTRACE_STORES TraceStores
    )
{
    USHORT Index;
    USHORT StoreIndex;

    if (!TraceStores) {
        return;
    }

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {
        CloseTraceStore(&TraceStores->Stores[StoreIndex]);
    }
}

DWORD
GetTraceStoresAllocationSize(const USHORT NumberOfTraceStores)
{
    SHORT Delta;
    USHORT ExtraSize;
    USHORT DefaultNumberOfTraceStores = (
        RTL_FIELD_SIZE(TRACE_STORES, Stores) /
        sizeof(TRACE_STORE)
    );

    Delta = (NumberOfTraceStores - DefaultNumberOfTraceStores);

    if (Delta <= 0) {
        return sizeof(TRACE_STORES);
    }

    ExtraSize = sizeof(TRACE_STORE) * Delta * ElementsPerTraceStore;

    return sizeof(TRACE_STORES) + ExtraSize;
}

VOID
PrefaultFutureTraceStorePage(_Inout_ PTRACE_STORE TraceStore)
{
    BOOL Success;
    PTRACE_STORE_MEMORY_MAP PrefaultMemoryMap;

    if (!TraceStore) {
        return;
    }

    Success = PopTraceStoreMemoryMap(
        &TraceStore->PrefaultMemoryMaps,
        &PrefaultMemoryMap
    );

    if (!Success) {
        return;
    }

    PrefaultPage(PrefaultMemoryMap->NextAddress);

    ReturnFreeTraceStoreMemoryMap(TraceStore, PrefaultMemoryMap);
}

VOID
CALLBACK
PrefaultFuturePageCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
    )
{
    PrefaultFutureTraceStorePage((PTRACE_STORE)Context);
}

_Success_(return != 0)
BOOL
PrepareNextTraceStoreMemoryMap(
    _Inout_ PTRACE_STORE TraceStore
    )
{
    USHORT NumaNode;
    BOOL Success;
    BOOL IsMetadata;
    BOOL HaveAddress;
    PRTL Rtl;
    HRESULT Result;
    PVOID PreferredBaseAddress;
    PVOID OriginalPreferredBaseAddress;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    FILE_STANDARD_INFO FileInfo;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    LARGE_INTEGER CurrentFileOffset;
    LARGE_INTEGER NewFileOffset;
    LARGE_INTEGER DistanceToMove;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE_STATS Stats;
    TRACE_STORE_STATS DummyStats = { 0 };

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    Rtl = TraceStore->Rtl;

    //
    // We may not have a stats struct available yet if this is the first
    // call to PrepareNextTraceStoreMemoryMap().  If that's the case, just
    // point the pointer at a dummy one.  This simplifies the rest of the
    // code in the function.
    //

    Stats = TraceStore->pStats;

    if (!Stats) {
        Stats = &DummyStats;
    }

    if (!Rtl) {
        return FALSE;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    if (!MemoryMap->FileHandle) {
        goto Error;
    }

    if (!GetTraceStoreMemoryMapFileInfo(MemoryMap, &FileInfo)) {
        goto Error;
    }

    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // Get the current file offset.
    //

    DistanceToMove.QuadPart = 0;

    Success = SetFilePointerEx(TraceStore->FileHandle,
                               DistanceToMove,
                               &CurrentFileOffset,
                               FILE_CURRENT);

    if (!Success) {
        goto Error;
    }

    if (CurrentFileOffset.QuadPart != MemoryMap->FileOffset.QuadPart) {

        //
        // This shouldn't occur if all our memory map machinery isn't working
        // correctly.
        //

        __debugbreak();
    }

    //
    // Determine the distance we need to move the file pointer.
    //

    DistanceToMove.QuadPart = (
        MemoryMap->FileOffset.QuadPart +
        MemoryMap->MappingSize.QuadPart
    );

    //
    // Adjust the file pointer from the current position.
    //

    Success = SetFilePointerEx(MemoryMap->FileHandle,
                               DistanceToMove,
                               &NewFileOffset,
                               FILE_BEGIN);

    if (!Success) {
        goto Error;
    }

    //
    // If the new file offset is past the end of the file, extend it.
    //

    if (FileInfo.EndOfFile.QuadPart < NewFileOffset.QuadPart) {

        if (TraceStore->IsReadonly) {

            //
            // Something has gone wrong if we're extending a readonly store.
            //

            __debugbreak();

            goto Error;
        }

        if (!SetEndOfFile(MemoryMap->FileHandle)) {

            goto Error;
        }
    }

    //
    // Create a new file mapping for this memory map's slice of data.
    //

    MemoryMap->MappingHandle = CreateFileMapping(
        MemoryMap->FileHandle,
        NULL,
        TraceStore->CreateFileMappingProtectionFlags,
        NewFileOffset.HighPart,
        NewFileOffset.LowPart,
        NULL
    );

    if (MemoryMap->MappingHandle == NULL ||
        MemoryMap->MappingHandle == INVALID_HANDLE_VALUE) {
        DWORD LastError = GetLastError();
        goto Error;
    }

    OriginalPreferredBaseAddress = NULL;

    //
    // MemoryMap->BaseAddress will be the next contiguous address for this
    // mapping.  It is set by ConsumeNextTraceStoreMemoryMap().  We use it
    // as our preferred base address if we can.
    //

    PreferredBaseAddress = MemoryMap->BaseAddress;

    AddressPointer = MemoryMap->pAddress;

    if (IsMetadata || (AddressPointer == NULL)) {

        //
        // We don't attempt to re-use addresses if we're metadata, and we can't
        // attempt re-use if there is no backing address struct.
        //

        HaveAddress = FALSE;

        goto TryMapMemory;
    }

    HaveAddress = TRUE;

    //
    // Take a local copy of the address.
    //

    Result = Rtl->RtlCopyMappedMemory(&Address,
                                      AddressPointer,
                                      sizeof(Address));

    if (FAILED(Result)) {

        //
        // Disable the address and go straight to preparation.
        //

        HaveAddress = FALSE;
        AddressPointer = NULL;
        MemoryMap->pAddress = NULL;
        goto TryMapMemory;

    }

TryMapMemory:

    MemoryMap->BaseAddress = MapViewOfFileEx(
        MemoryMap->MappingHandle,
        TraceStore->MapViewOfFileDesiredAccess,
        MemoryMap->FileOffset.HighPart,
        MemoryMap->FileOffset.LowPart,
        MemoryMap->MappingSize.LowPart,
        PreferredBaseAddress
    );

    if (!MemoryMap->BaseAddress) {

        if (PreferredBaseAddress) {

            //
            // Make a note of the original preferred base address, clear it,
            // then attempt the mapping again.
            //

            OriginalPreferredBaseAddress = PreferredBaseAddress;
            PreferredBaseAddress = NULL;
            Stats->PreferredAddressUnavailable++;
            goto TryMapMemory;
        }

        goto Error;

    }

    if (IsMetadata) {
        goto Finalize;
    }

    if (!PreferredBaseAddress && OriginalPreferredBaseAddress) {

        //
        // The mapping succeeded, but not at our original preferred address.
        // When we implement relocation support, we'll need to do that here.
        //

        //
        // We copy the original preferred base address back so that it can be
        // picked up in the section below where it is saved to the address
        // struct.
        //

        PreferredBaseAddress = OriginalPreferredBaseAddress;

    }

    if (!HaveAddress) {
        goto Finalize;
    }

    //
    // Record all of the mapping information in our address record.
    //

    Address.PreferredBaseAddress = PreferredBaseAddress;
    Address.BaseAddress = MemoryMap->BaseAddress;
    Address.FileOffset.QuadPart = MemoryMap->FileOffset.QuadPart;
    Address.MappedSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    //
    // Fill in the thread and processor information.
    //

    Address.FulfillingThreadId = FastGetCurrentThreadId();

    GetCurrentProcessorNumberEx(&Address.FulfillingProcessor);

    Success = GetNumaProcessorNodeEx(&Address.FulfillingProcessor, &NumaNode);

    Address.FulfillingNumaNode = (Success ? (UCHAR)NumaNode : 0);


    //
    // Take a local copy of the timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Copy it to the Prepared timestamp.
    //

    Address.Timestamp.Prepared.QuadPart = Elapsed.QuadPart;

    //
    // Calculate the elapsed time spent awaiting preparation.
    //

    Elapsed.QuadPart -= Address.Timestamp.Requested.QuadPart;

    Address.Elapsed.AwaitingPreparation.QuadPart = Elapsed.QuadPart;

    //
    // Finally, copy the updated record back to the memory-mapped backing
    // store.
    //

    Result = Rtl->RtlCopyMappedMemory(
        AddressPointer,
        &Address,
        sizeof(Address)
    );

    if (FAILED(Result)) {

        //
        // Disable the address struct.
        //

        MemoryMap->pAddress = NULL;

    } else if (MemoryMap->pAddress != AddressPointer) {

        //
        // Invariant check: this should never get hit.
        //

        __debugbreak();

    }

    //
    // Intentional follow-on.
    //

Finalize:

    //
    // Initialize the next address to the base address.
    //

    MemoryMap->NextAddress = MemoryMap->BaseAddress;

    if (!TraceStore->NoPrefaulting) {

        PVOID BaseAddress = MemoryMap->BaseAddress;

        //
        // Prefault the first two pages.  The AllocateRecords function will
        // take care of prefaulting subsequent pages.
        //

        PrefaultPage(BaseAddress);
        PrefaultNextPage(BaseAddress);
    }

    Success = TRUE;
    PushTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, MemoryMap);
    SetEvent(TraceStore->NextMemoryMapAvailableEvent);

    goto End;

Error:
    Success = FALSE;

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }

    ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);

End:
    return Success;
}

VOID
CALLBACK
PrepareNextTraceStoreMemoryMapCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
    )
{
    PrepareNextTraceStoreMemoryMap((PTRACE_STORE)Context);
}

BOOL
FlushTraceStoreMemoryMap(_Inout_ PTRACE_STORE_MEMORY_MAP MemoryMap)
{
    BOOL Success;

    if (!MemoryMap) {
        return FALSE;
    }

    if (MemoryMap->BaseAddress) {

        Success = FlushViewOfFile(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.LowPart
        );

        if (!Success) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
UnmapTraceStoreMemoryMap(_Inout_ PTRACE_STORE_MEMORY_MAP MemoryMap)
{
    if (!MemoryMap) {
        return FALSE;
    }

    if (MemoryMap->BaseAddress) {
        UnmapViewOfFile(MemoryMap->BaseAddress);
    }

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
    }

    return TRUE;
}

BOOL
ReleasePrevTraceStoreMemoryMap(_Inout_ PTRACE_STORE TraceStore)
{
    PRTL Rtl;
    HRESULT Result;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    TRACE_STORE_ADDRESS Address;
    LARGE_INTEGER PreviousTimestamp;
    LARGE_INTEGER Elapsed;
    PLARGE_INTEGER ElapsedPointer;

    if (!PopTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    UnmapTraceStoreMemoryMap(MemoryMap);

    if (!MemoryMap->pAddress) {
        goto Finish;
    }

    Rtl = TraceStore->Rtl;

    //
    // Take a local copy of the address record, update timestamps and
    // calculate elapsed time, then save the local record back to the
    // backing TRACE_STORE_ADDRESS struct.
    //

    Result = Rtl->RtlCopyMappedMemory(
        &Address,
        MemoryMap->pAddress,
        sizeof(Address)
    );

    if (FAILED(Result)) {

        //
        // Ignore and continue.
        //

        goto Finish;
    }

    //
    // Get a local copy of the elapsed start time.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Copy it to the Released timestamp.
    //

    Address.Timestamp.Released.QuadPart = Elapsed.QuadPart;

    //
    // Determine what state this memory map was in at the time of being closed.
    // For a memory map that has progressed through the normal lifecycle, it'll
    // typically be in 'AwaitingRelease' at this point.  However, we could be
    // getting called against an active memory map or prepared memory map if
    // we're getting released as a result of closing the trace store.
    //

    if (Address.Timestamp.Retired.QuadPart != 0) {

        //
        // Normal memory map awaiting retirement.  Elapsed.AwaitingRelease
        // will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Retired.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingRelease;

    } else if (Address.Timestamp.Consumed.QuadPart != 0) {

        //
        // An active memory map.  Elapsed.Active will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Consumed.QuadPart;
        ElapsedPointer = &Address.Elapsed.Active;

    } else if (Address.Timestamp.Prepared.QuadPart != 0) {

        //
        // A prepared memory map awaiting consumption.
        // Elapsed.AwaitingConsumption will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Prepared.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingConsumption;

    } else {

        //
        // A memory map that wasn't even prepared.  Highly unlikely.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Requested.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingPreparation;
    }

    //
    // Calculate the elapsed time.
    //

    Elapsed.QuadPart -= PreviousTimestamp.QuadPart;

    //
    // Update the target elapsed time.
    //

    ElapsedPointer->QuadPart = Elapsed.QuadPart;

    //
    // Copy the local record back to the backing store and ignore the
    // return value.
    //

    Rtl->RtlCopyMappedMemory(MemoryMap->pAddress,
                             &Address,
                             sizeof(Address));

Finish:
    ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);
    return TRUE;
}

VOID
CALLBACK
ReleasePrevMemoryMapCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
)
{
    ReleasePrevTraceStoreMemoryMap((PTRACE_STORE)Context);
}

VOID
SubmitTraceStoreFileExtensionThreadpoolWork(
    _Inout_     PTRACE_STORE    TraceStore
)
{
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);
}

BOOL
CreateMemoryMapsForTraceStore(
    _Inout_ PTRACE_STORE TraceStore,
    _Inout_ PTRACE_CONTEXT TraceContext,
    _In_ ULONG NumberOfItems
    )
{
    PTRACE_STORE_MEMORY_MAP MemoryMaps;
    SIZE_T AllocationSize;
    ULONG Index;

    if (!NumberOfItems) {
        NumberOfItems = InitialFreeMemoryMaps;
    }

    AllocationSize = sizeof(TRACE_STORE_MEMORY_MAP) * NumberOfItems;

    MemoryMaps = HeapAlloc(TraceContext->HeapHandle,
                           HEAP_ZERO_MEMORY,
                           AllocationSize);

    if (!MemoryMaps) {
        return FALSE;
    }

    //
    // Carve the chunk of memory allocated above into list items, then push
    // them onto the free list.
    //

    for (Index = 0; Index < NumberOfItems; Index++) {
        PTRACE_STORE_MEMORY_MAP MemoryMap = &MemoryMaps[Index];

        //
        // Ensure the ListItem is aligned on the MEMORY_ALLOCATION_ALIGNMENT.
        // This is a requirement for the underlying SLIST_ENTRY.
        //
        if (((ULONG_PTR)MemoryMap & (MEMORY_ALLOCATION_ALIGNMENT - 1)) != 0) {
            __debugbreak();
        }

        PushTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps, MemoryMap);
    }

    return TRUE;
}

_Success_(return != 0)
BOOL
LoadNextTraceStoreAddress(
    _In_    PTRACE_STORE TraceStore,
    _Out_   PPTRACE_STORE_ADDRESS AddressPointer
    )
{
    PRTL Rtl;
    BOOL Success;
    PVOID Buffer;
    HRESULT Result;
    USHORT NumaNode;
    TRACE_STORE_ADDRESS Address;
    PALLOCATE_RECORDS AllocateRecords;
    PTRACE_STORE AddressStore;
    PTRACE_CONTEXT Context;

    ULARGE_INTEGER AddressRecordSize = { sizeof(Address) };
    ULARGE_INTEGER NumberOfAddressRecords = { 1 };

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AddressPointer)) {
        return FALSE;
    }

    if (IsMetadataTraceStore(TraceStore)) {

        //
        // Should never be called against a metadata store.
        //

        __debugbreak();

        return FALSE;
    }

    if (PAGE_SIZE % AddressRecordSize.QuadPart) {

        //
        // The record isn't evenly divisible by PAGE_SIZE.
        //

        __debugbreak();
    }

    Rtl = TraceStore->Rtl;
    Context = TraceStore->TraceContext;
    AddressStore = TraceStore->AddressStore;
    AllocateRecords = AddressStore->AllocateRecords;

    Buffer = AllocateRecords(Context,
                             AddressStore,
                             &AddressRecordSize,
                             &NumberOfAddressRecords);

    if (!Buffer) {
        return FALSE;
    }

    SecureZeroMemory(&Address, sizeof(Address));

    Address.MappedSequenceId = (
        InterlockedIncrement(&TraceStore->MappedSequenceId)
    );

    Address.ProcessId = FastGetCurrentProcessId();
    Address.RequestingThreadId = FastGetCurrentThreadId();

    GetCurrentProcessorNumberEx(&Address.RequestingProcessor);

    Success = GetNumaProcessorNodeEx(&Address.RequestingProcessor, &NumaNode);

    if (Success) {
        Address.RequestingNumaNode = (UCHAR)NumaNode;
    } else {
        Address.RequestingNumaNode = 0;
    }


    Result = Rtl->RtlCopyMappedMemory(Buffer,
                                      &Address,
                                      sizeof(Address));

    if (SUCCEEDED(Result)) {
        *AddressPointer = (PTRACE_STORE_ADDRESS)Buffer;
        return TRUE;
    }

    return FALSE;
}

BOOL
RecordTraceStoreAllocation(
    _Inout_ PTRACE_STORE     TraceStore,
    _In_    PULARGE_INTEGER  RecordSize,
    _In_    PULARGE_INTEGER  NumberOfRecords
    )
{
    PTRACE_STORE_ALLOCATION Allocation;

    if (TraceStore->IsReadonly) {
        __debugbreak();
    }

    if (IsMetadataTraceStore(TraceStore)) {
        Allocation = &TraceStore->Allocation;

        //
        // Metadata trace stores should never have variable record sizes.
        //

        if (Allocation->RecordSize.QuadPart != RecordSize->QuadPart) {
            return FALSE;
        }

    } else {

        Allocation = TraceStore->pAllocation;

        if (Allocation->NumberOfRecords.QuadPart == 0 ||
            Allocation->RecordSize.QuadPart != RecordSize->QuadPart) {

            PVOID Address;
            ULARGE_INTEGER AllocationRecordSize = { sizeof(*Allocation) };
            ULARGE_INTEGER NumberOfAllocationRecords = { 1 };

            //
            // Allocate a new metadata record.
            //

            Address = TraceStore->AllocationStore->AllocateRecords(
                TraceStore->TraceContext,
                TraceStore->AllocationStore,
                &AllocationRecordSize,
                &NumberOfAllocationRecords
            );

            if (!Address) {
                return FALSE;
            }

            Allocation = (PTRACE_STORE_ALLOCATION)Address;
            Allocation->RecordSize.QuadPart = RecordSize->QuadPart;
            Allocation->NumberOfRecords.QuadPart = 0;

            TraceStore->pAllocation = Allocation;
        }
    }

    //
    // Update the record count.
    //

    Allocation->NumberOfRecords.QuadPart += NumberOfRecords->QuadPart;
    return TRUE;
}

BOOL
ConsumeNextTraceStoreMemoryMap(
    _Inout_ PTRACE_STORE TraceStore
    )
{
    BOOL Success;
    BOOL IsMetadata;
    HRESULT Result;
    PRTL Rtl;
    PTRACE_STORE_MEMORY_MAP PrevPrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_MEMORY_MAP PrepareMemoryMap;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    LARGE_INTEGER RequestedTimestamp;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE_STATS Stats;
    TRACE_STORE_STATS DummyStats = { 0 };

    Rtl = TraceStore->Rtl;

    //
    // We need to switch out the current memory map for a new one in order
    // to satisfy this allocation.  This involves: a) retiring the current
    // memory map (sending it to the "prev" list), b) popping an available
    // one off the ready list, and c) preparing a "next memory map" based
    // off the ready one for submission to the threadpool.
    //
    // Note: we want our allocator to be as low-latency as possible, so
    // dropped trace records are preferable to indeterminate waits for
    // resources to be available.
    //


    //
    // We may not have a stats struct available yet if this is the first
    // call to ConsumeNextTraceStoreMemoryMap().  If that's the case, just
    // point the pointer at a dummy one.  This simplifies the rest of the
    // code in the function.
    //

    Stats = TraceStore->pStats;

    if (!Stats) {
        Stats = &DummyStats;
    }

    PrevPrevMemoryMap = TraceStore->PrevMemoryMap;

    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // Retire the previous previous memory map if it exists.
    //

    if (!PrevPrevMemoryMap) {
        goto StartPreparation;
    }

    TraceStore->PrevMemoryMap = NULL;

    if (TraceStore->NoRetire) {
        goto StartPreparation;
    }

    //
    // We need to retire this memory map.  If we're metadata or there's no
    // underlying address record for this memory map, we can go straight to the
    // retire logic.  Otherwise, we need to update the various timestamps.
    //

    AddressPointer = PrevPrevMemoryMap->pAddress;

    if (IsMetadata || (AddressPointer == NULL)) {
        goto RetireOldMemoryMap;
    }

    //
    // Take a local copy of the address record.
    //

    Result = Rtl->RtlCopyMappedMemory(&Address,
                                      AddressPointer,
                                      sizeof(Address));

    if (FAILED(Result)) {

        PrevPrevMemoryMap->pAddress = NULL;
        goto RetireOldMemoryMap;
    }

    //
    // Take a local timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Copy to the Retired timestamp.
    //

    Address.Timestamp.Retired.QuadPart = Elapsed.QuadPart;

    //
    // Calculate the memory map's active elapsed time.
    //

    Elapsed.QuadPart -= Address.Timestamp.Consumed.QuadPart;

    Address.Elapsed.Active.QuadPart = Elapsed.QuadPart;

    //
    // Copy back to the memory mapped backing store.
    //

    Result = Rtl->RtlCopyMappedMemory(AddressPointer,
                                      &Address,
                                      sizeof(Address));

    if (FAILED(Result)) {

        PrevPrevMemoryMap->pAddress = NULL;
    }

RetireOldMemoryMap:

    PushTraceStoreMemoryMap(
        &TraceStore->CloseMemoryMaps,
        PrevPrevMemoryMap
    );

    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);

    //
    // Pop a memory map descriptor off the free list to use for the
    // PrepareMemoryMap.
    //

StartPreparation:

    Success = PopFreeTraceStoreMemoryMap(TraceStore,
                                         &PrepareMemoryMap);

    if (!Success) {

        Success = CreateMemoryMapsForTraceStore(TraceStore,
                                                TraceStore->TraceContext,
                                                0);

        if (Success) {

            Success = PopFreeTraceStoreMemoryMap(TraceStore,
                                                 &PrepareMemoryMap);
        }

        if (!Success) {
            Stats->DroppedRecords++;
            Stats->ExhaustedFreeMemoryMaps++;
            return FALSE;
        }
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {

        //
        // Our allocations are outpacing the next memory map preparation
        // being done asynchronously in the threadpool, so drop the record.
        //

        Stats->AllocationsOutpacingNextMemoryMapPreparation++;
        Stats->DroppedRecords++;

        //
        // Return the PrepareMemoryMap back to the free list.
        //

        ReturnFreeTraceStoreMemoryMap(TraceStore, PrepareMemoryMap);
        return FALSE;
    }

    //
    // We've now got the two things we need: a free memory map to fill in with
    // the details of the next memory map to prepare (PrepareMemoryMap), and
    // the ready memory map (MemoryMap) that contains an active mapping ready
    // for use.
    //

    if (TraceStore->MemoryMap) {

        TraceStore->PrevAddress = TraceStore->MemoryMap->PrevAddress;
        MemoryMap->PrevAddress = TraceStore->MemoryMap->PrevAddress;
        TraceStore->PrevMemoryMap = TraceStore->MemoryMap;
    }

    TraceStore->MemoryMap = MemoryMap;

    if (IsMetadata) {

        //
        // Skip the TRACE_STORE_ADDRESS logic for metadata stores.
        //

        goto PrepareMemoryMap;
    }

    if (!MemoryMap->pAddress) {

        //
        // There was a STATUS_IN_PAGE_ERROR preventing an address record for
        // this memory map.
        //

        goto PrepareMemoryMap;
    }

    //
    // Take a local copy of the address record, update timestamps and
    // calculate elapsed time, then save the local record back to the
    // backing TRACE_STORE_ADDRESS struct.
    //

    Result = Rtl->RtlCopyMappedMemory(&Address,
                                      MemoryMap->pAddress,
                                      sizeof(Address));

    if (FAILED(Result)) {

        //
        // Ignore and continue.
        //

        MemoryMap->pAddress = NULL;
        goto PrepareMemoryMap;
    }

    //
    // Take a local copy of the timestamp.  We'll use this for both the "next"
    // memory map's Consumed timestamp and the "prepare" memory map's Requested
    // timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed);

    //
    // Save the timestamp before we start fiddling with it.
    //

    RequestedTimestamp.QuadPart = Elapsed.QuadPart;

    //
    // Update the Consumed timestamp.
    //

    Address.Timestamp.Consumed.QuadPart = Elapsed.QuadPart;

    //
    // Calculate elapsed time between Prepared and Consumed.
    //

    Elapsed.QuadPart -= Address.Timestamp.Prepared.QuadPart;

    //
    // Save as the AwaitingConsumption timestamp.
    //

    Address.Elapsed.AwaitingConsumption.QuadPart = Elapsed.QuadPart;

    //
    // Copy the local record back to the backing store and ignore the
    // return value.
    //

    Rtl->RtlCopyMappedMemory(MemoryMap->pAddress,
                             &Address,
                             sizeof(Address));

PrepareMemoryMap:

    //
    // Prepare the next memory map with the relevant offset details based
    // on the new memory map and submit it to the threadpool.
    //

    PrepareMemoryMap->FileHandle = MemoryMap->FileHandle;
    PrepareMemoryMap->MappingSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    PrepareMemoryMap->FileOffset.QuadPart = (
        MemoryMap->FileOffset.QuadPart +
        MemoryMap->MappingSize.QuadPart
    );

    //
    // Set the base address to the next contiguous address for this mapping.
    // PrepareNextTraceStoreMemoryMap() will attempt to honor this if it can.
    //

    PrepareMemoryMap->BaseAddress = (
        (PVOID)RtlOffsetToPointer(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.LowPart
        )
    );

    if (IsMetadata) {
        goto SubmitPreparedMemoryMap;
    }

    //
    // Attempt to load the next address record and fill in the relevant details.
    // This will become the prepared memory map's address record if everything
    // goes successfully.
    //

    Success = LoadNextTraceStoreAddress(TraceStore, &AddressPointer);

    if (!Success) {

        //
        // Ignore and go straight to submission.
        //

        goto SubmitPreparedMemoryMap;
    }

    //
    // Take a local copy.
    //

    Result = Rtl->RtlCopyMappedMemory(
        &Address,
        AddressPointer,
        sizeof(Address)
    );

    if (FAILED(Result)) {

        //
        // Ignore and go straight to submission.
        //

        goto SubmitPreparedMemoryMap;
    }

    //
    // Copy the timestamp we took earlier to the Requested timestamp.
    //

    Address.Timestamp.Requested.QuadPart = RequestedTimestamp.QuadPart;

    //
    // Copy the local record back to the backing store.
    //

    Result = Rtl->RtlCopyMappedMemory(AddressPointer,
                                      &Address,
                                      sizeof(Address));

    if (SUCCEEDED(Result)) {

        //
        // Update the memory map to point at the address struct.
        //

        PrepareMemoryMap->pAddress = AddressPointer;

    } else if (PrepareMemoryMap->pAddress != NULL) {

        //
        // Invariant check: this should never get hit.
        //

        __debugbreak();
    }

SubmitPreparedMemoryMap:
    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, PrepareMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    return TRUE;
}

_Check_return_
_Success_(return != 0)
LPVOID
AllocateRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
    )
{
    BOOL Success;
    PTRACE_STORE_MEMORY_MAP PrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    ULONG_PTR AllocationSize;
    PVOID ReturnAddress = NULL;
    PVOID NextAddress;
    PVOID EndAddress;
    PVOID PrevPage;
    PVOID NextPage;
    PVOID PageAfterNextPage;

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return NULL;
    }

    MemoryMap = TraceStore->MemoryMap;

    if (!MemoryMap) {
        return NULL;
    }

#ifdef _M_X64

    AllocationSize = (
        (ULONG_PTR)(
            RecordSize->QuadPart *
            NumberOfRecords->QuadPart
        )
    );

    AllocationSize = ALIGN_UP(AllocationSize, sizeof(ULONG_PTR));

    if (AllocationSize > (ULONG_PTR)MemoryMap->MappingSize.QuadPart) {
        return NULL;
    }

#else
    {
        ULARGE_INTEGER Size = { 0 };

        //
        // Ignore allocation attempts over 2GB on 32-bit.
        //

        if (RecordSize->HighPart != 0 || NumberOfRecords->HighPart != 0) {
            return NULL;
        }

        Size->QuadPart = UInt32x32To64(RecordSize->LowPart,
                                       NumberOfRecords->LowPart);

        if (Size->HighPart != 0) {
            return NULL;
        }

        AllocationSize = (ULONG_PTR)Size->LowPart;

    }

    AllocationSize = ALIGN_UP(AllocationSize, sizeof(ULONG_PTR));

    if (AllocationSize > MemoryMap->MappingSize.LowPart) {
        return NULL;
    }
#endif

    NextAddress = (
        (PVOID)RtlOffsetToPointer(
            MemoryMap->NextAddress,
            AllocationSize
        )
    );

    EndAddress = (
        (PVOID)RtlOffsetToPointer(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.LowPart
        )
    );

    PrevPage = (PVOID)ALIGN_DOWN(MemoryMap->NextAddress, PAGE_SIZE);
    NextPage = (PVOID)ALIGN_DOWN(NextAddress, PAGE_SIZE);
    PageAfterNextPage = (PVOID)((ULONG_PTR)NextPage + PAGE_SIZE);

    if (NextAddress > EndAddress) {

        ULONG_PTR PrevMemoryMapAllocSize;
        ULONG_PTR NextMemoryMapAllocSize;

        PrevMemoryMap = MemoryMap;

        PrevMemoryMapAllocSize = (
            (ULONG_PTR)EndAddress -
            (ULONG_PTR)PrevMemoryMap->NextAddress
        );

        NextMemoryMapAllocSize = (
            (ULONG_PTR)NextAddress -
            (ULONG_PTR)EndAddress
        );

        if (!ConsumeNextTraceStoreMemoryMap(TraceStore)) {
            return NULL;
        }

        MemoryMap = TraceStore->MemoryMap;

        if (PrevMemoryMapAllocSize == 0) {

            //
            // No spill necessary.
            //

            ReturnAddress = MemoryMap->BaseAddress;

            MemoryMap->NextAddress = (
                (PVOID)RtlOffsetToPointer(
                    MemoryMap->BaseAddress,
                    AllocationSize
                )
            );

        } else {

            //
            // The requested allocation will spill over into the next memory
            // map.  This is fine as long as the next memory map is mapped at a
            // contiguous address.
            //

            if (MemoryMap->BaseAddress != EndAddress) {

                //
                // Ugh, non-contiguous mapping.
                //

                if (!TraceStore->IsReadonly &&
                    !IsMetadataTraceStore(TraceStore) &&
                    !HasVaryingRecordSizes(TraceStore)) {

                    __debugbreak();
                }

                ReturnAddress = MemoryMap->BaseAddress;

                MemoryMap->NextAddress = (
                    (PVOID)RtlOffsetToPointer(
                        MemoryMap->BaseAddress,
                        AllocationSize
                    )
                );

            } else {

                //
                // The mapping is contiguous.
                //


                //
                // Our return address will be the value of the previous memory
                // map's next address.
                //

                ReturnAddress = PrevMemoryMap->NextAddress;

                //
                // Update the previous address fields.
                //

                PrevMemoryMap->PrevAddress = PrevMemoryMap->NextAddress;
                TraceStore->PrevAddress = PrevMemoryMap->NextAddress;
                MemoryMap->PrevAddress = PrevMemoryMap->NextAddress;

                //
                // Adjust the new memory map's NextAddress to account for the
                // bytes we had to spill over.
                //

                MemoryMap->NextAddress = (
                    (PVOID)RtlOffsetToPointer(
                        MemoryMap->BaseAddress,
                        NextMemoryMapAllocSize
                    )
                );

                if (MemoryMap->NextAddress != NextAddress) {
                    __debugbreak();
                }
            }

        }

    } else {

        if (TraceStore->NoPrefaulting) {
            goto UpdateAddresses;
        }

        //
        // If this allocation crosses a page boundary, we prefault the page
        // after the next page in a separate thread (as long as it's still
        // within our allocated range).  (We do this in a separate thread
        // as a page fault may put the thread into an alertable wait (i.e.
        // suspends it) until the underlying I/O completes if the request
        // couldn't be served by the cache manager.  That would adversely
        // affect the latency of our hot-path tracing code where
        // allocations are done quite frequently.)
        //

        if (PrevPage != NextPage) {

            if (PageAfterNextPage < EndAddress) {

                PTRACE_STORE_MEMORY_MAP PrefaultMemoryMap;

                Success = PopFreeTraceStoreMemoryMap(
                    TraceStore,
                    &PrefaultMemoryMap
                );

                if (!Success) {
                    goto UpdateAddresses;
                }

                //
                // Prefault the page after the next page after this
                // address.  That is, prefault the page that is two
                // pages away from whatever page NextAddress is in.
                //

                PrefaultMemoryMap->NextAddress = PageAfterNextPage;

                PushTraceStoreMemoryMap(
                    &TraceStore->PrefaultMemoryMaps,
                    PrefaultMemoryMap
                );

                SubmitThreadpoolWork(TraceStore->PrefaultFuturePageWork);
            }
        }

        //
        // Update the relevant memory map addresses.
        //

UpdateAddresses:
        ReturnAddress           = MemoryMap->NextAddress;
        MemoryMap->PrevAddress  = MemoryMap->NextAddress;
        TraceStore->PrevAddress = MemoryMap->NextAddress;

        MemoryMap->NextAddress = NextAddress;

    }

    if (!IsMetadataTraceStore(TraceStore) && !TraceStore->IsReadonly) {

        if (!TraceStore->RecordSimpleAllocation) {

            RecordTraceStoreAllocation(TraceStore,
                                       RecordSize,
                                       NumberOfRecords);

        } else {

            TraceStore->pAllocation->NumberOfAllocations.QuadPart += 1;
            TraceStore->pAllocation->AllocationSize.QuadPart += AllocationSize;
        }

        TraceStore->pEof->EndOfFile.QuadPart += AllocationSize;
    }

    TraceStore->TotalNumberOfAllocations.QuadPart += 1;
    TraceStore->TotalAllocationSize.QuadPart += AllocationSize;

    if (MemoryMap->NextAddress == EndAddress) {

        //
        // This memory map has been filled entirely; attempt to consume the
        // next one.  Ignore the return code; we can't do much at this point
        // if it fails.
        //

        ConsumeNextTraceStoreMemoryMap(TraceStore);
    }

    return ReturnAddress;
}

LPVOID
GetNextRecord(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PULARGE_INTEGER RecordSize
)
{
    ULARGE_INTEGER RecordCount = { 1 };
    return AllocateRecords(TraceContext, TraceStore, RecordSize, &RecordCount);
}

VOID
FreeRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PVOID           Buffer
    )
{
    //
    // Not currently implemented.
    //
    return;
}

BOOL
InitializeTraceSession(
    _In_                                 PRTL           Rtl,
    _Inout_bytecap_(*SizeOfTraceSession) PTRACE_SESSION TraceSession,
    _In_                                 PULONG         SizeOfTraceSession
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

    TraceSession->Rtl = Rtl;

    GetSystemTimeAsFileTime(&TraceSession->SystemTime);
    return TRUE;

}

BOOL
BindTraceStoreToTraceContext(
    _Inout_ PTRACE_STORE TraceStore,
    _Inout_ PTRACE_CONTEXT TraceContext
    )
{
    BOOL Success;
    BOOL IsMetadata;
    DWORD WaitResult;
    HRESULT Result;
    PRTL Rtl;
    PTRACE_STORE_ADDRESS AddressPointer;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_EOF Eof;
    PTRACE_STORE_TIME Time;
    PTRACE_STORE_STATS Stats;
    PLARGE_INTEGER Elapsed;
    TRACE_STORE_ADDRESS Address;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    if (!TraceContext->HeapHandle) {
        return FALSE;
    }

    Rtl = TraceStore->Rtl;

    //
    // Initialize all of our singly-linked list heads.
    //

    InitializeSListHead(&TraceStore->CloseMemoryMaps);
    InitializeSListHead(&TraceStore->PrepareMemoryMaps);
    InitializeSListHead(&TraceStore->FreeMemoryMaps);
    InitializeSListHead(&TraceStore->NextMemoryMaps);
    InitializeSListHead(&TraceStore->PrefaultMemoryMaps);

    //
    // Create the initial set of memory map records.
    //

    Success = CreateMemoryMapsForTraceStore(
        TraceStore,
        TraceContext,
        InitialFreeMemoryMaps
    );

    if (!Success) {
        return FALSE;
    }

    Success = PopFreeTraceStoreMemoryMap(TraceStore, &FirstMemoryMap);

    if (!Success) {
        return FALSE;
    }

    TraceStore->NextMemoryMapAvailableEvent = (
        CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL
        )
    );

    if (!TraceStore->NextMemoryMapAvailableEvent) {
        return FALSE;
    }

    TraceStore->AllMemoryMapsAreFreeEvent = (
        CreateEvent(
            NULL,
            FALSE,
            FALSE,
            NULL
        )
    );

    if (!TraceStore->AllMemoryMapsAreFreeEvent) {
        return FALSE;
    }

    TraceStore->PrepareNextMemoryMapWork = CreateThreadpoolWork(
        &PrepareNextTraceStoreMemoryMapCallback,
        TraceStore,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!TraceStore->PrepareNextMemoryMapWork) {
        return FALSE;
    }

    TraceStore->PrefaultFuturePageWork = CreateThreadpoolWork(
        &PrefaultFuturePageCallback,
        TraceStore,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!TraceStore->PrefaultFuturePageWork) {
        return FALSE;
    }

    TraceStore->CloseMemoryMapWork = CreateThreadpoolWork(
        &ReleasePrevMemoryMapCallback,
        TraceStore,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!TraceStore->CloseMemoryMapWork) {
        return FALSE;
    }

    TraceStore->TraceContext = TraceContext;

    FirstMemoryMap->FileHandle = TraceStore->FileHandle;
    FirstMemoryMap->MappingSize.QuadPart = TraceStore->MappingSize.QuadPart;

    //
    // If we're metadata, go straight to submission of the prepared memory map.
    //

    IsMetadata = IsMetadataTraceStore(TraceStore);

    if (IsMetadata) {
        goto SubmitFirstMemoryMap;
    }

    //
    // Attempt to load the TRACE_STORE_ADDRESS record for the memory map.
    //

    Success = LoadNextTraceStoreAddress(TraceStore, &AddressPointer);

    if (!Success) {

        //
        // If we couldn't load the address, just go straight to submission of
        // the memory map.
        //

        goto SubmitFirstMemoryMap;
    }

    //
    // The remaining logic deals with initializing an address structure for
    // first use.  This is normally dealt with by ConsumeNextTraceStoreMemory-
    // Map() as part of preparation of the next memory map, but we need to do it
    // manually here for the first map.
    //

    //
    // Take a local copy.
    //

    Result = Rtl->RtlCopyMappedMemory(
        &Address,
        AddressPointer,
        sizeof(Address)
    );

    if (FAILED(Result)) {

        //
        // Ignore and go straight to submission.
        //

        goto SubmitFirstMemoryMap;
    }

    //
    // Set the Requested timestamp.
    //

    Elapsed = &Address.Timestamp.Requested;
    TraceStoreQueryPerformanceCounter(TraceStore, Elapsed);

    //
    // Zero out everything else.
    //

    Address.Timestamp.Prepared.QuadPart = 0;
    Address.Timestamp.Consumed.QuadPart = 0;
    Address.Timestamp.Retired.QuadPart = 0;
    Address.Timestamp.Released.QuadPart = 0;

    Address.Elapsed.AwaitingPreparation.QuadPart = 0;
    Address.Elapsed.AwaitingConsumption.QuadPart = 0;
    Address.Elapsed.Active.QuadPart = 0;
    Address.Elapsed.AwaitingRelease.QuadPart = 0;


    //
    // Copy the address back.
    //

    Result = Rtl->RtlCopyMappedMemory(AddressPointer,
                                      &Address,
                                      sizeof(Address));

    if (SUCCEEDED(Result)) {

        //
        // Update the memory map to point at the address struct.
        //

        FirstMemoryMap->pAddress = AddressPointer;

    }

SubmitFirstMemoryMap:
    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, FirstMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    WaitResult = WaitForSingleObject(TraceStore->NextMemoryMapAvailableEvent,
                                     INFINITE);

    if (WaitResult != WAIT_OBJECT_0) {
        return FALSE;
    }

    Success = ConsumeNextTraceStoreMemoryMap(TraceStore);

    if (!Success) {
        return FALSE;
    }

    if (IsMetadata) {

        //
        // Metadata stores just use the TraceStore-backed, non-memory-mapped
        // TRACE_STORE_INFO struct.
        //

        TraceStore->pInfo = &TraceStore->Info;

    } else {

        PTRACE_STORE AllocationStore;
        PTRACE_STORE InfoStore;

        AllocationStore = TraceStore->AllocationStore;
        TraceStore->pAllocation = (
            (PTRACE_STORE_ALLOCATION)AllocationStore->MemoryMap->BaseAddress
        );

        InfoStore = TraceStore->InfoStore;
        TraceStore->pInfo = (
            (PTRACE_STORE_INFO)InfoStore->MemoryMap->BaseAddress
        );

    }

    Eof = TraceStore->pEof = &TraceStore->pInfo->Eof;
    Time = TraceStore->pTime = &TraceStore->pInfo->Time;
    Stats = TraceStore->pStats = &TraceStore->pInfo->Stats;

    if (!TraceStore->IsReadonly) {
        PTRACE_STORE_TIME SourceTime = &TraceContext->Time;

        //
        // Initialize Eof.
        //

        Eof->EndOfFile.QuadPart = 0;

        //
        // Copy time.
        //

        Rtl->RtlCopyMappedMemory(Time, SourceTime, sizeof(*Time));

        //
        // Zero out stats.
        //

        SecureZeroMemory(Stats, sizeof(*Stats));
    }

    return TRUE;
}

_Success_(return != 0)
BOOL
InitializeTraceContext(
    _In_     PRTL            Rtl,
    _Inout_bytecap_(*SizeOfTraceContext) PTRACE_CONTEXT  TraceContext,
    _In_     PULONG          SizeOfTraceContext,
    _In_     PTRACE_SESSION  TraceSession,
    _In_     PTRACE_STORES   TraceStores,
    _In_     PTP_CALLBACK_ENVIRON  ThreadpoolCallbackEnvironment,
    _In_opt_ PVOID UserData,
    _In_     BOOL Readonly,
    _In_     BOOL Compress
    )
{
    USHORT Index;
    USHORT StoreIndex;

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
        *SizeOfTraceContext = sizeof(*TraceContext);
        return FALSE;
    }

    if (!Rtl) {
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

    TraceContext->HeapHandle = HeapCreate(0,
                                          InitialTraceContextHeapSize,
                                          MaximumTraceContextHeapSize);

    if (!TraceContext->HeapHandle) {
        return FALSE;
    }

    if (!InitializeTraceStoreTime(Rtl, &TraceContext->Time)) {
        return FALSE;
    }

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {

        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex];
        PTRACE_STORE AllocationStore = TraceStore + 1;
        PTRACE_STORE AddressStore = TraceStore + 2;
        PTRACE_STORE InfoStore = TraceStore + 3;

#define BIND_STORE(Name)                                              \
    if (!BindTraceStoreToTraceContext(##Name##Store, TraceContext)) { \
        return FALSE;                                                 \
    }

        BIND_STORE(Allocation);
        BIND_STORE(Address);
        BIND_STORE(Info);

        //
        // The TraceStore needs to come last as it requires the metadata stores
        // to be bound and mapped before it can finalize its own binding.
        //

        BIND_STORE(Trace);

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

VOID
Debugbreak()
{
    __debugbreak();
}

#ifdef __cpp
} // extern "C"
#endif

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
