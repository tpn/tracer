#include "Rtl.h"

PCSPECIFICHANDLER __C_specific_handler = NULL;

INIT_ONCE InitOnceCSpecificHandler = INIT_ONCE_STATIC_INIT;
INIT_ONCE InitOnceSystemTimerFunction = INIT_ONCE_STATIC_INIT;

BOOL
CALLBACK
SetCSpecificHandlerCallback(
    PINIT_ONCE InitOnce,
    PVOID Parameter,
    PVOID *lpContext
)
{
    __C_specific_handler = (PCSPECIFICHANDLER)Parameter;
    return TRUE;
}

BOOL
SetCSpecificHandler(PCSPECIFICHANDLER Handler)
{
    BOOL Status;

    Status = InitOnceExecuteOnce(
        &InitOnceCSpecificHandler,
        SetCSpecificHandlerCallback,
        Handler,
        NULL
    );

    return Status;
}

BOOL
CALLBACK
GetSystemTimerFunctionCallback(
    _Inout_     PINIT_ONCE  InitOnce,
    _Inout_     PVOID       Parameter,
    _Inout_opt_ PVOID       *lpContext
)
{
    HMODULE Module;
    FARPROC Proc;
    static SYSTEM_TIMER_FUNCTION SystemTimerFunction = { 0 };

    if (!lpContext) {
        return FALSE;
    }

    Module = GetModuleHandle(TEXT("kernel32"));
    if (Module == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "GetSystemTimePreciseAsFileTime");
    if (Proc) {
        SystemTimerFunction.GetSystemTimePreciseAsFileTime = (PGETSYSTEMTIMEPRECISEASFILETIME)Proc;
    } else {
        Module = LoadLibrary(TEXT("ntdll"));
        if (!Module) {
            return FALSE;
        }
        Proc = GetProcAddress(Module, "NtQuerySystemTime");
        if (!Proc) {
            return FALSE;
        }
        SystemTimerFunction.NtQuerySystemTime = (PNTQUERYSYSTEMTIME)Proc;
    }

    *((PPSYSTEM_TIMER_FUNCTION)lpContext) = &SystemTimerFunction;
    return TRUE;
}

PSYSTEM_TIMER_FUNCTION
GetSystemTimerFunction()
{
    BOOL Status;
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction;

    Status = InitOnceExecuteOnce(
        &InitOnceSystemTimerFunction,
        GetSystemTimerFunctionCallback,
        NULL,
        (LPVOID *)&SystemTimerFunction
    );

    if (!Status) {
        return NULL;
    } else {
        return SystemTimerFunction;
    }
}

_Check_return_
BOOL
CallSystemTimer(
    _Out_       PFILETIME   SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
)
{
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction = NULL;

    if (ppSystemTimerFunction) {
        if (*ppSystemTimerFunction) {
            SystemTimerFunction = *ppSystemTimerFunction;
        } else {
            SystemTimerFunction = GetSystemTimerFunction();
            *ppSystemTimerFunction = SystemTimerFunction;
        }
    } else {
        SystemTimerFunction = GetSystemTimerFunction();
    }

    if (!SystemTimerFunction) {
        return FALSE;
    }

    if (SystemTimerFunction->GetSystemTimePreciseAsFileTime) {
        SystemTimerFunction->GetSystemTimePreciseAsFileTime(SystemTime);
    } else if (SystemTimerFunction->NtQuerySystemTime) {
        if (!SystemTimerFunction->NtQuerySystemTime((PLARGE_INTEGER)SystemTime)) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    return TRUE;
}

_Check_return_
PVOID
CopyToMemoryMappedMemory(
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
)
{
    __try {
 
        return memcpy(Destination, Source, Size);

    } __except (GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {

        return NULL;
    }
}

_Check_return_
BOOL
LoadRtlSymbols(_Inout_ PRTL Rtl)
{

    if (!(Rtl->Kernel32Module = LoadLibraryA("kernel32"))) {
        return FALSE;
    }

    if (!(Rtl->NtdllModule = LoadLibraryA("ntdll"))) {
        return FALSE;
    }

    Rtl->GetSystemTimePreciseAsFileTime = (PGETSYSTEMTIMEPRECISEASFILETIME)
        GetProcAddress(Rtl->Kernel32Module, "GetSystemTimePreciseAsFileTime");

    Rtl->NtQuerySystemTime = (PNTQUERYSYSTEMTIME)
        GetProcAddress(Rtl->NtdllModule, "NtQuerySystemTime");

    if (Rtl->GetSystemTimePreciseAsFileTime) {
        Rtl->SystemTimerFunction.GetSystemTimePreciseAsFileTime = 
            Rtl->GetSystemTimePreciseAsFileTime;
    } else if (Rtl->NtQuerySystemTime) {
        Rtl->SystemTimerFunction.NtQuerySystemTime =
            Rtl->NtQuerySystemTime;
    } else {
        return FALSE;
    }

    if (!(Rtl->RtlCharToInteger = (PRTLCHARTOINTEGER)
        GetProcAddress(Rtl->NtdllModule, "RtlCharToInteger"))) {
        return FALSE;
    }

    if (!(Rtl->__C_specific_handler = (PCSPECIFICHANDLER)
        GetProcAddress(Rtl->NtdllModule, "__C_specific_handler"))) {
        return FALSE;
    }

    if (!SetCSpecificHandler(Rtl->__C_specific_handler)) {
        return FALSE;
    }

    return TRUE;
}


_Check_return_
BOOL
LoadRtlExFunctions(
    _In_opt_ HMODULE RtlExModule,
    _Inout_  PRTLEXFUNCTIONS RtlExFunctions
    )
{
    if (!RtlExModule) {
        return FALSE;
    }

    if (!RtlExFunctions) {
        return FALSE;
    }

    if (!(RtlExFunctions->CopyToMappedMemory = (PCOPYTOMAPPEDMEMORY)
        GetProcAddress(RtlExModule, "CopyToMappedMemory"))) {
        return FALSE;
    }

    return TRUE;
}

_Check_return_
BOOL
LoadRtlExSymbols(
    _In_opt_ HMODULE RtlExModule,
    _Inout_  PRTL    Rtl
)
{
    HMODULE Module;

    if (!Rtl) {
        return FALSE;
    }

    if (RtlExModule) {
        Module = RtlExModule;

    } else {

        DWORD Flags = (
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS          |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
        );

        if (!GetModuleHandleEx(Flags, (LPCTSTR)&LoadRtlExFunctions, &Module)) {
            return FALSE;
        }

        if (!Module) {
            return FALSE;
        }
    }

    if (!LoadRtlExFunctions(Module, &Rtl->RtlExFunctions)) {
        return FALSE;
    }

    return TRUE;

}

BOOL
InitializeRtl(
    _Out_bytecap_(*SizeOfRtl) PRTL   Rtl,
    _Inout_                   PULONG SizeOfRtl
    )
{
    if (!Rtl) {
        if (SizeOfRtl) {
            *SizeOfRtl = sizeof(*Rtl);
        }
        return FALSE;
    }

    if (!SizeOfRtl) {
        return FALSE;
    }

    if (*SizeOfRtl < sizeof(*Rtl)) {
        *SizeOfRtl = sizeof(*Rtl);
        return FALSE;
    } else {
        *SizeOfRtl = sizeof(*Rtl);
    }

    SecureZeroMemory(Rtl, sizeof(*Rtl));

    if (!LoadRtlSymbols(Rtl)) {
        return FALSE;
    }

    if (!LoadRtlExSymbols(NULL, Rtl)) {
        return FALSE;
    }

    return TRUE;
}