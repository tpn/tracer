/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the TraceStore component.

--*/

#pragma once

#include "targetver.h"

//
// <concurrencysal.h> appears to need _PREFAST_ defined.
//

#ifndef _PREFAST_
#define _PREFAST_
#endif

#include <sal.h>
#include <concurrencysal.h>

#include <Windows.h>
#include <Strsafe.h>
#include "../Rtl/Rtl.h"
#include "../Rtl/Sqlite.h"
#include "../Rtl/Cu.h"
#include "../Rtl/__C_specific_handler.h"
#include "../Rtl/atexit.h"
#include "../TracerConfig/TracerConfig.h"
#include "../TracerHeap/TracerHeap.h"
#include "../DebugEngine/DebugEngine.h"
#include "TraceStoreIndex.h"
#include "TraceStore.h"
#include "TraceStorePrivate.h"
#include "TraceStoreConstants.h"

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
