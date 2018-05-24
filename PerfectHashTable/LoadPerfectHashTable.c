/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    LoadPerfectHashTable.c

Abstract:

    This module implements the loader routine for the PerfectHashTable
    component.  It is responsible for loading a perfect hash table that was
    created via CreatePerfectHashTable();

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOLEAN
LoadPerfectHashTable(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PERFECT_HASH_TABLE_LOAD_FLAGS LoadFlags,
    PCUNICODE_STRING Path,
    PPERFECT_HASH_TABLE *PerfectHashTablePointer
    )
/*++

Routine Description:

    Initializes a new PERFECT_HASH_TABLE structure based on an on-disk persisted
    version of the table created via CreatePerfectHashTable().

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure that
        will be used to allocate memory for the underlying PERFECT_HASH
        structure.

    LoadFlags - Currently unused.

    Path - Supplies a pointer to a UNICODE_STRING structure representing the
        fully-qualified, NULL-terminated path of the file to be used to load
        the table.

    PerfectHashTablePointer - Supplies the address of a variable that will
        receive the address of the newly created PERFECT_HASH_TABLE structure
        if the routine is successful, or NULL if the routine fails.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOLEAN Success;
    PPERFECT_HASH_TABLE PerfectHashTable;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PerfectHashTablePointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *PerfectHashTablePointer = NULL;

    //
    // XXX: implement.
    //

    PerfectHashTable = NULL;

    goto Error;

    //
    // We've completed initialization, indicate success and jump to the end.
    //

    Success = TRUE;

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the caller's pointer and return.
    //
    // N.B. PerfectHash could be NULL here, which is fine.
    //

    *PerfectHashTablePointer = PerfectHashTable;

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
