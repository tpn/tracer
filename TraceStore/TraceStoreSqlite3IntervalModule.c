/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3IntervalModule.c

Abstract:

    WIP.

--*/

#include "stdafx.h"

//
// This is required in order to link without the CRT.
//

//LONG _fltused;

//
// Define the interval Trace Store sqlite3 module function pointers.
//

SQLITE3_MODULE TraceStoreSqlite3IntervalModule = {
    0,  // Version
    0,  // Padding
    TraceStoreSqlite3IntervalModuleCreate,
    TraceStoreSqlite3IntervalModuleCreate,
    TraceStoreSqlite3IntervalModuleBestIndex,
    TraceStoreSqlite3IntervalModuleDisconnect,
    NULL, // TraceStoreSqlite3IntervalModuleDestroy,
    TraceStoreSqlite3IntervalModuleOpenCursor,
    TraceStoreSqlite3IntervalModuleCloseCursor,
    TraceStoreSqlite3IntervalModuleFilter,
    TraceStoreSqlite3IntervalModuleNext,
    TraceStoreSqlite3IntervalModuleEof,
    TraceStoreSqlite3IntervalModuleColumn,
    TraceStoreSqlite3IntervalModuleRowid,
    NULL, // TraceStoreSqlite3IntervalModuleUpdate,
    NULL, // TraceStoreSqlite3IntervalModuleBegin,
    NULL, // TraceStoreSqlite3IntervalModuleSync,
    NULL, // TraceStoreSqlite3IntervalModuleCommit,
    NULL, // TraceStoreSqlite3IntervalModuleRollback,
    TraceStoreSqlite3IntervalModuleFindFunction,
    NULL, // TraceStoreSqlite3IntervalModuleRename,
    NULL, // TraceStoreSqlite3IntervalModuleSavepoint,
    NULL, // TraceStoreSqlite3IntervalModuleRelease,
    NULL, // TraceStoreSqlite3IntervalModuleRollbackTo,
};


