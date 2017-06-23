/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    RtlConstants.h

Abstract:

    This module declares constants used by the Rtl component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

RTL_DATA CONST UNICODE_STRING RtlInjectionSignalEventNamePrefix;
RTL_DATA CONST UNICODE_STRING RtlInjectionWaitEventNamePrefix;

RTL_DATA CONST PCSTR RtlFunctionNames[];
RTL_DATA CONST PCSTR RtlExFunctionNames[];
RTL_DATA CONST PCSTR DbgHelpFunctionNames[];
RTL_DATA CONST PCSTR CuFunctionNames[];

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
