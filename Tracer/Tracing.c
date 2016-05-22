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

static const USHORT TraceStoreMetadataStructSize = sizeof(TRACE_STORE_METADATA);
static const USHORT TraceStoreAddressStructSize = sizeof(TRACE_STORE_ADDRESS);
static const USHORT TraceStoreEofStructSize = sizeof(TRACE_STORE_EOF);

static const ULONG DefaultTraceStoreMappingSize         = (1 << 21); // 2MB

static const ULONG DefaultMetadataTraceStoreSize        = (1 << 21); // 2MB
static const ULONG DefaultMetadataTraceStoreMappingSize = (1 << 16); // 64KB

static const ULONG DefaultAddressTraceStoreSize         = (1 << 21); // 2MB
static const ULONG DefaultAddressTraceStoreMappingSize  = (1 << 16); // 64KB

static const ULONG DefaultEofTraceStoreSize             = (1 << 16); // 64KB
static const ULONG DefaultEofTraceStoreMappingSize      = (1 << 16); // 64KB

BOOL
LoadNextTraceStoreAddress(
    _In_    PTRACE_STORE TraceStore,
    _Out_   PPTRACE_STORE_ADDRESS AddressPointer
    );

FORCEINLINE
BOOL
IsMetadataTraceStore(_In_ PTRACE_STORE TraceStore)
{
    return (TraceStore->MetadataStore == NULL);
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
        TraceStoreMetadataSuffixLength
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
    Longest.QuadPart += TraceStoreMetadataSuffixLength;

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
        TraceStoreMetadataSuffixLength
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
    Longest.QuadPart += TraceStoreMetadataSuffixLength;

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
    _Inout_ PSLIST_HEADER ListHead,
    _Inout_ PPTRACE_STORE_MEMORY_MAP MemoryMap
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
    _Inout_     PTRACE_STORE MetadataStore,
    _Inout_     PTRACE_STORE AddressStore,
    _Inout_     PTRACE_STORE EofStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
    )
{
    BOOL Success;
    HRESULT Result;
    WCHAR MetadataPath[_OUR_MAX_PATH];
    WCHAR AddressPath[_OUR_MAX_PATH];
    WCHAR EofPath[_OUR_MAX_PATH];
    PCWSTR MetadataSuffix = TraceStoreMetadataSuffix;
    PCWSTR AddressSuffix = TraceStoreAddressSuffix;
    PCWSTR EofSuffix = TraceStoreEofSuffix;

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

    INIT_METADATA_PATH(Metadata);
    INIT_METADATA_PATH(Address);
    INIT_METADATA_PATH(Eof);

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

    INIT_METADATA(Metadata);
    INIT_METADATA(Address);
    INIT_METADATA(Eof);

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
    _In_        BOOL            Readonly
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
    LARGE_INTEGER Frequency;
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

    Success = CreateDirectory(BaseDirectory, NULL);
    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            return FALSE;
        }
    }

    TraceStores->Rtl = Rtl;

    TraceStores->NumberOfTraceStores = NumberOfTraceStores;
    TraceStores->ElementsPerTraceStore = ElementsPerTraceStore;

    if (Readonly) {
        CreateFileDesiredAccess = GENERIC_READ;
        CreateFileMappingProtectionFlags = PAGE_READONLY;
        MapViewOfFileDesiredAccess = FILE_MAP_READ;
    } else {
        CreateFileDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        CreateFileMappingProtectionFlags = PAGE_READWRITE;
        MapViewOfFileDesiredAccess =  FILE_MAP_READ | FILE_MAP_WRITE;
    }

    QueryPerformanceFrequency(&Frequency);

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {

        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex];
        PTRACE_STORE MetadataStore = TraceStore + 1;
        PTRACE_STORE AddressStore = TraceStore + 2;
        PTRACE_STORE EofStore = TraceStore + 3;

        LPCWSTR FileName = TraceStoreFileNames[Index];
        DWORD InitialSize = Sizes[Index];
        ULONG MappingSize = DefaultTraceStoreMappingSize;

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
        TraceStore->Frequency.QuadPart = Frequency.QuadPart;

        Success = InitializeTraceStore(
            Rtl,
            Path,
            TraceStore,
            MetadataStore,
            AddressStore,
            EofStore,
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
    if (!TraceStore) {
        return;
    }

    if (!MemoryMap || !*MemoryMap) {
        return;
    }

    PushTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, *MemoryMap);
    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);
    *MemoryMap = NULL;
}

