/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    dllmain.c

Abstract:

    This is the DLL main entry point for the Rtl DLL.  It is responsible for
    hooking into the DLL_PROCESS_ATTACH and DLL_PROCESS_DETACH messages in
    order to facilitate atexit() rundown functionality.

--*/

#include "stdafx.h"

BOOL
APIENTRY
_DllMainCRTStartup(
    _In_    HMODULE     Module,
    _In_    DWORD       Reason,
    _In_    LPVOID      Reserved
    )
{
    BOOL IsProcessTerminating;

    switch (Reason) {
        case DLL_PROCESS_ATTACH:
            if (!InitializeGlobalRtlAtExitRundown()) {
                return FALSE;
            }
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            IsProcessTerminating = (Reserved != NULL);
            RundownGlobalAtExitFunctions(IsProcessTerminating);
            DestroyGlobalRtlAtExitRundown();
            break;
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
