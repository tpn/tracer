/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    InitializeTracerConfig.c

Abstract:

    This module implements routines related to tracer path handling.  Routines
    are provided for loading an internal path, and making a path at runtime.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
MakeTracerPath(
    PTRACER_CONFIG TracerConfig,
    PCUNICODE_STRING Filename,
    PTRACER_BINARY_TYPE_INDEX BinaryTypeIndexPointer,
    PPUNICODE_STRING PathPointer
    )
/*++

Routine Description:

    This routine creates a fully-qualified Unicode string representation of
    Filename, rooted in the active tracer directory.

Arguments:

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure to use for
        the path creation.

    Filename - Supplies a pointer to a UNICODE_STRING structure representing
        the filename to convert into a fully-qualified path name.

    BinaryTypeIndexPointer - Supplies an optional pointer to an enumeration of
        type TRACER_BINARY_TYPE_INDEX in order to create a path rooted in a
        directory tree that isn't currently active (as per the current flags).

    PathPointer - Supplies a pointer to a variable that receives the address
        of the new path.  If the incoming pointer is NULL, a new UNICODE_STRING
        structure will be allocated; if it is non-NULL, it is assumed the caller
        has already allocated sufficient space for the UNICODE_STRING in the
        pointed-to location.

Return Value:

    TRUE on success, FALSE on FAILURE.

--*/
{
    BOOL IncludeStruct;
    USHORT Length;
    USHORT MaximumLength;
    USHORT SizeInBytes;
    ULONG Bytes;
    ULONG Count;
    TRACER_FLAGS Flags;
    PWCHAR Dest;
    PWCHAR Source;
    PALLOCATOR Allocator;
    PUNICODE_STRING Path;
    PUNICODE_STRING InstallationDir;
    PUNICODE_STRING IntermediatePath;
    TRACER_BINARY_TYPE_INDEX BinaryTypeIndex;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Filename)) {
        return FALSE;
    }

    if (!IsValidUnicodeStringWithMinimumLengthInChars(Filename, 1)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathPointer)) {
        return FALSE;
    }

    Path = *PathPointer;

    if (ARGUMENT_PRESENT(Path)) {
        IncludeStruct = FALSE;
    } else {
        IncludeStruct = TRUE;
    }

    //
    // Initialize various local variables.
    //

    Flags = TracerConfig->Flags;
    Allocator = TracerConfig->Allocator;

    if (ARGUMENT_PRESENT(BinaryTypeIndexPointer)) {
        BinaryTypeIndex = *BinaryTypeIndexPointer;
    } else {
        BinaryTypeIndex = ExtractTracerBinaryTypeIndexFromFlags(Flags);
    }

    //
    // Initialize paths.
    //

    InstallationDir = &TracerConfig->Paths.InstallationDirectory;
    IntermediatePath = IntermediatePaths[BinaryTypeIndex];

    //
    // Calculate the length of the final joined path.  IntermediatePath
    // will have leading and trailing slashes.
    //

    Length = (
        InstallationDir->Length +
        IntermediatePath->Length +
        Filename->Length
    );

    //
    // Account for the trailing NULL and align up to pointer size.
    //

    MaximumLength = ALIGN_UP_USHORT_TO_POINTER_SIZE(Length + sizeof(WCHAR));

    //
    // This is our allocation size in bytes.
    //

    SizeInBytes = (USHORT)MaximumLength;

    if (IncludeStruct) {

        //
        // Account for the UNICODE_STRING structure if necessary.
        //

        SizeInBytes += sizeof(UNICODE_STRING);

        Path = (PUNICODE_STRING)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                SizeInBytes
            )
        );

        if (!Path) {
            return FALSE;
        }

        //
        // Carve out the buffer pointer.
        //

        Path->Buffer = (PWSTR)RtlOffsetToPointer(Path, sizeof(UNICODE_STRING));

    } else {

        //
        // Caller has provided the UNICODE_STRING structure; we just need to
        // allocate the buffer.
        //

        Path->Buffer = (PWCHAR)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                SizeInBytes
            )
        );

        if (!Path->Buffer) {
            return FALSE;
        }
    }

    //
    // Copy the installation directory.  Note we track bytes and count
    // (i.e. number of chars) separately; the lengths are in bytes but
    // __movsw() works a WORD (WCHAR) at a time.
    //

    Dest = Path->Buffer;
    Source = InstallationDir->Buffer;
    Bytes = InstallationDir->Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);

    //
    // Copy the intermediate bit of the path (e.g. "\\x64\\Debug\\").
    //

    Dest += Count;
    Source = IntermediatePath->Buffer;
    Bytes = IntermediatePath->Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);

    //
    // Copy the filename.
    //

    Dest += Count;
    Source = Filename->Buffer;
    Bytes = Filename->Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);

    //
    // And set the final trailing NULL.
    //

    Dest += Count;
    *Dest = L'\0';

    //
    // Update the caller's pointer if applicable.
    //

    if (IncludeStruct) {
        *PathPointer = Path;
    }

    //
    // Update the lengths on the string and return success.
    //

    Path->Length = Length;
    Path->MaximumLength = MaximumLength;

    return TRUE;
}

_Use_decl_annotations_
BOOL
LoadTracerPath(
    PTRACER_CONFIG TracerConfig,
    TRACER_PATH_TYPE PathType,
    USHORT Index
    )
/*++

Routine Description:

    A helper routine for initializing TRACER_CONFIG Path variables.  This
    calculates the required UNICODE_STRING Buffer length to hold the full
    DLL path, which will be a concatenation of the InstallationDirectory,
    IntermediatePath (i.e. "x64\\Release") and the final DLL name, including
    all joining slashes and trailing NULL.  The TracerConfig's Allocator is
    used to allocate sufficient space, then the three strings are copied over.

Arguments:

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure for which
        the path is being loaded.

    PathType - Supplies the type of path being loaded (e.g. DLL vs PTX).

    Index - Supplies the 0-based index into the PathOffsets[] array which
        is used to resolve a pointer to the UNICODE_STRING variable in the
        TRACER_PATHS structure, as well as a pointer to the UNICODE_STRING
        for the relevant DLL suffix for that variable.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT Offset;
    PTRACER_PATHS Paths;
    PCUNICODE_STRING Filename;
    PUNICODE_STRING TargetPath;
    PCTRACER_OFFSET_TO_PATH_ENTRY PathEntry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!IsValidTracerPathTypeAndIndex(PathType, Index)) {
        return FALSE;
    }

    //
    // Load the offset and filename for this combination of path type and index.
    //

    PathEntry = &TracerPathOffsets[PathType].Entries[Index];
    Offset = PathEntry->Offset;
    Filename = PathEntry->Filename;

    //
    // Resolve the address of the final target path.
    //

    Paths = &TracerConfig->Paths;
    TargetPath = (PUNICODE_STRING)((((ULONG_PTR)Paths) + Offset));

    //
    // Dispatch to MakeTracerPath() for final path preparation.
    //

    return MakeTracerPath(TracerConfig, Filename, NULL, &TargetPath);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
