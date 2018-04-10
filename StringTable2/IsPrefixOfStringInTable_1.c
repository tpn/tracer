/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    IsPrefixOfStringInTable_*.c

Abstract:

    These modules implement the IsPrefixOfStringInTable routine.

;   N.B. Keep this header identical between files to declutter diff output.

--*/

#include "stdafx.h"

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfStringInTable_1(
    PSTRING_TABLE StringTable,
    PSTRING String,
    PSTRING_MATCH Match
    )
/*++

Routine Description:

    Searches a string table to see if any strings "prefix match" the given
    search string.  That is, whether any string in the table "starts with
    or is equal to" the search string.

    This routine performs a simple linear scan of the string table looking for
    a prefix match against each slot.

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
    BYTE Left;
    BYTE Right;
    ULONG Index;
    ULONG Count;
    PSTRING_ARRAY StringArray;
    PSTRING TargetString;

    //IACA_VC_START();

    StringArray = StringTable->pStringArray;

    if (StringArray->MinimumLength > String->Length) {
        return NO_MATCH_FOUND;
    }

    for (Count = 0; Count < StringArray->NumberOfElements; Count++) {

        TargetString = &StringArray->Strings[Count];

        if (String->Length < TargetString->Length) {
            continue;
        }

        for (Index = 0; Index < TargetString->Length; Index++) {
            Left = String->Buffer[Index];
            Right = TargetString->Buffer[Index];
            if (Left != Right) {
                break;
            }
        }

        if (Index == TargetString->Length) {

            if (ARGUMENT_PRESENT(Match)) {

                Match->Index = (BYTE)Count;
                Match->NumberOfMatchedCharacters = (BYTE)Index;
                Match->String = TargetString;

            }

            return (STRING_TABLE_INDEX)Count;
        }

    }

    //IACA_VC_END();

    return NO_MATCH_FOUND;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
