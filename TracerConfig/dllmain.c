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

            //
            // We only want to destroy the global tracer configuration if
            // we're being unloaded dynamically (when Reserved is NULL),
            // and not if we're being called as part of process tear down.
            //

            if (Reserved == NULL) {
                DestroyGlobalTracerConfig();
            }
            break;
    }

    return TRUE;
}
