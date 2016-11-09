/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerPrivate.h

Abstract:

    This is the private header file for the PythonTracer component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
CopyPythonTraceEvent(
    _Out_ PPYTHON_TRACE_EVENT DestEvent,
    _In_ _Const_ PPYTHON_TRACE_EVENT SourceEvent
    )
/*++

Routine Description:

    This is a helper routine that can be used to safely copy a trace event
    structure when either the source or destination is backed by memory
    mapped memory.  Internally, it is simply a __movsq() wrapped in a
    __try/__except block that catches STATUS_IN_PAGE_ERROR exceptions.

Arguments:

    DestEvent - Supplies a pointer to the destination event to which the
        source event will be copied.

    SourceAddress - Supplies a pointer to the source event to copy into
        the destination event.

Return Value:

    TRUE on success, FALSE if a STATUS_IN_PAGE_ERROR occurred.

--*/
{
    TRY_MAPPED_MEMORY_OP {
        __movsq((PDWORD64)DestEvent,
                (PDWORD64)SourceEvent,
                sizeof(*DestEvent) >> 3);
        return TRUE;
    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
