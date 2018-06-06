/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTableConstants.h

Abstract:

    This module defines constants related to the PerfectHashTable component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Define two magic numbers for the Magic field of the TABLE_INFO_ON_DISK_HEADER
// structure.
//

#define TABLE_INFO_ON_DISK_MAGIC_LOWPART  0x25101981
#define TABLE_INFO_ON_DISK_MAGIC_HIGHPART 0x17071953

//
// Declare an array of creation routines.  This is intended to be indexed by
// the PERFECT_HASH_TABLE_ALGORITHM_ID enumeration.
//

const PCREATE_PERFECT_HASH_TABLE_IMPL CreationRoutines[];

//
// Declare an array of loader routines.  This is intended to be indexed by
// the PERFECT_HASH_TABLE_ALGORITHM_ID enumeration.
//

const PLOAD_PERFECT_HASH_TABLE_IMPL LoaderRoutines[];

//
// Declare an array of index routines.  This is intended to be indexed by
// the PERFECT_HASH_TABLE_ALGORITHM_ID enumeration.
//

const PPERFECT_HASH_TABLE_INDEX IndexRoutines[];

//
// Declare an array of hash routines.  This is intended to be indexed by
// the PERFECT_HASH_TABLE_HASH_FUNCTION_ID enumeration.
//

const PPERFECT_HASH_TABLE_HASH HashRoutines[];

//
// Declare an array of hash mask routines.  This is intended to be indexed by
// the PERFECT_HASH_TABLE_HASH_FUNCTION_ID enumeration.
//

const PPERFECT_HASH_TABLE_MASK_HASH MaskHashRoutines[];

//
// Declare an array of index mask routines.  This is intended to be indexed by
// the PERFECT_HASH_TABLE_HASH_FUNCTION_ID enumeration.
//

const PPERFECT_HASH_TABLE_MASK_INDEX MaskIndexRoutines[];

//
// Declare an array of seeded hash routines.  This is intended to be indexed by the
// PERFECT_HASH_TABLE_HASH_FUNCTION_ID enumeration.
//

const PPERFECT_HASH_TABLE_SEEDED_HASH SeededHashRoutines[];

//
// Helper inline routine for initializing the extended vtbl interface.
//

FORCEINLINE
VOID
InitializeExtendedVtbl(
    _In_ PPERFECT_HASH_TABLE Table,
    _Inout_ PPERFECT_HASH_TABLE_VTBL_EX Vtbl
    )
{
    Vtbl->AddRef = PerfectHashTableAddRef;
    Vtbl->Release = PerfectHashTableRelease;
    Vtbl->Insert = PerfectHashTableInsert;
    Vtbl->Lookup = PerfectHashTableLookup;
    Vtbl->Delete = PerfectHashTableDelete;
    Vtbl->Index = IndexRoutines[Table->AlgorithmId];
    Vtbl->Hash = HashRoutines[Table->HashFunctionId];
    Vtbl->MaskHash = MaskHashRoutines[Table->MaskFunctionId];
    Vtbl->MaskIndex = MaskIndexRoutines[Table->MaskFunctionId];
    Vtbl->SeededHash = SeededHashRoutines[Table->HashFunctionId];
    Table->Vtbl = Vtbl;
}

//
// Declare an array of routines used to obtain the size in bytes of the extended
// vtbl used by each routine.  The Create and Load routines factor this into the
// allocation size of the PERFECT_HASH_TABLE structure.
//
// This is intended to be indexed by the PERFECT_HASH_TABLE_ALGORITHM_ID
// enumeration.
//

const PGET_VTBL_EX_SIZE GetVtblExSizeRoutines[];

//
// Declare the array of algorithm names.
//

const PCUNICODE_STRING AlgorithmNames[];

//
// Declare the array of hash function names.
//

const PCUNICODE_STRING HashFunctionNames[];

//
// Declare the array of mask function names.
//

const PCUNICODE_STRING MaskFunctionNames[];

//
// Object (e.g. events, shared memory sections) name prefixes for the runtime
// context.
//

const PCUNICODE_STRING ContextObjectPrefixes[];

const BYTE NumberOfContextEventPrefixes;
const BYTE NumberOfContextObjectPrefixes;

//
// Helper inline function for programmatically determining how many events
// are present in the context based on the FirstEvent and LastEvent addresses.
// Used by CreatePerfectHashTableContext() and DestroyPerfectHashTableContext().
//

FORCEINLINE
BYTE
GetNumberOfContextEvents(
    _In_ PPERFECT_HASH_TABLE_CONTEXT Context
    )
{
    BYTE NumberOfEvents;

    //
    // Calculate the number of event handles based on the first and last event
    // indicators in the context structure.  The additional sizeof(HANDLE)
    // accounts for the fact that we're going from 0-based address offsets
    // to 1-based counts.
    //

    NumberOfEvents = (BYTE)(

        (ULONG_PTR)(

            sizeof(HANDLE) +

            RtlOffsetFromPointer(
                &Context->LastEvent,
                &Context->FirstEvent
            )

        ) / (ULONG_PTR)sizeof(HANDLE)
    );

    //
    // Sanity check the number of events matches the number of event prefixes.
    //

    ASSERT(NumberOfEvents == NumberOfContextEventPrefixes);

    return NumberOfEvents;
}


#ifdef __cplusplus
}; // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
