/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    CreateStringArray.c

Abstract:

    This module implements the functionality to create a new STRING_ARRAY
    structure as part of the steps required to create a new STRING_TABLE
    structure. Routines are currently provided for creating an array from a
    delimited string.

--*/

#include "stdafx.h"

_Use_decl_annotations_
PSTRING_ARRAY
CreateStringArrayFromDelimitedString(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PSTRING String,
    CHAR Delimiter,
    USHORT StringTablePaddingOffset,
    USHORT StringTableStructSize,
    PPSTRING_TABLE StringTablePointer
    )
/*++

Routine Description:

    Creates a new STRING_ARRAY structure that contains entries for each string
    in the delimited input String.  E.g. if String->Buffer pointed to a string
    "foo;bar", then calling this method with a Delimiter of ';' would create a
    new STRING_ARRAY with two elements, 'foo' and 'bar'.

    If the routine determines the STRING_ARRAY structure can fit within the
    trailing padding of a STRING_TABLE structure, it will allocate memory for
    the table instead and initialize itself at the relevant location offset,
    updating StringTablePointer with the base address of the allocation.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to allocate all memory required by the structure during its
        creation.

    String - Supplies a pointer to a STRING structure that represents the
        delimited string to construct the STRING_ARRAY from.

    Delimiter - Supplies the character that delimits individual elements of
        the String pointer (e.g. ':', ' ').

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
    BOOL Final;
    BOOL Success;

    USHORT Index;
    USHORT Count;
    USHORT StringLength;
    USHORT MinimumLength;
    USHORT MaximumLength;
    USHORT NumberOfElements;
    USHORT AlignedMaxLength;
    USHORT AlignedStringTablePaddingOffset;
    USHORT StringTableRemainingSpace;

    ULONG AlignedSize;
    ULONG StringBufferAllocSize;
    ULONG TotalAllocSize;
    ULONG BufferOffset;

    ULONG AlignedBufferOffset;
    ULONG PaddingSize;
    ULONG StringElementsSize;

    LONG_INTEGER Length;

    ULONG BitmapIndex;
    ULONG PreviousBitmapIndex;

    HANDLE HeapHandle = NULL;

    PCHAR Source;
    PCHAR DestBuffer;

    PSTRING DestString;

    PSTRING_TABLE StringTable = NULL;
    PSTRING_ARRAY NewArray = NULL;

    PRTL_BITMAP Bitmap;
    PRTL_FIND_SET_BITS RtlFindSetBits;
    PRTL_FIND_CLEAR_BITS RtlFindClearBits;
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;
    PRTL_NUMBER_OF_CLEAR_BITS RtlNumberOfClearBits;

    //
    // Set aside a 32-byte stack-allocated bitmap buffer.
    //

    CHAR StackBitmapBuffer[_MAX_FNAME >> 3];
    RTL_BITMAP StackBitmap = { _MAX_FNAME, (PULONG)&StackBitmapBuffer };
    Bitmap = &StackBitmap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringTablePointer)) {
        return NULL;
    }

    //
    // Create a bitmap index for the delimiter characters contained within the
    // string.
    //

    Success = Rtl->CreateBitmapIndexForString(
        Rtl,
        String,
        Delimiter,
        &HeapHandle,
        &Bitmap,
        FALSE,
        NULL
    );

    if (!Success) {
        return NULL;
    }

    //
    // Initialize function pointer aliases.
    //

    RtlFindSetBits = Rtl->RtlFindSetBits;
    RtlFindClearBits = Rtl->RtlFindClearBits;
    RtlNumberOfSetBits = Rtl->RtlNumberOfSetBits;
    RtlNumberOfClearBits = Rtl->RtlNumberOfClearBits;

    //
    // Make sure there's at least one non-delimiter character in the input
    // string.
    //

    if (RtlNumberOfClearBits(Bitmap) == 0) {
        goto Error;
    }

    //
    // Initialize variables before the loop.
    //

    StringBufferAllocSize = 0;
    MinimumLength = (USHORT)-1;
    MaximumLength = 0;
    Count = 0;

    //
    // Find the first non-delimiter character.  We've already checked that
    // there is at least one non-delimiter character via the NumberOfClearBits
    // call above, so we don't need to test this result for BITS_NOT_FOUND.
    //

    PreviousBitmapIndex = RtlFindClearBits(Bitmap, 1, 0);

    //
    // Enumerate over the delimited string, using the bitmap to carve out each
    // individual element, and calculate the total allocation size required to
    // store a 16-byte aligned copy of each string.
    //

    Final = FALSE;

    do {

        BitmapIndex = RtlFindSetBits(Bitmap, 1, PreviousBitmapIndex);

        if (BitmapIndex < PreviousBitmapIndex ||
            BitmapIndex == BITS_NOT_FOUND) {

            //
            // If no delimiter was found (or the bitmap index has wrapped) we
            // are on the last element, so use the string length instead.
            //

            BitmapIndex = String->Length;
            Final = TRUE;

        }

        Length.LongPart = BitmapIndex - PreviousBitmapIndex;

        //
        // Make sure the length is within limits.
        //

        if (Length.HighPart) {
            goto Error;
        }

        //
        // Calculate the aligned size, then add to our running count.
        //

        AlignedSize = ALIGN_UP(Length.LowPart, 16);
        StringBufferAllocSize += AlignedSize;

        //
        // Update minimum and maximum length if applicable.
        //

        if (Length.LowPart < MinimumLength) {
            MinimumLength = Length.LowPart;
        }

        if (Length.LowPart > MaximumLength) {
            MaximumLength = Length.LowPart;
        }

        //
        // Update our counter.
        //

        Count++;

        if (Final) {

            //
            // This was the last element.
            //

            break;

        }

        //
        // Update the previous bitmap index.
        //

        PreviousBitmapIndex = RtlFindClearBits(Bitmap, 1, BitmapIndex + 1);

        if (PreviousBitmapIndex < BitmapIndex) {

            //
            // The search has wrapped, we're done.
            //

            break;

        } else if (PreviousBitmapIndex == BITS_NOT_FOUND) {

            //
            // No more clear bits, we're done.
            //

            break;

        }

        //
        // We've got another element to process, continue the loop.
        //

    } while (1);

    //
    // Capture the number of elements we processed.
    //

    NumberOfElements = Count;

    //
    // Calculate the size required for the STRING elements, minus 1 to account
    // for the fact that STRING_ARRAY includes a single STRING struct at the
    // end of it via the ANYSIZE_ARRAY size specifier.
    //

    StringElementsSize = (NumberOfElements - 1) * sizeof(STRING);

    //
    // Calculate the offset where the first string buffer will reside, and the
    // aligned value of the offset.  Make sure the final buffer offset is
    // 16-bytes aligned, adjusting padding as necessary.
    //

    BufferOffset = sizeof(STRING_ARRAY) + StringElementsSize;
    AlignedBufferOffset = ALIGN_UP(BufferOffset, 16);
    PaddingSize = (AlignedBufferOffset - BufferOffset);

    //
    // Calculate the final size.
    //

    TotalAllocSize = (

        //
        // Account for the STRING_ARRAY structure.
        //

        sizeof(STRING_ARRAY) +

        //
        // Account for the array of STRING structures, minus 1.
        //

        StringElementsSize +

        //
        // Account for any alignment padding we needed to do.
        //

        PaddingSize +

        //
        // Account for the backing buffer sizes.
        //

        StringBufferAllocSize

    );

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
            Allocator->Calloc(
                Allocator->Context,
                1,
                StringTableStructSize
            )
        );

        if (!StringTable) {

            //
            // If we couldn't allocate 512-bytes for the table, I don't think
            // there is much point trying to allocate <= 512-bytes for just
            // the array if memory is that tight.
            //

            goto Error;

        }

        //
        // Allocation was successful, carve out the pointer to the NewArray.
        // (We use RtlOffsetToPointer() here instead of StringTable->StringArray
        // as the former will be done against the aligned pading size and isn't
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
            Allocator->Calloc(
                Allocator->Context,
                1,
                TotalAllocSize
            )
        );

    }

    //
    // Ensure we allocated sufficient space.
    //

    if (!NewArray) {
        goto Error;
    }

    //
    // Initialize the scalar fields.
    //

    NewArray->SizeInQuadwords = (USHORT)(TotalAllocSize >> 3);
    NewArray->NumberOfElements = NumberOfElements;

    NewArray->MinimumLength = MinimumLength;
    NewArray->MaximumLength = MaximumLength;

    //
    // Initialize the StringTable field; if it's NULL at this point, that's ok.
    //

    NewArray->StringTable = StringTable;

    //
    // Initialize string source and destination string pointers and the count
    // of elements.
    //

    Source = String->Buffer;
    DestString = NewArray->Strings;
    Count = NewArray->NumberOfElements;

    //
    // Initialize the destination buffer to the point after the new STRING_ARRAY
    // struct and trailing array of actual STRING structs.  We then carve out
    // new buffer pointers for the destination strings as we loop through the
    // source string array and copy strings over.
    //

    DestBuffer = (PCHAR)(RtlOffsetToPointer(NewArray, BufferOffset));

    //
    // Reset the previous bitmap index.
    //

    PreviousBitmapIndex = RtlFindClearBits(Bitmap, 1, 0);

    //
    // Advance the Source pointer to the first character.
    //

    Source += PreviousBitmapIndex;

    //
    // Loop through the delimiter bitmap again and prime each destination
    // string based.
    //

    Final = FALSE;

    for (Index = 0; Index < NumberOfElements; Index++, DestString++) {

        BitmapIndex = RtlFindSetBits(Bitmap, 1, PreviousBitmapIndex);

        if (BitmapIndex < PreviousBitmapIndex ||
            BitmapIndex == BITS_NOT_FOUND) {

            //
            // If no delimiter was found (or the bitmap index has wrapped) we
            // are on the last element, so use the string length instead.
            //

            BitmapIndex = String->Length;
            Final = TRUE;

        }

        StringLength = (USHORT)(BitmapIndex - PreviousBitmapIndex);

        AlignedMaxLength = ALIGN_UP(StringLength, 16);

        //
        // Fill out the destination string details.
        //

        DestString->Length = StringLength;
        DestString->MaximumLength = AlignedMaxLength;
        DestString->Buffer = DestBuffer;

        //
        // Copy the source string over.
        //

        __movsb(DestString->Buffer, Source, StringLength);

        //
        // Carve out the next destination buffer.
        //

        DestBuffer += AlignedMaxLength;

        //
        // Compute the CRC32 checksum and store in the hash field.
        //

        DestString->Hash = ComputeCrc32ForString(DestString);

        if (Final) {

            //
            // This was the last element.
            //

            break;

        }

        //
        // Update the previous bitmap index to the next clear bit.
        //

        PreviousBitmapIndex = RtlFindClearBits(Bitmap, 1, BitmapIndex + 1);

        //
        // Ensure our invariant loop logic is correct.
        //

        if (PreviousBitmapIndex < BitmapIndex ||
            PreviousBitmapIndex == BITS_NOT_FOUND) {

            goto Error;

        } else {

            ULONG Adjustment;

            //
            // Point the string buffer at the next non-delimited character.
            //

            Adjustment = (
                PreviousBitmapIndex - 
                (PreviousBitmapIndex - Length.LowPart)
            ) + 1;

            Source += Adjustment;

        }
    }

    //
    // We're done.
    //

    goto End;

Error:

    if (NewArray) {

        PVOID Address;

        //
        // If StringTable has a value here, it is the address that should be
        // freed, not NewArray.
        //

        Address = (PVOID)StringTable;
        if (!Address) {
            Address = (PVOID)NewArray;
        }

        Allocator->Free(Allocator->Context, Address);

        //
        // Clear both pointers.
        //

        StringTable = NULL;
        NewArray = NULL;

    }

    //
    // Intentional follow-on.
    //

End:

    //
    // Update the caller's StringTablePointer (which may be NULL if we didn't
    // allocate a StringTable) and return the StringArray.
    //

    *StringTablePointer = StringTable;


    if ((ULONG_PTR)StackBitmap.Buffer != (ULONG_PTR)Bitmap->Buffer) {

        //
        // We'll hit this point if a new bitmap had to be allocated because
        // our stack-allocated one was too small.  Make sure we free it here.
        //

        HeapFree(HeapHandle, 0, Bitmap->Buffer);
    }

    return NewArray;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
