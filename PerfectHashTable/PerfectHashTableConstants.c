/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableConstants.c

Abstract:

    This module declares constants used by the PerfectHashTable component.

--*/

#include "stdafx.h"

//
// Declare the array of creation routines.
//

const PCREATE_PERFECT_HASH_TABLE_IMPL CreationRoutines[] = {
    NULL,
    CreatePerfectHashTableImplChm01,
    NULL
};

//
// Define the array of loader routines.
//

const PLOAD_PERFECT_HASH_TABLE_IMPL LoaderRoutines[] = {
    NULL,
    LoadPerfectHashTableImplChm01,
    NULL
};

//
// Define the array of hash routines.
//

const PPERFECT_HASH_TABLE_HASH HashRoutines[] = {
    NULL,
    PerfectHashTableHash01,
    PerfectHashTableHash02,
    NULL
};

//
// Define the array of mask routines.
//

const PPERFECT_HASH_TABLE_MASK MaskRoutines[] = {
    NULL,
    PerfectHashTableMaskModulus,
    PerfectHashTableMaskShift,
    PerfectHashTableMaskAnd,
    PerfectHashTableMaskXorAnd,
    NULL
};

//
// Define the array of lookup-index routines.
//

const PPERFECT_HASH_TABLE_INDEX IndexRoutines[] = {
    NULL,
    PerfectHashTableIndexImplChm01,
    NULL
};

//
// Define the array of vtbl sizes.
//

const PGET_VTBL_EX_SIZE GetVtblExSizeRoutines[] = {
    NULL,
    GetVtblExSizeChm01,
    NULL,
};

//
// Array of UNICODE_STRING event prefix names used by the runtime context.
//

const UNICODE_STRING ContextShutdownEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_ShutdownEvent_");

const UNICODE_STRING ContextSucceededEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_SucceededEvent_");

const UNICODE_STRING ContextFailedEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_FailedEvent_");

const UNICODE_STRING ContextCompletedEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_CompletedEvent_");

const UNICODE_STRING ContextPreparedFileEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_PreparedFileEvent_");

const UNICODE_STRING ContextSavedFileEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_SavedFileEvent_");

const PCUNICODE_STRING ContextObjectPrefixes[] = {
    &ContextShutdownEventPrefix,
    &ContextSucceededEventPrefix,
    &ContextFailedEventPrefix,
    &ContextCompletedEventPrefix,
    &ContextPreparedFileEventPrefix,
    &ContextSavedFileEventPrefix,
};

//
// We only have events at the moment so number of event prefixes will equal
// number of object prefixes.
//

const USHORT NumberOfContextEventPrefixes = ARRAYSIZE(ContextObjectPrefixes);
const USHORT NumberOfContextObjectPrefixes = ARRAYSIZE(ContextObjectPrefixes);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
