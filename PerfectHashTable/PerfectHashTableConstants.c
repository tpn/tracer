/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableConstants.c

Abstract:

    This module defines constants used by the PerfectHashTable component.

--*/

#include "stdafx.h"

//
// Define the array of creation routines.
//

const PCREATE_PERFECT_HASH_TABLE_IMPL CreationRoutines[] = {
    NULL,
    CreatePerfectHashTableImplChm01,
    NULL
};

//
// Array of UNICODE_STRING event prefix names used by the runtime context.
//


const UNICODE_STRING ContextObjectPrefixes[] = {
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_ShutdownEvent_"),
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_SucceededEvent_"),
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_FailedEvent_"),
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_CompletedEvent_"),
};

//
// We only have events at the moment so number of event prefixes will equal
// number of object prefixes.
//

const USHORT NumberOfContextEventPrefixes = ARRAYSIZE(ContextObjectPrefixes);
const USHORT NumberOfContextObjectPrefixes = ARRAYSIZE(ContextObjectPrefixes);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
