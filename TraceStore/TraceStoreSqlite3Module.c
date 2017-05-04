/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Module.c

Abstract:

    WIP.

--*/

#include "stdafx.h"

//
// This is required in order to link without the CRT.
//

LONG _fltused;

//
// Define the default Trace Store sqlite3 module function pointers.
//

SQLITE3_MODULE TraceStoreSqlite3Module = {
    0,  // Version
    0,  // Padding
    TraceStoreSqlite3ModuleCreate,
    TraceStoreSqlite3ModuleCreate,
    TraceStoreSqlite3ModuleBestIndex,
    TraceStoreSqlite3ModuleDisconnect,
    NULL, // TraceStoreSqlite3ModuleDestroy,
    TraceStoreSqlite3ModuleOpenCursor,
    TraceStoreSqlite3ModuleCloseCursor,
    TraceStoreSqlite3ModuleFilter,
    TraceStoreSqlite3ModuleNext,
    TraceStoreSqlite3ModuleEof,
    TraceStoreSqlite3ModuleColumn,
    TraceStoreSqlite3ModuleRowid,
    NULL, // TraceStoreSqlite3ModuleUpdate,
    NULL, // TraceStoreSqlite3ModuleBegin,
    NULL, // TraceStoreSqlite3ModuleSync,
    NULL, // TraceStoreSqlite3ModuleCommit,
    NULL, // TraceStoreSqlite3ModuleRollback,
    NULL, // TraceStoreSqlite3ModuleFindFunction,
    NULL, // TraceStoreSqlite3ModuleRename,
    NULL, // TraceStoreSqlite3ModuleSavepoint,
    NULL, // TraceStoreSqlite3ModuleRelease,
    NULL, // TraceStoreSqlite3ModuleRollbackTo,
};

//
// Define a dummy column function used by default.
//

_Use_decl_annotations_
LONG
TraceStoreSqlite3DefaultColumnImpl(
    PCSQLITE3 Sqlite3,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_SQLITE3_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    Sqlite3->ResultNull(Context);
    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleCreate(
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

    Failed = Sqlite3->DeclareVirtualTable(Sqlite3Db, TraceStore->Sqlite3Schema);
    if (Failed) {
        return Failed;
    }

    *VirtualTable = &TraceStore->Sqlite3VirtualTable.AsSqlite3Vtab;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleBestIndex(
    PSQLITE3_VTAB VirtualTable,
    PSQLITE3_INDEX_INFO IndexInfo
    )
{
    LONG Index;
    PCSQLITE3 Sqlite3;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_SQLITE3_DB Db;
    PSQLITE3_INDEX_CONSTRAINT Constraint;

    TraceStore = CONTAINING_RECORD(VirtualTable,
                                   TRACE_STORE,
                                   Sqlite3VirtualTable);

    Db = TraceStore->Db;
    Sqlite3 = Db->Sqlite3;

    for (Index = 0; Index < IndexInfo->NumberOfConstraints; Index++) {
        Constraint = IndexInfo->Constraints + Index;
        if (!Constraint->IsUsable) {
            continue;
        }
    }

    IndexInfo->IndexNumber = 1;
    IndexInfo->EstimatedCost = 1.0;
    IndexInfo->EstimatedRows = TraceStore->Totals->NumberOfRecords.QuadPart;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleDisconnect(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleDestroy(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleOpenCursor(
    PSQLITE3_VTAB VirtualTable,
    PSQLITE3_VTAB_CURSOR *CursorPointer
    )
{
    PCSQLITE3 Sqlite3;
    PTRACE_STORE TraceStore;
    PTRACE_STORE_TOTALS Totals;
    PTRACE_STORE_SQLITE3_DB Db;
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    TraceStore = CONTAINING_RECORD(VirtualTable,
                                   TRACE_STORE,
                                   Sqlite3VirtualTable);

    Db = TraceStore->Db;
    Sqlite3 = Db->Sqlite3;
    Totals = TraceStore->Totals;

    Cursor = (PTRACE_STORE_SQLITE3_CURSOR)Sqlite3->Malloc(sizeof(*Cursor));
    if (!Cursor) {
        return SQLITE_NOMEM;
    }

    ZeroStructPointer(Cursor);

    //
    // Initialize the cursor.
    //

    Cursor->Db = Db;
    Cursor->TraceStore = TraceStore;
    Cursor->Traits = *TraceStore->pTraits;
    Cursor->Sqlite3 = Sqlite3;
    Cursor->Sqlite3Column = TraceStore->Sqlite3Column;
    Cursor->TotalNumberOfAllocations = Totals->NumberOfAllocations.QuadPart;

    if (!TraceStore->IsMetadata) {
        Cursor->Address = TraceStore->ReadonlyAddresses;
        Cursor->MemoryMap = &TraceStore->FlatMemoryMap;
        Cursor->AddressRange = TraceStore->ReadonlyAddressRanges;
        Cursor->TotalNumberOfRecords = Totals->NumberOfRecords.QuadPart;
    } else {
        Cursor->MemoryMap = &TraceStore->SingleMemoryMap;
        Cursor->TotalNumberOfRecords = Cursor->TotalNumberOfAllocations;
    }

    //
    // Check for the rare case that our flat mapping hasn't finished loading
    // yet.
    //

    if (!TraceStore->FlatMappingLoaded) {
        ULONG WaitResult;
        HANDLE Event;

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

    Cursor->FirstRow.AsVoid = Cursor->MemoryMap->BaseAddress;
    Cursor->CurrentRow.AsVoid = Cursor->MemoryMap->BaseAddress;

    QueryPerformanceCounter(&Cursor->OpenedTimestamp);

    *CursorPointer = &Cursor->AsSqlite3VtabCursor;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleCloseCursor(
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

    Cursor->Sqlite3->Free(Cursor);

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleFilter(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor,
    LONG IndexNumber,
    PCSZ IndexString,
    LONG ArgumentCount,
    PSQLITE3_VALUE *Arguments
    )
{
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    Cursor->Rowid = 0;

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleNext(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor
    )
{
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    Cursor->Rowid++;

    //
    // Need to calculate the next address, plus see if we need to switch base
    // addresses because we've exhausted that memory map...
    //
    // This functionality should probably be encapsulated in a ReadRecords()
    // type function (that parallels AllocateRecords()).
    //

    Cursor->CurrentRow.AsVoid = (PVOID)(
        ((ULONG_PTR)Cursor->CurrentRow.AsVoid) + 8
        //Cursor->RecordSize
    );

    return SQLITE_OK;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleEof(
    PSQLITE3_VTAB_CURSOR Sqlite3Cursor
    )
{
    PTRACE_STORE_SQLITE3_CURSOR Cursor;

    Cursor = CONTAINING_RECORD(Sqlite3Cursor,
                               TRACE_STORE_SQLITE3_CURSOR,
                               AsSqlite3VtabCursor);

    return (Cursor->Rowid >= Cursor->TotalNumberOfRecords);
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleColumn(
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
TraceStoreSqlite3ModuleRowid(
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
TraceStoreSqlite3ModuleUpdate(
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
TraceStoreSqlite3ModuleBegin(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleSync(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleCommit(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRollback(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleFindFunction(
    PSQLITE3_VTAB VirtualTable,
    LONG ArgumentNumber,
    PCSZ FunctionName,
    PSQLITE3_FUNCTION Function,
    PPVOID Argument
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRename(
    PSQLITE3_VTAB VirtualTable,
    PCSZ NewName
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleSavepoint(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRelease(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_READONLY;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRollbackTo(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_READONLY;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
