/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTablePrivate.h

Abstract:

    This is the private header file for the StringTable component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
// Function typedefs and inline functions for internal modules.
////////////////////////////////////////////////////////////////////////////////

COPY_STRING_ARRAY CopyStringArray;
DESTROY_STRING_TABLE DestroyStringTable;
CREATE_STRING_ARRAY_FROM_DELIMITED_STRING CreateStringArrayFromDelimitedString;
CREATE_STRING_TABLE CreateSingleStringTable;
SEARCH_STRING_TABLE_SLOTS_FOR_FIRST_PREFIX_MATCH
    SearchStringTableSlotsForFirstPrefixMatch;
MALLOC StringTableMalloc;
CALLOC StringTableCalloc;
REALLOC StringTableRealloc;
FREE StringTableFree;
FREE_POINTER StringTableFreePointer;
INITIALIZE_ALLOCATOR StringTableInitializeAllocator;
DESTROY_ALLOCATOR StringTableDestroyAllocator;

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
