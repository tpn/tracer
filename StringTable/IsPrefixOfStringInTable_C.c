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
IsPrefixOfStringInTable_C(
    PSTRING_TABLE StringTable,
    PSTRING String,
    PPSTRING_MATCH MatchPointer
    )
/*++

Routine Description:

    Searches a string table to see if any strings match the prefix of a search
    string.

Arguments:

    StringTable - Supplies a pointer to a STRING_TABLE struct.
    
    String - Supplies a pointer to a STRING struct that contains the string to
        search for.

    MatchPointer - Supplies a pointer to a variable that contains the address
        of a STRING_MATCH structure.

Return Value:

    TRUE if success, FALSE on failure.  TRUE does not indicate a match; check
    the STRING_MATCH Match parameter's Index to see if a match occurred.

--*/
{
    BOOL Found = FALSE;
    WIDE_CHARACTER Char = { 0 };
    USHORT Index;
    USHORT NumberOfElements;
    PSTRING_MATCH Match;
    PSTRING_TABLE Table;


    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTable)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(MatchPointer)) {
        return FALSE;
    }

    Match = *MatchPointer;

    if (!Match) {
        return FALSE;
    }

    //
    // Arguments are valid, begin checking the table.
    //

    Table = StringTable;

    goto Start;

Start:

    NumberOfElements = GetNumberOfStringsInTable(Table);

    for (Index = 0; Index < NumberOfElements; Index++) {

    }

    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
