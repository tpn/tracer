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
    PALLOCATOR Allocator,
    PSTRING_TABLE StringTable
    )
/*++

Routine Description:

    Destroys a StringTable structure.

Arguments:

    Allocator - Supplies a pointer to the ALLOCATOR struct that was used to
        initialized the STRING_TABLE structure.

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

    if (!ARGUMENT_PRESENT(Allocator)) {

        //
        // We can't do much without an allocator to free the underlying structs.
        //

        return;
    }

    //
    // If the string array isn't embedded, free it first.
    //

    if (!HasEmbeddedStringArray(StringTable)) {
        Allocator->Free(Allocator->Context, StringTable->pStringArray);
        StringTable->pStringArray = NULL;
    }

    //
    // Free the StringTable and return.
    //

    Allocator->Free(Allocator->Context, StringTable);

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
