/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracerCorePrivate.h

Abstract:

    This is the private header file for the TracerCore component.  It defines function
    typedefs and function declarations for all major (i.e. not local to the
    module) functions available for use by individual modules within this
    component.

--*/

#ifndef _TRACER_CORE_INTERNAL_BUILD
#error TracerCorePrivate.h being included but _TRACER_CORE_INTERNAL_BUILD not set.
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Private function declarations.
//

#pragma component(browser, off)
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
