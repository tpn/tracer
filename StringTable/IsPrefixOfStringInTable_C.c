/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    IsPrefixOfStringInTable_C.c

Abstract:

    This module implements an IsPrefixOfStringInTable function in C.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
IsPrefixOfStringInSingleTable_C(
    PSTRING_TABLE StringTable,
    PSTRING String,
    PSTRING_MATCH Match
    )
/*++

Routine Description:

    Searches a string table to see if any strings match the prefix of a search
    string.

Arguments:

    StringTable - Supplies a pointer to a STRING_TABLE struct.

    String - Supplies a pointer to a STRING struct that contains the string to
        search for.

    Match - Optionally supplies a pointer to a variable that contains the
        address of a STRING_MATCH structure.

Return Value:

    TRUE if success, FALSE on failure.  TRUE does not indicate a match; check
    the STRING_MATCH Match parameter's Index to see if a match occurred.

--*/
{
    BOOL Found = FALSE;
    CHAR FirstChar;
    USHORT Mask;
    USHORT Bitmap;
    USHORT Count;
    PSTRING_ARRAY StringArray;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTable)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return FALSE;
    }

    StringArray = StringTable->pStringArray;

    //
    // If the minimum length of the string array is greater than the length of
    // our search string, there can't be a prefix match.
    //

    if (StringArray->MinimumLength > String->Length) {
        return FALSE;
    }

    //
    // See if the first character is in the table.
    //

    FirstChar = String->Buffer[0];
    Mask = IsFirstCharacterInStringTable(StringTable, FirstChar);

    if (Mask == 0) {

        //
        // The first character wasn't found anywhere in the table, so we can
        // terminate our search early here.
        //

        return FALSE;
    }

    //
    // Get a bitmap where each bit corresponds to the index of a slot with a
    // length no greater than our length, which is a requirement if we're going
    // to prefix match against the search string.
    //

    Bitmap = GetBitmapForViablePrefixSlotsByLengths(StringTable, String);

    //
    // Exclude any bits not present in the first character mask.
    //

    Bitmap &= Mask;

    if (Bitmap == 0) {

        //
        // No bits remaining in the mask, so there are no prefix matches.
        //

        return FALSE;

    }

    //
    // Load the source string into an Xmm register.
    //



    //
    // See how many comparisons we need to perform.
    //

    Count = __popcnt16(Bitmap);

    switch (Count) {

        case 16:
        case 15:
        case 14:
        case 13:
        case 12:
        case 11:
        case 10:
        case  9:
        case  8:
        case  7:
        case  6:
        case  5:
        case  4:
        case  3:
        case  2:
        case  1: {


        }
    }

    //
    // For each indicated bit in the remaining mask, compare the string to the
    // slot's value.
    //

    return Found;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
