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
    PerfectHashTableHash03,
    PerfectHashTableHash04,
    NULL
};

//
// Define the array of seeded hash routines.
//

const PPERFECT_HASH_TABLE_SEEDED_HASH SeededHashRoutines[] = {
    NULL,
    PerfectHashTableSeededHash01,
    PerfectHashTableSeededHash02,
    PerfectHashTableSeededHash03,
    PerfectHashTableSeededHash04,
    NULL
};

//
// Define the array of hash mask routines.
//

const PPERFECT_HASH_TABLE_MASK_HASH MaskHashRoutines[] = {
    NULL,
    PerfectHashTableMaskHashModulus,
    PerfectHashTableMaskHashAnd,
    PerfectHashTableMaskHashXorAnd,

    //
    // The PerfectHashTableFoldAutoMaskFunctionId slot is next.  This is a
    // psuedo ID that don't actually match to a mask implementation.  The
    // algorithm is required to detect when this mask function is being used
    // and swap out the Vtbl pointer to one of the following fold methods
    // depending on the table size.  Thus, we use a NULL pointer in this array
    // such that we'll trap on the first attempt to mask if this hasn't been
    // done.
    //

    NULL,

    PerfectHashTableMaskHashFoldOnce,
    PerfectHashTableMaskHashFoldTwice,
    PerfectHashTableMaskHashFoldThrice,
    NULL
};

//
// Define the array of index mask routines.
//

const PPERFECT_HASH_TABLE_MASK_INDEX MaskIndexRoutines[] = {
    NULL,
    PerfectHashTableMaskIndexModulus,
    PerfectHashTableMaskIndexAnd,
    PerfectHashTableMaskIndexXorAnd,

    //
    // See above description regarding the following NULL slot.
    //

    NULL,

    PerfectHashTableMaskIndexFoldOnce,
    PerfectHashTableMaskIndexFoldTwice,
    PerfectHashTableMaskIndexFoldThrice,
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

const UNICODE_STRING ContextTryLargerTableSizeEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_TryLargerTableSizeEvent_");

const UNICODE_STRING ContextPreparedFileEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_PreparedFileEvent_");

const UNICODE_STRING ContextVerifiedEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_VerifiedEvent_");

const UNICODE_STRING ContextSavedFileEventPrefix =
    RTL_CONSTANT_STRING(L"PerfectHashTableContext_SavedFileEvent_");

const PCUNICODE_STRING ContextObjectPrefixes[] = {
    &ContextShutdownEventPrefix,
    &ContextSucceededEventPrefix,
    &ContextFailedEventPrefix,
    &ContextCompletedEventPrefix,
    &ContextTryLargerTableSizeEventPrefix,
    &ContextPreparedFileEventPrefix,
    &ContextVerifiedEventPrefix,
    &ContextSavedFileEventPrefix,
};

//
// We only have events at the moment so number of event prefixes will equal
// number of object prefixes.
//

const BYTE NumberOfContextEventPrefixes = ARRAYSIZE(ContextObjectPrefixes);
const BYTE NumberOfContextObjectPrefixes = ARRAYSIZE(ContextObjectPrefixes);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
