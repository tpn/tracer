/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStores.c

Abstract:

    This module implements functionality related to a collection of trace
    store structures, referred to as "trace stores".  Routines are provided
    for getting the allocation size of a TRACE_STORES structure as well as
    initializing both normal and readonly versions, as well as closing the
    structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeTraceStores(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PWSTR BaseDirectory,
    PTRACE_STORES TraceStores,
    PULONG SizeOfTraceStores,
    PULONG InitialFileSizes,
    PTRACE_FLAGS TraceFlags,
    PTRACE_STORE_FIELD_RELOCS FieldRelocations
    )
/*++

Routine Description:

    This routine initializes a TRACE_STORES structure.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    BaseDirectory - Supplies a pointer to a fully-qualified, NULL-terminated
        wide character string representing the base directory to initialize
        the individual trace stores in.

    TraceStores - Supplies a pointer to a TRACE_STORES structure to initialize.
        The caller is responsible for allocating a sufficiently-sized buffer,
        and must provide the size of the buffer via the SizeOfTraceStores
        parameter.

    SizeOfTraceStores - Supplies a pointer to a ULONG that contains the size
        of the buffer pointed to by the TraceStores parameter, in bytes.  The
        actual size of the buffer used will be written to this variable when
        the routine completes.

    InitialFileSizes - Supplies a pointer to an array of ULONG values that
        indicate the initial file sizes to be used for each trace store.  If
        NULL, default sizes are used.  Not applicable for read-only sessions.

    TraceFlags - Supplies a pointer to a TRACE_FLAGS structure, which provides
        additional information about how the trace stores are to be initialized.

    FieldRelocations - Supplies an optional pointer to an array of
        TRACE_STORE_FIELD_RELOCS structures to use for the trace stores.

Return Value:

    TRUE on success, FALSE on failure.  The required size of the TraceStores
    buffer can be obtained by passing in a NULL value for TraceStores.  The
    size will be written to the SizeOfTraceStores variable and FALSE will be
    returned.

--*/
{
    BOOL Success;
    BOOL Readonly;
    BOOL Compress;
    HRESULT Result;
    DWORD Index;
    DWORD StoreIndex;
    DWORD LastError;
    DWORD CreateFileDesiredAccess;
    DWORD CreateFileCreationDisposition;
    DWORD CreateFileMappingProtectionFlags;
    DWORD CreateFileFlagsAndAttributes;
    DWORD MapViewOfFileDesiredAccess;
    TRACE_FLAGS Flags;
    LPWSTR FileNameDest;
    ULONG LongestFilename = GetLongestTraceStoreFileName();
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
    LONG_INTEGER DirectorySizeInBytes;
    PULONG Sizes = InitialFileSizes;
    WCHAR Path[_OUR_MAX_PATH];

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(SizeOfTraceStores)) {
        return FALSE;
    }

    if (!TraceStores || *SizeOfTraceStores < TraceStoresAllocationSize) {
        *SizeOfTraceStores = TraceStoresAllocationSize;
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BaseDirectory)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceFlags)) {
        return FALSE;
    }

    Flags = *TraceFlags;
    Compress = Flags.Compress;
    Readonly = Flags.Readonly;

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

    //
    // We've got sufficient buffer space, zero the memory and proceed with
    // initialization.
    //

    SecureZeroMemory(TraceStores, TraceStoresAllocationSize);

    //
    // This could be our first C entry point, so set __C_specific_handler.
    //

    SetCSpecificHandler(Rtl->__C_specific_handler);

    //
    // Sanity check the directory length in bytes, then copy to BaseDirectory.
    //

    DirectorySizeInBytes.LongPart = (DirectoryLength.LowPart + 1) << 1;

    if (DirectorySizeInBytes.HighPart != 0) {
        return FALSE;
    }

    Success = AllocateAndCopyWideString(
        Allocator,
        DirectorySizeInBytes.LowPart,
        BaseDirectory,
        &TraceStores->BaseDirectory
    );

    if (!Success) {
        return FALSE;
    }

    //
    // If field relocations have been provided, initialize them now.
    // We only do this if we're not readonly; if we're readonly, relocation
    // information is loaded from the :relocation metadata store.
    //

    if (!Readonly && ARGUMENT_PRESENT(FieldRelocations)) {
        USHORT Outer;
        USHORT Inner;
        USHORT MaxInner;
        USHORT NumberOfElements;
        TRACE_STORE_ID TraceStoreId;

        PTRACE_STORE TraceStore;
        PTRACE_STORE_RELOC Reloc;
        PTRACE_STORE_RELOC RelocTarget;
        PTRACE_STORE_FIELD_RELOC FieldReloc;
        PTRACE_STORE_FIELD_RELOC FirstFieldReloc;
        PTRACE_STORE_FIELD_RELOCS FieldRelocs;

        Success = ValidateFieldRelocationsArray(
            FieldRelocations,
            &NumberOfElements,
            &MaxInner
        );

        if (!Success) {
            OutputDebugStringA("FieldRelocation validation failed.\n");
            return FALSE;
        }

        TraceStores->NumberOfFieldRelocationsElements = NumberOfElements;

        //
        // Initialize each TraceStores->Relocations[Index] slot.  We loop
        // through every slot, then loop through the caller's field relocations
        // array and see if the trace store IDs match.  We could be fancier to
        // avoid the n^2 loop overhead, but this isn't called frequently, and
        // the array sizes shouldn't be particularly large.
        //

        for (Index = 0; Index < MAX_TRACE_STORE_IDS; Index++) {
            Reloc = &TraceStores->Relocations[Index];
            Reloc->SizeOfStruct = sizeof(*Reloc);
            TraceStoreId = ArrayIndexToTraceStoreId((USHORT)Index);
            TraceStore = TraceStoreIdToTraceStore(TraceStores, TraceStoreId);

            for (Outer = 0; Outer < NumberOfElements; Outer++) {
                FieldRelocs = FieldRelocations + Outer;

                if (FieldRelocs->TraceStoreId != TraceStoreId) {
                    continue;
                }

                FirstFieldReloc = FieldRelocs->Relocations;
                AssertAlignedTraceStoreFieldReloc(FirstFieldReloc);

                //
                // Loop through the array of inner fields.  This serves two
                // purposes: a) tracking how many references are present, and
                // b) allowing us to register as a backref with the target.
                //

                for (Inner = 0; Inner < MaxInner; Inner++) {
                    USHORT TargetIndex;
                    TRACE_STORE_ID TargetId;
                    PTRACE_STORE Target;
                    PRTL_BITMAP Bitmap;

                    FieldReloc = FirstFieldReloc + Inner;
                    if (IsLastTraceStoreFieldRelocElement(FieldReloc)) {
                        break;
                    }

                    TargetId = FieldReloc->TraceStoreId;

                    if (TargetId == TraceStoreNullId) {
                        continue;
                    }

                    TargetIndex = TraceStoreIdToArrayIndex(TargetId);
                    Target = TraceStoreIdToTraceStore(TraceStores, TargetId);
                    RelocTarget = &TraceStores->Relocations[TargetIndex];
                    Bitmap = &RelocTarget->Bitmap;

                    if (++RelocTarget->NumberOfRelocationBackReferences == 1) {
                        PULONG Buffer;
                        Buffer = (PULONG)&RelocTarget->BitmapBuffer[0];
                        Target->IsRelocationTarget = TRUE;
                        Bitmap->SizeOfBitMap = MAX_TRACE_STORE_IDS;
                        Bitmap->Buffer = Buffer;
                        RelocTarget->BitmapBufferSizeInQuadwords = (
                            TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS
                        );
                    }

                    Rtl->RtlSetBit(Bitmap, TraceStoreId);
                }

                //
                // Finalize the TRACE_STORE_RELOC structure.
                //

                Reloc->NumberOfRelocations = Inner;
                Reloc->Relocations = FirstFieldReloc;
            }
        }
    }

    Path[DirectoryLength.LowPart] = L'\\';
    FileNameDest = &Path[DirectoryLength.LowPart+1];
    RemainingChars.QuadPart = (
        _OUR_MAX_PATH -
        DirectoryLength.LowPart -
        2
    );

    //
    // Initialize struct size fields and flags.
    //

    TraceStores->SizeOfStruct = (USHORT)sizeof(TRACE_STORES);
    TraceStores->SizeOfAllocation = (USHORT)TraceStoresAllocationSize;
    TraceStores->Flags = Flags;

    //
    // Initialize create file and map view flags.
    //

    if (Readonly) {
        CreateFileDesiredAccess = GENERIC_READ;
        CreateFileCreationDisposition = OPEN_EXISTING;
        CreateFileMappingProtectionFlags = PAGE_READONLY;
        MapViewOfFileDesiredAccess = FILE_MAP_READ;
    } else {
        CreateFileDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        CreateFileCreationDisposition = CREATE_NEW;
        CreateFileMappingProtectionFlags = PAGE_READWRITE;
        MapViewOfFileDesiredAccess = FILE_MAP_READ | FILE_MAP_WRITE;
    }

    //
    // Create the appropriate dwFileAndAttributes mask based on the flags.
    //

    if (Flags.EnableFileFlagRandomAccess) {
        CreateFileFlagsAndAttributes = FILE_FLAG_RANDOM_ACCESS;
    } else if (!Flags.DisableFileFlagSequentialScan) {
        CreateFileFlagsAndAttributes = FILE_FLAG_SEQUENTIAL_SCAN;
    }

    if (!Flags.DisableFileFlagOverlapped) {
        CreateFileFlagsAndAttributes |= FILE_FLAG_OVERLAPPED;
    }

    if (Flags.EnableFileFlagWriteThrough) {
        CreateFileFlagsAndAttributes |= FILE_FLAG_WRITE_THROUGH;
    }

    //
    // Ensure the base directory exists.
    //

    Success = CreateDirectoryW(BaseDirectory, NULL);
    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            return FALSE;
        }
    }

    //
    // If we're not read-only and the compress flag has been provided, attempt
    // to turn on directory compression.  This results in significant storage
    // space reductions and negligible overhead, and is on by default.
    //

    if (!Readonly && Compress) {
        HANDLE DirectoryHandle;
        USHORT CompressionFormat = COMPRESSION_FORMAT_DEFAULT;
        DWORD BytesReturned = 0;

        //
        // Obtain a directory handle with FILE_FLAG_BACKUP_SEMANTICS, which is
        // required in order for us to call the FSCTL_SET_COMPRESSION IoControl
        // below.
        //

        DirectoryHandle = CreateFileW(
            BaseDirectory,
            CreateFileDesiredAccess,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL
        );

        if (DirectoryHandle == INVALID_HANDLE_VALUE) {
            LastError = GetLastError();
            return FALSE;
        }

        //
        // Request compression on the directory.
        //

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

            //
            // We can't do anything useful here.
            //

            OutputDebugStringA("Failed to enable compression.\n");
        }

        CloseHandle(DirectoryHandle);
    }

    //
    // Initialize final elements.
    //

    TraceStores->Rtl = Rtl;
    TraceStores->NumberOfTraceStores = NumberOfTraceStores;
    TraceStores->ElementsPerTraceStore = ElementsPerTraceStore;
    TraceStores->Allocator = Allocator;

    InitializeListHead(&TraceStores->RundownListEntry);

    //
    // Loop through each trace store in the array and initialize it.
    //

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {
        TRACE_STORE_DECLS();
        PTRACE_STORE_RELOC Reloc;

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

        TraceStore->TraceStore = TraceStore;

        TraceStore->IsReadonly = Readonly;
        TraceStore->SequenceId = Index;
        TraceStore->TraceStoreId = ArrayIndexToTraceStoreId((USHORT)Index);
        TraceStore->TraceStoreIndex = StoreIndex;
        TraceStore->CreateFileDesiredAccess = CreateFileDesiredAccess;
        TraceStore->CreateFileCreationDisposition = (
            CreateFileCreationDisposition
        );
        TraceStore->CreateFileMappingProtectionFlags = (
            CreateFileMappingProtectionFlags
        );
        TraceStore->CreateFileFlagsAndAttributes = (
            CreateFileFlagsAndAttributes
        );
        TraceStore->MapViewOfFileDesiredAccess = (
            MapViewOfFileDesiredAccess
        );
        TraceStore->Allocator = Allocator;
        TraceStore->BindComplete = TraceStoreBindComplete;

        Reloc = &TraceStores->Relocations[Index];

        Success = InitializeTraceStore(
            Rtl,
            Path,
            TraceStore,
            MetadataInfoStore,
            AllocationStore,
            RelocationStore,
            AddressStore,
            AddressRangeStore,
            BitmapStore,
            InfoStore,
            InitialSize,
            MappingSize,
            &Flags,
            Reloc
        );

        if (!Success) {
            return FALSE;
        }
    }

    //
    // If we're not readonly, and global rundown hasn't been disabled, register
    // this trace stores structure.  This will ensure it gets run down when the
    // process exits but the user hasn't explicitly stopped tracing.  (This is
    // common in Python if a module does sys.exit().)
    //

    if (!Readonly && !TraceFlags->NoGlobalRundown) {
        Success = RegisterGlobalTraceStores(TraceStores);
    }

    return Success;
}

