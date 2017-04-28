/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    ModuleLoaderExeMain.c

Abstract:

    This module implements ModuleLoaderExeMain().  It is intended to be
    called from the mainCRTStartup() routine of an executable.

--*/

#include "stdafx.h"

ULONG
ModuleLoaderExeMain(VOID)
{
    PRTL Rtl;
    ALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;

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
    // Load all known tracer modules.
    //

    if (!LoadAllTracerModules(TracerConfig)) {
        goto Error;
    }

    //
    // Sleep until killed.
    //

    return SleepEx(INFINITE, TRUE);

Error:

    return 1;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
