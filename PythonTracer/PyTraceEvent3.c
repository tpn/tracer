/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PyTraceEvent3.c

Abstract:

    This module is a dummy tracer that doesn't actually do anything.  It is
    intended to be used to capture the overhead of enabling the Python tracing
    and profiling machinery.

--*/

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

_Use_decl_annotations_
BOOL
PyTraceEvent3(
    PPYTHON_TRACE_CONTEXT   Context,
    PPYFRAMEOBJECT          FrameObject,
    LONG                    EventType,
    PPYOBJECT               ArgObject
    )
{
    return TRUE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
