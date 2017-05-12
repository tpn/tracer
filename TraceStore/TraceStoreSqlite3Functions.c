/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Functions.c

Abstract:

    This module implements sqlite3 virtual table functions.

--*/

#include "stdafx.h"

//
// Count function.
//

SQLITE3_AGGREGATE_STEP_FUNCTION TraceStoreSqlite3CountStep;

_Use_decl_annotations_
VOID
TraceStoreSqlite3CountStep(
    PSQLITE3_CONTEXT Context,
    LONG ArgumentNumber,
    PSQLITE3_VALUE *ValuePointer
    )
{
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = TraceStoreSqlite3TlsGetCursor();

    //
    // Force the next Eof() callback to indicate we've reached the end of our
    // data set.  This avoids the need to stream through the entire record set.
    //

    Cursor->Flags.EofOverride = TRUE;

    return;
}

SQLITE3_AGGREGATE_FINAL_FUNCTION TraceStoreSqlite3CountFinal;

_Use_decl_annotations_
VOID
TraceStoreSqlite3CountFinal(
    PSQLITE3_CONTEXT Context
    )
{
    PCSQLITE3 Sqlite3;
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = TraceStoreSqlite3TlsGetCursor();
    Sqlite3 = Cursor->Sqlite3;

    Sqlite3->ResultInt64(Context, Cursor->TotalNumberOfRecords);

    return;
}

SQLITE3_FUNCTION_DESTROY TraceStoreSqlite3CountDestroy;

_Use_decl_annotations_
VOID
TraceStoreSqlite3CountDestroy(
    PVOID UserContext
    )
{
    return;
}

//
// Dummy default functions.
//

_Use_decl_annotations_
VOID
TraceStoreSqlite3DefaultScalarFunction(
    PSQLITE3_CONTEXT Context,
    LONG ArgumentNumber,
    PSQLITE3_VALUE *ValuePointer
    )
{
    __debugbreak();
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
    __debugbreak();
    return;
}

_Use_decl_annotations_
VOID
TraceStoreSqlite3DefaultAggregateFinalFunction(
    PSQLITE3_CONTEXT Context
    )
{
    __debugbreak();
    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
