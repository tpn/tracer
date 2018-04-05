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
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_ARRAY StringArray,
    USHORT StringTablePaddingOffset,
    USHORT StringTableStructSize,
    PPSTRING_TABLE StringTablePointer
    )
/*++

Routine Description:

    Performs a deep-copy of a STRING_ARRAY structure using the given Allocator.
    If the array will fit within the trailing space of a STRING_TABLE structure,
    the routine will allocate space for the STRING_TABLE instead.

    N.B.: Strings in the new array will have their Hash field set to the CRC32
          value of the character values (excluding any NULLs) of their buffer.

Arguments:

    StringTableAllocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE structure.  This is kept separate from the
        StringTableAllocator due to the stringent alignment requirements of the
        string table.

    StringArray - Supplies a pointer to an initialized STRING_ARRAY structure
        to be copied.

    StringTablePaddingOffset - Supplies a USHORT value indicating the number
        of bytes from the STRING_TABLE structure where the padding begins.
        This value is used in conjunction with StringTableStructSize below
        to determine if the STRING_ARRAY will fit within the table.

    StringTableStructSize - Supplies a USHORT value indicating the size of the
        STRING_TABLE structure, in bytes.  This is used in conjunction with the
        StringTablePaddingOffset parameter above.

    StringTablePointer - Supplies a pointer to a variable that receives the
        address of the STRING_TABLE structure if one could be allocated.  If
        not, the pointer will be set to NULL.

Return Value:

    A pointer to a valid PSTRING_ARRAY structure on success, NULL on failure.

--*/
{
    BOOL Success;

    USHORT Index;
    USHORT Count;
    USHORT MinimumLength;
    USHORT MaximumLength;
    USHORT AlignedMaxLength;
    USHORT AlignedStringTablePaddingOffset;
    USHORT StringTableRemainingSpace;

    ULONG TotalAllocSize;
    ULONG StructSize;
    ULONG ElementsSize;
    ULONG BufferOffset;

    PCHAR DestBuffer;

    PSTRING DestString;
    PSTRING SourceString;

    PSTRING_TABLE StringTable;
    PSTRING_ARRAY NewArray;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTableAllocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringArray)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringTablePointer)) {
        return NULL;
    }

    //
    // Get the required allocation size and minimum/maximum lengths.
    //

    Success = GetStringArrayAllocationInfo(
        StringArray,
        &TotalAllocSize,
        &StructSize,
        &ElementsSize,
        &BufferOffset,
        &MinimumLength,
        &MaximumLength
    );

    if (!Success) {
        return NULL;
    }

    //
    // Check to see if we can fit our entire allocation within the remaining
    // space of the string table.
    //

    AlignedStringTablePaddingOffset = (
        ALIGN_UP_POINTER(StringTablePaddingOffset)
    );

    StringTableRemainingSpace = (
        StringTableStructSize -
        AlignedStringTablePaddingOffset
    );

    if (StringTableRemainingSpace >= TotalAllocSize) {

        //
        // We can fit our copy of the STRING_ARRAY within the trailing padding
        // bytes of the STRING_TABLE, so, allocate sufficient space for that
        // struct, then carve out our table pointer.
        //

        StringTable = (PSTRING_TABLE)(
            StringTableAllocator->AlignedCalloc(
                StringTableAllocator->Context,
                1,
                StringTableStructSize,
                STRING_TABLE_ALIGNMENT
            )
        );

        if (!StringTable) {

            //
            // If we couldn't allocate 512-bytes for the table, I don't think
            // there is much point trying to allocate <= 512-bytes for just
            // the array if memory is that tight.
            //

            return NULL;

        }

        //
        // Allocation was successful, carve out the pointer to the NewArray.
        // (We use RtlOffsetToPointer() here instead of StringTable->StringArray
        // as the former will be done against the aligned padding size and isn't
        // dependent upon knowing anything about the STRING_TABLE struct other
        // than the offset and struct size parameters passed in as arguments.)
        //

        NewArray = (PSTRING_ARRAY)(
            RtlOffsetToPointer(
                StringTable,
                AlignedStringTablePaddingOffset
            )
        );

    } else {

        //
        // We can't embed ourselves within the trailing STRING_TABLE padding.
        // Clear the pointer to make it clear no StringTable was allocated,
        // and then try allocating sufficient space just for the STRING_ARRAY.
        //

        StringTable = NULL;

        NewArray = (PSTRING_ARRAY)(
            StringArrayAllocator->Calloc(
                StringArrayAllocator->Context,
                1,
                TotalAllocSize
            )
        );

    }

    //
    // Ensure we allocated sufficient space.
    //

    if (!NewArray) {
        return NULL;
    }

    //
    // Initialize the scalar fields.
    //

    NewArray->SizeInQuadwords = (USHORT)(TotalAllocSize >> 3);
    NewArray->NumberOfElements = StringArray->NumberOfElements;

    NewArray->MinimumLength = MinimumLength;
    NewArray->MaximumLength = MaximumLength;

    //
    // Initialize string source and destination string pointers and the count
    // of elements.
    //

    SourceString = StringArray->Strings;
    DestString = NewArray->Strings;
    Count = StringArray->NumberOfElements;

    //
    // Initialize the StringTable field; if it's NULL at this point, that's ok.
    //

    NewArray->StringTable = StringTable;

    //
    // Initialize the destination buffer to the point after the new STRING_ARRAY
    // struct and trailing array of actual STRING structs.  We then carve out
    // new buffer pointers for the destination strings as we loop through the
    // source string array and copy strings over.
    //

    DestBuffer = (PCHAR)(RtlOffsetToPointer(NewArray, BufferOffset));

    //
    // Loop through the strings of the source string array and copy into the
    // new array, including carving out the relevant buffer space.
    //

    for (Index = 0; Index < Count; Index++, SourceString++, DestString++) {

        AlignedMaxLength = ALIGN_UP(SourceString->Length, 16);

        DestString->Length = SourceString->Length;
        DestString->MaximumLength = AlignedMaxLength;
        DestString->Buffer = DestBuffer;

        //
        // Copy the source string over.
        //

        CopyMemory(DestString->Buffer,
                   SourceString->Buffer,
                   SourceString->Length);

        //
        // Carve out the next destination buffer.
        //

        DestBuffer += AlignedMaxLength;

        //
        // Compute the CRC32 checksum and store in the hash field.
        //

        DestString->Hash = ComputeCrc32ForString(DestString);
    }

    //
    // Update the caller's StringTablePointer (which may be NULL if we didn't
    // allocate a StringTable) and return the StringArray.
    //

    *StringTablePointer = StringTable;

    return NewArray;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
