/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    AsmPrivate.h

Abstract:

    This is the private header file for the Asm component.  It defines function
    typedefs and function declarations for all major (i.e. not local to the
    module) functions available for use by individual modules within this
    component.

--*/

#ifndef _ASM_INTERNAL_BUILD
#error AsmPrivate.h being included but _ASM_INTERNAL_BUILD not set.
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
