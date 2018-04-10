/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    DestroyStringTable.c

Abstract:

    This module implements the functionality to destroy a previously created
    STRING_TABLE structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
DestroyStringTable(
    PALLOCATOR StringTableAllocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_TABLE StringTable
    )
/*++

Routine Description:

    Destroys a StringTable structure.

Arguments:

    StringTableAllocator - Supplies a pointer to the ALLOCATOR structure that
        was used to create the STRING_TABLE.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure that
        may have been used to create the STRING_ARRAY.

    StringTable - Supplies a pointer to a STRING_TABLE struct to be destroyed.

Return Value:

    None.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTable)) {
        return;
    }

    if (!ARGUMENT_PRESENT(StringTableAllocator)) {
        return;
    }

    if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
        return;
    }

    //
    // If the string array isn't embedded, free it first.
    //

    if (!HasEmbeddedStringArray(StringTable)) {
        StringArrayAllocator->Free(
            StringArrayAllocator->Context,
            StringTable->pStringArray
        );
        StringTable->pStringArray = NULL;
    }

    //
    // Free the StringTable and return.
    //

    StringTableAllocator->AlignedFree(StringTableAllocator->Context,
                                      StringTable);

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
