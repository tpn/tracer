/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Ext.h

Abstract:

    This is the main header file for the TraceStore sqlite3 extension component.
    It defines structures and functions related to the implementation of the
    component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Define the trace store database structure.
//

typedef struct _TRACE_STORE_SQLITE3_DB {
    PRTL Rtl;
    PALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING TraceSessionDirectory;
    PTRACE_STORES TraceStores;
    PTRACE_SESSION TraceSession;
    PTRACE_CONTEXT TraceContext;

    //
    // Threadpool and callback environment.
    //

    ULONG MaximumProcessorCount;
    PTP_POOL Threadpool;
    TP_CALLBACK_ENVIRON ThreadpoolCallbackEnviron;

    //
    // A threadpool with 1 thread limited to cancellations.
    //

    PTP_POOL CancellationThreadpool;
    TP_CALLBACK_ENVIRON CancellationThreadpoolCallbackEnviron;


    PALLOCATOR Sqlite3Allocator;
    PSQLITE3_DB Sqlite3Db;
    PSQLITE3 Sqlite3;
} TRACE_STORE_SQLITE3_DB;

//
// Define Trace Store sqlite3 cursor and vtab structures.
//

typedef struct _TRACE_STORE_SQLITE3_CURSOR {

    ULONGLONG Rowid;

    //
    // Inline SQLITE3_VTAB_CURSOR structure.
    //
    // N.B.: This structure must always come last.
    //

    union {
        struct {
            PSQLITE3_VTAB VirtualTable;
        };
        SQLITE3_VTAB_CURSOR AsSqlite3VtabCursor;
    };

} TRACE_STORE_SQLITE3_CURSOR;

typedef struct _TRACE_STORE_SQLITE3_VTAB {

    PRTL Rtl;
    PTRACE_STORE TraceStore;

    //
    // Inline SQLITE3_VTAB structure.
    //
    // N.B.: This structure must always come last.
    //

    union {
        struct {
            union {
                PSQLITE3_MODULE Sqlite3Module;
                struct _TRACE_STORE_SQLITE3_MODULE *Module;
            };
            LONG NumberOfOpenCursors;
            LONG Padding;
            PCSZ ErrorMessage;
        };
        SQLITE3_VTAB AsSqlite3Vtab;
    };

} TRACE_STORE_SQLITE3_VTAB;

//
// Declare sqlite3 module functions.
//

extern SQLITE3_CREATE TraceStoreSqlite3ModuleCreate;
extern SQLITE3_CONNECT TraceStoreSqlite3ModuleConnect;
extern SQLITE3_BEST_INDEX TraceStoreSqlite3ModuleBestIndex;
extern SQLITE3_DISCONNECT TraceStoreSqlite3ModuleDisconnect;
extern SQLITE3_DESTROY TraceStoreSqlite3ModuleDestroy;
extern SQLITE3_OPEN TraceStoreSqlite3ModuleOpen;
extern SQLITE3_CLOSE TraceStoreSqlite3ModuleClose;
extern SQLITE3_FILTER TraceStoreSqlite3ModuleFilter;
extern SQLITE3_NEXT TraceStoreSqlite3ModuleNext;
extern SQLITE3_EOF TraceStoreSqlite3ModuleEof;
extern SQLITE3_COLUMN TraceStoreSqlite3ModuleColumn;
extern SQLITE3_ROWID TraceStoreSqlite3ModuleRowid;
extern SQLITE3_UPDATE TraceStoreSqlite3ModuleUpdate;
extern SQLITE3_BEGIN TraceStoreSqlite3ModuleBegin;
extern SQLITE3_SYNC TraceStoreSqlite3ModuleSync;
extern SQLITE3_COMMIT TraceStoreSqlite3ModuleCommit;
extern SQLITE3_ROLLBACK TraceStoreSqlite3ModuleRollback;
extern SQLITE3_FIND_FUNCTION TraceStoreSqlite3ModuleFindFunction;
extern SQLITE3_RENAME TraceStoreSqlite3ModuleRename;
extern SQLITE3_SAVEPOINT TraceStoreSqlite3ModuleSavepoint;
extern SQLITE3_RELEASE TraceStoreSqlite3ModuleRelease;
extern SQLITE3_ROLLBACK_TO TraceStoreSqlite3ModuleRollbackTo;

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
