/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Module.c

Abstract:

    WIP.

--*/

#include "stdafx.h"

SQLITE3_MODULE TraceStoreSqlite3Module = {
    0,  // Version
    0,  // Padding
    TraceStoreSqlite3ModuleCreate,
    TraceStoreSqlite3ModuleConnect,
    TraceStoreSqlite3ModuleBestIndex,
    TraceStoreSqlite3ModuleDisconnect,
    TraceStoreSqlite3ModuleDestroy,
    TraceStoreSqlite3ModuleOpen,
    TraceStoreSqlite3ModuleClose,
    TraceStoreSqlite3ModuleFilter,
    TraceStoreSqlite3ModuleNext,
    TraceStoreSqlite3ModuleEof,
    TraceStoreSqlite3ModuleColumn,
    TraceStoreSqlite3ModuleRowid,
    TraceStoreSqlite3ModuleUpdate,
    TraceStoreSqlite3ModuleBegin,
    TraceStoreSqlite3ModuleSync,
    TraceStoreSqlite3ModuleCommit,
    TraceStoreSqlite3ModuleRollback,
    TraceStoreSqlite3ModuleFindFunction,
    TraceStoreSqlite3ModuleRename,
    TraceStoreSqlite3ModuleSavepoint,
    TraceStoreSqlite3ModuleRelease,
    TraceStoreSqlite3ModuleRollbackTo,
};

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleCreate(
    PSQLITE3_DB Database,
    PVOID Aux,
    LONG NumberOfArguments,
    PCSZ Arguments,
    PSQLITE3_VTAB *VirtualTable
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleBestIndex(
    PSQLITE3_VTAB VirtualTable,
    PSQLITE3_INDEX_INFO IndexInfo
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleDisconnect(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleDestroy(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleOpen(
    PSQLITE3_VTAB VirtualTable,
    PSQLITE3_VTAB_CURSOR *Cursor
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleClose(
    PSQLITE3_VTAB_CURSOR Cursor
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleFilter(
    PSQLITE3_VTAB_CURSOR Cursor,
    LONG IndexNumber,
    PCSZ IndexString,
    LONG ArgumentCount,
    PSQLITE3_VALUE *Arguments
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleNext(
    PSQLITE3_VTAB_CURSOR Cursor
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleEof(
    PSQLITE3_VTAB_CURSOR Cursor
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleColumn(
    PSQLITE3_VTAB_CURSOR Cursor,
    PSQLITE3_CONTEXT Context,
    LONG ColumnNumber
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRowid(
    PSQLITE3_VTAB_CURSOR Cursor,
    PSQLITE3_INT64 *Rowid
    )
{
    return SQLITE_ERROR;
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
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleBegin(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleSync(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleCommit(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRollback(
    PSQLITE3_VTAB VirtualTable
    )
{
    return SQLITE_ERROR;
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
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleSavepoint(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRelease(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_ERROR;
}

_Use_decl_annotations_
LONG
TraceStoreSqlite3ModuleRollbackTo(
    PSQLITE3_VTAB VirtualTable,
    LONG Unknown
    )
{
    return SQLITE_ERROR;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
