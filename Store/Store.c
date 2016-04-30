// Copyright Trent Nelson <trent@trent.me>

#include "Store.h"

INIT_ONCE InitOnceFindLongestStoreFileName = INIT_ONCE_STATIC_INIT;

INIT_ONCE InitOnceSystemTimerFunction = INIT_ONCE_STATIC_INIT;

static const LARGE_INTEGER MaximumMappingSize = { 1 << 31 }; // 2GB

static const SIZE_T InitialTraceContextHeapSize = (1 << 21); // 2MB
static const SIZE_T MaximumTraceContextHeapSize = (1 << 26); // 64MB

static const USHORT InitialFreeMemoryMaps = 32;

static const ULONG DefaultStoreMappingSize = (1 << 21); // 2MB
static const ULONG DefaultMetadataStoreSize = (1 << 21); // 2MB
static const ULONG DefaultMetadataStoreMappingSize = (1 << 16); // 64KB

#define MAX_UNICODE_STRING 255
#define _OUR_MAX_PATH MAX_UNICODE_STRING

FORCEINLINE
BOOL
IsMetadataStore(_In_ PSTORE Store)
{
    return (Store->MetadataStore == NULL);
}

BOOL
CALLBACK
FindLongestStoreFileNameCallback(
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
        StoreMetadataSuffixLength
    );

    for (Index = 0; Index < NumberOfStores; Index++) {
        LPCWSTR FileName = StoreFileNames[Index];
        if (FAILED(StringCchLengthW(FileName, MaxPath, (PSIZE_T)&Length.QuadPart))) {
            return FALSE;
        }
        if (Length.QuadPart > Longest.QuadPart) {
            Longest.QuadPart = Length.QuadPart;
        }
    }
    Longest.QuadPart += StoreMetadataSuffixLength;

    *((PDWORD64)lpContext) = Longest.QuadPart;

    return TRUE;
}

