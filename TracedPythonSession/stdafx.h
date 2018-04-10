/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the TracedPythonSession component.

--*/

#pragma once

#include "targetver.h"

#include <Windows.h>

#include "../Rtl/Rtl.h"
#include "../Python/Python.h"
#include "../TracerConfig/TracerConfig.h"
#include "../TracerHeap/TracerHeap.h"
#include "../TraceStore/TraceStore.h"
#include "../StringTable.h"
#include "../PythonTracer/PythonTracer.h"
#include "TracedPythonSession.h"

#ifdef _TRACED_PYTHON_SESSION_INTERNAL_BUILD
#include "TracedPythonSessionPrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
