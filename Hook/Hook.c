#include "Hook.h"

#include "mhook\mhook.h"

BOOL
Hook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction)
{
    return Mhook_SetHook(Rtl, ppSystemFunction, pHookFunction);
}

RTL_API
BOOL
Unhook(PRTL Rtl, PVOID *ppHookedFunction)
{
    return Mhook_Unhook(Rtl, ppHookedFunction);
}