DWORD
GetLongestStoreFileName()
{
    BOOL  Status;
    ULARGE_INTEGER Longest;

    Status = InitOnceExecuteOnce(
        &InitOnceFindLongestStoreFileName,
        FindLongestStoreFileNameCallback,
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
ReturnFreeStoreMemoryMap(
    _Inout_ PSTORE Store,
    _Inout_ PSTORE_MEMORY_MAP MemoryMap
)
{
    SecureZeroMemory(MemoryMap, sizeof(*MemoryMap));
    InterlockedPushEntrySList(&Store->FreeMemoryMaps, &MemoryMap->ListEntry);
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
PopStoreMemoryMap(
    _Inout_ PSLIST_HEADER ListHead,
    _Inout_ PPSTORE_MEMORY_MAP MemoryMap
)
{
    PSLIST_ENTRY ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *MemoryMap = CONTAINING_RECORD(ListEntry, STORE_MEMORY_MAP, ListEntry);
    return TRUE;
}

FORCEINLINE
VOID
PushStoreMemoryMap(
    _Inout_ PSLIST_HEADER ListHead,
    _Inout_ PSTORE_MEMORY_MAP MemoryMap
)
{
    InterlockedPushEntrySList(ListHead, &MemoryMap->ListEntry);
}

FORCEINLINE
BOOL
GetStoreMemoryMapFileInfo(
    _In_    PSTORE_MEMORY_MAP MemoryMap,
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
CreateStore(
    _In_        PRTL                    Rtl,
    _In_        PUNICODE_STRING         PathPointer,
    _Inout_opt_ PPSTORE                 StorePointer,
    _In_        PTP_CALLBACK_ENVIRON    ThreadpoolCallbackEnvironment,
    _In_opt_    PULONG                  InitialFileSize,
    _In_opt_    PULONG                  MappingSize,
    _In_opt_    HANDLE                  HeapHandle,
    _In_opt_    PVOID                   UserData
    )
{
    HANDLE Heap;
    PSTORE Store;
    PUNICODE_STRING Path;

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathPointer)) {
        return FALSE;
    }

    //
    // Absolute minimum path length of a fully qualified path is 4:
    // e.g. "C:\a".  (Shift by 1 to get character count.)
    //

    if ((PathPointer->Length >> 1) < 4) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StorePointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ThreadpoolCallbackEnvironment)) {
        return FALSE;
    }

    if (ARGUMENT_PRESENT(HeapHandle)) {
        Heap = HeapHandle;
    } else {
        Heap = GetProcessHeap();
        if (!Heap) {
            return FALSE;
        }
    }

    Store = (PSTORE)HeapAlloc(Heap, HEAP_ZERO_MEMORY, sizeof(*Store));
    if (!Store) {
        return FALSE;
    }

    Path = &Store->Path;
    Path.Length = PathPointer->Length;
    Path.MaximumLength = PathPointer->MaximumLength;

    Path.Buffer = (PCHAR)HeapAlloc(Heap, 0, Path.Lenght);
    if (!Path.Buffer) {
        goto Error;

    }



    InitializeSListHead(&Store->CloseMemoryMaps);
    InitializeSListHead(&Store->PrepareMemoryMaps);
    InitializeSListHead(&Store->FreeMemoryMaps);
    InitializeSListHead(&Store->NextMemoryMaps);
    InitializeSListHead(&Store->PrefaultMemoryMaps);


    if (!Store->FileHandle) {
        // We're a metadata store.
        Store->FileHandle = CreateFileW(
            Path,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_FLAG_OVERLAPPED,
            NULL
        );
    }

    if (Store->FileHandle == INVALID_HANDLE_VALUE) {
        goto error;
    }

    Store->InitialSize.HighPart = 0;
    Store->InitialSize.LowPart = InitialSize;
    Store->ExtensionSize.HighPart = 0;
    Store->ExtensionSize.LowPart = InitialSize;

    if (!MappingSize) {
        MappingSize = DefaultStoreMappingSize;
    }

    Store->MappingSize.HighPart = 0;
    Store->MappingSize.LowPart = MappingSize;

    //
    // Cap the mapping size to the maximum if necessary.
    //

    if (Store->MappingSize.QuadPart > MaximumMappingSize.QuadPart) {
        Store->MappingSize.QuadPart = MaximumMappingSize.QuadPart;
    }

    Store->AllocateRecords = AllocateRecords;
    Store->FreeRecords = FreeRecords;

    if (!CreateMemoryMapsForStore(Store, InitialFreeMemoryMaps)) {
        return FALSE;
    }

    if (!PopStoreMemoryMap(&Store->FreeMemoryMaps, &FirstMemoryMap)) {
        return FALSE;
    }

    Store->NextMemoryMapAvailableEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!Store->NextMemoryMapAvailableEvent) {
        return FALSE;
    }

    Store->PrepareNextMemoryMapWork = CreateThreadpoolWork(
        &PrepareNextStoreMemoryMapCallback,
        Store,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!Store->PrepareNextMemoryMapWork) {
        return FALSE;
    }

    Store->PrefaultFuturePageWork = CreateThreadpoolWork(
        &PrefaultFuturePageCallback,
        Store,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!Store->PrefaultFuturePageWork) {
        return FALSE;
    }

    Store->CloseMemoryMapWork = CreateThreadpoolWork(
        &ReleasePrevMemoryMapCallback,
        Store,
        TraceContext->ThreadpoolCallbackEnvironment
    );

    if (!Store->CloseMemoryMapWork) {
        return FALSE;
    }

    Store->TraceContext = TraceContext;

    FirstMemoryMap->FileHandle = Store->FileHandle;
    FirstMemoryMap->MappingSize.QuadPart = Store->MappingSize.QuadPart;

    PushStoreMemoryMap(&Store->PrepareMemoryMaps, FirstMemoryMap);
    SubmitThreadpoolWork(Store->PrepareNextMemoryMapWork);

    if (WaitForSingleObject(Store->NextMemoryMapAvailableEvent, INFINITE) == WAIT_OBJECT_0 &&
        ConsumeNextStoreMemoryMap(Store)) {

        if (!IsMetadataStore(Store)) {
            PSTORE MetadataStore = Store->MetadataStore;
            Store->pMetadata = (PSTORE_METADATA)MetadataStore->MemoryMap->BaseAddress;
        }

        return TRUE;

    }