_Use_decl_annotations_
BOOL
InitializeReadonlyTraceStores(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PWSTR BaseDirectory,
    PTRACE_STORES TraceStores,
    PULONG SizeOfTraceStores,
    PTRACE_FLAGS TraceFlags
    )
/*++

Routine Description:

    This routine initializes a readonly TRACE_STORES structure from an existing
    base directory.  It is a convenience method that is equivalent to calling
    InitializeTraceStores() with TraceFlags->Readonly set to TRUE and the
    InitialFileSizes set to a NULL pointer.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    BaseDirectory - Supplies a pointer to a fully-qualified, NULL-terminated
        wide character string representing the base directory to load the trace
        stores from.  The directory must exist and all expected stores must also
        exist.

    TraceStores - Supplies a pointer to a TRACE_STORES structure to initialize.
        The caller is responsible for allocating a sufficiently-sized buffer,
        and must provide the size of the buffer via the SizeOfTraceStores
        parameter.

    SizeOfTraceStores - Supplies a pointer to a ULONG that contains the size
        of the buffer pointed to by the TraceStores parameter, in bytes.  The
        actual size of the buffer used will be written to this variable when
        the routine completes.

    TraceFlags - Supplies a pointer to a TRACE_FLAGS structure, which provides
        additional information about how the trace stores are to be initialized.
        The Readonly flag will automatically be set by this routine before the
        flags are passed on to InitializeTraceStores().

Return Value:

    TRUE on success, FALSE on failure.  The required size of the TraceStores
    buffer can be obtained by passing in a NULL value for TraceStores.  The
    size will be written to the SizeOfTraceStores variable and FALSE will be
    returned.

--*/
{
    TRACE_FLAGS Flags = { 0 };

    //
    // Load the caller's flags if the pointer is non-NULL.
    //

    if (ARGUMENT_PRESENT(TraceFlags)) {
        Flags = *TraceFlags;
    }

    //
    // Set the readonly flag.
    //

    Flags.Readonly = TRUE;

    return InitializeTraceStores(Rtl,
                                 Allocator,
                                 BaseDirectory,
                                 TraceStores,
                                 SizeOfTraceStores,
                                 NULL,
                                 &Flags,
                                 NULL);
}

