/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the TraceStoreSqlite3Ext component.

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
#include "../Rtl/__C_specific_handler.h"
#include "../TracerConfig/TracerConfig.h"
#include "../TraceStore/TraceStore.h"
#include "TraceStoreSqlite3Ext.h"

#ifdef _TRACE_STORE_INTERNAL_BUILD
#include "TraceStoreSqlite3ExtPrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
