/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TestInjection.h

Abstract:

    This is the header file for the TestInjection component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

typedef
ULONG
(TEST_INJECTION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PDEBUG_ENGINE_SESSION DebugEngineSession
    );
typedef TEST_INJECTION *PTEST_INJECTION;

#pragma component(browser, off)
TEST_INJECTION TestInjection;
ULONG TestInjectionMain(VOID);
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
