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
    USHORT Index;
    USHORT Length;
    USHORT NumberOfStrings;
    ULONG OccupiedBitmap;
    ULONG ContinuationBitmap;
    PSTRING_TABLE StringTable;
    PSTRING_ARRAY StringArray;
    PSTRING String;
    PSTRING_SLOT Slot;
    SLOT_LENGTHS Lengths = { 0 };
    STRING_SLOT FirstChars = { 0 };

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

    Slot = StringTable->Slots-1;
    String = StringArray->Strings-1;

    OccupiedBitmap = 0;
    ContinuationBitmap = 0;
    NumberOfStrings = StringArray->NumberOfElements;

    for (Index = 0, ++String, ++Slot; Index < NumberOfStrings; Index++) {
        __m128i Octword;

        //
        // Set the occupied bit.
        //

        BitTestAndSet(&OccupiedBitmap, Index);

        //
        // Set the string length for the slot.
        //

        Length = Lengths.Slots[Index] = String->Length;

        //
        // If it's longer than 16 bytes, make a note of it in the
        // continuation bitmap.
        //

        if (Length > 16) {
            ContinuationBitmap |= (1 << Index);
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

        Octword = _mm_load_si128((__m128i *)String->Buffer);
        _mm_storeu_si128(&(*Slot).OctChars, Octword);

    }

    //
    // Store the slot lengths.
    //

    __try {

        _mm256_storeu_si256(&(StringTable->Lengths.OctSlots), Lengths.OctSlots);

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        //
        // Presume the exception is an illegal instruction fault because we
        // don't have AVX support, so just use two 128-bit moves.
        //

        _mm_storeu_si128(&(StringTable->Lengths.LowSlots), Lengths.LowSlots);
        _mm_storeu_si128(&(StringTable->Lengths.HighSlots), Lengths.HighSlots);

    }

    //
    // Store the first characters.
    //

    _mm_storeu_si128(&(StringTable->FirstChars.OctChars), FirstChars.OctChars);

    //
    // Store the occupied and continuation bitmaps.
    //

    StringTable->OccupiedBitmap = (USHORT)OccupiedBitmap;
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
