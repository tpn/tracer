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