_Use_decl_annotations_
BOOL
CloseTraceStores(
    PTRACE_STORES TraceStores
    )
/*++

Routine Description:

    This routine closes a TRACE_STORES structure that has been previously
    initialized.

Arguments:

    TraceStores - Supplies a pointer to a TRACE_STORES structure to close.
        If NULL, the routine immediately returns.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStores)) {
        return FALSE;
    }

    //
    // If there's a rundown parameter set, we're still attached to a rundown,
    // so unregister first.
    //

    if (TraceStores->Rundown != NULL) {
        if (!UnregisterTraceStores(TraceStores)) {
            __debugbreak();
            return FALSE;
        }
    }

    CloseTraceStoresInline(TraceStores);

    return TRUE;
}

_Use_decl_annotations_
ULONG
GetTraceStoresAllocationSize(
    USHORT NumberOfTraceStores
    )
/*++

Routine Description:

    This routine returns the number of bytes required to create a TRACE_STORES
    structure for a given number of trace stores.

Arguments:

    NumberOfTraceStores - Supplies the number of top-level (e.g. not including
        including supporting metadata stores) for which the allocation size
        is to be calculated.

Return Value:

    Number of bytes required for the allocation.

--*/
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

    return (sizeof(TRACE_STORES) + ExtraSize);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
