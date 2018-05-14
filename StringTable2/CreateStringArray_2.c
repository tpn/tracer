/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

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
CreateStringArrayFromDelimitedString_2(
    PRTL Rtl,
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PCSTRING String,
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

    This version differs from version 1 in that the Rtl bitmap routines are
    not used for string processing.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    StringTableAllocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE structure.  This is kept separate from the
        StringTableAllocator due to the stringent alignment requirements of the
        string table.

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
    BOOLEAN Final;
    BOOLEAN IsSuperfluous;

    USHORT Index;
    USHORT Count;
    USHORT StringLength;
    USHORT MinimumLength;
    USHORT MaximumLength;
    USHORT NumberOfElements;
    USHORT AlignedMaxLength;
    USHORT AlignedStringTablePaddingOffset;
    USHORT StringTableRemainingSpace;
    USHORT FinalLength;

    ULONG AlignedSize;
    ULONG StringBufferAllocSize;
    ULONG TotalAllocSize;
    ULONG BufferOffset;

    ULONG AlignedBufferOffset;
    ULONG PaddingSize;
    ULONG StringElementsSize;

    ULONG_INTEGER Length;

    PCHAR Char;
    PCHAR Start;
    PCHAR FinalStart;
    PCHAR PreviousDelimiter;
    PCHAR DestBuffer;

    PSTRING DestString;

    PSTRING_TABLE StringTable = NULL;
    PSTRING_ARRAY NewArray = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringTableAllocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return NULL;
    }

    if (String->Length == 0) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringTablePointer)) {
        return NULL;
    }

    //
    // Initialize variables before the loop.
    //

    StringBufferAllocSize = 0;
    MinimumLength = (USHORT)-1;
    MaximumLength = 0;
    Count = 0;

    Start = String->Buffer;
    PreviousDelimiter = NULL;
    Length.LongPart = 0;

    //
    // Enumerate over the string buffer, looking for delimiters, such that
    // individual beginning and end markers for each string can be ascertained.
    // Use this information to calculate the total allocation size required to
    // store a 16-byte aligned copy of each string.
    //

    Final = FALSE;

    for (Index = 0; Index < String->Length; Index++) {

        //
        // Resolve the character for this offset.
        //

        Char = &String->Buffer[Index];

        //
        // If it's not a delimiter character, continue the search.
        //

        if (*Char != Delimiter) {
            continue;
        }

        //
        // We've found a delimiter character, and need to determine if it's
        // depicting the valid end point of string, or if it's superfluous.
        // A superfluous delimiter is one that is at the very start of the
        // string, or one that immediately follows a previous delimiter (i.e.
        // "foo;;bar").
        //

        IsSuperfluous = (
            Char == Start || (
                PreviousDelimiter &&
                (Char - sizeof(*Char)) == PreviousDelimiter
            )
        );

        if (IsSuperfluous) {

            //
            // The delimiter is superfluous.  Mark it as our previous delimiter
            // and continue the loop.
            //

            PreviousDelimiter = Char;
            continue;
        }

        //
        // We've found a seemingly valid delimiter.  We need to determine where
        // the string started, which we can ascertain from either the start of
        // the string (e.g. String->Buffer) if the previous delimiter is null,
        // or previous delimiter + sizeof(*Char) if it is not null.
        //

        if (!PreviousDelimiter) {

            //
            // We initialize Start to String->Buffer prior to entering the loop,
            // so we do not need to do anything here.
            //

            NOTHING;

        } else {

            //
            // Otherwise, assume the start is the character after the last
            // delimiter we saw.
            //

            Start = PreviousDelimiter + sizeof(*Char);

        }

        //
        // Calculate the length for this string element.
        //

        Length.LongPart = (ULONG)((ULONG_PTR)Char - (ULONG_PTR)Start);

        //
        // Sanity check the size.
        //

        ASSERT(!Length.HighPart);

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
        // Increment our counter.
        //

        Count++;

        //
        // Update the previous delimiter to point at this delimiter.
        //

        PreviousDelimiter = Char;

    }

    //
    // We've finished enumerating all the characters in the string buffer.  The
    // delimited input string is not required to terminate the final string
    // element with a delimiter, thus, we need to check if there's a final
    // string here.
    //
    // There's a final string if any of the following conditions hold true:
    //
    //  - If previous delimiter is null (i.e. no delimiters seen).
    //  - Else, if previous delimiter was not final character of string.
    //

    Final = FALSE;

    if (!PreviousDelimiter) {

        //
        // No delimiter was seen, so the "delimited string" passed to us on
        // input was simply a single string.  If this is the case, then there's
        // an invariant that count should be 0 at this point.  Assert this now.
        //

        ASSERT(Count == 0);

        //
        // Toggle the final flag to true.  Start will already be set to the
        // value of String->Buffer.  Assert this now.
        //

        Final = TRUE;
        ASSERT(Start == String->Buffer);

    } else if (Char != PreviousDelimiter) {

        //
        // The string didn't end with a delimiter, which means a final string
        // will be present, e.g. the "bar" string in "foo;bar".  Toggle the
        // final flag to true and set the start pointer to the character after
        // the previous delimiter.
        //

        Final = TRUE;
        Start = PreviousDelimiter + sizeof(*Char);

    }

    //
    // Check to see if the final flag was set and process accordingly.
    //

    if (Final) {

        //
        // N.B. The remaining logic is identical to how a match is handled in
        //      the loop above.
        //

        //
        // Calculate the length for this string element.
        //

        Length.LongPart = (ULONG)(
            ((ULONG_PTR)(String->Buffer + String->Length)) -
            ((ULONG_PTR)Start)
        );

        //
        // Sanity check the size.
        //

        ASSERT(!Length.HighPart);

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

        //
        // Update the final start pointer and length, simplifying our life
        // later in the routine.
        //

        FinalStart = Start;
        FinalLength = Length.LowPart;
    }

    //
    // We've finished the initial pre-processing of the string, identifying the
    // number of underlying elements and the required allocation sizes.  Verify
    // the count; if it's 0, the user has failed to provide a valid input
    // string (e.g. ";;;;;;;;;").
    //

    if (!Count) {
        goto End;
    }

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

            goto Error;

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
    // Initialize destination string pointer.  As the STRING structures are
    // contiguous, we can just bump this pointer (e.g. DestString++) to advance
    // to the next record whilst carving out individual string elements.
    //

    DestString = NewArray->Strings;

    //
    // Initialize the destination buffer to the point after the new STRING_ARRAY
    // struct and trailing array of actual STRING structs.  We then carve out
    // new buffer pointers for the destination strings as we loop through the
    // source string array and copy strings over.
    //

    DestBuffer = (PCHAR)(RtlOffsetToPointer(NewArray, BufferOffset));

    //
    // Reset the count.  We can perform an invariant test at the end of this
    // loop to ensure we carved out the number of strings we were expecting to.
    //

    Count = 0;

    //
    // Reset other variables prior to loop entry.
    //

    Start = String->Buffer;
    PreviousDelimiter = NULL;
    Length.LongPart = 0;

    //
    // Enumerate over the string buffer for a second time, carving out the
    // relevant STRING structures as we detect delimiters.  The general logic
    // here for identifying each string element is identical to the first loop.
    //


    for (Index = 0; Index < String->Length; Index++) {

        //
        // Resolve the character for this offset.
        //

        Char = &String->Buffer[Index];

        //
        // If it's not a delimiter character, continue the search.
        //

        if (*Char != Delimiter) {
            continue;
        }

        //
        // We've found a delimiter character, and need to determine if it's
        // depicting the valid end point of string, or if it's superfluous.
        // A superfluous delimiter is one that is at the very start of the
        // string, or one that immediately follows a previous delimiter (i.e.
        // "foo;;bar").
        //

        IsSuperfluous = (
            Char == Start || (
                PreviousDelimiter &&
                (Char - sizeof(*Char)) == PreviousDelimiter
            )
        );

        if (IsSuperfluous) {

            //
            // The delimiter is superfluous.  Mark it as our previous delimiter
            // and continue the loop.
            //

            PreviousDelimiter = Char;
            continue;
        }

        //
        // We've found a seemingly valid delimiter.  We need to determine where
        // the string started, which we can ascertain from either the start of
        // the string (e.g. String->Buffer) if the previous delimiter is null,
        // or previous delimiter + sizeof(*Char) if it is not null.
        //

        if (!PreviousDelimiter) {

            //
            // We initialize Start to String->Buffer prior to entering the loop,
            // so we do not need to do anything here.
            //

            NOTHING;

        } else {

            //
            // Otherwise, assume the start is the character after the last
            // delimiter we saw.
            //

            Start = PreviousDelimiter + sizeof(*Char);

        }

        //
        // Calculate the length for this string element.
        //

        StringLength = (USHORT)((ULONG_PTR)Char - (ULONG_PTR)Start);

        //
        // Calculate the aligned length.
        //

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

        CopyMemory(DestString->Buffer, Start, StringLength);

        //
        // Carve out the next destination buffer.
        //

        DestBuffer += AlignedMaxLength;

        //
        // Compute the CRC32 checksum and store in the hash field.
        //

        DestString->Hash = ComputeCrc32ForString(DestString);

        //
        // Increment our count and advance our DestString pointer.
        //

        Count++;
        DestString++;

        //
        // Update the previous delimiter to point at this delimiter.
        //

        PreviousDelimiter = Char;

    }

    //
    // Check to see if the final flag was set and process accordingly.
    //

    if (Final) {

        //
        // N.B. We can use the values we saved earlier for this final string
        //

        Start = FinalStart;
        StringLength = FinalLength;

        //
        // N.B. Remaining logic is identical to how a match is handled in
        //      the loop above.
        //

        //
        // Calculate the aligned length.
        //

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

        CopyMemory(DestString->Buffer, Start, StringLength);

        //
        // Compute the CRC32 checksum and store in the hash field.
        //

        DestString->Hash = ComputeCrc32ForString(DestString);

        //
        // Increment our count.
        //

        Count++;
    }

    //
    // Invariant check: our count should equal the number of elements we
    // captured earlier.
    //

    ASSERT(Count == NumberOfElements);

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
        if (Address) {
            StringTableAllocator->AlignedFree(StringTableAllocator->Context,
                                              Address);
        } else {
            Address = (PVOID)NewArray;
            StringArrayAllocator->Free(StringArrayAllocator->Context, Address);
        }

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
    // allocate a StringTable or encountered an error).
    //

    *StringTablePointer = StringTable;

    //
    // Return the new string array.
    //

    return NewArray;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
