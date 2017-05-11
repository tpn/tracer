/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Functions.c

Abstract:

    This module implements sqlite3 virtual table functions.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
TraceStoreSqlite3DefaultScalarFunction(
    PSQLITE3_CONTEXT Context,
    LONG ArgumentNumber,
    PSQLITE3_VALUE *ValuePointer
    )
{
    return;
}

_Use_decl_annotations_
VOID
TraceStoreSqlite3DefaultAggregateStepFunction(
    PSQLITE3_CONTEXT Context,
    LONG ArgumentNumber,
    PSQLITE3_VALUE *ValuePointer
    )
{
    return;
}

_Use_decl_annotations_
VOID
TraceStoreSqlite3DefaultAggregateFinalFunction(
    PSQLITE3_CONTEXT Context
    )
{
    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
