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
#include "Asm.h"

#ifdef _ASM_INTERNAL_BUILD
#include "AsmPrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
