/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStorePrivate.h

Abstract:

    This is the private header file for the TraceStore component.  It declares
    all major (i.e. not local to the module) functions used by all individual
    modules within this component.

--*/

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

INITIALIZE_TRACE_STORES InitializeTraceStores;
CLOSE_TRACE_STORE CloseTraceStore;

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
