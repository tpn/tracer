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
    PALLOCATOR Allocator,
    PSTRING_ARRAY StringArray
    )
/*++

Routine Description:

    Allocates space for a STRING_TABLE structure using the provided Allocator,
    then initializes it using the provided STRING_ARRAY.  The routine will copy
    any string data it needs from the STRING_ARRAY, so the caller is free to
    destroy that structure after the table has been successfully created.

    The Allocator will be used for all memory allocations. DestroyStringTable()
    must be called against the returned PSTRING_TABLE when the structure is no
    longer needed in order to ensure resources are released.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to allocate all memory required by the structure during its
        creation.

    StringArray - Supplies a pointer to an initialized STRING_ARRAY structure
        that contains the STRING structures that are to be added to the table.

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

    if (!ARGUMENT_PRESENT(Allocator)) {
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
        return CreateSingleStringTable(Allocator, StringArray);
    }

    return NULL;

}

_Use_decl_annotations_
PSTRING_TABLE
CreateSingleStringTable(
    PALLOCATOR Allocator,
    PSTRING_ARRAY SourceStringArray
    )
/*++

Routine Description:

    This routine is an optimized version of CreateStringTable() when the
    string array contains no more than 16 strings.  See the documentation
    for CreateStringTable() for more information.

--*/
{
    USHORT Count;
    USHORT Index;
    USHORT Length;
    USHORT HighestBit;
    ULONG OccupiedBitmap;
    ULONG ContinuationBitmap;
    PSTRING_TABLE StringTable;
    PSTRING_ARRAY StringArray;
    PSTRING String;
    PSTRING_SLOT Slot;
    STRING_SLOT FirstChars;
    SLOT_LENGTHS Lengths;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(SourceStringArray)) {
        return NULL;
    }

    if (SourceStringArray->NumberOfElements == 0) {
        return NULL;
    }

    //
    // Copy the incoming string array.
    //

    StringArray = CopyStringArray(
        Allocator,
        SourceStringArray,
        FIELD_OFFSET(STRING_TABLE, StringArray),
        sizeof(STRING_TABLE),
        &StringTable
    );

    if (!StringArray) {
        return NULL;
    }

    //
    // If StringTable has no value, CopyStringArray() wasn't able to allocate
    // sufficient space for both the table and itself, so, we need to allocate
    // new space ourselves.
    //

    if (!StringTable) {

        StringTable = (PSTRING_TABLE)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                sizeof(STRING_TABLE)
            )
        );

        if (!StringTable) {
            return NULL;
        }
    }

    //
    // Make sure the fields that are sensitive to alignment are in fact aligned
    // correctly.
    //

    if (!AssertStringTableFieldAlignment(StringTable)) {
        DestroyStringTable(Allocator, StringTable);
        return NULL;
    }

    //
    // At this point, we have copied the incoming StringArray, with each string
    // buffer aligned to 16-bytes, and we've allocated sufficient space for the
    // StringTable structure.  Enumerate over all of the strings, set the
    // continuation bit if the length > 16, set the relevant slot length, set
    // the relevant first character entry, then move the first 16-bytes of the
    // string into the relevant slot via an aligned SSE mov.
    //

    //
    // Set the pointers to one before their offset.  Initialize counters and
    // clear stack-based structures.
    //

    Slot = StringTable->Slots;
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

        CharsXmm = _mm_load_si128((__m128i *)String->Buffer);
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

    _mm256_store_si256(&(StringTable->Lengths.SlotsYmm), Lengths.SlotsYmm);

    //
    // Store the first characters.
    //

    _mm_store_si128(&(StringTable->FirstChars.CharsXmm), FirstChars.CharsXmm);

    //
    // Generate and store the occupied bitmap.  Each bit, from low to high,
    // corresponds to the index of a slot.  When set, the slot is occupied.
    // When clear, it is not.  So, fill bits from the highest bit set down.
    //

    HighestBit = (1 << (Index-1));
    StringTable->OccupiedBitmap = (USHORT)_blsmsk_u32(HighestBit);

    //
    // Store the continuation bitmap.
    //

    StringTable->ContinuationBitmap = (USHORT)(ContinuationBitmap);

    //
    // Wire up the string array to the table.
    //

    StringTable->pStringArray = StringArray;

    //
    // Set the IsPrefixOfStringInTable function to our C function.
    //

    StringTable->IsPrefixOfStringInTable = IsPrefixOfStringInSingleTable_C;

    //
    // And we're done, return the table.
    //

    return StringTable;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
