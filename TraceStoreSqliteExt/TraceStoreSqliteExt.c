/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqliteExt.h

Abstract:

    This is the header file for the TraceStore's sqlite3 extension component.

--*/

#include "stdafx.h"


LONG
TraceStoreSqliteExtInit(
    PSQLITE3_DB Database,
    PCSZ *ErrorMessagePointer,
    PCSQLITE3 Sqlite
    )
/*++

Routine Description:

    This is the main entry point for the TraceStore sqlite3 extension module.
    It is called by sqlite3 when TraceStore.dll is loaded as an extension.

Arguments:

    Database - Supplies a pointer to the active sqlite3 database.

    ErrorMessagePointer - Supplies a variable that optionally receives the
        address of an error message if initialization fails.

    Sqlite - Supplies a pointer to the sqlite3 API routines.

Return Value:

    SQLITE_OK on success, an appropriate error code on error.

--*/
{
    //
    //
    //

    return SQLITE_OK;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
