#include <minwindef.h>
#include <winnt.h>

#include "TracerConfigPrivate.h"

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
            return InitializeGlobalTracerConfig();
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            DestroyGlobalTracerConfig();
            break;
    }

    return TRUE;
}
