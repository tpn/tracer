/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.h

Abstract:

    This module declares constants used by the PythonTracer component.

--*/

#include "stdafx.h"

PPY_TRACE_EVENT PythonTraceEventTypeToFunctionPointer[] = {
    PyTraceEvent1,
    PyTraceEvent2,
    PyTraceEvent3,
    PyTraceEvent4
};

_Use_decl_annotations_
PPY_TRACE_EVENT
GetFunctionPointerForTraceEventType(
    PYTHON_TRACE_EVENT_TYPE TraceEventType
    )
{
    if (TraceEventType == PythonTraceEventNull ||
        TraceEventType >= PythonTraceEventInvalid) {
        return NULL;
    }

    return PythonTraceEventTypeToFunctionPointer[TraceEventType-1];
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
