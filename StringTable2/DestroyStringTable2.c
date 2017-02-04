/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    DestroyStringTable2.c

Abstract:

    This module implements the functionality to destroy a previously created
    STRING_TABLE2 structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
DestroyStringTable2(
    PALLOCATOR StringTable2Allocator,
    PALLOCATOR StringArrayAllocator,
    PSTRING_TABLE2 StringTable2
    )
/*++

Routine Description:

    Destroys a StringTable2 structure.

Arguments:

    StringTable2Allocator - Supplies a pointer to the ALLOCATOR structure that
        was used to create the STRING_TABLE2.

    StringArrayAllocator - Supplies a pointer to an ALLOCATOR structure that
        may have been used to create the STRING_ARRAY.

    StringTable2 - Supplies a pointer to a STRING_TABLE2 struct to be destroyed.

Return Value:

    None.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTable2)) {
        return;
    }

    if (!ARGUMENT_PRESENT(StringTable2Allocator)) {
        return;
    }

    if (!ARGUMENT_PRESENT(StringArrayAllocator)) {
        return;
    }

    //
    // If the string array isn't embedded, free it first.
    //

    if (!HasEmbeddedStringArray(StringTable2)) {
        StringArrayAllocator->Free(
            StringArrayAllocator->Context,
            StringTable2->pStringArray
        );
        StringTable2->pStringArray = NULL;
    }

    //
    // Free the StringTable2 and return.
    //

    StringTable2Allocator->Free(StringTable2Allocator->Context, StringTable2);

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
