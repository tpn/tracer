/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PyTraceEvent4.c

Abstract:

    This is currently a placeholder module.

--*/

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

_Use_decl_annotations_
BOOL
PyTraceEvent4(
    PPYTHON_TRACE_CONTEXT Context,
    PPYTHON_FUNCTION Function,
    PPYTHON_EVENT_TRAITS EventTraits,
    PPYFRAMEOBJECT FrameObject,
    PPYOBJECT ArgObject
    )
{
    return TRUE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