VOID
CloseStore(
    _In_ PTRACE_STORE TraceStore
)
{
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    if (!TraceStore) {
        return;
    }

    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->PrevMemoryMap);
    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->MemoryMap);

    while (PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {
        SubmitCloseMemoryMapThreadpoolWork(TraceStore, &MemoryMap);
    }

    if (TraceStore->FileHandle) {
        FlushFileBuffers(TraceStore->FileHandle);
        CloseHandle(TraceStore->FileHandle);
        TraceStore->FileHandle = NULL;
    }

    if (TraceStore->NextMemoryMapAvailableEvent) {
        CloseHandle(TraceStore->NextMemoryMapAvailableEvent);
        TraceStore->NextMemoryMapAvailableEvent = NULL;
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
    if (!TraceStore) {
        return;
    }

    if (TraceStore->MetadataStore) {
        CloseStore(TraceStore->MetadataStore);
        TraceStore->MetadataStore = NULL;
    }

    if (TraceStore->AddressStore) {
        CloseStore(TraceStore->AddressStore);
        TraceStore->AddressStore = NULL;
    }

    if (TraceStore->EofStore) {
        CloseStore(TraceStore->EofStore);
        TraceStore->EofStore = NULL;
    }

    CloseStore(TraceStore);
}

void
CloseTraceStores(PTRACE_STORES TraceStores)
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
    BOOL Success;
    BOOL IsMetadata;
    BOOL HaveAddress;
    BOOL SetPreferredBaseAddress;
    PRTL Rtl;
    HRESULT Result;
    PVOID PreferredBaseAddress;
    PVOID OriginalPreferredBaseAddress;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    FILE_STANDARD_INFO FileInfo;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    ULONGLONG MappedSequenceId;
    LARGE_INTEGER CurrentFileOffset;
    LARGE_INTEGER NewFileOffset;
    LARGE_INTEGER DistanceToMove;
    LARGE_INTEGER Timestamp;
    LARGE_INTEGER Elapsed;

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    Rtl = TraceStore->Rtl;

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

    PreferredBaseAddress = NULL;
    OriginalPreferredBaseAddress = NULL;

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

    if (TraceStore->NoPreferredAddressReuse) {

        //
        // If we've been instructed not to use the preferred address, we can
        // skip the following logic and go straight to mapping the memory.
        // We do this check after we copy the AddressPointer to the local
        // Address struct because we're in 'HaveAddress = TRUE' mode and we
        // expect the Address struct to be set correctly later on.
        //

        goto TryMapMemory;
    }

    //
    // The - 1 here is to reflect the fact that the trace store's mapped
    // sequence ID will already be incremented by this stage.
    //

    MappedSequenceId = TraceStore->MappedSequenceId.QuadPart - 1;

    //
    // If the mapping sequence, size and file offset line up, and the
    // address is under 128TB, use the preferred base address.
    //

    SetPreferredBaseAddress = (
        Address.MappedSequenceId.QuadPart == MappedSequenceId          &&
        Address.MappedSize.QuadPart == MemoryMap->MappingSize.QuadPart &&
        Address.FileOffset.QuadPart == MemoryMap->FileOffset.QuadPart  &&
        (ULONG_PTR)Address.BaseAddress < ((1ULL << 47) - 1)
    );

    if (SetPreferredBaseAddress) {

        //
        // Everything lines up, try use this as the base address.
        //

        PreferredBaseAddress = Address.BaseAddress;

    } else {

        //
        // As soon as one address record doesn't line up we disable all
        // future re-use attempts.
        //

        TraceStore->NoPreferredAddressReuse = TRUE;
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
    // Record all of the mapping information in our address record.  Note that
    // we don't copy over the mapping sequence ID, that should already be set
    // correctly.
    //

    Address.PreferredBaseAddress = PreferredBaseAddress;
    Address.BaseAddress = MemoryMap->BaseAddress;
    Address.MappedSize.QuadPart = MemoryMap->MappingSize.QuadPart;
    Address.FileOffset.QuadPart = MemoryMap->FileOffset.QuadPart;

    //
    // Take a local copy of the timestamp.
    //

    QueryPerformanceCounter(&Timestamp);

    //
    // Copy it to the Prepared timestamp.
    //

    Address.Timestamp.Prepared.QuadPart = Timestamp.QuadPart;

    //
    // Calculate the elapsed time spent awaiting preparation.
    //

    Elapsed.QuadPart = (
        Timestamp.QuadPart -
        Address.Timestamp.Requested.QuadPart
    );

    Elapsed.QuadPart *= TIMESTAMP_TO_SECONDS;
    Elapsed.QuadPart /= TraceStore->Frequency.QuadPart;

    //
    // Copy it to the elapsed AwaitingPreparation timestamp.
    //

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
    LARGE_INTEGER Timestamp;
    LARGE_INTEGER Elapsed;

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
    // Take a local copy of the timestamp.
    //

    QueryPerformanceCounter(&Timestamp);

    //
    // Copy it to the Released timestamp.
    //

    Address.Timestamp.Released.QuadPart = Timestamp.QuadPart;

    //
    // Calculate the elapsed time between when the memory map was submitted
    // for retirement and now.
    //

    Elapsed.QuadPart = (
        Timestamp.QuadPart -
        Address.Timestamp.Retired.QuadPart
    );

    Elapsed.QuadPart *= TIMESTAMP_TO_SECONDS;
    Elapsed.QuadPart /= TraceStore->Frequency.QuadPart;

    //
    // Update the address record with the elapsed time.
    //

    Address.Elapsed.AwaitingRelease.QuadPart = Elapsed.QuadPart;

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
    PVOID Buffer;
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

    *AddressPointer = (PTRACE_STORE_ADDRESS)Buffer;
    TraceStore->pAddress = (PTRACE_STORE_ADDRESS)Buffer;

    return TRUE;
}

BOOL
RecordTraceStoreAllocation(
    _Inout_ PTRACE_STORE     TraceStore,
    _In_    PULARGE_INTEGER  RecordSize,
    _In_    PULARGE_INTEGER  NumberOfRecords
    )
{
    PTRACE_STORE_METADATA Metadata;

    if (TraceStore->IsReadonly) {
        __debugbreak();
    }

    if (IsMetadataTraceStore(TraceStore)) {
        Metadata = &TraceStore->Metadata;

        //
        // Metadata trace stores should never have variable record sizes.
        //

        if (Metadata->RecordSize.QuadPart != RecordSize->QuadPart) {
            return FALSE;
        }

    } else {

        Metadata = TraceStore->pMetadata;

        if (Metadata->NumberOfRecords.QuadPart == 0 ||
            Metadata->RecordSize.QuadPart != RecordSize->QuadPart) {

            PVOID Address;
            ULARGE_INTEGER MetadataRecordSize = { sizeof(*Metadata) };
            ULARGE_INTEGER NumberOfMetadataRecords = { 1 };

            //
            // Allocate a new metadata record.
            //

            Address = TraceStore->MetadataStore->AllocateRecords(
                TraceStore->TraceContext,
                TraceStore->MetadataStore,
                &MetadataRecordSize,
                &NumberOfMetadataRecords
            );

            if (!Address) {
                return FALSE;
            }

            Metadata = (PTRACE_STORE_METADATA)Address;
            Metadata->RecordSize.QuadPart = RecordSize->QuadPart;
            Metadata->NumberOfRecords.QuadPart = 0;

            TraceStore->pMetadata = Metadata;
        }
    }

    //
    // Update the record count.
    //

    Metadata->NumberOfRecords.QuadPart += NumberOfRecords->QuadPart;
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
    ULONGLONG MappedSequenceId;
    LARGE_INTEGER Timestamp;
    LARGE_INTEGER Elapsed;

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

    QueryPerformanceCounter(&Timestamp);

    //
    // Copy to the Retired timestamp.
    //

    Address.Timestamp.Retired.QuadPart = Timestamp.QuadPart;

    //
    // Calculate the memory map's active elapsed time.
    //

    Elapsed.QuadPart = (
        Timestamp.QuadPart -
        Address.Timestamp.Consumed.QuadPart
    );

    Elapsed.QuadPart *= TIMESTAMP_TO_SECONDS;
    Elapsed.QuadPart /= TraceStore->Frequency.QuadPart;

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

    Success = PopTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps,
                                     &PrepareMemoryMap);

    if (!Success) {

        Success = CreateMemoryMapsForTraceStore(TraceStore,
                                                TraceStore->TraceContext,
                                                0);

        if (Success) {

            Success = PopTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps,
                                             &PrepareMemoryMap);

        }

        if (!Success) {
            ++TraceStore->DroppedRecords;
            ++TraceStore->ExhaustedFreeMemoryMaps;
            return FALSE;
        }
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {

        //
        // Our allocations are outpacing the next memory map preparation
        // being done asynchronously in the threadpool, so drop the record.
        //

        ++TraceStore->AllocationsOutpacingNextMemoryMapPreparation;
        ++TraceStore->DroppedRecords;

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

    QueryPerformanceCounter(&Timestamp);

    //
    // Update the Consumed timestamp.
    //

    Address.Timestamp.Consumed.QuadPart = Timestamp.QuadPart;

    //
    // Calculate elapsed time between Prepared and Consumed.
    //

    Elapsed.QuadPart = (
        Timestamp.QuadPart -
        Address.Timestamp.Prepared.QuadPart
    );

    Elapsed.QuadPart *= TIMESTAMP_TO_SECONDS;
    Elapsed.QuadPart /= TraceStore->Frequency.QuadPart;

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

    MappedSequenceId = TraceStore->MappedSequenceId.QuadPart++;

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

    Address.Timestamp.Requested.QuadPart = Timestamp.QuadPart;

    //
    // Zero out all other timestamps and elapsed.
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
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    ULONG_PTR AllocationSize;
    PVOID ReturnAddress = NULL;
    PVOID NextAddress;
    PVOID EndAddress;
    PVOID ThisPage;
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

    //
    // XXX todo: ensure an allocation doesn't span a page; put a fake
    // allocation in there to extend.
    //

    ThisPage = (PVOID)ALIGN_DOWN(MemoryMap->NextAddress, PAGE_SIZE);
    PrevPage = (PVOID)ALIGN_DOWN(MemoryMap->NextAddress, PAGE_SIZE);
    NextPage = (PVOID)ALIGN_DOWN(NextAddress, PAGE_SIZE);
    PageAfterNextPage = (PVOID)((ULONG_PTR)NextPage + PAGE_SIZE);

    if (NextAddress > EndAddress) {

        if (!ConsumeNextTraceStoreMemoryMap(TraceStore)) {
            return NULL;
        }

        MemoryMap = TraceStore->MemoryMap;
        ReturnAddress = MemoryMap->BaseAddress;

        MemoryMap->NextAddress = (
            (PVOID)RtlOffsetToPointer(
                MemoryMap->BaseAddress,
                AllocationSize
            )
        );

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

                Success = PopTraceStoreMemoryMap(
                    &TraceStore->FreeMemoryMaps,
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

        if (!TraceStore->RecordSimpleMetadata) {

            RecordTraceStoreAllocation(TraceStore,
                                       RecordSize,
                                       NumberOfRecords);

        } else {

            TraceStore->pMetadata->NumberOfAllocations.QuadPart += 1;
            TraceStore->pMetadata->AllocationSize.QuadPart += AllocationSize;
        }

        TraceStore->pEof->EndOfFile.QuadPart += AllocationSize;
    }

    TraceStore->TotalNumberOfAllocations.QuadPart += 1;
    TraceStore->TotalAllocationSize.QuadPart += AllocationSize;

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
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;

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

    Success = PopTraceStoreMemoryMap(
        &TraceStore->FreeMemoryMaps,
        &FirstMemoryMap
    );

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
    // Advance the trace store's mapped sequence ID counter.
    //

    TraceStore->MappedSequenceId.QuadPart++;

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

    Rtl = TraceStore->Rtl;

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

    QueryPerformanceCounter(&Address.Timestamp.Requested);

    //
    // Zero out everything else.
    //

    Address.MappedSequenceId.QuadPart = 0;

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

    if (!IsMetadata) {
        PTRACE_STORE MetadataStore;
        PTRACE_STORE EofStore;

        MetadataStore = TraceStore->MetadataStore;
        TraceStore->pMetadata = (
            (PTRACE_STORE_METADATA)MetadataStore->MemoryMap->BaseAddress
        );

        EofStore = TraceStore->EofStore;
        TraceStore->pEof = (
            (PTRACE_STORE_EOF)EofStore->MemoryMap->BaseAddress
        );

        if (!TraceStore->IsReadonly) {
            TraceStore->pEof->EndOfFile.QuadPart = 0;
        }
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
    _In_     BOOL Readonly
)
{
    USHORT Index;
    USHORT StoreIndex;
    LARGE_INTEGER Frequency;

    if (!Rtl) {
        return FALSE;
    }

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

    QueryPerformanceFrequency(&Frequency);
    TraceContext->PerformanceCounterFrequency.QuadPart = Frequency.QuadPart;

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {

        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex];
        PTRACE_STORE MetadataStore = TraceStore + 1;
        PTRACE_STORE AddressStore = TraceStore + 2;
        PTRACE_STORE EofStore = TraceStore + 3;

#define BIND_STORE(Name)                                              \
    if (!BindTraceStoreToTraceContext(##Name##Store, TraceContext)) { \
        return FALSE;                                                 \
    }

        BIND_STORE(Metadata);
        BIND_STORE(Address);
        BIND_STORE(Eof);

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
