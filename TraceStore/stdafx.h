/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the TraceStore component.

--*/

#pragma once

#ifdef _TRACE_STORE_INTERNAL_BUILD

//
// This is an internal build of the TraceStore component.
//

#define TRACE_STORE_API __declspec(dllexport)
#define TRACE_STORE_DATA extern __declspec(dllexport)

#else

//
// We're being included by an external component.
//

#define TRACE_STORE_API __declspec(dllimport)
#define TRACE_STORE_DATA extern __declspec(dllimport)

#endif

#include "targetver.h"

#include <Windows.h>
#include <sal.h>
#include <Strsafe.h>
#include "../Rtl/Rtl.h"
#include "../TracerConfig/TracerConfig.h"
#include "TraceStoreIndex.h"
#include "TraceStore.h"
#include "TraceStorePrivate.h"
#include "TraceStoreConstants.h"

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