_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleCreate(
    PSQLITE3_DB Sqlite3Db,
    PVOID Aux,
    LONG NumberOfArguments,
    PCSZ Arguments,
    PSQLITE3_VTAB *VirtualTable
    )
{
    ULONG Failed;
    PCSQLITE3 Sqlite3;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_SQLITE3_DB Db;

    TraceStore = (PTRACE_STORE)Aux;
    Db = TraceStore->Db;
    Sqlite3 = Db->Sqlite3;

    Failed = (
        Sqlite3->DeclareVirtualTable(
            Sqlite3Db,
            TraceStore->Sqlite3IntervalSchema
        )
    );
    if (Failed) {
        __debugbreak();
        return Failed;
    }

    *VirtualTable = &TraceStore->Sqlite3IntervalVirtualTable.AsSqlite3Vtab;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleBestIndex(
    PSQLITE3_VTAB VirtualTable,
    PSQLITE3_INDEX_INFO IndexInfo
    )
{
    LONG Index;
    BOOL RowidConstraint = TRUE;
    LONG ConstraintCount = 0;
    ULONG Shifted;
    LONGLONG EstimatedRows;
    PCSQLITE3 Sqlite3;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_TOTALS Totals;
    PTRACE_STORE_SQLITE3_DB Db;
    PSQLITE3_INDEX_CONSTRAINT Constraint;
    PSQLITE3_INDEX_CONSTRAINT_USAGE ConstraintUsage;
    TRACE_STORE_SQLITE3_ROWID_INDEX RowidIndex;
    TRACE_STORE_SQLITE3_ROWID_INDEX FinalRowidIndex;

    TraceStore = CONTAINING_RECORD(VirtualTable,
                                   TRACE_STORE,
                                   Sqlite3IntervalVirtualTable);

    Db = TraceStore->Db;
    Sqlite3 = Db->Sqlite3;

    if (IndexInfo->NumberOfConstraints > 2) {
        __debugbreak();
        return SQLITE_ERROR;
    }

    Totals = TraceStore->Totals;
    if (TraceStore->IsMetadata) {
        EstimatedRows = Totals->NumberOfAllocations.QuadPart;
    } else {
        EstimatedRows = Totals->NumberOfRecords.QuadPart;
    }

    FinalRowidIndex.AsULong = 0;

    for (Index = 0; Index < IndexInfo->NumberOfConstraints; Index++) {
        Constraint = IndexInfo->Constraints + Index;
        if (!Constraint->IsUsable) {
            continue;
        }

        if (Constraint->ColumnNumber != -1) {
            RowidConstraint = FALSE;
            continue;
        }

        ConstraintUsage = &IndexInfo->ConstraintUsage[ConstraintCount++];
        ConstraintUsage->ArgumentIndex = ConstraintCount;
        ConstraintUsage->Omit = TRUE;

        RowidIndex.AsULong = 0;

        switch (Constraint->Op) {

            case EqualConstraint:
                RowidIndex.EqualConstraint = TRUE;
                IndexInfo->IndexFlags = SQLITE_INDEX_SCAN_UNIQUE;
                break;

            case GreaterThanConstraint:
                RowidIndex.GreaterThanConstraint = TRUE;
                break;

            case LessThanOrEqualConstraint:
                RowidIndex.EqualConstraint = TRUE;
                RowidIndex.LessThanConstraint = TRUE;
                break;

            case LessThanConstraint:
                RowidIndex.LessThanConstraint = TRUE;
                break;

            case GreaterThanOrEqualConstraint:
                RowidIndex.EqualConstraint = TRUE;
                RowidIndex.GreaterThanConstraint = TRUE;
                break;

            case MatchConstraint:
            case LikeConstraint:
            case GlobConstraint:
            case RegExpConstraint:
            default:

                //
                // These are all invalid for rowid.
                //

                __debugbreak();
                break;
        }

        //
        // Update the final row index.
        //

        Shifted = (RowidIndex.AsULong & RowidMaskConstraint) << (Index * 3);
        FinalRowidIndex.AsULong |= Shifted;

    }

    IndexInfo->EstimatedRows = EstimatedRows;

    if (ConstraintCount == 0 || !RowidConstraint) {
        IndexInfo->IndexNumber = 0;
        IndexInfo->EstimatedCost = 1.0 * (DOUBLE)IndexInfo->EstimatedRows;
        return SQLITE_OK;
    }

    FinalRowidIndex.IsRowidIndex = TRUE;
    IndexInfo->IndexNumber = FinalRowidIndex.AsLong;
    IndexInfo->EstimatedCost = (DOUBLE)ConstraintCount * 1.0;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleDisconnect(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleDestroy(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleOpenCursor(
    PSQLITE3_VTAB VirtualTable,
    PSQLITE3_VTAB_CURSOR *CursorPointer
    )
{
    HANDLE Event;
    ULONG WaitResult;
    PCSQLITE3 Sqlite3;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_TOTALS Totals;
    PTRACE_STORE_SQLITE3_DB Db;
    PTRACE_STORE_SQLITE3_CURSOR Cursor;
    PTRACE_STORE_INTERVALS Intervals;

    TraceStore = CONTAINING_RECORD(VirtualTable,
                                   TRACE_STORE,
                                   Sqlite3IntervalVirtualTable);

    Db = TraceStore->Db;
    Sqlite3 = Db->Sqlite3;

    Cursor = (PTRACE_STORE_SQLITE3_CURSOR)Sqlite3->Malloc(sizeof(*Cursor));
    if (!Cursor) {
        return SQLITE_NOMEM;
    }

    ZeroStructPointer(Cursor);

    Totals = TraceStore->Totals;
    Intervals = &TraceStore->Intervals;

    //
    // Initialize the cursor.
    //

    Cursor->Db = Db;
    Cursor->Sqlite3 = Sqlite3;
    Cursor->Intervals = Intervals;
    Cursor->TraceStore = TraceStore;
    Cursor->Sqlite3Column = TraceStore->Sqlite3Column;
    Cursor->IntervalRecordSize = sizeof(TRACE_STORE_INTERVAL);

    Cursor->TotalNumberOfRecords = Intervals->NumberOfIntervals;
    Cursor->Flags.FixedRecordSizeAndAlwaysPowerOf2 = TRUE;
    Cursor->Flags.IsIntervalCursor = TRUE;

    //
    // Check for the rare case that our flat mapping hasn't finished loading
    // yet.
    //

    if (!TraceStore->FlatMappingLoaded) {

        Event = Db->TraceContext->LoadingCompleteEvent;
        WaitResult = WaitForSingleObject(Event, INFINITE);

        if (WaitResult != WAIT_OBJECT_0) {
            __debugbreak();
            return SQLITE_ERROR;
        }

        if (Db->TraceContext->FailedCount > 0) {
            __debugbreak();
            return SQLITE_ERROR;
        }
    }

    //
    // Wait for the interval loading complete event to be signaled.
    //

    Event = Intervals->LoadingCompleteEvent;
    WaitResult = WaitForSingleObject(Event, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
        __debugbreak();
        return SQLITE_ERROR;
    }

    QueryPerformanceCounter(&Cursor->OpenedTimestamp);

    TraceStoreSqlite3TlsSetCursor(Cursor);

    *CursorPointer = &Cursor->AsSqlite3VtabCursor;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleCloseCursor(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor
    )
{
    LARGE_INTEGER CloseTimestamp;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    TraceTimeQueryPerformanceCounter(Cursor->TraceStore->Time,
                                     &Elapsed,
                                     &CloseTimestamp);

    TraceStoreSqlite3TlsSetCursor(NULL);

    Cursor->Sqlite3->Free(Cursor);

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleFilter(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor,
    LONG IndexNumber,
    PCSZ IndexString,
    LONG NumberOfArguments,
    PSQLITE3_VALUE *Arguments
    )
{
    BYTE Index;
    BYTE Count;
    ULONG MaskedConstraint;
    ULONGLONG Rowid;
    ULONGLONG ExpectedLastRow;

    PCSQLITE3 Sqlite3;
    PTRACE_STORE_INTERVALS Intervals;
    PTRACE_STORE_SQLITE3_CURSOR Cursor;
    TRACE_STORE_SQLITE3_ROWID_INDEX RowidIndex;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    Cursor->FilterArguments = Arguments;
    Cursor->NumberOfArguments = NumberOfArguments;
    Cursor->FilterIndexNumber = IndexNumber;
    Cursor->FilterIndexStringBuffer = IndexString;

    Intervals = Cursor->Intervals;

    Cursor->Rowid = Cursor->FirstRowid = 0;
    Cursor->MaxRowid = Cursor->Intervals->LastInterval->IntervalIndex;
    Cursor->LastRowid = Cursor->MaxRowid;

    Cursor->FirstRow.AsInterval = Intervals->FirstInterval;
    Cursor->CurrentRow.AsInterval = Intervals->FirstInterval;
    Cursor->LastRow.AsInterval = Intervals->LastInterval;

    //
    // Invariant check.
    //

    ExpectedLastRow = (
        Cursor->FirstRowRaw + (
            Cursor->MaxRowid *
            Cursor->IntervalRecordSize
        )
    );

    if (ExpectedLastRow != Cursor->LastRowRaw) {
        __debugbreak();
        return SQLITE_ERROR;
    }

    RowidIndex.AsLong = IndexNumber;
    if (!RowidIndex.IsRowidIndex) {
        return SQLITE_OK;
    }

    //
    // Toggle the rowid tag back to 0 now that we've confirmed it's a rowid
    // index.  This is done as a precautionary measure as we shift the row
    // index raw value right when extracting predicates.
    //

    RowidIndex.IsRowidIndex = FALSE;

    if (NumberOfArguments < 1 || NumberOfArguments > 2) {

        //
        // There should only ever be 1 or 2 arguments for rowid-based filtering.
        //

        __debugbreak();
        return SQLITE_ERROR;
    }

    Count = (BYTE)NumberOfArguments;
    Sqlite3 = Cursor->Sqlite3;

    for (Index = 0; Index < Count; Index++) {

        Rowid = (ULONGLONG)Sqlite3->ValueInt64(Arguments[Index]);

        //
        // Extract the low three bits.
        //

        MaskedConstraint = RowidIndex.AsULong & RowidMaskConstraint;

        switch (MaskedConstraint) {
            case RowidEqualConstraint:

                //
                // We shouldn't see equal constraints after the first iteration.
                //

                if (Index != 0) {
                    __debugbreak();
                    return SQLITE_ERROR;
                }

                Cursor->FirstRowid = Rowid;
                Cursor->LastRowid = Rowid;
                break;

            case RowidGreaterThanConstraint:
                Cursor->FirstRowid = Rowid + 1;
                break;

            case RowidGreaterThanOrEqualConstraint:
                Cursor->FirstRowid = Rowid;
                break;

            case RowidLessThanConstraint:
                Cursor->LastRowid = Rowid - 1;
                break;

            case RowidLessThanOrEqualConstraint:
                Cursor->LastRowid = Rowid;
                break;

            default:
                __debugbreak();
                return SQLITE_ERROR;
        }

        //
        // Shift the row index right three bits such that the next argument's
        // predicate is in the lower three bits.
        //

        RowidIndex.AsULong >>= 3;
    }

    //
    // Sanity check invariants.
    //

    if (Cursor->FirstRowid > Cursor->LastRowid) {
        __debugbreak();
        return SQLITE_ERROR;
    } else if (Cursor->FirstRowid > Cursor->MaxRowid) {
        __debugbreak();
        return SQLITE_ERROR;
    } else if (Cursor->LastRowid > Cursor->MaxRowid) {
        __debugbreak();
        return SQLITE_ERROR;
    }

    //
    // Advance the first and current rows to the appropriate offset based on the
    // first rowid.  Likewise for the last row and last rowid.
    //

    Cursor->CurrentRow.AsVoid = Cursor->FirstRow.AsVoid = (
        RtlOffsetToPointer(
            Intervals->FirstInterval,
            Cursor->FirstRowid * Cursor->IntervalRecordSize
        )
    );

    Cursor->LastRow.AsVoid = (
        RtlOffsetToPointer(
            Intervals->FirstInterval,
            Cursor->LastRowid * Cursor->IntervalRecordSize
        )
    );

    //
    // Ensure our last row isn't past the last interval.
    //

    if (Cursor->LastRowRaw > (ULONGLONG)Intervals->LastInterval) {
        __debugbreak();
        return SQLITE_ERROR;
    }

    //
    // Point our cursor at the first row.
    //

    Cursor->Rowid = Cursor->FirstRowid;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleNext(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor
    )
{
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    Cursor->Rowid++;
    Cursor->CurrentRowRaw += Cursor->IntervalRecordSize;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleEof(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor
    )
{
    BOOL Eof;
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    Eof = (
        Cursor->Flags.EofOverride ||
        Cursor->Rowid > Cursor->LastRowid
    );

    return Eof;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleColumn(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    return Cursor->Sqlite3Column(Cursor->Sqlite3,
                                 Cursor->TraceStore,
                                 Cursor,
                                 Context,
                                 ColumnNumber);
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleRowid(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor,
    PSQLITE3_INT64 RowidPointer
    )
{
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    *RowidPointer = Cursor->Rowid;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleUpdate(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown1,
    PSQLITE3_VALUE *Value,
    PSQLITE3_INT64 Unknown2
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleBegin(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleSync(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleCommit(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleRollback(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleFindFunction(
    PSQLITE3_VTAB VirtualTable,
    LONG ArgumentNumber,
    PCSZ FunctionName,
    PSQLITE3_SCALAR_FUNCTION *ScalarFunctionPointer,
    PVOID *ArgumentPointer
    )
{
    USHORT MatchIndex;
    STRING String;
    PCSQLITE3 Sqlite3;
    STRING_MATCH Match;
    PTRACE_STORE TraceStore;
    PSTRING_TABLE StringTable;
    PTRACE_STORE_SQLITE3_DB Db;
    TRACE_STORE_SQLITE3_FUNCTION_ID FunctionId;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    __debugbreak();

    TraceStore = CONTAINING_RECORD(VirtualTable,
                                   TRACE_STORE,
                                   Sqlite3IntervalVirtualTable);

    Db = TraceStore->Db;
    Sqlite3 = Db->Sqlite3;

    String.Length = (USHORT)strlen(FunctionName);
    String.MaximumLength = String.Length;
    String.Buffer = (PSTR)FunctionName;

    StringTable = Db->FunctionStringTable1;
    IsPrefixOfStringInTable = Db->StringTableApi.IsPrefixOfStringInTable;

    MatchIndex = IsPrefixOfStringInTable(StringTable, &String, &Match);
    if (MatchIndex == NO_MATCH_FOUND) {
        return SQLITE_ERROR;
    }

    FunctionId = MatchIndex;

    switch (FunctionId) {

        case CountFunctionId:
            break;

        case AverageFunctionId:
            break;

        case SumFunctionId:
            break;

        default:
            break;
    }

    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleRename(
    PSQLITE3_VTAB VirtualTable,
    PCSZ NewName
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleSavepoint(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleRelease(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3IntervalModuleRollbackTo(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_READONLY;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
