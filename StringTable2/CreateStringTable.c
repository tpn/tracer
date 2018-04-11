/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    CreateStringTable.c

Abstract:

    This module implements the functionality to create a STRING_TABLE structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
PSTRING_TABLE
CreateStringTable(
    PRTL Rtl,
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_ARRAY StringArray,
    BOOL CopyArray
    )
/*++

Routine Description:

    Allocates space for a STRING_TABLE structure using the provided allocators,
    then initializes it using the provided STRING_ARRAY.  If CopyArray is set
    to TRUE, the routine will copy the string array such that the caller is
    free to destroy it after the table has been successfully created.  If it
    is set to FALSE and StringArray->StringTable has a non-NULL value, it is
    assumed that sufficient space has already been allocated for the string
    table and this pointer will be used to initialize the rest of the structure.

    The Allocator will be used for all memory allocations. DestroyStringTable()
    must be called against the returned PSTRING_TABLE when the structure is no
    longer needed in order to ensure resources are released.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    StringTableAllocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE structure.  This is kept separate from the
        StringTableAllocator due to the stringent alignment requirements of the
        string table.

    StringArray - Supplies a pointer to an initialized STRING_ARRAY structure
        that contains the STRING structures that are to be added to the table.

    CopyArray - Supplies a boolean value indicating whether or not the
        StringArray structure should be deep-copied during creation.  This is
        typically set when the caller wants to be able to free the structure
        as soon as this call returns (or can't guarantee it will persist past
        this function's invocation, i.e. if it was stack allocated).

Return Value:

    A pointer to a valid PSTRING_TABLE structure on success, NULL on failure.
    Call DestroyStringTable() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    USHORT NumberOfTables;

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

    //
    // Determine how many tables we're going to need.
    //

    NumberOfTables = GetNumberOfTablesRequiredForStringArray(StringArray);

    //
    // (We only support a single table for now.)
    //

    if (NumberOfTables == 1) {

        return CreateSingleStringTable(Rtl,
                                       StringTableAllocator,
                                       StringArrayAllocator,
                                       StringArray,
                                       CopyArray);
    }

    return NULL;

}

_Use_decl_annotations_
PSTRING_TABLE
CreateSingleStringTable(
    PRTL Rtl,
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_ARRAY SourceStringArray,
    BOOL CopyArray
    )
/*++

Routine Description:

    This routine is an optimized version of CreateStringTable() when the
    string array contains no more than 16 strings.  See the documentation
    for CreateStringTable() for more information.

--*/
{
    BYTE Byte;
    BYTE Count;
    BYTE Index;
    BYTE Length;
    BYTE NumberOfElements;
    ULONG HighestBit;
    ULONG OccupiedMask;
    PULONG Bits;
    USHORT OccupiedBitmap;
    USHORT ContinuationBitmap;
    PSTRING_TABLE StringTable;
    PSTRING_ARRAY StringArray;
    PSTRING String;
    PSTRING_SLOT Slot;
    STRING_SLOT UniqueChars;
    SLOT_INDEX UniqueIndex;
    SLOT_INDEX LengthIndex;
    SLOT_LENGTHS Lengths;
    LENGTH_INDEX_TABLE LengthIndexTable;
    PCHARACTER_BITMAP Bitmap;
    SLOT_BITMAPS SlotBitmaps;
    PLENGTH_INDEX_ENTRY Entry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTableAllocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(SourceStringArray)) {
        return NULL;
    }

    if (SourceStringArray->NumberOfElements == 0) {
        return NULL;
    }

    //
    // Copy the incoming string array if applicable.
    //

    if (CopyArray) {

        StringArray = CopyStringArray(
            StringTableAllocator,
            StringArrayAllocator,
            SourceStringArray,
            FIELD_OFFSET(STRING_TABLE, StringArray),
            sizeof(STRING_TABLE),
            &StringTable
        );

        if (!StringArray) {
            return NULL;
        }

    } else {

        //
        // We're not copying the array, so initialize StringArray to point at
        // the caller's SourceStringArray, and StringTable to point at the
        // array's StringTable field (which will be non-NULL if sufficient
        // space has been allocated).
        //

        StringArray = SourceStringArray;
        StringTable = StringArray->StringTable;

    }

    //
    // If StringTable has no value, we've either been called with CopyArray set
    // to FALSE, or CopyStringArray() wasn't able to allocate sufficient space
    // for both the table and itself.  Either way, we need to allocate space for
    // the table.
    //

    if (!StringTable) {

        StringTable = (PSTRING_TABLE)(
            StringTableAllocator->AlignedCalloc(
                StringTableAllocator->Context,
                1,
                sizeof(STRING_TABLE),
                STRING_TABLE_ALIGNMENT
            )
        );

        if (!StringTable) {
            return NULL;
        }
    }

    //
    // Make sure the fields that are sensitive to alignment are, in fact,
    // aligned correctly.
    //

    if (!AssertStringTableFieldAlignment(StringTable)) {
        DestroyStringTable(StringTableAllocator,
                           StringArrayAllocator,
                           StringTable);
        return NULL;
    }

    //
    // At this point, we have copied the incoming StringArray if necessary,
    // and we've allocated sufficient space for the StringTable structure.
    // Enumerate over all of the strings, set the continuation bit if the
    // length > 16, set the relevant slot length, set the relevant unique
    // character entry, then move the first 16-bytes of the string into the
    // relevant slot via an aligned SSE mov.
    //

    //
    // Initialize pointers and counters, clear stack-based structures.
    //

    Slot = StringTable->Slots;
    String = StringArray->Strings;

    OccupiedBitmap = 0;
    ContinuationBitmap = 0;
    NumberOfElements = (BYTE)StringArray->NumberOfElements;
    UniqueChars.CharsXmm = _mm_setzero_si128();
    UniqueIndex.IndexXmm = _mm_setzero_si128();
    LengthIndex.IndexXmm = _mm_setzero_si128();

    //
    // Set all the slot lengths to 0x7f up front instead of defaulting
    // to zero.  This allows for simpler logic when searching for a prefix
    // string, which involves broadcasting a search string's length to a Xmm
    // register, then doing _mm_cmpgt_epi8() against the lengths array and
    // the string length.  If we left the lengths as 0 for unused slots, they
    // would get included in the resulting comparison register (i.e. the high
    // bits would be set to 1), so we'd have to do a subsequent masking of
    // the result at some point using the OccupiedBitmap.  By defaulting the
    // lengths to 0x7f, we ensure they'll never get included in any cmpgt-type
    // SIMD matches.  (We use 0x7f instead of 0xff because the _mm_cmpgt_epi8()
    // intrinsic assumes packed signed integers.)
    //

    Lengths.SlotsXmm = _mm_set1_epi8(0x7f);

    ZeroStruct(LengthIndexTable);
    ZeroStruct(SlotBitmaps);

    for (Count = 0; Count < NumberOfElements; Count++) {

        XMMWORD CharsXmm;

        //
        // Set the string length for the slot.
        //

        Length = Lengths.Slots[Count] = (BYTE)String->Length;

        //
        // Set the appropriate bit in the continuation bitmap if the string is
        // longer than 16 bytes.
        //

        if (Length > 16) {
            ContinuationBitmap |= (Count == 0 ? 1 : 1 << (Count + 1));
        }

        if (Count == 0) {

            Entry = &LengthIndexTable.Entry[0];
            Entry->Index = 0;
            Entry->Length = Length;

        } else {

            //
            // Perform a linear scan of the length-index table in order to
            // identify an appropriate insertion point.
            //

            for (Index = 0; Index < Count; Index++) {
                if (Length < LengthIndexTable.Entry[Index].Length) {
                    break;
                }
            }

            if (Index != Count) {

                //
                // New entry doesn't go at the end of the table, so shuffle
                // everything else down.
                //

                Rtl->RtlMoveMemory(&LengthIndexTable.Entry[Index + 1],
                                   &LengthIndexTable.Entry[Index],
                                   (Count - Index) * sizeof(*Entry));
            }

            Entry = &LengthIndexTable.Entry[Index];
            Entry->Index = Count;
            Entry->Length = Length;
        }

        //
        // Copy the first 16-bytes of the string into the relevant slot.  We
        // have taken care to ensure everything is 16-byte aligned by this
        // stage, so we can use SSE intrinsics here.
        //

        CharsXmm = _mm_load_si128((PXMMWORD)String->Buffer);
        _mm_store_si128(&(*Slot).CharsXmm, CharsXmm);

        //
        // Advance our pointers.
        //

        ++Slot;
        ++String;

    }

    //
    // Store the slot lengths.
    //

    _mm_store_si128(&(StringTable->Lengths.SlotsXmm), Lengths.SlotsXmm);

    //
    // Loop through the strings in order of shortest to longest and construct
    // the uniquely-identifying character table with corresponding index.
    //


    for (Count = 0; Count < NumberOfElements; Count++) {
        Entry = &LengthIndexTable.Entry[Count];
        Length = Entry->Length;
        Slot = &StringTable->Slots[Entry->Index];

        //
	// Iterate over each character in the slot and find the first one
	// without a corresponding bit set.
        //

        for (Index = 0; Index < Length; Index++) {
            Bitmap = &SlotBitmaps.Bitmap[Index];
            Bits = (PULONG)&Bitmap->Bits[0];
            Byte = Slot->Char[Index];
            if (!BitTestAndSet(Bits, Byte)) {
                break;
            }
        }

        UniqueChars.Char[Count] = Byte;
        UniqueIndex.Index[Count] = Index;
        LengthIndex.Index[Count] = Entry->Index;
    }

    //
    // Loop through the elements again such that the unique chars are stored
    // in the order they appear in the table.
    //

    for (Count = 0; Count < NumberOfElements; Count++) {
        for (Index = 0; Index < NumberOfElements; Index++) {
            if (LengthIndex.Index[Index] == Count) {
                StringTable->UniqueChars.Char[Count] = UniqueChars.Char[Index];
                StringTable->UniqueIndex.Index[Count] = UniqueIndex.Index[Index];
                break;
            }
        }
    }

    //
    // Generate and store the occupied bitmap.  Each bit, from low to high,
    // corresponds to the index of a slot.  When set, the slot is occupied.
    // When clear, it is not.  So, fill bits from the highest bit set down.
    //

    HighestBit = (1 << (StringArray->NumberOfElements-1));
    OccupiedMask = _blsmsk_u32(HighestBit);
    StringTable->OccupiedBitmap = (USHORT)OccupiedMask;

    //
    // Store the continuation bitmap.
    //

    StringTable->ContinuationBitmap = (USHORT)(ContinuationBitmap);

    //
    // Wire up the string array to the table.
    //

    StringTable->pStringArray = StringArray;

    //
    // And we're done, return the table.
    //

    return StringTable;
}

