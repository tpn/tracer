/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the TracerExe component.

--*/

#pragma once

#include "targetver.h"

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../Rtl/HeapAllocator.h"
#include "../Rtl/__C_specific_handler.h"
#include "../DebugEngine/DebugEngine.h"
#include "../TraceStore/TraceStore.h"
#include "../TracerConfig/TracerConfig.h"
#include "../TracerCore/TracerCore.h"

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
