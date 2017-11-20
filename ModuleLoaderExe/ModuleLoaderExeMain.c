/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    ModuleLoaderExeMain.c

Abstract:

    This module implements ModuleLoaderExeMain().  It is intended to be
    called from the mainCRTStartup() routine of an executable.

--*/

#include "stdafx.h"

VOID
MaybeBreak(BOOL Break)
{
    if (Break) {
        __debugbreak();
    }
}

ULONG
ModuleLoaderExeMain(VOID)
{
    BOOL Success;
    PRTL Rtl;
    ULONG NumberOfCharsRead;
    HMODULE Module;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;
    ALLOCATOR Allocator;
    CPINFOEXW CodePageInfoEx;
    ULONG CurrentConsoleCodePage;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    WCHAR Path[_MAX_PATH];

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

    SecureZeroMemory(&Path, sizeof(Path));

    //
    // Load all known tracer modules.
    //

    if (!LoadAllTracerModules(TracerConfig)) {
        goto Error;
    }

    //
    // Call our dummy MaybeBreak() function to allow a cdb/windbg session to
    // break properly.
    //

    MaybeBreak(FALSE);

    //
    // Get standard console handles.
    //

    StandardInput = GetStdHandle(STD_INPUT_HANDLE);
    if (!StandardInput) {
        goto SleepForever;
    }

    StandardOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!StandardOutput) {
        goto SleepForever;
    }

    StandardError = GetStdHandle(STD_ERROR_HANDLE);
    if (!StandardError) {
        goto SleepForever;
    }

    CurrentConsoleCodePage = GetConsoleCP();
    Success = GetCPInfoExW(CurrentConsoleCodePage,
                           0,
                           &CodePageInfoEx);

    if (!Success) {
        goto SleepForever;
    }

    while (TRUE) {

        Success = ReadConsoleW(StandardInput,
                               &Path,
                               sizeof(Path) >> 1,
                               &NumberOfCharsRead,
                               NULL);

        if (!Success) {
            continue;
        }

        if (Path[NumberOfCharsRead] != L'\0') {
            Path[NumberOfCharsRead] = L'\0';
        }

        if (Path[NumberOfCharsRead-1] == L'\n') {
            Path[NumberOfCharsRead-1] = L'\0';
        }

        if (Path[NumberOfCharsRead-2] == L'\r') {
            Path[NumberOfCharsRead-2] = L'\0';
        }

        Module = LoadLibraryW(Path);
    }

    //
    // Sleep until killed.
    //

SleepForever:

    return SleepEx(INFINITE, TRUE);

Error:

    return 1;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
