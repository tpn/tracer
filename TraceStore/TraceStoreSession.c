/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSession.c

Abstract:

    This module implements trace store session functionality.  Functions
    are provided for initializing a trace store session record.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeTraceSession(
    PRTL Rtl,
    PTRACE_SESSION TraceSession,
    PULONG SizeOfTraceSession
    )
/*++

Routine Description:

    This routine initializes an allocated TRACE_SESSION record.

Arguments:

    TBD.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    if (!TraceSession) {
        if (SizeOfTraceSession) {
            *SizeOfTraceSession = sizeof(*TraceSession);
        }
        return FALSE;
    }

    if (!SizeOfTraceSession) {
        return FALSE;
    }

    if (*SizeOfTraceSession < sizeof(*TraceSession)) {
        return FALSE;
    } else if (*SizeOfTraceSession == 0) {
        *SizeOfTraceSession = sizeof(*TraceSession);
    }

    SecureZeroMemory(TraceSession, sizeof(*TraceSession));

    TraceSession->Rtl = Rtl;

    GetSystemTimeAsFileTime(&TraceSession->SystemTime);
    return TRUE;

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
