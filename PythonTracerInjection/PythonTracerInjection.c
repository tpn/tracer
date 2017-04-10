/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerInjection.c

Abstract:

    This module implements the necessary routines to facilitate tracer
    injection of Python modules.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializePythonTracerInjection(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION DebugEngineSession
    )
/*++

Routine Description:

    This function initializes tracer injection for Python.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure.

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    DebugEngineSession - Supplies a pointer to an initialized
        DEBUG_ENGINE_SESSION structure.

Return Value:

    TRUE on Success, FALSE if an error occurred.

--*/
{
    BOOL Success;

    Success = TRUE;

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
