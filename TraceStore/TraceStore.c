/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStore.c

Abstract:

    This module implements generic trace store functionality unrelated to the
    main memory map machinery.  Functions are provided to initialize, bind,
    close, truncate and run down a trace store.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeStore(
    PCWSTR       Path,
    PTRACE_STORE TraceStore,
    ULONG        InitialSize,
    ULONG        MappingSize
    )
/*++

Routine Description:

    This routine initializes a trace store at the given path.  It opens a handle
    for the path if the handle hasn't already been opened, and initializes the
    default values for the TRACE_STORE struct.

Arguments:

    Path - Supplies a pointer to a NULL-terminated wide string representing
        the file name to pass to CreateFileW().

    TraceStore - Supplies a pointer to the TRACE_STORE structure to be
        initialized by this routine.

    InitialSize - Supplies the initial size in bytes for the trace store.  If
        zero, the default initial size is used.

    MappingSize - Supplies the mapping size in bytes to be used for each trace
        store memory map.  If zero, the default mapping size is used.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    //
    // If the file handle hasn't been set, we're a metadata store.  So, open
    // the underlying path.
    //

    if (!TraceStore->FileHandle) {

        //
        // We're a metadata store.  (Break if we're not.)
        //

        if (!TraceStore->IsMetadata) {
            __debugbreak();
        }

        TraceStore->FileHandle = CreateFileW(
            Path,
            TraceStore->CreateFileDesiredAccess,
            FILE_SHARE_READ,
            NULL,
            TraceStore->CreateFileCreationDisposition,
            FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
            NULL
        );
    }

    //
    // Ensure we've got a valid handle.
    //

    if (!TraceStore->FileHandle ||
        TraceStore->FileHandle == INVALID_HANDLE_VALUE) {

        goto Error;
    }

    //
    // Initialize trait information.
    //

    if (!InitializeTraceStoreTraits(TraceStore)) {
        goto Error;
    }


    //
    // Initialize default values.
    //

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

    TraceStore->AllocateRecords = TraceStoreAllocateRecords;
    TraceStore->AllocateAlignedRecords = NULL;
    TraceStore->AllocateAlignedOffsetRecords = NULL;

    //
    // Return success.
    //

    return TRUE;

Error:

    //
    // Attempt to close the trace store if an error occurs.
    //

    CloseTraceStore(TraceStore);

    return FALSE;
}

/*++

VOID
INIT_METADATA_PATH(
    Name
    );

Routine Description:

    This is a helper macro for initializing a trace store's metadata's path
    name.  It copies the the trace store name and metadata suffix into the
    relevant string buffer, then calls InitializeTraceStorePath().

    This macro is used by InitializeTraceStore().

Arguments:

    Name - Name of the metadata store to initialized (e.g. 'Allocation').

Return Value:

    None.

--*/
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

/*++

VOID
INIT_METADATA(
    Name
    );

Routine Description:

    This is a helper macro for initializing a trace store's metadata store.
    It initializes various fields and then calls InitializeStore().

    This macro is used by InitializeTraceStore().

Arguments:

    Name - Name of the metadata store to initialized (e.g. 'Allocation').

Return Value:

    None.

--*/
#define INIT_METADATA(Name)                                              \
    Name##Store->TraceFlags = TraceStore->TraceFlags;                    \
    Name##Store->pTraits = (PTRACE_STORE_TRAITS)&##Name##StoreTraits;    \
    Name##Store->IsMetadata = TRUE;                                      \
    Name##Store->IsReadonly = TraceStore->IsReadonly;                    \
    Name##Store->NoPrefaulting = TraceStore->NoPrefaulting;              \
    Name##Store->SequenceId = TraceStore->SequenceId;                    \
    Name##Store->TraceStoreId = TraceStore->TraceStoreId;                \
    Name##Store->TraceStoreMetadataId = TraceStoreMetadata##Name##Id;    \
    Name##Store->TraceStore = TraceStore;                                \
    Name##Store->Allocator = Allocator;                                  \
    Name##Store->MetadataInfoStore = MetadataInfoStore;                  \
    Name##Store->AllocationStore = AllocationStore;                      \
    Name##Store->RelocationStore = RelocationStore;                      \
    Name##Store->AddressStore = AddressStore;                            \
    Name##Store->BitmapStore = BitmapStore;                              \
    Name##Store->InfoStore = InfoStore;                                  \
    Name##Store->BindComplete = (                                        \
        TraceStoreMetadataIdToBindComplete(TraceStoreMetadata##Name##Id) \
    );                                                                   \
    Name##Store->CreateFileDesiredAccess = (                             \
        TraceStore->CreateFileDesiredAccess                              \
    );                                                                   \
    Name##Store->CreateFileCreationDisposition = (                       \
        TraceStore->CreateFileCreationDisposition                        \
    );                                                                   \
    Name##Store->CreateFileMappingProtectionFlags = (                    \
        TraceStore->CreateFileMappingProtectionFlags                     \
    );                                                                   \
    Name##Store->CreateFileFlagsAndAttributes = (                        \
        TraceStore->CreateFileFlagsAndAttributes                         \
    );                                                                   \
    Name##Store->MapViewOfFileDesiredAccess = (                          \
        TraceStore->MapViewOfFileDesiredAccess                           \
    );                                                                   \
                                                                         \
    Success = InitializeStore(                                           \
        &##Name##Path[0],                                                \
        ##Name##Store,                                                   \
        Default##Name##TraceStoreSize,                                   \
        Default##Name##TraceStoreMappingSize                             \
    );                                                                   \
                                                                         \
    if (!Success) {                                                      \
        goto Error;                                                      \
    }                                                                    \
                                                                         \
    TraceStore->##Name##Store = ##Name##Store


