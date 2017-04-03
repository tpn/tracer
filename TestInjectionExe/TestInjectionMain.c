/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TestInjectionExe.c

Abstract:

    This module implements the TestInjectionMain() function.

--*/

#include "stdafx.h"

#pragma optimize("", off)
ULONG
TestInjectionMain(VOID)
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

    SecureZeroMemory(&StringArrayAllocator, sizeof(StringArrayAllocator));
    SecureZeroMemory(&StringTableAllocator, sizeof(StringTableAllocator));

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

    SetCSpecificHandler(Rtl->__C_specific_handler);

    //
    // Extract the target process ID and thread ID from the command line.
    //

    InitFlags.InitializeFromCurrentProcess = TRUE;
    DebugEngineDllPath = &TracerConfig->Paths.DebugEngineDllPath;
    Success = LoadAndInitializeDebugEngineSession(
        DebugEngineDllPath,
        Rtl,
        &Allocator,
        InitFlags,
        TracerConfig,
        &TracerConfig->Paths.StringTableDllPath,
        &StringArrayAllocator,
        &StringTableAllocator,
        &Session,
        &DestroyDebugEngineSession
    );

    if (!Success) {
        OutputDebugStringA("LoadAndInitializeDebugEngineSession() failed.\n");
        goto Error;
    };

    //
    // Run tests.
    //

    ExitCode = TestInjection(Rtl, &Allocator, TracerConfig, Session);
    goto End;

Error:
    ExitCode = 1;

End:
    return ExitCode;
}
#pragma optimize("", on)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
