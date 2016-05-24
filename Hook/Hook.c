#include "Hook.h"

#include "mhook\mhook.h"

BOOL
Hook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key)
{
    //return Mhook_ForceHook(Rtl, ppSystemFunction, pHookFunction, Key);
    return Mhook_SetHook(Rtl, ppSystemFunction, pHookFunction, Key);
}

RTL_API
BOOL
Unhook(PRTL Rtl, PVOID *ppHookedFunction, PVOID Key)
{
    return Mhook_Unhook(Rtl, ppHookedFunction, Key);
}

VOID
WINAPI
HookEntry(PHOOKED_FUNCTION_ENTRY Entry)
{
    //
    //
    PFUNCTION Function = Entry->Function;
    DWORD64 HomeRcx = Entry->HomeRcx;

}

VOID
WINAPI
HookExit(PHOOKED_FUNCTION_ENTRY Entry)
{
    PFUNCTION Function = Entry->Function;

}


VOID HookPush2(VOID);

VOID
HookPush(VOID)
{
    HookPush2();
}

VOID
InitializeFunction(
    _In_     PRTL       Rtl,
    _In_     PFUNCTION  Function
)
{
    Function->Rtl = Rtl;
    Function->HookProlog = HookProlog;
    Function->HookEpilog = HookEpilog;
}

BOOL
HookFunction(PRTL Rtl, PFUNCTION Function)
{
    BOOL Success;
    Function->NewAddress = Function->OldAddress;
    Success = Mhook_SetHook(Rtl,
                            (PPVOID)&Function->NewAddress,
                            (PVOID)Function->HookProlog,
                            Function);

    return Success;
}
