/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the Trl component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../Rtl/Rtl.h"
#include "Trl.h"

#ifdef _TRL_INTERNAL_BUILD
#include "TrlPrivate.h"
#endif

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
