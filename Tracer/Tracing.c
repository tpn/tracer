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

static const ULONG DefaultTraceStoreMappingSize = (1 << 21); // 2MB
static const ULONG DefaultMetadataTraceStoreSize = (1 << 21); // 2MB
static const ULONG DefaultMetadataTraceStoreMappingSize = (1 << 16); // 64KB

#define MAX_UNICODE_STRING 255
#define _OUR_MAX_PATH MAX_UNICODE_STRING

FORCEINLINE
BOOL
IsMetadataTraceStore(_In_ PTRACE_STORE TraceStore)
{
    return (TraceStore->MetadataStore == NULL);
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

FORCEINLINE
VOID
ReturnFreeTraceStoreMemoryMap(
    _Inout_ PTRACE_STORE TraceStore,
    _Inout_ PTRACE_STORE_MEMORY_MAP MemoryMap
)
{
    SecureZeroMemory(MemoryMap, sizeof(*MemoryMap));
    InterlockedPushEntrySList(&TraceStore->FreeMemoryMaps, &MemoryMap->ListEntry);
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

    *MemoryMap = CONTAINING_RECORD(ListEntry, TRACE_STORE_MEMORY_MAP, ListEntry);
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

BOOL
InitializeStore(
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
)
{

    if (!Path || !TraceStore) {
        return FALSE;
    }

    if (!TraceStore->FileHandle) {
        // We're a metadata store.
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

    return TRUE;
error:
    CloseTraceStore(TraceStore);
    return FALSE;
}

BOOL
InitializeTraceStore(
    _In_        PRTL Rtl,
    _In_        PCWSTR Path,
    _Inout_     PTRACE_STORE TraceStore,
    _Inout_     PTRACE_STORE MetadataStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
)
{
    BOOL Success;
    HRESULT Result;
    WCHAR MetadataPath[_OUR_MAX_PATH];

    if (!Path || !TraceStore || !Rtl) {
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

    TraceStore->Rtl = Rtl;

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
        DefaultMetadataTraceStoreSize,
        DefaultMetadataTraceStoreMappingSize
    );

    if (!Success) {
        return FALSE;
    }

    MetadataStore->NumberOfRecords.QuadPart = 1;
    MetadataStore->RecordSize.QuadPart = sizeof(TRACE_STORE_METADATA);

    TraceStore->MetadataStore = MetadataStore;

    if (!InitializeStore(Path, TraceStore, InitialSize, MappingSize)) {
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
    _In_        PRTL            Rtl,
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PULONG          SizeOfTraceStores,
    _In_opt_    PULONG          InitialFileSizes
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

    TraceStores->Rtl = Rtl;

    for (Index = 0, StoreIndex = 0; Index < NumberOfTraceStores; Index++, StoreIndex += 2) {
        PTRACE_STORE TraceStore = &TraceStores->Stores[StoreIndex];
        PTRACE_STORE MetadataStore = TraceStore+1;
        LPCWSTR FileName = TraceStoreFileNames[Index];
        DWORD InitialSize = Sizes[Index];
        ULONG MappingSize = DefaultTraceStoreMappingSize;
        Result = StringCchCopyW(FileNameDest, RemainingChars.QuadPart, FileName);
        if (FAILED(Result)) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        Success = InitializeTraceStore(
            Rtl,
            Path,
            TraceStore,
            MetadataStore,
            InitialSize,
            MappingSize
        );
        if (!Success) {
            return FALSE;
        }
    }

    TraceStores->NumberOfTraceStores = NumberOfTraceStores;

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

DWORD
GetTraceStoresAllocationSize(const USHORT NumberOfTraceStores)
{
    //
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
    //
    return (
        sizeof(TRACE_STORES) + (
            (sizeof(TRACE_STORE) * NumberOfTraceStores * 2) -
            sizeof(TRACE_STORE)
        )
    );
}

VOID
PrefaultFutureTraceStorePage(_Inout_ PTRACE_STORE TraceStore)
{
    PTRACE_STORE_MEMORY_MAP PrefaultMemoryMap;

    if (!TraceStore) {
        return;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->PrefaultMemoryMaps, &PrefaultMemoryMap)) {
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

BOOL
PrepareNextTraceStoreMemoryMap(_Inout_ PTRACE_STORE TraceStore)
{
    BOOL Result;
    BOOL Success;
    FILE_STANDARD_INFO FileInfo;
    LARGE_INTEGER CurrentFileOffset, NewFileOffset, DistanceToMove;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    if (!TraceStore) {
        return FALSE;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    if (!MemoryMap->FileHandle) {
        goto error;
    }

    if (!GetTraceStoreMemoryMapFileInfo(MemoryMap, &FileInfo)) {
        goto error;
    }

    //
    // Get the current file offset.
    //
    DistanceToMove.QuadPart = 0;
    if (!SetFilePointerEx(TraceStore->FileHandle, DistanceToMove, &CurrentFileOffset, FILE_CURRENT)) {
        goto error;
    }

    if (CurrentFileOffset.QuadPart != MemoryMap->FileOffset.QuadPart) {
        //
        // XXX: what should we do here?  Can this happen?
        //
        __debugbreak();
    }

    //
    // Determine the distance we need to move the file pointer.
    //
    DistanceToMove.QuadPart = MemoryMap->FileOffset.QuadPart + MemoryMap->MappingSize.QuadPart;

    //
    // Adjust the file pointer from the current position.
    //
    Success = SetFilePointerEx(MemoryMap->FileHandle,
                               DistanceToMove,
                               &NewFileOffset,
                               FILE_BEGIN);

    if (!Success) {
        goto error;
    }

    //
    // If the new file offset is past the end of the file, extend it.
    //
    if (FileInfo.EndOfFile.QuadPart < NewFileOffset.QuadPart) {

        if (!SetEndOfFile(MemoryMap->FileHandle)) {
            goto error;
        }
    }

    MemoryMap->MappingHandle = CreateFileMapping(
        MemoryMap->FileHandle,
        NULL,
        PAGE_READWRITE,
        NewFileOffset.HighPart,
        NewFileOffset.LowPart,
        NULL
    );

    if (MemoryMap->MappingHandle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    MemoryMap->BaseAddress = MapViewOfFile(
        MemoryMap->MappingHandle,
        FILE_MAP_READ | FILE_MAP_WRITE,
        MemoryMap->FileOffset.HighPart,
        MemoryMap->FileOffset.LowPart,
        MemoryMap->MappingSize.LowPart
    );

    if (!MemoryMap->BaseAddress) {
        DWORD LastError = GetLastError();
        goto error;
    }

    MemoryMap->NextAddress = MemoryMap->BaseAddress;

    //
    // Prefault the first two pages.  The AllocateRecords function will
    // take care of prefaulting subsequent pages.
    //
    PrefaultPage(MemoryMap->BaseAddress);
    PrefaultNextPage(MemoryMap->BaseAddress);

    Result = TRUE;
    PushTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, MemoryMap);
    SetEvent(TraceStore->NextMemoryMapAvailableEvent);

    goto end;

error:
    Result = FALSE;

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }

    ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);

end:
    return Result;
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
        Success = FlushViewOfFile(MemoryMap->BaseAddress, MemoryMap->MappingSize.LowPart);
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
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    if (!PopTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    UnmapTraceStoreMemoryMap(MemoryMap);
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

BOOL
RecordTraceStoreAllocation(
    _Inout_ PTRACE_STORE     TraceStore,
    _In_    PULARGE_INTEGER  RecordSize,
    _In_    PULARGE_INTEGER  NumberOfRecords
)
{
    PTRACE_STORE_METADATA Metadata;

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

        if (!Metadata->RecordSize.QuadPart) {
            Metadata->RecordSize.QuadPart = RecordSize->QuadPart;
        }

        if (Metadata->RecordSize.QuadPart != RecordSize->QuadPart) {
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
            Metadata->RecordSize.QuadPart = MetadataRecordSize.QuadPart;
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
    PTRACE_STORE_MEMORY_MAP PrevPrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    PTRACE_STORE_MEMORY_MAP PrepareMemoryMap;

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

    //
    // Retire the previous previous memory map if it exists.
    //

    if (PrevPrevMemoryMap) {
        TraceStore->PrevMemoryMap = NULL;
        PushTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, PrevPrevMemoryMap);
        SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);
    }

    //
    // Pop a memory map descriptor off the free list to use for the PrepareMemoryMap.
    //

    if (!PopTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps, &PrepareMemoryMap)) {

        if (!(CreateMemoryMapsForTraceStore(TraceStore, TraceStore->TraceContext, 0) &&
              PopTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps, &PrepareMemoryMap))) {

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
    // We've now got the two things we need: a free memory map to fill in with the
    // details of the next memory map to prepare (PrepareMemoryMap), and the ready
    // memory map (NextMemoryMap) that contains an active mapping ready for use.
    //

    if (TraceStore->MemoryMap) {
        TraceStore->PrevAddress = MemoryMap->PrevAddress = TraceStore->MemoryMap->PrevAddress;
        TraceStore->PrevMemoryMap = TraceStore->MemoryMap;
    }

    TraceStore->MemoryMap = MemoryMap;

    //
    // Prepare the next memory map with the relevant offset details based
    // on the new memory map and submit it to the threadpool.
    //

    PrepareMemoryMap->FileHandle = MemoryMap->FileHandle;
    PrepareMemoryMap->MappingSize.QuadPart = MemoryMap->MappingSize.QuadPart;
    PrepareMemoryMap->FileOffset.QuadPart = MemoryMap->FileOffset.QuadPart + MemoryMap->MappingSize.QuadPart;

    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, PrepareMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    return TRUE;
}

_Check_return_
LPVOID
AllocateRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
)
{
    PTRACE_STORE_MEMORY_MAP MemoryMap;
    ULONG_PTR AllocationSize;
    PVOID ReturnAddress = NULL;
    PVOID NextAddress;
    PVOID EndAddress;
    PVOID PrevPage;
    PVOID NextPage;

    if (!TraceStore) {
        return NULL;
    }

    MemoryMap = TraceStore->MemoryMap;

    if (!MemoryMap) {
        return NULL;
    }

#ifdef _M_X64
    AllocationSize = (ULONG_PTR)(RecordSize->QuadPart * NumberOfRecords->QuadPart);
    if (AllocationSize > (ULONG_PTR)MemoryMap->MappingSize.QuadPart) {
        return NULL;
    }
#else
    {
        ULARGE_INTEGER Size = { 0 };
        // Ignore allocation attempts over 2GB on 32-bit.
        if (RecordSize->HighPart != 0 || NumberOfRecords->HighPart != 0) {
            return NULL;
        }
        Size->QuadPart = UInt32x32To64(RecordSize->LowPart, NumberOfRecords->LowPart);
        if (Size->HighPart != 0) {
            return NULL;
        }
        AllocationSize = (ULONG_PTR)Size->LowPart;
    }
    if (AllocationSize > MemoryMap->MappingSize.LowPart) {
        return NULL;
    }
#endif

    NextAddress = (PVOID)RtlOffsetToPointer(MemoryMap->NextAddress, AllocationSize);
    EndAddress = (PVOID)RtlOffsetToPointer(MemoryMap->BaseAddress, MemoryMap->MappingSize.LowPart);

    if (NextAddress > EndAddress) {

        if (!ConsumeNextTraceStoreMemoryMap(TraceStore)) {
            return NULL;
        }

        MemoryMap = TraceStore->MemoryMap;
        ReturnAddress = MemoryMap->BaseAddress;
        MemoryMap->NextAddress = (PVOID)RtlOffsetToPointer(MemoryMap->BaseAddress, AllocationSize);

    } else {

        //
        // If this allocation cross a page boundary, we prefault the page
        // after the next page in a separate thread (as long as it's still
        // within our allocated range).
        //
        PrevPage = (PVOID)ALIGN_DOWN(MemoryMap->NextAddress, PAGE_SIZE);
        NextPage = (PVOID)ALIGN_DOWN(NextAddress, PAGE_SIZE);

        if (PrevPage != NextPage) {

            PVOID PageAfterNextPage = (PVOID)((ULONG_PTR)NextPage + PAGE_SIZE);

            if (PageAfterNextPage < EndAddress) {

                PTRACE_STORE_MEMORY_MAP PrefaultMemoryMap;

                if (PopTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps, &PrefaultMemoryMap)) {

                    //
                    // Prefault the page after the next page after this
                    // address.  That is, prefault the page that is two
                    // pages away from whatever page NextAddress is in.
                    //
                    PrefaultMemoryMap->NextAddress = PageAfterNextPage;

                    PushTraceStoreMemoryMap(&TraceStore->PrefaultMemoryMaps, PrefaultMemoryMap);
                    SubmitThreadpoolWork(TraceStore->PrefaultFuturePageWork);
                }
            }
        }

        ReturnAddress = MemoryMap->PrevAddress = TraceStore->PrevAddress = MemoryMap->NextAddress;
        MemoryMap->NextAddress = NextAddress;

    }

    RecordTraceStoreAllocation(TraceStore, RecordSize, NumberOfRecords);
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
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;

    if (!TraceStore) {
        return FALSE;
    }

    if (!TraceContext) {
        return FALSE;
    }

    if (!TraceContext->HeapHandle) {
        return FALSE;
    }

    InitializeSListHead(&TraceStore->CloseMemoryMaps);
    InitializeSListHead(&TraceStore->PrepareMemoryMaps);
    InitializeSListHead(&TraceStore->FreeMemoryMaps);
    InitializeSListHead(&TraceStore->NextMemoryMaps);
    InitializeSListHead(&TraceStore->PrefaultMemoryMaps);

    if (!CreateMemoryMapsForTraceStore(TraceStore,
                                       TraceContext,
                                       InitialFreeMemoryMaps)) {

        return FALSE;
    }

    if (!PopTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps, &FirstMemoryMap)) {
        return FALSE;
    }

    TraceStore->NextMemoryMapAvailableEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
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

    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, FirstMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    if (WaitForSingleObject(TraceStore->NextMemoryMapAvailableEvent, INFINITE) == WAIT_OBJECT_0 &&
        ConsumeNextTraceStoreMemoryMap(TraceStore)) {

        if (!IsMetadataTraceStore(TraceStore)) {
            PTRACE_STORE MetadataStore = TraceStore->MetadataStore;
            TraceStore->pMetadata = (PTRACE_STORE_METADATA)MetadataStore->MemoryMap->BaseAddress;
        }

        return TRUE;

    }

    return FALSE;
}

BOOL
InitializeTraceContext(
    _In_                                    PRTL                    Rtl,
    _Inout_bytecap_(*SizeOfTraceContext)    PTRACE_CONTEXT          TraceContext,
    _In_                                    PULONG                  SizeOfTraceContext,
    _In_                                    PTRACE_SESSION          TraceSession,
    _In_                                    PTRACE_STORES           TraceStores,
    _In_                                    PTP_CALLBACK_ENVIRON    ThreadpoolCallbackEnvironment,
    _In_opt_                                PVOID                   UserData
)
{
    USHORT Index;

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

    QueryPerformanceFrequency(&TraceContext->PerformanceCounterFrequency);

    for (Index = 0; Index < TraceStores->NumberOfTraceStores * 2; Index += 2) {
        PTRACE_STORE TraceStore = &TraceStores->Stores[Index];
        PTRACE_STORE MetadataStore = &TraceStores->Stores[Index+1];

        //
        // Bind the metadata store first so that the trace store can update its pMetadata
        // pointer.
        //

        if (!BindTraceStoreToTraceContext(MetadataStore, TraceContext)) {
            return FALSE;
        }

        if (!BindTraceStoreToTraceContext(TraceStore, TraceContext)) {
            return FALSE;
        }

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

// vim: set ts=8 sw=4 sts=4 expandtab si ai:
