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
CALLBACK
HookEntry(_In_ PHOOKED_FUNCTION_CALL Call)
{
    PHOOKED_FUNCTION Function = Call->HookedFunction;

    if (Function->EntryCallback) {
        Function->EntryCallback(Call);
    }
}

VOID
CALLBACK
HookExit(_In_ PHOOKED_FUNCTION_CALL Call)
{
    PHOOKED_FUNCTION Function = Call->HookedFunction;

    if (Function->ExitCallback) {
        Function->ExitCallback(Call);
    }
}

VOID
InitializeHookedFunction(
    _In_     PRTL               Rtl,
    _In_     PHOOKED_FUNCTION   Function
)
{
    Function->Rtl = Rtl;
    Function->HookProlog = (PROC)HookProlog;
    Function->HookEpilog = (PROC)HookEpilog;
    Function->HookEntry = HookEntry;
    Function->HookExit = HookExit;
}

BOOL
HookFunction(
    _In_ PRTL Rtl,
    _In_ PPVOID SystemFunctionPointer,
    _In_ PHOOKED_FUNCTION Function
    )
{
    BOOL Success;
    Function->ContinuationAddress = Function->OriginalAddress;

    Success = Mhook_SetFunctionHook(Rtl,
                                    SystemFunctionPointer,
                                    Function);

    return Success;
}
