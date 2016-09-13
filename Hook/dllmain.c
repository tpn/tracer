#include "stdafx.h"

extern BOOL InitializeTracerConfig(VOID);
extern VOID DestroyTracerConfig(VOID);

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
            return InitializeTracerConfig();
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            DestroyTracerConfig();
            break;
    }

    return TRUE;
}
