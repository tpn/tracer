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
    PCWSTR        Path,
    PTRACE_STORE  TraceStore,
    LARGE_INTEGER InitialSize,
    LARGE_INTEGER MappingSize,
    PCTRACE_STORE_TRAITS Traits
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

    Traits - Supplies a pointer to the traits to use for this trace store.
        This is mandatory only for write sessions; for readonly sessions, the
        value is ignored.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    ULONG LastError;

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
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
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
        TraceStore->LastError = GetLastError();
        goto Error;
    }

    //
    // Save initial trait information if applicable.
    //

    if (!TraceStore->IsReadonly && Traits) {

        TraceStore->InitialTraits = *Traits;
        if (!InitializeTraceStoreTraits(TraceStore)) {
            goto Error;
        }

        //
        // If the compress trait has been set, attempt to enable compression
        // on the file handle.
        //

        if (WantsCompression(*Traits)) {
            HANDLE Handle;
            USHORT CompressionFormat = COMPRESSION_FORMAT_DEFAULT;
            DWORD BytesReturned = 0;

            //
            // Obtain a handle with FILE_FLAG_BACKUP_SEMANTICS, which is
            // required in order for us to call the FSCTL_SET_COMPRESSION
            // IoControl below.
            //

            Handle = CreateFileW(Path,
                                 TraceStore->CreateFileDesiredAccess,
                                 FILE_SHARE_READ  |
                                 FILE_SHARE_WRITE |
                                 FILE_SHARE_DELETE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_BACKUP_SEMANTICS,
                                 NULL);

            if (!Handle || Handle == INVALID_HANDLE_VALUE) {
                LastError = GetLastError();
                return FALSE;
            }

            //
            // Request compression on the file handle.
            //

            Success = DeviceIoControl(
                Handle,                         // hDevice
                FSCTL_SET_COMPRESSION,          // dwIoControlCode
                &CompressionFormat,             // lpInBuffer
                sizeof(CompressionFormat),      // nInBufferSize
                NULL,                           // lpOutBuffer
                0,                              // nOutBufferSize
                &BytesReturned,                 // lpBytesReturned
                NULL                            // lpOverlapped
            );

            if (!Success) {

                //
                // We can't do anything useful here.
                //

                OutputDebugStringA("Failed to enable compression.\n");
            }

            CloseHandle(Handle);
        }
    }

    //
    // Initialize default values.
    //

    TraceStore->InitialSize.QuadPart = InitialSize.QuadPart;
    TraceStore->ExtensionSize.QuadPart = InitialSize.QuadPart;

    if (!MappingSize.QuadPart) {
        MappingSize.QuadPart = DefaultTraceStoreMappingSize.QuadPart;
    }

    TraceStore->MappingSize.QuadPart = MappingSize.QuadPart;

    //
    // Initialize allocators now for metadata.  They don't participate in the
    // suspended allocation machinery as we control them entirely.  Normal
    // trace stores have their allocators initialized when bound to a trace
    // context.
    //

    if (TraceStore->IsMetadata) {
        TraceStore->AllocateRecords = TraceStoreAllocateRecords;
        TraceStore->AllocateRecordsWithTimestamp = (
            TraceStoreAllocateRecordsWithTimestamp
        );
    }

    if (!InitializeTraceStoreAllocator(TraceStore)) {
        goto Error;
    }

    //
    // Return success.
    //

    return TRUE;

Error:

    //
    // Attempt to close the trace store if an error occurs.
    //

    CloseStore(TraceStore);

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

#define METADATA_STORE_INDEX_OFFSET_FROM_TRACE_STORE(Name) ( \
    ((                                                       \
        FIELD_OFFSET(TRACE_STORE, Name##Store) -             \
        FIELD_OFFSET(TRACE_STORE, MetadataInfoStore)         \
    ) / sizeof(ULONG_PTR)) + 1                               \
)