_Use_decl_annotations_
BOOL
InitializeTraceStore(
    PRTL Rtl,
    PCWSTR Path,
    PTRACE_STORE TraceStore,
    PTRACE_STORE MetadataInfoStore,
    PTRACE_STORE AllocationStore,
    PTRACE_STORE RelocationStore,
    PTRACE_STORE AddressStore,
    PTRACE_STORE BitmapStore,
    PTRACE_STORE InfoStore,
    ULONG InitialSize,
    ULONG MappingSize,
    PTRACE_FLAGS TraceFlags,
    PTRACE_STORE_RELOC Reloc
    )
/*++

Routine Description:

    This routine initializes a trace store and its associated metadata stores.

Arguments:

    Rtl - Supplies a pointer to an RTL struct.

    Path - Supplies a pointer to a NULL-terminated wide character array of the
        fully-qualified trace store path.

    TraceStore - Supplies a pointer to a TRACE_STORE struct that will be
        initialized by this routine.

    MetadataInfoStore - Supplies a pointer to a TRACE_STORE struct that will be
        used as the metadata info metadata store for the given TraceStore being
        initialized.

    AllocationStore - Supplies a pointer to a TRACE_STORE struct that will be
        used as the allocation metadata store for the given TraceStore being
        initialized.

    RelocationStore - Supplies a pointer to a TRACE_STORE struct that will be
        used as the relocation metadata store for the given TraceStore being
        initialized.

    AddressStore - Supplies a pointer to a TRACE_STORE struct that will be
        used as the address metadata store for the given TraceStore being
        initialized.

    BitmapStore - Supplies a pointer to a TRACE_STORE struct that will be
        used as the free space bitmap metadata store for the given TraceStore
        being initialized.

            N.B.: Not yet implemented.

    InfoStore - Supplies a pointer to a TRACE_STORE struct that will be
        used as the info metadata store for the given TraceStore being
        initialized.

    InitialSize - Supplies the initial size in bytes for the trace store.  If
        zero, the default initial size is used.

    MappingSize - Supplies the mapping size in bytes to be used for each trace
        store memory map.  If zero, the default mapping size is used.

    TraceFlags - Supplies a pointer to a TRACE_FLAGS structure to be used when
        initializing the trace store.  This is used to control things like
        whether or not the trace stores are readonly or compressed.

    Reloc - Supplies a pointer to a TRACE_STORE_RELOC structure that contains
        field relocation information for this trace store.  If the store does
        not use relocations, Reloc->NumberOfRelocations should be set to 0.
        If present, this structure will be written to the RelocationStore when
        the store is bound to a context.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    TRACE_FLAGS Flags;
    PALLOCATOR Allocator;
    HRESULT Result;
    WCHAR MetadataInfoPath[_OUR_MAX_PATH];
    WCHAR AllocationPath[_OUR_MAX_PATH];
    WCHAR RelocationPath[_OUR_MAX_PATH];
    WCHAR AddressPath[_OUR_MAX_PATH];
    WCHAR BitmapPath[_OUR_MAX_PATH];
    WCHAR InfoPath[_OUR_MAX_PATH];
    PCWSTR MetadataInfoSuffix = TraceStoreMetadataInfoSuffix;
    PCWSTR AllocationSuffix = TraceStoreAllocationSuffix;
    PCWSTR RelocationSuffix = TraceStoreRelocationSuffix;
    PCWSTR AddressSuffix = TraceStoreAddressSuffix;
    PCWSTR BitmapSuffix = TraceStoreBitmapSuffix;
    PCWSTR InfoSuffix = TraceStoreInfoSuffix;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(MetadataInfoStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AllocationStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(RelocationStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AddressStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BitmapStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(InfoStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceFlags)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Reloc)) {
        return FALSE;
    }

    Allocator = TraceStore->Allocator;

    if (!Allocator) {
        return FALSE;
    }

    Flags = *TraceFlags;

    //
    // Initialize the paths of the metadata stores first.
    //

    INIT_METADATA_PATH(MetadataInfo);
    INIT_METADATA_PATH(Allocation);
    INIT_METADATA_PATH(Relocation);
    INIT_METADATA_PATH(Address);
    INIT_METADATA_PATH(Bitmap);
    INIT_METADATA_PATH(Info);

    TraceStore->Rtl = Rtl;

    //
    // Initialize the TraceStore's path.
    //

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
        TraceStore->CreateFileCreationDisposition,
        TraceStore->CreateFileFlagsAndAttributes,
        NULL
    );

    if (!TraceStore->FileHandle ||
        TraceStore->FileHandle == INVALID_HANDLE_VALUE) {

        //
        // We weren't able to open the file handle successfully.  In lieu of
        // better error logging, a breakpoint can be set here in a debug build
        // to see what the last error was.
        //

        DWORD LastError = GetLastError();
        goto Error;
    }

    //
    // Initialize the relevant flags.
    //

    if (Flags.Readonly) {
        TraceStore->IsReadonly = TRUE;
    }

    if (Flags.DisablePrefaultPages) {
        TraceStore->NoPrefaulting = TRUE;
    }

    if (Flags.NoTruncate) {
        TraceStore->NoTruncate = TRUE;
    }

    if (Reloc->NumberOfRelocations > 0) {
        TraceStore->HasRelocations = TRUE;
        TraceStore->pReloc = Reloc;
    }

    TraceStore->TraceFlags = Flags;

    //
    // Now that we've created the trace store file, create the NTFS streams
    // for the metadata trace store files.
    //

    INIT_METADATA(MetadataInfo);
    INIT_METADATA(Allocation);
    INIT_METADATA(Relocation);
    INIT_METADATA(Address);
    INIT_METADATA(Bitmap);
    INIT_METADATA(Info);

    TraceStore->IsMetadata = FALSE;

    //
    // Now initialize the TraceStore itself.
    //

    if (!InitializeStore(Path, TraceStore, InitialSize, MappingSize)) {
        goto Error;
    }

    //
    // Return success.
    //

    return TRUE;

Error:

    //
    // Attempt to close the trace store on any error.
    //

    CloseTraceStore(TraceStore);

    return FALSE;
}

_Use_decl_annotations_
VOID
InitializeTraceStoreSListHeaders(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine initializes the relevant SLIST_HEADER structures embedded
    within the trace store structure.  It is called by InitializeTraceContext()
    and InitializeReadonlyTraceContext() prior to creating events.

Arguments:

    TraceStore - Supplies a pointer to an initialized TRACE_STORE structure
        for which the SLIST_HEADER structures will be initialized.

Return Value:

    None.

--*/
{
    InitializeSListHead(&TraceStore->CloseMemoryMaps);
    InitializeSListHead(&TraceStore->PrepareMemoryMaps);
    InitializeSListHead(&TraceStore->FreeMemoryMaps);
    InitializeSListHead(&TraceStore->NextMemoryMaps);
    InitializeSListHead(&TraceStore->PrefaultMemoryMaps);
}

