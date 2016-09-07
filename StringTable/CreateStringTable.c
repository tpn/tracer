/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    CreateStringTable.c

Abstract:

    This module implements the functionality to create a STRING_TABLE structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
PSTRING_TABLE
CreateStringTable(
    PALLOCATOR Allocator,
    PSTRING_ARRAY StringArray
    )
/*++

Routine Description:

    Allocates space for a STRING_TABLE structure using the provided Allocator,
    then initializes it using the provided STRING_ARRAY.  The routine will copy
    any string data it needs from the STRING_ARRAY, so the caller is free to
    destroy that structure after the table has been successfully created.

    The Allocator will be used for all memory allocations. DestroyStringTable()
    must be called against the returned PSTRING_TABLE when the structure is no
    longer needed in order to ensure resources are released.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to allocate all memory required by the structure during its
        creation.

    StringArray - Supplies a pointer to an initialized STRING_ARRAY structure
        that contains the STRING structures that are to be added to the table.

Return Value:

    A pointer to a valid PSTRING_TABLE structure on success, NULL on failure.
    Call DestroyStringTable() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    USHORT Index;
    USHORT NumberOfTables;
    USHORT LongestLength;
    PUSHORT LongestLengthPerTable;
    PSTRING String;
    PSTRING_TABLE StringTable;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(StringArray)) {
        return NULL;
    }

    //
    // Determine how many tables we're going to need.
    //

    NumberOfTables = GetNumberOfTablesRequiredForStringArray(StringArray);

    //
    // Determine the longest string present in the string table, as well as the
    // longest string present in each 16
    //

    LongestLength = 0;

    for (Index = 0; Index < StringArray->NumberOfElements; Index++) {
        String = &StringArray->Strings[Index];
        if (String->Length > LongestLength) {
            LongestLength = String->Length;
        }
    }

    goto End;

Error:

    if (StringTable) {

        //
        // Free the backing memory for the string table and clear the pointer.
        //

        Allocator->FreePointer(Allocator->Context, &StringTable);
    }

End:

    if (LongestLengthPerTable) {
        Allocator->FreePointer(Allocator->Context, &LongestLengthPerTable);
    }

    return StringTable;

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
