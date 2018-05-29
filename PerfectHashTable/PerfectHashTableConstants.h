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
// Object (e.g. events, shared memory sections) name prefixes for the runtime
// context.
//

const PCUNICODE_STRING ContextObjectPrefixes[];

const USHORT NumberOfContextEventPrefixes;
const USHORT NumberOfContextObjectPrefixes;

#ifdef __cplusplus
}; // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
