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
#define ERROR_THREAD_ENTRY_RETURNED             ((ULONG)-3)

/*
DECLSPEC_NORETURN
VOID
_InjectionThunk(
    PRTL_INJECTION_THUNK_CONTEXT Thunk
    )
{
    HMODULE ModuleHandle;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY ThreadEntry;

    Thunk->ReturnAddressInThunk = _ReturnAddress();
    Thunk->AddressOfReturnAddress = _AddressOfReturnAddress();

    //
    // Load the InjectionThunk.dll module.
    //

    ModuleHandle = Thunk->LoadLibraryW(Thunk->InjectionThunkDllPath);
    if (!ModuleHandle) {
        Thunk->ExitThread(LOAD_LIBRARY_INJECTION_THUNK_FAILED);
    }

    //
    // Resolve the injection entry point.
    //

    ThreadEntry = (PRTLP_INJECTION_REMOTE_THREAD_ENTRY)(
        GetProcAddress(
            ModuleHandle,
            Thunk->InjectionThunkName
        )
    );

    if (!ThreadEntry) {
        Thunk->ExitThread(GET_PROC_ADDRESS_FUNCTION_NAME_FAILED);
    }

    ThreadEntry((PRTL_INJECTION_CONTEXT)Thunk);

    //
    // If things are operating correctly, this should never get hit.
    //

    __debugbreak();

    Thunk->ExitThread(ERROR_THREAD_ENTRY_RETURNED);
}
*/

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
