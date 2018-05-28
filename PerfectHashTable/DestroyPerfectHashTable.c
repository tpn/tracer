/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    DestroyPerfectHashTable.c

Abstract:

    This module implements the destroy routine for the PerfectHashTable
    component.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOLEAN
DestroyPerfectHashTable(
    PPERFECT_HASH_TABLE *PerfectHashTablePointer,
    PBOOLEAN IsProcessTerminating
    )
/*++

Routine Description:

    Destroys a previously created PERFECT_HASH_TABLE structure, freeing all
    memory unless the IsProcessTerminating flag is TRUE.

Arguments:

    PerfectHashTablePointer - Supplies the address of a variable that contains
        the address of the PERFECT_HASH_TABLE structure to destroy.  This
        variable will be cleared (i.e. the pointer will be set to NULL) if the
        routine destroys the structure successfully (returns TRUE).

    IsProcessTerminating - Optionally supplies a pointer to a boolean flag
        indicating whether or not the process is terminating.  If the pointer
        is non-NULL and the underlying value is TRUE, the method returns success
        immediately.  (If the process is terminating, there is no need to walk
        any internal data structures and individually free elements.)

Return Value:

    TRUE on success, FALSE on failure.  If successful, a NULL pointer will be
    written to the PerfectHashTablePointer parameter.

--*/
{
    PRTL Rtl;
    BOOLEAN Success;
    PALLOCATOR Allocator;
    PPERFECT_HASH_TABLE PerfectHashTable;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(PerfectHashTablePointer)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(*PerfectHashTablePointer)) {
        goto Error;
    }

    if (ARGUMENT_PRESENT(IsProcessTerminating)) {
        if (*IsProcessTerminating) {

            //
            // Fast-path exit.  Clear the caller's pointer and return success.
            //

            *PerfectHashTablePointer = NULL;
            return TRUE;
        }
    }

    //
    // A valid pointer has been provided, and the process isn't terminating.
    // Initialize aliases and continue with destroy logic.
    //

    PerfectHashTable = *PerfectHashTablePointer;
    Rtl = PerfectHashTable->Rtl;
    Allocator = PerfectHashTable->Allocator;

    //
    // Sanity check the perfect hash structure size matches what we expect.
    //

    ASSERT(PerfectHashTable->SizeOfStruct == sizeof(*PerfectHashTable));

    //
    // Free the underlying memory and clear the caller's pointer.
    //

    Allocator->FreePointer(Allocator->Context, PerfectHashTablePointer);

    Success = TRUE;

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
