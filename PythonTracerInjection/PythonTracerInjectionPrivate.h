/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerInjectionPrivate.h

Abstract:

    This is the private header file for the PythonTracerInjection component.
    It defines function typedefs and function declarations for all major
    (i.e. not local to the module) functions available for use by individual
    modules within this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

extern CONST DEBUGEVENTCALLBACKS PythonTracerInjectionDebugEventCallbacks;


#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
