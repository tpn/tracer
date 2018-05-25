/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    CreatePerfectHashTable.c

Abstract:

    This module implements the creation routine for the PerfectHashTable
    component.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOLEAN
CreatePerfectHashTable(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PPERFECT_HASH_TABLE_ANY_API AnyApi,
    PPERFECT_HASH_TABLE_CONTEXT Context,
    PERFECT_HASH_TABLE_CREATE_FLAGS CreateFlags,
    PERFECT_HASH_TABLE_ALGORITHM Algorithm,
    PPERFECT_HASH_TABLE_KEYS Keys,
    PCUNICODE_STRING HashTablePath,
    PPERFECT_HASH_TABLE *PerfectHashTablePointer
    )
/*++

Routine Description:

    Creates and initializes a PERFECT_HASH_TABLE structure from a given set
    of keys, using the requested algorithm.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure that
        will be used for all memory allocations.

    AnyApi - Supplies a pointer to the active API structure in use.

    Context - Supplies a pointer to an initialized PERFECT_HASH_TABLE_CONTEXT
        structure that can be used by the underlying algorithm in order to
        search for perfect hash solutions in parallel.

    CreateFlags - Supplies creation flags that affect the underlying behavior
        of the perfect hash table creation.

    Algorithm - Supplies the algorithm to use.

    Keys - Supplies a pointer to a PERFECT_HASH_TABLE_KEYS structure obtained
        from LoadPerfectHashTableKeys().

    HashTablePath - Optionally supplies a pointer to a UNICODE_STRING structure
        that represents the fully-qualified, NULL-terminated path of the backing
        file used to save the hash table.  If NULL, the file name of the keys
        file will be used, with ".pht" appended to it.

    PerfectHashTablePointer - Supplies the address of a variable that will
        receive the address of the newly created PERFECT_HASH structure if
        the routine is successful (returns TRUE), or NULL if the routine failed.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOLEAN Success;
    HANDLE FileHandle = NULL;
    LARGE_INTEGER AllocSize;
    LONG_INTEGER PathBufferSize;
    PPERFECT_HASH_TABLE Table = NULL;
    UNICODE_STRING Suffix = RTL_CONSTANT_STRING(L".pht1");
    PPERFECT_HASH_TABLE_API Api;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AnyApi)) {

        return FALSE;

    } else {

        //
        // Initialize Api alias.  We'll use this during error handling in order
        // to call the table destroy routine.
        //

        Api = &AnyApi->Api;
    }

    if (!ARGUMENT_PRESENT(Keys)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PerfectHashTablePointer)) {
        return FALSE;
    }

    if (ARGUMENT_PRESENT(HashTablePath) &&
        !IsValidMinimumDirectoryNullTerminatedUnicodeString(HashTablePath)) {

        return FALSE;
    }

    //
    // CreateFlags aren't currently used.
    //

    if (CreateFlags.AsULong != 0) {
        return FALSE;
    }

    if (!IsValidPerfectHashTableAlgorithm(Algorithm)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *PerfectHashTablePointer = NULL;

    //
    // Calculate the allocation size required for the structure, including the
    // memory required to take a copy of the backing file name path.
    //

    AllocSize.QuadPart = sizeof(*Table);

    if (ARGUMENT_PRESENT(HashTablePath)) {

        //
        // Use the length of the caller-provided path, plus a trailing NULL.
        //

        PathBufferSize.LongPart = (
            HashTablePath->Length +
            sizeof(HashTablePath->Buffer[0])
        );

    } else {

        //
        // No path has been provided by the caller, so we'll use the path of
        // the keys file with ".pht" appended.  Perform a quick invariant check
        // first: maximum length should be 1 character (2 bytes) larger than
        // length.  (This is handled in LoadPerfectHashTableKeys().)
        //

        ASSERT(Keys->Path.Length + sizeof(Keys->Path.Buffer[0]) ==
               Keys->Path.MaximumLength);

        PathBufferSize.LongPart = (Keys->Path.MaximumLength + Suffix.Length);

    }

    //
    // Sanity check we haven't overflowed MAX_USHORT for the path buffer size.
    //

    ASSERT(!PathBufferSize.HighPart);

    //
    // Add the path buffer size to the structure allocation size, then check
    // we haven't overflowed MAX_ULONG.
    //

    AllocSize.QuadPart += PathBufferSize.LongPart;

    ASSERT(!AllocSize.HighPart);

    //
    // Allocate space for the structure and backing path.
    //

    Table = (PPERFECT_HASH_TABLE)(
        Allocator->Calloc(Allocator,
                          1,
                          AllocSize.LowPart)
    );

    if (!Table) {
        return FALSE;
    }

    //
    // Allocation was successful, continue with initialization.
    //

    Table->SizeOfStruct = sizeof(*Table);
    Table->Rtl = Rtl;
    Table->Allocator = Allocator;
    Table->Flags.AsULong = 0;
    Table->Keys = Keys;
    Table->AnyApi = AnyApi;
    Table->Context = Context;

    //
    // Common initialization is complete, dispatch remaining work to the
    // algorithm's creation routine.
    //

    Success = CreationRoutines[Algorithm](Table);
    if (!Success) {
        goto Error;
    }

    //
    // We're done!  Jump to the end of the routine to finish up.
    //

    goto End;

Error:

    Success = FALSE;

    //
    // Call the destroy routine on the table if one is present.
    //

    if (Table) {

        if (!Api->DestroyPerfectHashTable(&Table, NULL)) {

            //
            // There's nothing we can do here.
            //

            NOTHING;
        }

        //
        // N.B. DestroyPerfectHashTable() should clear the Table pointer.
        //      Assert that invariant now.
        //

        ASSERT(Table == NULL);
    }

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the caller's pointer and return.
    //
    // N.B. Table could be NULL here, which is fine.
    //

    *PerfectHashTablePointer = Table;

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
