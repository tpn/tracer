/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    dllmain.c

Abstract:

    This module is the DLL main entry point for the TracerConfig component.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
_DebugBreak(
    VOID
    )
{
    __debugbreak();
}

BOOL
APIENTRY
_DllMainCRTStartup(
    _In_    HMODULE     Module,
    _In_    DWORD       Reason,
    _In_    LPVOID      Reserved
    )
{
    switch (Reason) {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
