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
LONG
(TEST_INJECTION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PDEBUG_ENGINE_SESSION DebugEngineSession
    );
typedef TEST_INJECTION *PTEST_INJECTION;

typedef STARTUPINFOW *PSTARTUPINFOW;

BOOL
CreateThunkExe(
    PSTARTUPINFOW StartupInfo,
    PPROCESS_INFORMATION ProcessInfo
    );

extern PUNICODE_STRING InjectionThunkActiveDllPath;
extern PUNICODE_STRING InjectionThunkDebugDllPath;
extern PUNICODE_STRING InjectionThunkReleaseDllPath;

extern PUNICODE_STRING TestInjectionActiveExePath;
extern PUNICODE_STRING TestInjectionDebugExePath;
extern PUNICODE_STRING TestInjectionReleaseExePath;

extern ULONG TestDummyLong;

TEST_INJECTION TestInjection;

BOOL
TestInjectThunk(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    );

BOOL
TestInjectionObjects(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    );

ULONG TestInjectionMain(VOID);

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
