/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the PythonTracer component.

--*/

#pragma once

#include "targetver.h"

#include <Windows.h>
#include <wincrypt.h>
#include "../Rtl/Rtl.h"
#include "../Rtl/__C_specific_handler.h"
#include "../Rtl/AtExitEx.h"
#include "../Python/Python.h"
#include "../TraceStore/TraceStore.h"
#include "../StringTable.h"
#include "PythonTracer.h"

#ifdef _PYTHON_TRACER_INTERNAL_BUILD
#include "PythonTracerConstants.h"
#include "PythonTracerPrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
