/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    stdafx.h

Abstract:

    This is the precompiled header file for the TracerConfig component.

--*/

#pragma once

#include "targetver.h"

#include "../Rtl/Rtl.h"
#include "../TracerHeap/TracerHeap.h"
#include "TracerConfig.h"
#include "TracerConfigConstants.h"

#ifdef _TRACER_CONFIG_INTERNAL_BUILD
#include "TracerConfigPrivate.h"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
