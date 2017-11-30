/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TracerExeMain.c

Abstract:

    This module implements TracerExeMain().  It is intended to be called from
    the mainCRTStartup() routine of an executable.

--*/

#include "stdafx.h"

ULONG
TracerExeMain(VOID)
{
    BOOL Success;
    PRTL Rtl;
    ULONG ExitCode;
    ALLOCATOR Allocator;
    ALLOCATOR StringTableAllocator;
    ALLOCATOR StringArrayAllocator;

    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    PUNICODE_STRING DebugEngineDllPath;
    PUNICODE_STRING TracingDllPath = NULL;
    PDEBUG_ENGINE_SESSION Session;
    PTRACER_INJECTION_MODULES InjectionModules;
    PDESTROY_DEBUG_ENGINE_SESSION DestroyDebugEngineSession;
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags = { 0 };

    //
    // Initialize the default heap allocator.  This is a thin wrapper around
    // the generic Win32 Heap functions.
    //

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        goto Error;
    }

    //
    // Zero the allocators for the StringTable component.
    //

    ZeroStruct(StringArrayAllocator);
    ZeroStruct(StringTableAllocator);

    //
    // Initialize TracerConfig and Rtl.
    //

    RegistryPath = (PUNICODE_STRING)&TracerRegistryPath;

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            &Allocator,
            RegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    //
    // Load the tracer modules that support injection.
    //

    CHECKED_MSG(
        CreateAndInitializeTracerInjectionModules(
            Rtl,
            &Allocator,
            TracerConfig,
            &InjectionModules
        ),
        "CreateAndInitializeTracerInjectionModules()"
    );

    //
    // Extract the target process ID and thread ID from the command line.
    //

    InitFlags.StartServer = FALSE;
    InitFlags.InitializeFromCommandLine = TRUE;
    DebugEngineDllPath = &TracerConfig->Paths.DebugEngineDllPath;
    Success = LoadAndInitializeDebugEngineSessionWithInjectionIntent(
        DebugEngineDllPath,
        Rtl,
        &Allocator,
        InitFlags,
        TracerConfig,
        &TracerConfig->Paths.StringTableDllPath,
        &StringArrayAllocator,
        &StringTableAllocator,
        InjectionModules,
        &Session,
        &DestroyDebugEngineSession
    );

    if (!Success) {
        OutputDebugStringA("LoadAndInitializeDebugEngineSession"
                           "WithInjectionIntent() failed.\n");
        goto Error;
    };

    //
    // Main loop.
    //

#if 0
    RunOnce = TRUE;

    while (TRUE) {
        if (RunOnce) {
            Session->WaitForEvent(Session, 0, 3000);
        } else {
            Sleep(3000);
        }
    }
#endif

    Success = Session->EventLoop(Session);
    //SleepEx(INFINITE, TRUE);

    ExitCode = 0;
    goto End;

Error:
    ExitCode = 1;

End:
    return ExitCode;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
