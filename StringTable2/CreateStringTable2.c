/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    CreateStringTable2.c

Abstract:

    This module implements the functionality to create a STRING_TABLE2 structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
PSTRING_TABLE2
CreateStringTable2(
    PALLOCATOR StringTable2Allocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_ARRAY StringArray,
    BOOL CopyArray
    )
/*++

Routine Description:

    Allocates space for a STRING_TABLE2 structure using the provided allocators,
    then initializes it using the provided STRING_ARRAY.  If CopyArray is set
    to TRUE, the routine will copy the string array such that the caller is
    free to destroy it after the table has been successfully created.  If it
    is set to FALSE and StringArray->StringTable2 has a non-NULL value, it is
    assumed that sufficient space has already been allocated for the string
    table and this pointer will be used to initialize the rest of the structure.

    The Allocator will be used for all memory allocations. DestroyStringTable2()
    must be called against the returned PSTRING_TABLE2 when the structure is no
    longer needed in order to ensure resources are released.

Arguments:

    StringTable2Allocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE2.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE2 structure.  This is kept separate from the
        StringTable2Allocator due to the stringent alignment requirements of the
        string table.

    StringArray - Supplies a pointer to an initialized STRING_ARRAY structure
        that contains the STRING structures that are to be added to the table.

    CopyArray - Supplies a boolean value indicating whether or not the
        StringArray structure should be deep-copied during creation.  This is
        typically set when the caller wants to be able to free the structure
        as soon as this call returns (or can't guarantee it will persist past
        this function's invocation, i.e. if it was stack allocated).

Return Value:

    A pointer to a valid PSTRING_TABLE2 structure on success, NULL on failure.
    Call DestroyStringTable2() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    USHORT NumberOfTables;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTable2Allocator)) {
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

        return CreateSingleStringTable2(StringTable2Allocator,
                                       StringArrayAllocator,
                                       StringArray,
                                       CopyArray);
    }

    return NULL;

}

_Use_decl_annotations_
PSTRING_TABLE2
CreateSingleStringTable2(
    PALLOCATOR StringTable2Allocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_ARRAY SourceStringArray,
    BOOL CopyArray
    )
/*++

Routine Description:

    This routine is an optimized version of CreateStringTable2() when the
    string array contains no more than 16 strings.  See the documentation
    for CreateStringTable2() for more information.

--*/
{
    USHORT Count;
    USHORT Index;
    USHORT Length;
    USHORT HighestBit;
    ULONG OccupiedBitmap;
    ULONG ContinuationBitmap;
    PSTRING_TABLE2 StringTable2;
    PSTRING_ARRAY StringArray;
    PSTRING String;
    PSTRING_SLOT Slot;
    STRING_SLOT FirstChars;
    SLOT_LENGTHS Lengths;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTable2Allocator)) {
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
            StringTable2Allocator,
            StringArrayAllocator,
            SourceStringArray,
            FIELD_OFFSET(STRING_TABLE2, StringArray),
            sizeof(STRING_TABLE2),
            &StringTable2
        );

        if (!StringArray) {
            return NULL;
        }

    } else {

        //
        // We're not copying the array, so initialize StringArray to point at
        // the caller's SourceStringArray, and StringTable2 to point at the
        // array's StringTable2 field (which will be non-NULL if sufficient
        // space has been allocated).
        //

        StringArray = SourceStringArray;
        StringTable2 = StringArray->StringTable2;

    }

    //
    // If StringTable2 has no value, we've either been called with CopyArray set
    // to FALSE, or CopyStringArray() wasn't able to allocate sufficient space
    // for both the table and itself.  Either way, we need to allocate space for
    // the table.
    //

    if (!StringTable2) {

        StringTable2 = (PSTRING_TABLE2)(
            StringTable2Allocator->Calloc(
                StringTable2Allocator->Context,
                1,
                sizeof(STRING_TABLE2)
            )
        );

        if (!StringTable2) {
            return NULL;
        }
    }

    //
    // Make sure the fields that are sensitive to alignment are, in fact,
    // aligned correctly.
    //

    if (!AssertStringTable2FieldAlignment(StringTable2)) {
        DestroyStringTable2(StringTable2Allocator,
                           StringArrayAllocator,
                           StringTable2);
        return NULL;
    }

    //
    // At this point, we have copied the incoming StringArray if necessary,
    // and we've allocated sufficient space for the StringTable2 structure.
    // Enumerate over all of the strings, set the continuation bit if the
    // length > 16, set the relevant slot length, set the relevant first
    // character entry, then move the first 16-bytes of the string into the
    // relevant slot via an aligned SSE mov.
    //

    //
    // Initialize pointers and counters, clear stack-based structures.
    //

    Slot = StringTable2->Slots;
    String = StringArray->Strings;

    Index = 0;
    OccupiedBitmap = 0;
    ContinuationBitmap = 0;
    Count = StringArray->NumberOfElements;
    FirstChars.CharsXmm = _mm_setzero_si128();

    //
    // Set all the slot lengths to 0x7ffff up front instead of defaulting
    // to zero.  This allows for simpler logic when searching for a prefix
    // string, which involves broadcasting a search string's length to a Ymm
    // register, then doing _mm256_cmpgt_epi16() against the lengths array and
    // the string length.  If we left the lengths as 0 for unused slots, they
    // would get included in the resulting comparison register (i.e. the high
    // bits would be set to 1), so we'd have to do a subsequent masking of
    // the result at some point using the OccupiedBitmap.  By defaulting the
    // lengths to MAX_SHORT (0x7ffff), we ensure they'll never get included in
    // any cmpgt-type SIMD matches.  (We use 0x7fff instead of 0xffff because
    // the _mm256_cmpgt_epi16() intrinsic assumes packed signed integers.)
    //

    Lengths.SlotsYmm = _mm256_set1_epi16(0x7fff);

    do {

        XMMWORD CharsXmm;

        //
        // Set the string length for the slot.
        //

        Length = Lengths.Slots[Index] = String->Length;

        //
        // Set the appropriate bit in the continuation bitmap if the string is
        // longer than 16 bytes.
        //

        if (Length > 16) {
            ContinuationBitmap |= (1 << (Index+1));
        }

        //
        // Save the first character of the string.
        //

        FirstChars.Char[Index] = String->Buffer[0];

        //
        // Copy the first 16-bytes of the string into the relevant slot.  We
        // have taken care to ensure everything is 16-byte aligned by this
        // stage, so we can use SSE intrinsics here.
        //

        CharsXmm = _mm_load_si128((PXMMWORD)String->Buffer);
        _mm_store_si128(&(*Slot).CharsXmm, CharsXmm);

        ++Index;

        //
        // Advance our pointers.
        //

        ++Slot;
        ++String;

    } while (--Count);

    //
    // Store the slot lengths.
    //

    _mm256_store_si256(&(StringTable2->Lengths.SlotsYmm), Lengths.SlotsYmm);

    //
    // Store the first characters.
    //

    _mm_store_si128(&(StringTable2->FirstChars.CharsXmm), FirstChars.CharsXmm);

    //
    // Generate and store the occupied bitmap.  Each bit, from low to high,
    // corresponds to the index of a slot.  When set, the slot is occupied.
    // When clear, it is not.  So, fill bits from the highest bit set down.
    //

    HighestBit = (1 << (Index-1));
    StringTable2->OccupiedBitmap = (USHORT)_blsmsk_u32(HighestBit);

    //
    // Store the continuation bitmap.
    //

    StringTable2->ContinuationBitmap = (USHORT)(ContinuationBitmap);

    //
    // Wire up the string array to the table.
    //

    StringTable2->pStringArray = StringArray;

    //
    // Initialize the IsPrefixOfStringInTable and DestroyStringTable2 function
    // pointers.
    //

    StringTable2->IsPrefixOfStringInTable = IsPrefixOfStringInSingleTableInline;
    StringTable2->DestroyStringTable2 = DestroyStringTable2;

    //
    // And we're done, return the table.
    //

    return StringTable2;
}