Error:
    if (Path && Path->Buffer) {
        HeapFree(Heap, 0, Path->Buffer);
    }

    if (Store) {
        HeapFree(Heap, 0, Store);
    }

    return FALSE;
}

BOOL
InitializeStore(
    _In_        PRTL Rtl,
    _In_        PCWSTR Path,
    _Inout_     PSTORE Store,
    _Inout_     PSTORE MetadataStore,
    _In_opt_    ULONG InitialSize,
    _In_opt_    ULONG MappingSize
)
{
    BOOL Success;
    HRESULT Result;
    WCHAR MetadataPath[_OUR_MAX_PATH];

    if (!Path || !Store || !Rtl) {
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
        StoreMetadataSuffix
    );
    if (FAILED(Result)) {
        return FALSE;
    }

    Store->Rtl = Rtl;

    // Create the data file first, before the :metadata stream.
    Store->FileHandle = CreateFileW(
        Path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (Store->FileHandle == INVALID_HANDLE_VALUE) {
        DWORD LastError = GetLastError();
        goto error;
    }

    Success = InitializeStore(
        &MetadataPath[0],
        MetadataStore,
        DefaultMetadataStoreSize,
        DefaultMetadataStoreMappingSize
    );

    if (!Success) {
        return FALSE;
    }

    MetadataStore->NumberOfRecords.QuadPart = 1;
    MetadataStore->RecordSize.QuadPart = sizeof(STORE_METADATA);

    Store->MetadataStore = MetadataStore;

    if (!InitializeStore(Path, Store, InitialSize, MappingSize)) {
        goto error;
    }

    return TRUE;
error:
    CloseStore(Store);
    return FALSE;
}

PSTORES
GetMetadataStoresFromTracesStores(
    _In_    PSTORES   Stores
)
{
    return (PSTORES)(
        (DWORD_PTR)&Stores->Stores[0] +
        (sizeof(STORE) * Stores->NumberOfStores)
    );
}

BOOL
InitializeStores(
    _In_        PRTL            Rtl,
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PSTORES   Stores,
    _Inout_     PULONG          SizeOfStores,
    _In_opt_    PULONG          InitialFileSizes
)
{
    BOOL Success;
    HRESULT Result;
    DWORD Index, StoreIndex;
    DWORD LastError;
    WCHAR Path[_OUR_MAX_PATH];
    LPWSTR FileNameDest;
    DWORD LongestFilename = GetLongestStoreFileName();
    DWORD StoresAllocationSize = GetStoresAllocationSize(NumberOfStores);
    DWORD LongestPossibleDirectoryLength = (
        _OUR_MAX_PATH   -
        1               - // '\'
        1               - // final NUL
        LongestFilename
    );
    LARGE_INTEGER DirectoryLength;
    LARGE_INTEGER RemainingChars;
    PULONG Sizes = InitialFileSizes;

    if (!SizeOfStores) {
        return FALSE;
    }

    if (!Stores || *SizeOfStores < StoresAllocationSize) {
        *SizeOfStores = StoresAllocationSize;
        return FALSE;
    }

    if (!Rtl) {
        return FALSE;
    }

    if (!BaseDirectory) {
        return FALSE;
    }

    if (!Sizes) {
        Sizes = (PULONG)&InitialStoreFileSizes[0];
    }

    Result = StringCchLengthW(
        BaseDirectory,
        LongestPossibleDirectoryLength,
        (PSIZE_T)&DirectoryLength.QuadPart
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

    SecureZeroMemory(Stores, sizeof(Stores));

    Success = CreateDirectory(BaseDirectory, NULL);
    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            SetLastError(LastError);
            return FALSE;
        }
    }

    Stores->Rtl = Rtl;

    for (Index = 0, StoreIndex = 0; Index < NumberOfStores; Index++, StoreIndex += 2) {
        PSTORE Store = &Stores->Stores[StoreIndex];
        PSTORE MetadataStore = Store+1;
        LPCWSTR FileName = StoreFileNames[Index];
        DWORD InitialSize = Sizes[Index];
        ULONG MappingSize = DefaultStoreMappingSize;
        Result = StringCchCopyW(FileNameDest, (SIZE_T)RemainingChars.QuadPart, FileName);
        if (FAILED(Result)) {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }
        Success = InitializeStore(
            Rtl,
            Path,
            Store,
            MetadataStore,
            InitialSize,
            MappingSize
        );
        if (!Success) {
            return FALSE;
        }
    }

    Stores->NumberOfStores = NumberOfStores;

    return TRUE;
}

