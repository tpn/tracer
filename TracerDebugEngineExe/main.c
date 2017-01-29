/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This is the main module for the TracerDebugEngineExe component.

    It implements a Main() function and provides a mainCRTStartup() entry point
    which calls Main().

--*/

#include "stdafx.h"

ULONG
Main(VOID)
{
    BOOL Success;
    PRTL Rtl;
    ULONG ExitCode;
    ALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    PUNICODE_STRING DebugEngineDllPath;
    PDEBUG_ENGINE_SESSION Session;
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
    // Extract the target process ID and thread ID from the command line.
    //

    InitFlags.InitializeFromCommandLine = TRUE;
    DebugEngineDllPath = &TracerConfig->Paths.DebugEngineDllPath;
    Success = LoadAndInitializeDebugEngineSession(DebugEngineDllPath,
                                                  Rtl,
                                                  &Allocator,
                                                  InitFlags,
                                                  &Session,
                                                  &DestroyDebugEngineSession);

    if (!Success) {
        OutputDebugStringA("LoadAndInitializeDebugEngineSession() failed.\n");
        goto Error;
    };

    ExitCode = 0;
    goto End;

Error:
    ExitCode = 1;

End:
    return ExitCode;
}

VOID
WINAPI
mainCRTStartup()
{
    ExitProcess(Main());
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