_Use_decl_annotations_
PSTRING_TABLE2
CreateStringTable2FromDelimitedString(
    PRTL Rtl,
    PALLOCATOR StringTable2Allocator,
    PALLOCATOR StringArrayAllocator,
    PCSTRING String,
    CHAR Delimiter
    )
/*++

Routine Description:

    This routine creates a new STRING_TABLE2 structure from a STRING_ARRAY
    structure that is created from a STRING structure, whose underlying
    character buffer is delimited by a Delimiter character, such as a space,
    colon, comma, etc.

    No reference will be kept to the String pointer or backing character buffer,
    so the caller is free to destroy it after this call completes.  That is,
    this routine will copy any data it needs during this call.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    StringTable2Allocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE2.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE2 structure.  This is kept separate from the
        StringTable2Allocator due to the stringent alignment requirements of the
        string table.

    String - Supplies a pointer to a STRING structure that represents the
        delimited string to construct a STRING_ARRAY and then subsequent
        STRING_TABLE2 from.

    Delimiter - Supplies the character that delimits individual elements of
        the String pointer (e.g. ':', ' ').

Return Value:

    A pointer to a valid PSTRING_TABLE2 structure on success, NULL on failure.
    Call DestroyStringTable2() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    PVOID Address;
    PSTRING_ARRAY StringArray;
    PSTRING_TABLE2 StringTable2;
    PSTRING_TABLE2 NewTable;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringTable2Allocator)) {
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
        StringTable2Allocator,
        StringArrayAllocator,
        String,
        Delimiter,
        FIELD_OFFSET(STRING_TABLE2, StringArray),
        sizeof(STRING_TABLE2),
        &StringTable2
    );

    if (!StringArray) {
        return NULL;
    }

    //
    // Create the string table from the newly-created string array.
    //

    NewTable = CreateStringTable2(StringTable2Allocator,
                                 StringArrayAllocator,
                                 StringArray,
                                 FALSE);

    if (!NewTable) {

        //
        // If StringTable2 has a value here, it is the address that should be
        // freed, not StringArray.
        //

        Address = (PVOID)StringTable2;
        if (Address) {
            StringTable2Allocator->Free(StringTable2Allocator->Context, Address);
        } else {
            Address = (PVOID)StringArray;
            StringArrayAllocator->Free(StringArrayAllocator->Context, Address);
        }
    }

    return NewTable;
}

_Use_decl_annotations_
PSTRING_TABLE2
CreateStringTable2FromDelimitedEnvironmentVariable(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PALLOCATOR StringTable2Allocator,
    PALLOCATOR StringArrayAllocator,
    PCSTR EnvironmentVariableName,
    CHAR Delimiter
    )
/*++

Routine Description:

    This routine creates a new STRING_TABLE2 structure by creating a new
    STRING_ARRAY structure from the delimited string value in the given
    environment variable name.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure which will be used
        to allocate temporary memory for reading the contents of the environment
        variable.

    StringTable2Allocator - Supplies a pointer to an ALLOCATOR structure which
        will be used for creating the STRING_TABLE2.  This allocator is kept
        separate from the one above due to the fact that string tables have
        stringent alignment requirements and will be using custom allocators
        that wouldn't necessarily be suitable for the temp buffer created for
        the environment variable's value.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure which
        may be used to create the STRING_ARRAY if it cannot fit within the
        padding of the STRING_TABLE2 structure.  This is kept separate from the
        StringTable2Allocator due to the stringent alignment requirements of the
        string table.

    EnvironmentVariableName - Supplies a pointer to a NULL-terminated string
        representing the name of the environment variable to create a string
        table from.

    Delimiter - Supplies the character that delimits individual elements of
        the environment variable's value (e.g. ';', ' ', etc).

Return Value:

    A pointer to a valid PSTRING_TABLE2 structure on success, NULL on failure.
    Call DestroyStringTable2() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    USHORT AlignedNumberOfCharacters;
    PCSTR Name;
    LONG Length;
    STRING String;
    LONG_INTEGER NumberOfChars;
    PSTRING_TABLE2 StringTable2 = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringTable2Allocator)) {
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
    // Pass the rest of the work over to CreateStringTable2FromDelimitedString().
    //

    StringTable2 = CreateStringTable2FromDelimitedString(
        Rtl,
        StringTable2Allocator,
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

    return StringTable2;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