VOID
CloseMemoryMap(_In_ PSTORE_MEMORY_MAP MemoryMap)
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
    _In_ PSTORE Store,
    _Inout_ PPSTORE_MEMORY_MAP MemoryMap
)
{
    if (!Store) {
        return;
    }

    if (!MemoryMap || !*MemoryMap) {
        return;
    }

    PushStoreMemoryMap(&Store->CloseMemoryMaps, *MemoryMap);
    SubmitThreadpoolWork(Store->CloseMemoryMapWork);
    *MemoryMap = NULL;
}

VOID
CloseStore(
    _In_ PSTORE Store
)
{
    PSTORE_MEMORY_MAP MemoryMap;

    if (!Store) {
        return;
    }

    SubmitCloseMemoryMapThreadpoolWork(Store, &Store->PrevMemoryMap);
    SubmitCloseMemoryMapThreadpoolWork(Store, &Store->MemoryMap);

    while (PopStoreMemoryMap(&Store->NextMemoryMaps, &MemoryMap)) {
        SubmitCloseMemoryMapThreadpoolWork(Store, &MemoryMap);
    }

    if (Store->FileHandle) {
        FlushFileBuffers(Store->FileHandle);
        CloseHandle(Store->FileHandle);
        Store->FileHandle = NULL;
    }

    if (Store->NextMemoryMapAvailableEvent) {
        CloseHandle(Store->NextMemoryMapAvailableEvent);
        Store->NextMemoryMapAvailableEvent = NULL;
    }

    if (Store->CloseMemoryMapWork) {
        CloseThreadpoolWork(Store->CloseMemoryMapWork);
        Store->CloseMemoryMapWork = NULL;
    }

    if (Store->PrepareNextMemoryMapWork) {
        CloseThreadpoolWork(Store->PrepareNextMemoryMapWork);
        Store->PrepareNextMemoryMapWork = NULL;
    }

    if (Store->PrefaultFuturePageWork) {
        CloseThreadpoolWork(Store->PrefaultFuturePageWork);
        Store->PrefaultFuturePageWork = NULL;
    }

}

VOID
CloseStore(
    _In_ PSTORE Store
)
{
    if (!Store) {
        return;
    }

    if (Store->MetadataStore) {
        CloseStore(Store->MetadataStore);
        Store->MetadataStore = NULL;
    }

    CloseStore(Store);
}

void
CloseStores(PSTORES Stores)
{
    USHORT Index;

    if (!Stores) {
        return;
    }

    for (Index = 0; Index < NumberOfStores * 2; Index += 2) {
        CloseStore(&Stores->Stores[Index]);
    }
}

