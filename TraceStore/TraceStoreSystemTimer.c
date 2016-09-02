/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSystemTimer.c

Abstract:

    This module exposes the highest precision system timer available on the
    current machine.  If GetSystemTimePreciseAsFileTime() is available, it is
    used, otherwise, NtQuerySystemTime() is used.  Functions are providing for
    getting the system timer and calling the system timer.

--*/

#include "stdafx.h"

INIT_ONCE InitOnceSystemTimerFunction = INIT_ONCE_STATIC_INIT;

BOOL
CALLBACK
GetSystemTimerFunctionCallback(
    PINIT_ONCE  InitOnce,
    PVOID       Parameter,
    PVOID       *lpContext
    )
{
    HMODULE Module;
    FARPROC Proc;
    static SYSTEM_TIMER_FUNCTION SystemTimerFunction = { 0 };

    if (!lpContext) {
        return FALSE;
    }

    Module = GetModuleHandleA("kernel32");
    if (Module == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Proc = GetProcAddressA(Module, "GetSystemTimePreciseAsFileTime");
    if (Proc) {
        SystemTimerFunction.GetSystemTimePreciseAsFileTime = (
            (PGETSYSTEMTIMEPRECISEASFILETIME)Proc
        );
    } else {
        Module = LoadLibraryA("ntdll");
        if (!Module) {
            return FALSE;
        }
        Proc = GetProcAddressA(Module, "NtQuerySystemTime");
        if (!Proc) {
            return FALSE;
        }
        SystemTimerFunction.NtQuerySystemTime = (PNTQUERYSYSTEMTIME)Proc;
    }

    *((PPSYSTEM_TIMER_FUNCTION)lpContext) = &SystemTimerFunction;
    return TRUE;
}


_Use_decl_annotations_
PSYSTEM_TIMER_FUNCTION
GetSystemTimerFunction(VOID)
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

_Use_decl_annotations_
BOOL
CallSystemTimer(
    PFILETIME   SystemTime,
    PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
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
        BOOL Success;

        Success = (
            SystemTimerFunction->NtQuerySystemTime(
                (PLARGE_INTEGER)SystemTime
            )
        );

        if (!Success) {
            return FALSE;
        }

    } else {
        return FALSE;
    }

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