/*++

VOID
INIT_METADATA(
    Name
    );

Routine Description:

    This is a helper macro for initializing a trace store's metadata store.
    It initializes various fields and then calls InitializeStore().

    This macro is used by InitializeTraceStore().

    N.B. When this macro was first written, there were only a couple of metadata
         stores per trace store.  As the number has grown, keeping this as a
         macro makes less and less sense given the amount of instruction bloat
         it adds to the caller.  It should be refactored into a normal function.

Arguments:

    Name - Name of the metadata store to initialized (e.g. 'Allocation').

Return Value:

    None.

--*/
#define INIT_METADATA(Name)                                                    \
    Name##Store->Rtl = TraceStore->Rtl;                                        \
    Name##Store->TraceFlags = TraceStore->TraceFlags;                          \
    Name##Store->IsMetadata = TRUE;                                            \
    Name##Store->IsReadonly = TraceStore->IsReadonly;                          \
    Name##Store->NoPrefaulting = TraceStore->NoPrefaulting;                    \
    Name##Store->SequenceId = TraceStore->SequenceId;                          \
    Name##Store->TraceStoreId = TraceStore->TraceStoreId;                      \
    Name##Store->TraceStoreIndex = TraceStore->TraceStoreIndex + (             \
        METADATA_STORE_INDEX_OFFSET_FROM_TRACE_STORE(Name)                     \
    );                                                                         \
    Name##Store->TraceStoreMetadataId = TraceStoreMetadata##Name##Id;          \
    Name##Store->TraceStore = TraceStore;                                      \
    Name##Store->pAllocator = Allocator;                                       \
    Name##Store->MetadataInfoStore = MetadataInfoStore;                        \
    Name##Store->AllocationStore = AllocationStore;                            \
    Name##Store->RelocationStore = RelocationStore;                            \
    Name##Store->AddressStore = AddressStore;                                  \
    Name##Store->AddressRangeStore = AddressRangeStore;                        \
    Name##Store->AllocationTimestampStore = AllocationTimestampStore;          \
    Name##Store->AllocationTimestampDeltaStore =AllocationTimestampDeltaStore; \
    Name##Store->InfoStore = InfoStore;                                        \
    Name##Store->BindComplete = (                                              \
        TraceStoreMetadataIdToBindComplete(TraceStoreMetadata##Name##Id)       \
    );                                                                         \
    Name##Store->CreateFileDesiredAccess = (                                   \
        TraceStore->CreateFileDesiredAccess                                    \
    );                                                                         \
    Name##Store->CreateFileCreationDisposition = (                             \
        TraceStore->CreateFileCreationDisposition                              \
    );                                                                         \
    Name##Store->CreateFileMappingProtectionFlags = (                          \
        TraceStore->CreateFileMappingProtectionFlags                           \
    );                                                                         \
    Name##Store->CreateFileFlagsAndAttributes = (                              \
        TraceStore->CreateFileFlagsAndAttributes                               \
    );                                                                         \
    Name##Store->MapViewOfFileDesiredAccess = (                                \
        TraceStore->MapViewOfFileDesiredAccess                                 \
    );                                                                         \
                                                                               \
    InitializeListHead(&Name##Store->MetadataListEntry);                       \
    AppendTailList(&TraceStore->MetadataListHead,                              \
                   &Name##Store->MetadataListEntry);                           \
                                                                               \
    InitializeAllocatorFromTraceStore(Name##Store, &Name##Store->Allocator);   \
                                                                               \
    Success = InitializeStore(                                                 \
        &##Name##Path[0],                                                      \
        ##Name##Store,                                                         \
        Default##Name##TraceStoreSize,                                         \
        Default##Name##TraceStoreMappingSize,                                  \
        &##Name##StoreTraits                                                   \
    );                                                                         \
                                                                               \
    if (!Success) {                                                            \
        goto Error;                                                            \
    }                                                                          \
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
    PTRACE_STORE AddressRangeStore,
    PTRACE_STORE AllocationTimestampStore,
    PTRACE_STORE AllocationTimestampDeltaStore,
    PTRACE_STORE SynchronizationStore,
    PTRACE_STORE InfoStore,
    LARGE_INTEGER InitialSize,
    LARGE_INTEGER MappingSize,
    PTRACE_FLAGS TraceFlags,
    PTRACE_STORE_RELOC Reloc,
    PCTRACE_STORE_TRAITS Traits
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

    AddressRangeStore - Supplies a pointer to a TRACE_STORE struct that will
        be used as the address range metadata store for the given TraceStore
        being initialized.

    AllocationTimestampStore - Supplies a pointer to a TRACE_STORE struct that
        will be used to record the performance counter value each time an
        allocation is performed.

    AllocationTimestampDeltaStore - Supplies a pointer to a TRACE_STORE struct
        that will be used to record the delta between two allocation timestamps.

    SynchronizationStore - Supplies a pointer to a TRACE_STORE struct that will
        be used as the synchronization metadata trace store.

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

    Traits - Supplies a pointer to the traits to use for this trace store.
        This is mandatory only for write sessions; for readonly sessions, the
        value is ignored.

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
    WCHAR AddressRangePath[_OUR_MAX_PATH];
    WCHAR AllocationTimestampPath[_OUR_MAX_PATH];
    WCHAR AllocationTimestampDeltaPath[_OUR_MAX_PATH];
    WCHAR SynchronizationPath[_OUR_MAX_PATH];
    WCHAR InfoPath[_OUR_MAX_PATH];
    PCWSTR MetadataInfoSuffix = TraceStoreMetadataInfoSuffix;
    PCWSTR AllocationSuffix = TraceStoreAllocationSuffix;
    PCWSTR RelocationSuffix = TraceStoreRelocationSuffix;
    PCWSTR AddressSuffix = TraceStoreAddressSuffix;
    PCWSTR AddressRangeSuffix = TraceStoreAddressRangeSuffix;
    PCWSTR AllocationTimestampSuffix = TraceStoreAllocationTimestampSuffix;
    PCWSTR AllocationTimestampDeltaSuffix = (
        TraceStoreAllocationTimestampDeltaSuffix
    );
    PCWSTR SynchronizationSuffix = TraceStoreSynchronizationSuffix;
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

    if (!ARGUMENT_PRESENT(AddressRangeStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AllocationTimestampStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AllocationTimestampDeltaStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SynchronizationStore)) {
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

    if (!ARGUMENT_PRESENT(Traits)) {
        return FALSE;
    }

    //
    // Arguments are valid, proceed with initialization.
    //

    Allocator = TraceStore->pAllocator;

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
    INIT_METADATA_PATH(AddressRange);
    INIT_METADATA_PATH(AllocationTimestamp);
    INIT_METADATA_PATH(AllocationTimestampDelta);
    INIT_METADATA_PATH(Synchronization);
    INIT_METADATA_PATH(Info);

    TraceStore->Rtl = Rtl;

    //
    // Initialize the TraceStore's path.
    //

    if (!InitializeTraceStorePath(Path, TraceStore)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Create the data file first before the streams.
    //

    TraceStore->FileHandle = CreateFileW(
        Path,
        TraceStore->CreateFileDesiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        TraceStore->CreateFileCreationDisposition,
        TraceStore->CreateFileFlagsAndAttributes,
        NULL
    );

    if (!TraceStore->FileHandle ||
        TraceStore->FileHandle == INVALID_HANDLE_VALUE) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
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
    INIT_METADATA(AddressRange);
    INIT_METADATA(AllocationTimestamp);
    INIT_METADATA(AllocationTimestampDelta);
    INIT_METADATA(Synchronization);
    INIT_METADATA(Info);

    TraceStore->IsMetadata = FALSE;

    //
    // Now initialize the TraceStore itself.
    //

    if (!InitializeStore(Path, TraceStore, InitialSize, MappingSize, Traits)) {
        __debugbreak();
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
    InitializeSListHead(&TraceStore->NonRetiredMemoryMaps);
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

    //
    // We use the following two bools to make it clearer what event
    // type is being created in each CreateEvent() statement.  The
    // values map to the `BOOL bManualReset` parameter of said routine.
    //

    BOOL AutoReset = FALSE;
    BOOL ManualReset = TRUE;

    //
    // Create the all memory maps are free event.  This event is signaled when
    // the count of active memory maps reaches zero.  The counter is decremented
    // atomically each time a memory map is pushed to the free list.  The event
    // is waited on during CloseStore().
    //

    TraceStore->AllMemoryMapsAreFreeEvent = (
        CreateEvent(
            NULL,
            AutoReset,
            FALSE,
            NULL
        )
    );

    if (!TraceStore->AllMemoryMapsAreFreeEvent) {
        return FALSE;
    }

    if (IsSingleRecord(*TraceStore->pTraits)) {

        //
        // Single record trace stores are configured to use the trace store's
        // embedded memory map (TraceStore->SingleMemoryMap), so we don't need
        // to do any more work in this case.
        //

        return TRUE;
    }

    if (!TraceStore->IsReadonly) {

        //
        // Create the next memory map available event.  This is signaled when
        // PrepareNextTraceStoreMemoryMap() has finished preparing the next
        // memory map for a trace store to consume.
        //

        TraceStore->NextMemoryMapAvailableEvent = (
            CreateEvent(
                NULL,
                AutoReset,
                FALSE,
                NULL
            )
        );

        if (!TraceStore->NextMemoryMapAvailableEvent) {
            return FALSE;
        }

    } else {

        TraceStore->ReadonlyMappingCompleteEvent = (
            CreateEvent(
                NULL,
                AutoReset,
                FALSE,
                NULL
            )
        );

        if (!TraceStore->ReadonlyMappingCompleteEvent) {
            return FALSE;
        }
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
    BOOL IsReadonly;
    BOOL IsMetadata;
    TRACE_STORE_TRAITS Traits;
    PTP_CALLBACK_ENVIRON CallbackEnv;

    //
    // Ensure traits have been set.
    //

    if (!TraceStore->pTraits) {
        __debugbreak();
        return FALSE;
    }

    Traits = *TraceStore->pTraits;
    IsReadonly = TraceStore->IsReadonly;
    IsMetadata = IsMetadataTraceStore(TraceStore);
    CallbackEnv = TraceContext->ThreadpoolCallbackEnvironment;

    //
    // The close memory map threadpool work item always gets created.
    //

    TraceStore->CloseMemoryMapWork = CreateThreadpoolWork(
        &CloseTraceStoreMemoryMapCallback,
        TraceStore,
        CallbackEnv
    );

    if (!TraceStore->CloseMemoryMapWork) {
        return FALSE;
    }

    if (!IsReadonly) {

        if (HasMultipleRecords(Traits)) {

            TraceStore->PrepareNextMemoryMapWork = CreateThreadpoolWork(
                &PrepareNextTraceStoreMemoryMapCallback,
                TraceStore,
                CallbackEnv
            );

            if (!TraceStore->PrepareNextMemoryMapWork) {
                return FALSE;
            }

            TraceStore->PrefaultFuturePageWork = CreateThreadpoolWork(
                &PrefaultFutureTraceStorePageCallback,
                TraceStore,
                CallbackEnv
            );

            if (!TraceStore->PrefaultFuturePageWork) {
                return FALSE;
            }
        }

    } else {

        TraceStore->PrepareReadonlyNonStreamingMemoryMapWork = (
            CreateThreadpoolWork(
                &PrepareReadonlyTraceStoreMemoryMapCallback,
                TraceStore,
                CallbackEnv
            )
        );

        if (!TraceStore->PrepareReadonlyNonStreamingMemoryMapWork) {
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
    store has been successfully bound to a context.  Is is responsible for
    setting the bind complete event and reverting the suspended allocation
    methods into their working state.

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

    ResumeTraceStoreAllocations(TraceStore);
    SetEvent(TraceStore->BindCompleteEvent);

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
        //__debugbreak();
        return FALSE;
    }

    //
    // If the NoTruncate flag has been set on this store, exit.
    //

    if (TraceStore->IsMetadata && TraceStore->NoTruncate) {
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
        TraceStore->LastError = GetLastError();
        //__debugbreak();
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
        TraceStore->LastError = GetLastError();
        //__debugbreak();
        return FALSE;
    }

    //
    // And set the end of file.
    //

    Success = SetEndOfFile(TraceStore->FileHandle);

    if (!Success) {
        TraceStore->LastError = GetLastError();
        //__debugbreak();
    }

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
    DWORD WaitResult;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        __debugbreak();
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

    if (TraceStore->PrepareNextMemoryMapWork) {
        WaitForThreadpoolWorkCallbacks(
            TraceStore->PrepareNextMemoryMapWork,
            FALSE
        );
        CloseThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);
        TraceStore->PrepareNextMemoryMapWork = NULL;
    }

    if (TraceStore->PrefaultFuturePageWork) {
        WaitForThreadpoolWorkCallbacks(TraceStore->PrefaultFuturePageWork,
                                       FALSE);
        CloseThreadpoolWork(TraceStore->PrefaultFuturePageWork);
        TraceStore->PrefaultFuturePageWork = NULL;
    }

    if (!TraceStore->CloseMemoryMapWork) {
        __debugbreak();
    }

    //
    // Close the previous and current memory maps in the threadpool.
    //

    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->PrevMemoryMap);
    SubmitCloseMemoryMapThreadpoolWork(TraceStore, &TraceStore->MemoryMap);

    //
    // Pop any prepared memory maps off the next memory map lists and submit
    // threadpool closes for them.
    //

    while (PopNextMemoryMap(TraceStore, &MemoryMap)) {
        SubmitCloseMemoryMapThreadpoolWork(TraceStore, &MemoryMap);
    }

    while (PopNonRetiredMemoryMap(TraceStore, &MemoryMap)) {
        SubmitCloseMemoryMapThreadpoolWork(TraceStore, &MemoryMap);
    }

    WaitForThreadpoolWorkCallbacks(TraceStore->CloseMemoryMapWork, FALSE);
    CloseThreadpoolWork(TraceStore->CloseMemoryMapWork);
    TraceStore->CloseMemoryMapWork = NULL;

    //
    // When all threadpool close work has completed, the 'all memory maps are
    // free' event will be set, so we wait on this here.
    //

    WaitResult = WaitForSingleObject(TraceStore->AllMemoryMapsAreFreeEvent, 0);
    if (WaitResult != WAIT_OBJECT_0) {
        __debugbreak();
    }

    //
    // All threadpool work has completed, we can proceed with closing the file
    // and associated events.
    //

    if (TraceStore->FileHandle) {
        TruncateStore(TraceStore);
        if (!FlushFileBuffers(TraceStore->FileHandle)) {
            TraceStore->LastError = GetLastError();
            __debugbreak();
        } else {
            if (!CloseHandle(TraceStore->FileHandle)) {
                TraceStore->LastError = GetLastError();
                __debugbreak();
            } else {
                TraceStore->FileHandle = NULL;
            }
        }
    }

    if (TraceStore->NextMemoryMapAvailableEvent) {
        if (!CloseHandle(TraceStore->NextMemoryMapAvailableEvent)) {
            TraceStore->LastError = GetLastError();
            __debugbreak();
        } else {
            TraceStore->NextMemoryMapAvailableEvent = NULL;
        }
    }

    if (TraceStore->AllMemoryMapsAreFreeEvent) {
        if (!CloseHandle(TraceStore->AllMemoryMapsAreFreeEvent)) {
            TraceStore->LastError = GetLastError();
            __debugbreak();
        } else {
            TraceStore->AllMemoryMapsAreFreeEvent = NULL;
        }
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
        __debugbreak();
        return;
    }

    //
    // Rundown the previous and current memory maps synchronously.
    //

    RundownTraceStoreMemoryMap(TraceStore, TraceStore->PrevMemoryMap);
    RundownTraceStoreMemoryMap(TraceStore, TraceStore->MemoryMap);

    //
    // Pop any prepared memory maps off the next and non-retired memory map
    // lists and run those down synchronously too.
    //

    while (PopNextMemoryMap(TraceStore, &MemoryMap)) {
        RundownTraceStoreMemoryMap(TraceStore, MemoryMap);
    }

    while (PopNonRetiredMemoryMap(TraceStore, &MemoryMap)) {
        RundownTraceStoreMemoryMap(TraceStore, MemoryMap);
    }

    //
    // Truncate the store, flush file buffers and close the handle.
    //

    if (TraceStore->FileHandle) {
        TruncateStore(TraceStore);
        if (!FlushFileBuffers(TraceStore->FileHandle)) {
            TraceStore->LastError = GetLastError();
            //__debugbreak();
        } else if (!CloseHandle(TraceStore->FileHandle)) {
            TraceStore->LastError = GetLastError();
            //__debugbreak();
        } else {
            TraceStore->FileHandle = NULL;
        }
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
        __debugbreak();
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
        __debugbreak();
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
    // Skip the trace store if it's excluded.
    //

    if (TraceStore->Excluded) {
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
        __debugbreak();
        return FALSE;
    }

    TracerConfig->NumberOfElementsPerTraceStore = ElementsPerTraceStore;
    TracerConfig->MaximumTraceStoreId = TraceStoreInvalidId;
    TracerConfig->MaximumTraceStoreIndex = TraceStoreInvalidIndex;
    TracerConfig->SizeOfTraceStoreStructure = sizeof(TRACE_STORE);

    return TRUE;
}

_Use_decl_annotations_
VOID
TraceStoreDebugBreak(
    PTRACE_STORE TraceStore
    )
{
    __debugbreak();
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
