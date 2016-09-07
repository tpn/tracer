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
    PPSTRING_TABLE StringTablePointer
    )
/*++

Routine Description:

    Destroys a StringTable structure.  This method is as forgiving as it can
    be with regards to the state of the StringTable pointed to by the first
    argument.

    If StringTablePointer is a valid non-NULL pointer, it will be cleared by
    this method (i.e. set to NULL).

Arguments:

    Allocator - Supplies a pointer to the ALLOCATOR struct that was used to
        initialized the STRING_TABLE structure.

    StringTablePointer - Supplies a pointer to a variable that contains the
        address of the STRING_TABLE struct to be destroyed.  This pointer
        will be cleared by this routine.

Return Value:

    None.

--*/
{
    PSTRING_TABLE StringTable;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringTablePointer)) {
        return;
    }

    StringTable = *StringTablePointer;

    //
    // Clear the caller's pointer immediately.
    //

    *StringTablePointer = NULL;

    if (!StringTable) {
        return;
    }

    //
    // If there's no allocator, we can't free anything.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return;
    }


    //
    // Free the StringTable and return.
    //

    Allocator->Free(Allocator->Context, StringTable);

    return;

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
