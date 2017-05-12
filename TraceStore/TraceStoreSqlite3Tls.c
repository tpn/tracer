/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Tls.c

Abstract:

    This module provides TLS glue to work around limited parameter availability
    during virtual function callbacks.  Tls routines are provided for dllmain
    PROCESS_ATTACH and PROCESS_DETACH events, and for cursors to register and
    unregister themselves with the current thread.

--*/

#include "stdafx.h"

//
// Our TLS index.  Assigned at PROCESS_ATTACH, free'd at PROCESS_DETACH.
//

ULONG TraceStoreSqlite3TlsIndex;

TRACE_STORE_SQLITE3_TLS_FUNCTION TraceStoreSqlite3TlsProcessAttach;

_Use_decl_annotations_
TraceStoreSqlite3TlsProcessAttach(
    HMODULE Module,
    ULONG   Reason,
    LPVOID  Reserved
    )
{
    TraceStoreSqlite3TlsIndex = TlsAlloc();

    if (TraceStoreSqlite3TlsIndex == TLS_OUT_OF_INDEXES) {
        OutputDebugStringA("TraceStore: TlsAlloc() out of indexes.\n");
        return FALSE;
    }

    return TRUE;
}


TRACE_STORE_SQLITE3_TLS_FUNCTION TraceStoreSqlite3TlsProcessDetach;

_Use_decl_annotations_
TraceStoreSqlite3TlsProcessDetach(
    HMODULE Module,
    ULONG   Reason,
    LPVOID  Reserved
    )
{
    BOOL IsProcessTerminating;

    IsProcessTerminating = (Reserved != NULL);

    if (IsProcessTerminating) {
        goto End;
    }

    if (TraceStoreSqlite3TlsIndex == TLS_OUT_OF_INDEXES) {
        goto End;
    }

    if (!TlsFree(TraceStoreSqlite3TlsIndex)) {
        OutputDebugStringA("TraceStore: TlsFree() failed.\n");
    }

    //
    // Note that we always return TRUE here, even if we had a failure.  We're
    // only called at DLL_PROCESS_DETACH, so there's not much we can do when
    // there is actually an error anyway.
    //

End:

    return TRUE;
}

//
// TLS Set/Get Cursor functions.
//

TRACE_STORE_SQLITE3_TLS_SET_CURSOR TraceStoreSqlite3TlsSetCursor;

_Use_decl_annotations_
BOOL
TraceStoreSqlite3TlsSetCursor(
    PTRACE_STORE_SQLITE3_CURSOR Cursor
    )
{
    return TlsSetValue(TraceStoreSqlite3TlsIndex, Cursor);
}

TRACE_STORE_SQLITE3_TLS_GET_CURSOR TraceStoreSqlite3TlsGetCursor;

_Use_decl_annotations_
PTRACE_STORE_SQLITE3_CURSOR
TraceStoreSqlite3TlsGetCursor(
    VOID
    )
{
    PVOID Value;

    Value = TlsGetValue(TraceStoreSqlite3TlsIndex);

    return (PTRACE_STORE_SQLITE3_CURSOR)Value;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
