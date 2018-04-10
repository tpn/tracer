/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    IsPrefixOfCStrInArray_*.c

Abstract:

    These modules implement the IsPrefixOfStringInTable routine.

    N.B. Keep this header identical between files to declutter diff output.

--*/

#include "stdafx.h"

//
// A prefix search using C-style NULL-terminated strings and no string table
// support.
//

_Use_decl_annotations_
STRING_TABLE_INDEX
IsPrefixOfCStrInArray(
    PCSZ *StringArray,
    PCSZ String,
    PSTRING_MATCH Match
    )
{
    PCSZ Left;
    PCSZ Right;
    PCSZ *Target;
    ULONG Index = 0;
    ULONG Count;

    for (Target = StringArray; *Target != NULL; Target++, Index++) {
        Count = 0;
        Left = String;
        Right = *Target;

        while (*Left && *Right && *Left++ == *Right++) {
            Count++;
        }

        if (Count > 0 && !*Right) {
            if (ARGUMENT_PRESENT(Match)) {
                Match->Index = (BYTE)Index;
                Match->NumberOfMatchedCharacters = (BYTE)Count;
                Match->String = NULL;
            }
            return (STRING_TABLE_INDEX)Index;
        }
    }

    return NO_MATCH_FOUND;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
