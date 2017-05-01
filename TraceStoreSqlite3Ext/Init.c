/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Init.c

Abstract:

    WIP.

--*/

#include "stdafx.h"

LONG
TraceStoreSqlite3ExtInit(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PSQLITE3_DB Database,
    PCSZ *ErrorMessagePointer,
    PCSQLITE3 Sqlite3
    )
/*++

Routine Description:

    This is the main entry point for the TraceStore sqlite3 extension module.
    It is called by our extension loader thunk (TraceStoreSqlite3ExtLoader).

    It is responsible for registering as module to sqlite3.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure.

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    Database - Supplies a pointer to the active sqlite3 database.

    ErrorMessagePointer - Supplies a variable that optionally receives the
        address of an error message if initialization fails.

    Sqlite3 - Supplies a pointer to the sqlite3 API routines.

Return Value:

    SQLITE_OK on success, either SQLITE_NOMEM or SQLITE_ERROR on error.

--*/
{

    //
    // WIP.
    //

    return SQLITE3_OK;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
