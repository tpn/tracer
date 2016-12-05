/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the Rtl component.

--*/

#pragma once

#include "targetver.h"

#include <Windows.h>
#include <sal.h>
#include <Psapi.h>

//
// Disable inconsistent SAL annotation warnings before importing the
// intrinsics headers.
//

#pragma warning(push)
#pragma warning(disable: 28251)
#include <intrin.h>
#include <mmintrin.h>
#pragma warning(pop)

#include "Rtl.h"

#ifdef _RTL_INTERNAL_BUILD
#include "atexit.h"
#include "AtExitEx.h"
#include "RtlPrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
