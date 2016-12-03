/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    dllmain.c

Abstract:

    This is the DLL main entry point for the TlsTracerHeap component.  It
    is responsible for hooking into the various process and thread attach and
    detach methods (via _DllMainCRTStartup) and calling the relevant callback.

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
    BOOL Success;

    switch (Reason) {

        case DLL_PROCESS_ATTACH:

            Success = TlsTracerHeapProcessAttach(Module, Reason, Reserved);
            break;

        case DLL_THREAD_ATTACH:

            Success = TlsTracerHeapThreadAttach(Module, Reason, Reserved);
            break;

        case DLL_THREAD_DETACH:

            Success = TlsTracerHeapThreadDetach(Module, Reason, Reserved);
            break;

        case DLL_PROCESS_DETACH:

            Success = TlsTracerHeapProcessDetach(Module, Reason, Reserved);
            break;
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
