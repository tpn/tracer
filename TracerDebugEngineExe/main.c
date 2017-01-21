/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This is the main module for the TracerDebugEngineExEngineExee component.
    It implements a Main() function and provides a mainCRTStartup() entry point
    which calls Main().

--*/

#include "stdafx.h"

ULONG
Main(VOID)
{
    PRTL Rtl;
    ULONG ExitCode;
    ALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    PDEBUG_ENGINE_SESSION Session;
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
    CHECKED_MSG(
        Rtl->CreateAndInitializeDebugEngineSession(
            Rtl,
            &Allocator,
            InitFlags,
            &Session
        ),
        "CreateAndInitializeDebugEngineSession()"
    );

    //
    // Start the session.
    //

    CHECKED_MSG(Session->Start(Session), "DebugSession->Start()");

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
