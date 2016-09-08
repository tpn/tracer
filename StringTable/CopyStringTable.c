/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    CopyStringArray.c

Abstract:

    This module implements the functionality to copy a STRING_ARRAY structure.
    It is primarily used to make a local copy of a STRING_ARRAY in creation of
    a STRING_TABLE.

--*/

#include "stdafx.h"

_Use_decl_annotations_
PSTRING_ARRAY
CopyStringArray(
    PALLOCATOR Allocator,
    PSTRING_ARRAY StringArray
    )
/*++

Routine Description:

    Performs a deep-copy of a STRING_ARRAY structure using the given Allocator.

    N.B.: Strings in the new array will have their Hash field set to the CRC32
          value of the character values (excluding any NULLs) of their buffer.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to allocate all memory required by the structure during its
        creation.

    StringArray - Supplies a pointer to an initialized STRING_ARRAY structure
        to be copied.

Return Value:

    A pointer to a valid PSTRING_ARRAY structure on success, NULL on failure.
    The PSTRING_ARRAY.

--*/
{
    BOOL Success;
    USHORT Bytes;
    USHORT Index;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    USHORT AlignedMaxLength;
    USHORT PreviousBufferLength;

    ULONG TotalAllocSize;
    ULONG StructSize;
    ULONG ElementsSize;

    PCHAR DestBuffer;

    PSTRING DestString;
    PSTRING SourceString;

    PSTRING_ARRAY NewArray;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringArray)) {
        return NULL;
    }

    //
    // Get the required allocation size and minimum/maximum lengths.
    //

    Success = GetStringArrayAllocationInfo(
        StringArray,
        0,  // Start
        0,  // End
        0,  // Alignment
        &AllocSize,
        &StructSize,
        &ElementsSize,
        &MinimumLength,
        &MaximumLength
    );

    if (!Success) {
        return NULL;
    }

    //
    // Allocate space for the array.
    //

    NewArray = (PSTRING_ARRAY)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSize
        )
    );

    if (!NewArray) {
        return NULL;
    }

    //
    // Initialize the scalar fields.
    //

    NewArray->Size = AllocSize;
    NewArray->NumberOfElements = StringArray->NumberOfElements;
    NewArray->MinimumLength = StringArray->MinimumLength;
    NewArray->MaximumLength = StringArray->MaximumLength;

    //
    // Initialize string source and dest pointers to one before the starting
    // positions.
    //

    SourceString = StringArray->Strings-1;
    DestString = NewArray->Strings-1;

    //
    // Initialize the destination buffer to the point after the new STRING_ARRAY
    // struct (and trailing array of actual STRING structs, minus 1 because the
    // struct includes 1 element by default (that detail is handled for us
    // by the GetStringArrayAllocationInfo() routine).
    //

    DestBuffer = (PCHAR)(
        RtlOffsetToPointer(
            NewArray,
            StructSize + ElementsSize
        )
    );

    //
    // Loop through the strings of the source string array and copy into the
    // new array, including carving out the relevant buffer space.
    //

    for (Index = 0; Index < StringArray->NumberOfElements; Index++) {

        ++SourceString;
        ++DestString;

        AlignedMaxLength = ALIGN_UP(SourceString->Length, 16);

        DestString->Length = SourceString->Length;
        DestString->MaximumLength = AlignedMaxLength;
        DestString->Buffer = DestBuffer;

        //
        // Copy the source string over.
        //

        __movsb(DestString->Buffer, SourceString->Buffer, SourceString->Length);

        //
        // Carve out the next destination buffer.
        //

        DestBuffer += AlignedMaxLength;

        //
        // Compute the CRC32 checksum and store in the hash field.
        //

        DestString->Hash = ComputeCrc32ForString(DestString);
    }

    return NewArray;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
