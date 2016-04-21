// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Rtl/Rtl.h"

typedef BOOL (*PHOOK)(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction);
typedef BOOL (*PUNHOOK)(PRTL Rtl, PVOID *ppHookedFunction);

RTL_API
BOOL
Hook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction);

RTL_API
BOOL
Unhook(PRTL Rtl, PVOID *ppHookedFunction);

#ifdef __cpplus
} // extern "C"
#endif