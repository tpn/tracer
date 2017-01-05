/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreConstants.h

Abstract:

    This module declares constants used by the PythonTracer component.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

CONST UNICODE_STRING RootRegistryPath;
CONST UNICODE_STRING RunHistoryDateFormat;
CONST UNICODE_STRING RunHistoryRegistryPathPrefix;
CONST UNICODE_STRING RunHistoryRegistryPathFormat;

CONST PPY_TRACE_EVENT PythonTraceEventTypeToFunctionPointer[];
CONST PPY_TRACE_CALLBACK PythonTraceCallbackWorkerTypeToFunctionPointer[];

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