_Use_decl_annotations_
BOOL
CreateTraceStoreEvents(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine creates the necessary events for a given trace store.  It
    is called by InitializeTraceContext() and InitializeReadonlyTraceContext()
    prior to creating threadpool work.

Arguments:

    TraceStore - Supplies a pointer to an initialized TRACE_STORE structure
        for which events will be generated.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{

    if (IsSingleRecord(*TraceStore->pTraits)) {

        //
        // Single record trace stores are configured to use the trace store's
        // embedded memory map (TraceStore->SingleMemoryMap), so we don't need
        // to do any more work in this case.
        //

        return TRUE;
    }

    //
    // Create the next memory map available event.  This is signaled when
    // PrepareNextTraceStoreMemoryMap() has finished preparing the next
    // memory map for a trace store to consume.
    //

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

    //
    // Create the all memory maps are free event.  This event is signaled when
    // the count of active memory maps reaches zero.  The counter is decremented
    // atomically each time a memory map is pushed to the free list.  The event
    // is waited on during CloseStore().
    //

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

    return TRUE;

}

_Use_decl_annotations_
BOOL
CreateTraceStoreThreadpoolWorkItems(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine creates the necessary threadpool work items (e.g. TP_WORK)
    for a given trace store based on the trace store's traits.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.  The work
        items will be bound to the ThreadpoolCallbackEnvironment specified by
        this structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which
        threadpool work items will be created.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Readonly;
    BOOL IsMetadata;
    TRACE_STORE_TRAITS Traits;

    //
    // Ensure traits have been set.
    //

    if (!TraceStore->pTraits) {
        return FALSE;
    }

    Traits = *TraceStore->pTraits;
    Readonly = TraceStore->IsReadonly;
    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // The close memory map threadpool work item always gets created.
    //

    TraceStore->CloseMemoryMapWork = CreateThreadpoolWork(
        &CloseTraceStoreMemoryMapCallback,
        TraceStore,
        TraceContext->ThreadpoolCallbackEnvironment
    );
    if (!TraceStore->CloseMemoryMapWork) {
        return FALSE;
    }

    if (HasMultipleRecords(Traits)) {
        TraceStore->PrepareNextMemoryMapWork = CreateThreadpoolWork(
            &PrepareNextTraceStoreMemoryMapCallback,
            TraceStore,
            TraceContext->ThreadpoolCallbackEnvironment
        );
        if (!TraceStore->PrepareNextMemoryMapWork) {
            return FALSE;
        }
    }

    if (!Readonly && HasMultipleRecords(Traits)) {
        TraceStore->PrefaultFuturePageWork = CreateThreadpoolWork(
            &PrefaultFutureTraceStorePageCallback,
            TraceStore,
            TraceContext->ThreadpoolCallbackEnvironment
        );
        if (!TraceStore->PrefaultFuturePageWork) {
            return FALSE;
        }
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
TraceStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine is called by BindStore() once a normal (non-metadata) trace
    store has been successfully bound to a context.

    N.B.: This routine currently doesn't do anything.

Arguments:

    TraceContext - Supplies a pointer to the TRACE_CONTEXT structure to which
        the trace store was bound.

    TraceStore - Supplies a pointer to the bound TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to the first TRACE_STORE_MEMORY_MAP
        used by the trace store.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return TRUE;
}

_Use_decl_annotations_
BOOL
TruncateStore(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine truncates a trace store.  When a trace store is initialized,
    a default initial file size is used.  When the store is extended, it is
    done in fixed sized blocks (that match the mapping size).

    When a trace store is closed, this TruncateStore() routine is called.  This
    sets the file's end-of-file pointer back to the exact byte position that
    was actually used during the trace session.  Without this, life would be
    complicated for downstream readers as they'd have to be able to determine
    where a trace store's contents actually ends versus where the file ends.

Arguments:

    TraceStore - Supplies a pointer to an initialized TRACE_STORE struct that
        will be truncated.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER TotalAllocationSize;
    FILE_STANDARD_INFO FileInfo;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    //
    // If the NoTruncate flag has been set on this store, exit.
    //

    if (TraceStore->NoTruncate) {
        return TRUE;
    }

    EndOfFile.QuadPart = TraceStore->Eof->EndOfFile.QuadPart;
    TotalAllocationSize.QuadPart = TraceStore->Totals->AllocationSize.QuadPart;

    if (EndOfFile.QuadPart != TotalAllocationSize.QuadPart) {

        //
        // This should never happen.
        //

        __debugbreak();
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

    //
    // Compare the current end of file with what we want to set it to.
    //

    if (FileInfo.EndOfFile.QuadPart == EndOfFile.QuadPart) {

        //
        // Both values already match (unlikely) -- there's nothing more to do.
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

_Use_decl_annotations_
VOID
CloseStore(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine closes a trace store.  It can be called against both normal
    trace stores and metadata trace stores.  It will close previous and next
    memory maps if applicable, and cancel any in-flight "prepare memory map"
    requests and wait for all memory maps associated with the trace store to
    be freed (indicating that no more threads are servicing trace store memory
    map requests).

    It will then truncate the trace store, flush file buffers and close the
    underlying trace store's handle.

    It then closes any outstanding threadpool work associated with the thread
    pool.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE struct to close.

Return Value:

    None.

--*/
{
    BOOL IsRundown;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    //
    // Divert to the rundown function if a rundown is active.
    //

    IsRundown = IsGlobalTraceStoresRundownActive();
    if (IsRundown) {
        RundownStore(TraceStore);
        return;
    }

    //
    // Close the previous and current memory maps in the threadpool.
    //

    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->PrevMemoryMap);
    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->MemoryMap);

    //
    // Pop any prepared memory maps off the next memory map list and submit
    // threadpool closes for them, too.
    //

    while (PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {
        SubmitCloseMemoryMapThreadpoolWork(TraceStore, &MemoryMap);
    }

    //
    // When all threadpool close work has completed, the 'all memory maps are
    // free' event will be set, so we wait on this here.
    //

    WaitForSingleObject(TraceStore->AllMemoryMapsAreFreeEvent, INFINITE);

    //
    // All threadpool work has completed, we can proceed with closing the file
    // and associated events.
    //

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

_Use_decl_annotations_
VOID
RundownStore(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    Runs down a trace store.  This will synchronously close any open memory
    maps, truncate the store and close the file handle.  It is intended to be
    called during rundown, typically triggered by abnormal exit of a process
    that has active trace stores that have been registered with the global
    rundown list.

    The logic is similar to CloseTraceStore(), except no threadpool operations
    are dispatched, as these can't be called during DLL_PROCESS_DETACH.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE struct to close.

Return Value:

    None.

--*/
{
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    //
    // Rundown the previous and current memory maps synchronously.
    //

    RundownTraceStoreMemoryMap(TraceStore->PrevMemoryMap);
    RundownTraceStoreMemoryMap(TraceStore->MemoryMap);

    //
    // Pop any prepared memory maps off the next memory map list and run those
    // down synchronously too.
    //

    while (PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {
        RundownTraceStoreMemoryMap(MemoryMap);
    }

    //
    // Truncate the store, flush file buffers and close the handle.
    //

    if (TraceStore->FileHandle) {
        TruncateStore(TraceStore);
        FlushFileBuffers(TraceStore->FileHandle);
        CloseHandle(TraceStore->FileHandle);
        TraceStore->FileHandle = NULL;
    }
}

_Use_decl_annotations_
VOID
CloseTraceStore(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine closes a normal trace store.  It is not intended to be called
    against metadata trace stores.  It will close the trace store, then close
    the metadata trace stores associated with that trace store.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE struct to close.

Return Value:

    None.

--*/
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    //
    // Ensure we haven't been called with a metadata store.
    //

    if (TraceStore->IsMetadata) {
        __debugbreak();
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

    //
    // Close the metadata stores.
    //

    CLOSE_METADATA_STORES();

}

_Use_decl_annotations_
VOID
RundownTraceStore(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine runs down a normal trace store.  It is not intended to be
    called against metadata trace stores.  It will run down the trace store,
    then rundown the metadata trace stores associated with that trace store.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE struct to close.

Return Value:

    None.

--*/
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    //
    // Ensure we haven't been called with a metadata store.
    //

    if (TraceStore->IsMetadata) {
        __debugbreak();
        return;
    }

    //
    // Rundown the trace store first before the metadata stores.  This is
    // important because closing the trace store will retire any active memory
    // maps, which will update the underlying MemoryMap->pAddress structures,
    // which will be backed by the AddressStore, which we can only access as
    // long as we haven't closed it.
    //

    RundownStore(TraceStore);

    //
    // Rundown the metadata stores.
    //

    RUNDOWN_METADATA_STORES();

}

_Use_decl_annotations_
BOOL
UpdateTracerConfigWithTraceStoreInfo(
    PTRACER_CONFIG TracerConfig
    )
/*++

Routine Description:

    This routine sets the number of trace store elements, maximum trace store
    ID, and maximum trace store index values of the tracer configuration
    structure.

Arguments:

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure to be
        updated.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    TracerConfig->NumberOfElementsPerTraceStore = ElementsPerTraceStore;
    TracerConfig->MaximumTraceStoreId = TraceStoreInvalidId;
    TracerConfig->MaximumTraceStoreIndex = TraceStoreInvalidIndex;
    TracerConfig->SizeOfTraceStoreStructure = sizeof(TRACE_STORE);

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
