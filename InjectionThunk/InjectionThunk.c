/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionThunk.c

Abstract:

    This module provides routines for the InjectionThunk component.

--*/

#include "stdafx.h"

#define LOAD_LIBRARY_INJECTION_THUNK_FAILED     ((ULONG)-1)
#define GET_PROC_ADDRESS_FUNCTION_NAME_FAILED   ((ULONG)-2)

DECLSPEC_NORETURN
VOID
InjectionThunk(
    PRTL_INJECTION_THUNK_CONTEXT Thunk
    )
{
    ULONG ExitCode;
    FARPROC Proc;
    HMODULE ModuleHandle;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY ThreadEntry;

    //
    // Load the InjectionThunk.dll module.
    //

    ModuleHandle = Thunk->LoadLibraryW(Thunk->DllPath);
    if (!ModuleHandle) {
        Thunk->ExitThread(LOAD_LIBRARY_INJECTION_THUNK_FAILED);
    }

    //
    // Resolve the injection entry point.
    //

    Proc = Thunk->GetProcAddress(ModuleHandle, Thunk->FunctionName);
    if (!Proc) {
        Thunk->ExitThread(GET_PROC_ADDRESS_FUNCTION_NAME_FAILED);
    }

    ThreadEntry = (PRTLP_INJECTION_REMOTE_THREAD_ENTRY)Proc;
    ExitCode = ThreadEntry(Thunk->Context);

    Thunk->ExitThread(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
