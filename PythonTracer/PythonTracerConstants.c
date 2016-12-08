/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.h

Abstract:

    This module declares constants used by the PythonTracer component.

--*/

#include "stdafx.h"

//
// Constant unicode strings.
//

#define ROOT_REGISTRY_PATH L"Software\\Tracer\\PythonTracer"
#define LAST_RUN_REGISTRY_PATH L"Software\\Tracer\\PythonTracer\\LastRun"

CONST UNICODE_STRING RootRegistryPath = \
    RTL_CONSTANT_STRING(ROOT_REGISTRY_PATH);

CONST UNICODE_STRING LastRunRegistryPath = \
    RTL_CONSTANT_STRING(LAST_RUN_REGISTRY_PATH);

//
// Trace event types.
//

CONST PPY_TRACE_EVENT PythonTraceEventTypeToFunctionPointer[] = {
    PyTraceEvent1,
    PyTraceEvent2,
    PyTraceEvent3,
    PyTraceEvent4
};


//
// CallbackWorker types.
//
//

CONST PPY_TRACE_CALLBACK PythonTraceCallbackWorkerTypeToFunctionPointer[] = {
    PyTraceCallbackWorker1,
    PyTraceCallbackWorker2
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
