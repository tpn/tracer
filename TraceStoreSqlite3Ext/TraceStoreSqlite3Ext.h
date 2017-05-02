/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3ExtLoader.h

Abstract:

    This is the main header file for the TraceStore sqlite3 extension component.
    It defines structures and functions related to the implementation of the
    component.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef
VOID
(INITIALIZE_ALLOCATOR_FROM_SQLITE3)(
    _Inout_ PALLOCATOR Allocator,
    _In_ PCSQLITE3 Sqlite3
    );
typedef INITIALIZE_ALLOCATOR_FROM_SQLITE3 *PINITIALIZE_ALLOCATOR_FROM_SQLITE3;
extern  INITIALIZE_ALLOCATOR_FROM_SQLITE3 InitializeAllocatorFromSqlite3;

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
