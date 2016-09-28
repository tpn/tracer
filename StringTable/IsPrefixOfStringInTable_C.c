/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    IsPrefixOfStringInTable_C.c

Abstract:

    This module implements an IsPrefixOfStringInTable function in C.

--*/

#include "stdafx.h"

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInSingleTable_C(
    PSTRING_TABLE StringTable,
    PSTRING String,
    PSTRING_MATCH Match
    )
/*++

Routine Description:

    Searches a string table to see if any strings "prefix match" the given
    search string.  That is, whether any string in the table "starts with
    or is equal to" the search string.

Arguments:

    StringTable - Supplies a pointer to a STRING_TABLE struct.

    String - Supplies a pointer to a STRING struct that contains the string to
        search for.

    Match - Optionally supplies a pointer to a variable that contains the
        address of a STRING_MATCH structure.  This will be populated with
        additional details about the match if a non-NULL pointer is supplied.

Return Value:

    Index of the prefix match if one was found, NO_MATCH_FOUND if not.

--*/
{
    CHAR FirstChar;
    USHORT Mask;
    USHORT Bitmap;
    STRING_TABLE_INDEX Index = NO_MATCH_FOUND;
    PSTRING_ARRAY StringArray;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTable)) {
        goto End;
    }

    if (!ARGUMENT_PRESENT(String)) {
        goto End;
    }

    StringArray = StringTable->pStringArray;

    //
    // If the minimum length of the string array is greater than the length of
    // our search string, there can't be a prefix match.
    //

    if (StringArray->MinimumLength > String->Length) {
        goto End;
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

        goto End;
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

        goto End;

    }

    //
    // Search the string table using the given bitmap of slots to search.
    //

    Index = SearchStringTableSlotsForFirstPrefixMatch(
        StringTable,
        String,
        Bitmap,
        Match
    );

End:

    return Index;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
