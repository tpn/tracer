/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqliteExt.h

Abstract:

    This is the header file for the TraceStore's sqlite3 extension component.

--*/

#include "stdafx.h"

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
