/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStore.c

Abstract:

    This module implements generic Trace Store functionality unrelated to the
    main memory map machinery.  Functions are provided for initializing and
    closing trace stores.

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
        // We're a metadata store.
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
    }

    //
    // Ensure we've got a valid handle.
    //

    if (!TraceStore->FileHandle ||
        TraceStore->FileHandle == INVALID_HANDLE_VALUE) {

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

    //
    // XXX TODO: initialize the aligned allocators.
    //

    TraceStore->NumberOfAllocations.QuadPart = 0;
    TraceStore->TotalAllocationSize.QuadPart = 0;

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
#define INIT_METADATA(Name)                                            \
    Name##Store->Flags = TraceStore->Flags;                            \
    Name##Store->IsMetadata = TRUE;                                    \
    Name##Store->IsReadonly = TraceStore->IsReadonly;                  \
    Name##Store->NoPrefaulting = TraceStore->NoPrefaulting;            \
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


_Use_decl_annotations_
BOOL
InitializeTraceStore(
    PRTL Rtl,
    PCWSTR Path,
    PTRACE_STORE TraceStore,
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
        whether or not the

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
    HRESULT Result;
    WCHAR AllocationPath[_OUR_MAX_PATH];
    WCHAR RelocationPath[_OUR_MAX_PATH];
    WCHAR AddressPath[_OUR_MAX_PATH];
    WCHAR BitmapPath[_OUR_MAX_PATH];
    WCHAR InfoPath[_OUR_MAX_PATH];
    PCWSTR AllocationSuffix = TraceStoreAllocationSuffix;
    PCWSTR RelocationSuffix = TraceStoreRelocationSuffix;
    PCWSTR AddressSuffix = TraceStoreAddressSuffix;
    PCWSTR BitmapSuffix = TraceStoreBitmapSuffix;
    PCWSTR InfoSuffix = TraceStoreInfoSuffix;
    TRACE_FLAGS Flags;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceFlags)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Reloc)) {
        return FALSE;
    }

    Flags = *TraceFlags;

    TraceStore->Reloc = Reloc;

    //
    // Initialize the paths of the metadata stores first.
    //

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
        OPEN_ALWAYS,
        FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
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

    if (Reloc->NumberOfRelocations > 0) {
        TraceStore->HasRelocations = TRUE;
    }

    TraceStore->Flags = Flags;

    //
    // Now that we've created the trace store file, create the NTFS streams
    // for the metadata trace store files.
    //

    INIT_METADATA(Allocation);
    INIT_METADATA(Relocation);
    INIT_METADATA(Address);
    INIT_METADATA(Bitmap);
    INIT_METADATA(Info);

    //
    // Now initialize the TraceStore itself.
    //

    TraceStore->IsMetadata = FALSE;

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
BOOL
TruncateStore(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine truncates a trace store.  When a trace store is initialized,
    a default initial file size is used.  When the store is extended, it is
    done in fixed sized blocks (that match the mapping size).

    When a trace store is closed, this TruncateStore() routine is called, which
    sets the file's end-of-file pointer back to the exact byte position that
    was actually used during the trace session.  This allows the trace file
    to be read in entirely until an EOS is read, which is useful for downstream
    consumers.

Arguments:

    TraceStore - Supplies a pointer to an initialized TRACE_STORE struct that
        will be truncated.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsMetadata;
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
    // Are we a metadata store?  Our truncation behavior is different if we are,
    // so capture this state upfront.
    //

    IsMetadata = IsMetadataTraceStore(TraceStore);

    TotalAllocationSize.QuadPart = TraceStore->TotalAllocationSize.QuadPart;

    if (!IsMetadata) {

        //
        // A metadata's end-of-file will be tracked directly in the pEof struct.
        //

        EndOfFile.QuadPart = TraceStore->pEof->EndOfFile.QuadPart;

        if (EndOfFile.QuadPart != TotalAllocationSize.QuadPart) {

            //
            // This should never happen.
            //

            __debugbreak();
        }

    } else {

        //
        // A trace store's end-of-file can be determined by from the total
        // allocation size associated with the store.  If it is currently set
        // to 0, we check the invariant that the trace store's number of allocs
        // is not 1, and then use the record size instead.
        //

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
    be free'd (indicating that no more threads are servicing trace store memory
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
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
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
CloseTraceStore(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine closes a normal trace store.  It is not intended to be called
    against metadata trace stores.  It will close the trace store, then close
    the meta data trace stores associated with that trace store.

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

    if (TraceStore->AllocationStore) {
        CloseStore(TraceStore->AllocationStore);
        TraceStore->AllocationStore = NULL;
    }

    if (TraceStore->RelocationStore) {
        CloseStore(TraceStore->RelocationStore);
        TraceStore->RelocationStore = NULL;
    }

    if (TraceStore->AddressStore) {
        CloseStore(TraceStore->AddressStore);
        TraceStore->AddressStore = NULL;
    }

    if (TraceStore->BitmapStore) {
        CloseStore(TraceStore->BitmapStore);
        TraceStore->BitmapStore = NULL;
    }

    if (TraceStore->InfoStore) {
        CloseStore(TraceStore->InfoStore);
        TraceStore->InfoStore = NULL;
    }

}

_Use_decl_annotations_
VOID
CALLBACK
PrefaultFutureTraceStorePageCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Context,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the prefault future trace store
    threadpool work.  It simply calls the PrefaultFutureTraceStorePage()
    inline routine (which forces (via volatile machinery) a read of the memory
    address we want to prefault, which will result in a hard or soft fault if
    the page isn't resident).

Arguments:

    Instance - Not used.

    Context - Supplies a pointer to a TRACE_STORE struct.

    Work - Not used.

Return Value:

    None.

--*/
{
    //
    // Ensure Context has a value.
    //

    if (!Context) {
        return;
    }

    PrefaultFutureTraceStorePage((PTRACE_STORE)Context);
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
