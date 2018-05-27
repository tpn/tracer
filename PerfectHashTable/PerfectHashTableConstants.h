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
// Declare an array of creation routines.  This is intended to be indexed by
// the PERFECT_HASH_TABLE_ALGORITHM enumeration.
//

const PCREATE_PERFECT_HASH_TABLE_IMPL CreationRoutines[];

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
