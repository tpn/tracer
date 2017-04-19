/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionThunk.h

Abstract:

    This is the main header file for the InjectionThunk component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define INJECTION_THUNK_CALL_CONV __stdcall

#ifdef _INJECTION_THUNK_INTERNAL_BUILD

//
// This is an internal build of the InjectionThunk component.
//

#define INJECTION_THUNK_API __declspec(dllexport)
#define INJECTION_THUNK_DATA extern __declspec(dllexport)

#include "stdafx.h"

#elif _INJECTION_THUNK_NO_API_EXPORT_IMPORT

//
// We're being included by someone who doesn't want dllexport or dllimport.
// This is useful for creating new .exe-based projects for things like unit
// testing or performance testing/profiling.
//

#define INJECTION_THUNK_API
#define INJECTION_THUNK_DATA extern

#else

//
// We're being included by an external component.
//

#define INJECTION_THUNK_API __declspec(dllimport)
#define INJECTION_THUNK_DATA extern __declspec(dllimport)

#include "../Rtl/Rtl.h"

#endif

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
