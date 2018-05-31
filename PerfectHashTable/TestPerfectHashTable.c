/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    TestPerfectHashTable.c

Abstract:

    This module implements the test routine for an individual instance of a
    PERFECT_HASH_TABLE structure.  It is primarily used by the module's
    self-test functionality.

--*/

#include "stdafx.h"

//
// Use a custom ASSERT() that immediately returns FALSE in order to simplify
// debugging and error handling at the moment.
//

#undef ASSERT
#define ASSERT(Condition)   \
    if (!(Condition)) {     \
        __debugbreak();     \
        goto Error;         \
    }

_Use_decl_annotations_
BOOLEAN
TestPerfectHashTable(
    PPERFECT_HASH_TABLE Table
    )
/*++

Routine Description:

    Tests an instance of a PERFECT_HASH_TABLE for correctness.

Arguments:

    Table - Supplies a pointer to an initialized PERFECT_HASH_TABLE structure
        for which the testing will be undertaken.  This structure is obtained
        via either CreatePerfectHashTable() or LoadPerfectHashTable().

Return Value:

    TRUE if all internal tests pass, FALSE if not.

    N.B. Currently, if an internal test fails, the behavior is to __debugbreak()
         in order to facilitate quicker debugging.

--*/
{
    PRTL Rtl;
    ULONG Value;
    ULONG Previous;
    BOOLEAN Success;
    HRESULT Result;
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Table)) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    Rtl = Table->Rtl;
    Allocator = Table->Allocator;

    //
    // Sanity check the perfect hash structure size matches what we expect.
    //

    ASSERT(Table->SizeOfStruct == sizeof(*Table));

    Result = Table->Vtbl->Insert(Table, 0x2e, 0xe2, &Previous);
    ASSERT(!FAILED(Result));

    Result = Table->Vtbl->Lookup(Table, 0x2e, &Value);
    ASSERT(!FAILED(Result));

    ASSERT(Value == 0xe2);

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