DWORD
GetStoresAllocationSize(const USHORT NumberOfStores)
{
    //
    // For each trace store, we also store a metadata trace store, hence the
    // "NumberOfStores * 2" part.  The subsequent "minus size-of-1-trace-store"
    // accounts for the fact that the STORES structure includes a single trace
    // store structure, e.g.:
    //      typedef struct _STORES {
    //          ...
    //          STORE Stores[1];
    //
    // So if NumberOfStores is 6, there will actually be space for
    // 12 STORE structures.
    //
    return (
        sizeof(STORES) + (
            (sizeof(STORE) * NumberOfStores * 2) -
            sizeof(STORE)
        )
    );
}

VOID
PrefaultFutureStorePage(_Inout_ PSTORE Store)
{
    PSTORE_MEMORY_MAP PrefaultMemoryMap;

    if (!Store) {
        return;
    }

    if (!PopStoreMemoryMap(&Store->PrefaultMemoryMaps, &PrefaultMemoryMap)) {
        return;
    }

    PrefaultPage(PrefaultMemoryMap->NextAddress);

    ReturnFreeStoreMemoryMap(Store, PrefaultMemoryMap);
}

VOID
CALLBACK
PrefaultFuturePageCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
)
{
    PrefaultFutureStorePage((PSTORE)Context);
}