_Use_decl_annotations_
PSTRING_TABLE
CreateStringTableFromDelimitedString(
    PRTL Rtl,
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PCSTRING String,
    CHAR Delimiter
    )
/*++

Routine Description:

    This routine creates a new STRING_TABLE structure from a STRING_ARRAY
    structure that is created from a STRING structure, whose underlying
    character buffer is delimited by a Delimiter character, such as a space,
    colon, comma, etc.

    No reference will be kept to the String pointer or backing character buffer,
    so the caller is free to destroy it after this call completes.  That is,
    this routine will copy any data it needs during this call.

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
        delimited string to construct a STRING_ARRAY and then subsequent
        STRING_TABLE from.

    Delimiter - Supplies the character that delimits individual elements of
        the String pointer (e.g. ':', ' ').

Return Value:

    A pointer to a valid PSTRING_TABLE structure on success, NULL on failure.
    Call DestroyStringTable() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    PVOID Address;
    PSTRING_ARRAY StringArray;
    PSTRING_TABLE StringTable;
    PSTRING_TABLE NewTable;

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

    //
    // Create the string array from the delimited string.
    //

    StringArray = CreateStringArrayFromDelimitedString(
        Rtl,
        StringTableAllocator,
        StringArrayAllocator,
        String,
        Delimiter,
        FIELD_OFFSET(STRING_TABLE, StringArray),
        sizeof(STRING_TABLE),
        &StringTable
    );

    if (!StringArray) {
        return NULL;
    }

    //
    // Create the string table from the newly-created string array.
    //

    NewTable = CreateStringTable(Rtl,
                                 StringTableAllocator,
                                 StringArrayAllocator,
                                 StringArray,
                                 FALSE);

    if (!NewTable) {

        //
        // If StringTable has a value here, it is the address that should be
        // freed, not StringArray.
        //

        Address = (PVOID)StringTable;
        if (Address) {
            StringTableAllocator->AlignedFree(StringTableAllocator->Context,
                                              Address);
        } else {
            Address = (PVOID)StringArray;
            StringArrayAllocator->Free(StringArrayAllocator->Context, Address);
        }
    }

    return NewTable;
}

_Use_decl_annotations_
PSTRING_TABLE
CreateStringTableFromDelimitedEnvironmentVariable(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PCSTR EnvironmentVariableName,
    CHAR Delimiter
    )
/*++

Routine Description:

    This routine creates a new STRING_TABLE structure by creating a new
    STRING_ARRAY structure from the delimited string value in the given
    environment variable name.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure which will be used
        to allocate temporary memory for reading the contents of the environment
        variable.

    StringTableAllocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE.  This allocator is kept
        separate from the one above due to the fact that string tables have
        stringent alignment requirements and will be using custom allocators
        that wouldn't necessarily be suitable for the temp buffer created for
        the environment variable's value.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE structure.  This is kept separate from the
        StringTableAllocator due to the stringent alignment requirements of the
        string table.

    EnvironmentVariableName - Supplies a pointer to a NULL-terminated string
        representing the name of the environment variable to create a string
        table from.

    Delimiter - Supplies the character that delimits individual elements of
        the environment variable's value (e.g. ';', ' ', etc).

Return Value:

    A pointer to a valid PSTRING_TABLE structure on success, NULL on failure.
    Call DestroyStringTable() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    USHORT AlignedNumberOfCharacters;
    PCSTR Name;
    LONG Length;
    STRING String;
    LONG_INTEGER NumberOfChars;
    PSTRING_TABLE StringTable = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringTableAllocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
        return NULL;
    }

    //
    // Get the number of characters required to store the environment variable's
    // value, including the trailing NULL byte.
    //

    Name = EnvironmentVariableName;
    Length = GetEnvironmentVariableA(Name, NULL, 0);

    if (Length == 0) {
        return NULL;
    }

    //
    // Remove the trailing NULL.
    //

    NumberOfChars.LongPart = Length - 1;

    //
    // Sanity check it's not longer than MAX_USHORT.
    //

    if (NumberOfChars.HighPart) {
        return NULL;
    }

    //
    // Align number of characters to a pointer boundary and account for the
    // additional byte required for the trailing NULL.
    //

    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfChars.LowPart + 1
        )
    );

    //
    // Allocate a buffer.
    //

    String.Buffer = (PCHAR)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedNumberOfCharacters
        )
    );

    if (!String.Buffer) {
        return NULL;
    }

    //
    // Initialize the string lengths.
    //

    String.Length = NumberOfChars.LowPart;
    String.MaximumLength = AlignedNumberOfCharacters;

    //
    // Call GetEnvironmentVariableA() again with the buffer to retrieve the
    // contents.
    //

    Length = GetEnvironmentVariableA(Name, String.Buffer, String.MaximumLength);

    if (Length != String.Length) {

        //
        // We failed to copy the expected number of bytes.
        //

        goto End;
    }

    //
    // Pass the rest of the work over to CreateStringTableFromDelimitedString().
    //

    StringTable = CreateStringTableFromDelimitedString(
        Rtl,
        StringTableAllocator,
        StringArrayAllocator,
        &String,
        Delimiter
    );

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Free our temporary string buffer that captured the environment variable
    // value.
    //

    Allocator->Free(Allocator->Context, String.Buffer);

    return StringTable;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
