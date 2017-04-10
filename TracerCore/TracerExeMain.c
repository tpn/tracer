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
    ULONG Index;
    ULONG ExitCode;
    HRESULT Result;
    HMODULE Module;
    ALLOCATOR Allocator;
    ALLOCATOR StringTableAllocator;
    ALLOCATOR StringArrayAllocator;

    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING Path;
    PUNICODE_STRING RegistryPath;
    PUNICODE_STRING DebugEngineDllPath;
    PUNICODE_STRING TracingDllPath = NULL;
    PDEBUG_ENGINE_SESSION Session;
    PTRACER_INJECTION_MODULES InjectionModules;
    PINITIALIZE_TRACER_INJECTION Initializer;
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

    InitFlags.InitializeFromCommandLine = TRUE;
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
    // Ensure we've got the name of the first program.
    //

    while (!Session->InitialModuleNameW.Length) {

        Result = Session->WaitForEvent(Session, 0, 100);

        if (Result != S_OK && Result != S_FALSE) {
            goto Error;
        }
    }

    //
    // Initialize all of the injection modules.
    //

    for (Index = 0; Index < InjectionModules->NumberOfModules; Index++) {

        Path = InjectionModules->Paths[Index];
        Module = InjectionModules->Modules[Index];
        Initializer = InjectionModules->Initializers[Index];

        Success = Initializer(Rtl, &Allocator, TracerConfig, Session);
        if (!Success) {
            __debugbreak();
            goto Error;
        }
    }

    while (TRUE) {
        Result = Session->WaitForEvent(Session, 0, INFINITE);

        if (Result != S_OK && Result != S_FALSE) {
            goto Error;
        }
    }

    ExitCode = 0;
    goto End;

Error:
    ExitCode = 1;

End:
    return ExitCode;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