BOOL
PrepareNextStoreMemoryMap(_Inout_ PSTORE Store)
{
    BOOL Result;
    BOOL Success;
    FILE_STANDARD_INFO FileInfo;
    LARGE_INTEGER CurrentFileOffset, NewFileOffset, DistanceToMove;
    PSTORE_MEMORY_MAP MemoryMap;

    if (!Store) {
        return FALSE;
    }

    if (!PopStoreMemoryMap(&Store->PrepareMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    if (!MemoryMap->FileHandle) {
        goto error;
    }

    if (!GetStoreMemoryMapFileInfo(MemoryMap, &FileInfo)) {
        goto error;
    }

    //
    // Get the current file offset.
    //
    DistanceToMove.QuadPart = 0;
    if (!SetFilePointerEx(Store->FileHandle, DistanceToMove, &CurrentFileOffset, FILE_CURRENT)) {
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
    PushStoreMemoryMap(&Store->NextMemoryMaps, MemoryMap);
    SetEvent(Store->NextMemoryMapAvailableEvent);

    goto end;

error:
    Result = FALSE;

    if (MemoryMap->MappingHandle) {
        CloseHandle(MemoryMap->MappingHandle);
        MemoryMap->MappingHandle = NULL;
    }

    ReturnFreeStoreMemoryMap(Store, MemoryMap);

end:
    return Result;
}

VOID
CALLBACK
PrepareNextStoreMemoryMapCallback(
    _Inout_     PTP_CALLBACK_INSTANCE   Instance,
    _Inout_opt_ PVOID                   Context,
    _Inout_     PTP_WORK                Work
)
{
    PrepareNextStoreMemoryMap((PSTORE)Context);
}

BOOL
FlushStoreMemoryMap(_Inout_ PSTORE_MEMORY_MAP MemoryMap)
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
UnmapStoreMemoryMap(_Inout_ PSTORE_MEMORY_MAP MemoryMap)
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
ReleasePrevStoreMemoryMap(_Inout_ PSTORE Store)
{
    PSTORE_MEMORY_MAP MemoryMap;

    if (!PopStoreMemoryMap(&Store->CloseMemoryMaps, &MemoryMap)) {
        return FALSE;
    }

    UnmapStoreMemoryMap(MemoryMap);
    ReturnFreeStoreMemoryMap(Store, MemoryMap);
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
    ReleasePrevStoreMemoryMap((PSTORE)Context);
}

VOID
SubmitStoreFileExtensionThreadpoolWork(
    _Inout_     PSTORE    Store
)
{
    SubmitThreadpoolWork(Store->PrepareNextMemoryMapWork);
}

BOOL
CreateMemoryMapsForStore(
    _Inout_ PSTORE Store,
    _Inout_ PTRACE_CONTEXT TraceContext,
    _In_ ULONG NumberOfItems
)
{
    PSTORE_MEMORY_MAP MemoryMaps;
    SIZE_T AllocationSize;
    ULONG Index;

    if (!NumberOfItems) {
        NumberOfItems = InitialFreeMemoryMaps;
    }

    AllocationSize = sizeof(STORE_MEMORY_MAP) * NumberOfItems;

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
        PSTORE_MEMORY_MAP MemoryMap = &MemoryMaps[Index];

        //
        // Ensure the ListItem is aligned on the MEMORY_ALLOCATION_ALIGNMENT.
        // This is a requirement for the underlying SLIST_ENTRY.
        //
        if (((ULONG_PTR)MemoryMap & (MEMORY_ALLOCATION_ALIGNMENT - 1)) != 0) {
            __debugbreak();
        }

        PushStoreMemoryMap(&Store->FreeMemoryMaps, MemoryMap);
    }

    return TRUE;
}

BOOL
RecordStoreAllocation(
    _Inout_ PSTORE     Store,
    _In_    PULARGE_INTEGER  RecordSize,
    _In_    PULARGE_INTEGER  NumberOfRecords
)
{
    PSTORE_METADATA Metadata;

    if (IsMetadataStore(Store)) {
        Metadata = &Store->Metadata;

        //
        // Metadata trace stores should never have variable record sizes.
        //

        if (Metadata->RecordSize.QuadPart != RecordSize->QuadPart) {
            return FALSE;
        }

    } else {

        Metadata = Store->pMetadata;

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

            Address = Store->MetadataStore->AllocateRecords(
                Store->TraceContext,
                Store->MetadataStore,
                &MetadataRecordSize,
                &NumberOfMetadataRecords
            );

            if (!Address) {
                return FALSE;
            }

            Metadata = (PSTORE_METADATA)Address;
            Metadata->RecordSize.QuadPart = MetadataRecordSize.QuadPart;
            Metadata->NumberOfRecords.QuadPart = 0;

            Store->pMetadata = Metadata;
        }

    }

    //
    // Update the record count.
    //
    Metadata->NumberOfRecords.QuadPart += NumberOfRecords->QuadPart;
    return TRUE;
}

BOOL
ConsumeNextStoreMemoryMap(
    _Inout_ PSTORE Store
)
{
    PSTORE_MEMORY_MAP PrevPrevMemoryMap;
    PSTORE_MEMORY_MAP MemoryMap;
    PSTORE_MEMORY_MAP PrepareMemoryMap;

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

    PrevPrevMemoryMap = Store->PrevMemoryMap;

    //
    // Retire the previous previous memory map if it exists.
    //

    if (PrevPrevMemoryMap) {
        Store->PrevMemoryMap = NULL;
        PushStoreMemoryMap(&Store->CloseMemoryMaps, PrevPrevMemoryMap);
        SubmitThreadpoolWork(Store->CloseMemoryMapWork);
    }

    //
    // Pop a memory map descriptor off the free list to use for the PrepareMemoryMap.
    //

    if (!PopStoreMemoryMap(&Store->FreeMemoryMaps, &PrepareMemoryMap)) {

        if (!(CreateMemoryMapsForStore(Store, Store->TraceContext, 0) &&
              PopStoreMemoryMap(&Store->FreeMemoryMaps, &PrepareMemoryMap))) {

            ++Store->DroppedRecords;
            ++Store->ExhaustedFreeMemoryMaps;
            return FALSE;

        }
    }

    if (!PopStoreMemoryMap(&Store->NextMemoryMaps, &MemoryMap)) {

        //
        // Our allocations are outpacing the next memory map preparation
        // being done asynchronously in the threadpool, so drop the record.
        //

        ++Store->AllocationsOutpacingNextMemoryMapPreparation;
        ++Store->DroppedRecords;

        //
        // Return the PrepareMemoryMap back to the free list.
        //
        ReturnFreeStoreMemoryMap(Store, PrepareMemoryMap);
        return FALSE;
    }

    //
    // We've now got the two things we need: a free memory map to fill in with the
    // details of the next memory map to prepare (PrepareMemoryMap), and the ready
    // memory map (NextMemoryMap) that contains an active mapping ready for use.
    //

    if (Store->MemoryMap) {
        Store->PrevAddress = MemoryMap->PrevAddress = Store->MemoryMap->PrevAddress;
        Store->PrevMemoryMap = Store->MemoryMap;
    }

    Store->MemoryMap = MemoryMap;

    //
    // Prepare the next memory map with the relevant offset details based
    // on the new memory map and submit it to the threadpool.
    //

    PrepareMemoryMap->FileHandle = MemoryMap->FileHandle;
    PrepareMemoryMap->MappingSize.QuadPart = MemoryMap->MappingSize.QuadPart;
    PrepareMemoryMap->FileOffset.QuadPart = MemoryMap->FileOffset.QuadPart + MemoryMap->MappingSize.QuadPart;

    PushStoreMemoryMap(&Store->PrepareMemoryMaps, PrepareMemoryMap);
    SubmitThreadpoolWork(Store->PrepareNextMemoryMapWork);

    return TRUE;
}

_Check_return_
LPVOID
AllocateRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PSTORE    Store,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
)
{
    PSTORE_MEMORY_MAP MemoryMap;
    ULONG_PTR AllocationSize;
    PULONG SizePointer;
    PVOID ReturnAddress = NULL;
    PVOID NextAddress;
    PVOID EndAddress;
    PVOID PrevPage;
    PVOID NextPage;

    if (!Store) {
        return NULL;
    }

    MemoryMap = Store->MemoryMap;

    if (!MemoryMap) {
        return NULL;
    }

    //
    // Account for the size header.
    //
    AllocationSize = sizeof(ULONG);

#ifdef _M_X64

    AllocationSize += (
        (ULONG_PTR)(
            RecordSize->QuadPart *
            NumberOfRecords->QuadPart
        )
    );

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

    if (NextAddress > EndAddress) {

        if (!ConsumeNextStoreMemoryMap(Store)) {
            return NULL;
        }

        MemoryMap = Store->MemoryMap;
        ReturnAddress = MemoryMap->BaseAddress;

        MemoryMap->NextAddress = (
            (PVOID)RtlOffsetToPointer(
                MemoryMap->BaseAddress,
                AllocationSize
            )
        );

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

                PSTORE_MEMORY_MAP PrefaultMemoryMap;

                if (PopStoreMemoryMap(&Store->FreeMemoryMaps, &PrefaultMemoryMap)) {

                    //
                    // Prefault the page after the next page after this
                    // address.  That is, prefault the page that is two
                    // pages away from whatever page NextAddress is in.
                    //
                    PrefaultMemoryMap->NextAddress = PageAfterNextPage;

                    PushStoreMemoryMap(&Store->PrefaultMemoryMaps, PrefaultMemoryMap);
                    SubmitThreadpoolWork(Store->PrefaultFuturePageWork);
                }
            }
        }

        ReturnAddress = MemoryMap->PrevAddress = Store->PrevAddress = MemoryMap->NextAddress;
        MemoryMap->NextAddress = NextAddress;

    }

    RecordStoreAllocation(Store, RecordSize, NumberOfRecords);
    SizePointer = (PULONG)ReturnAddress;
    *SizePointer = (ULONG)AllocationSize - sizeof(ULONG);
    return RtlOffsetToPointer(SizePointer, sizeof(ULONG));
}

LPVOID
GetNextRecord(
    PTRACE_CONTEXT TraceContext,
    PSTORE Store,
    PULARGE_INTEGER RecordSize
)
{
    ULARGE_INTEGER RecordCount = { 1 };
    return AllocateRecords(TraceContext, Store, RecordSize, &RecordCount);
}

VOID
FreeRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PSTORE    Store,
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

#ifdef __cpp
} // extern "C"
#endif

// vim: set ts=8 sw=4 sts=4 expandtab si ai:
