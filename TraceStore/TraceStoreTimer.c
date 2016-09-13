/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreTimer.c

Abstract:

    This module exposes the highest precision system timer available on the
    current machine.  If GetSystemTimePreciseAsFileTime() is available, it is
    used, otherwise, NtQuerySystemTime() is used.  Functions are providing for
    getting the system timer and calling the system timer.

--*/

#include "stdafx.h"

INIT_ONCE InitOnceTimerFunction = INIT_ONCE_STATIC_INIT;

BOOL
CALLBACK
TraceStoreGetTimerFunctionCallback(
    PINIT_ONCE  InitOnce,
    PVOID       Parameter,
    PVOID       *lpContext
    )
{
    HMODULE Module;
    FARPROC Proc;
    static TIMER_FUNCTION TimerFunction = { 0 };

    if (!lpContext) {
        return FALSE;
    }

    Module = GetModuleHandleW(L"kernel32");
    if (!Module || Module == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "GetSystemTimePreciseAsFileTime");
    if (Proc) {
        TimerFunction.GetSystemTimePreciseAsFileTime = (
            (PGETSYSTEMTIMEPRECISEASFILETIME)Proc
        );
    } else {
        Module = LoadLibraryW(L"ntdll");
        if (!Module) {
            return FALSE;
        }
        Proc = GetProcAddress(Module, "NtQuerySystemTime");
        if (!Proc) {
            return FALSE;
        }
        TimerFunction.NtQuerySystemTime = (PNTQUERYSYSTEMTIME)Proc;
    }

    *((PPTIMER_FUNCTION)lpContext) = &TimerFunction;
    return TRUE;
}


_Use_decl_annotations_
PTIMER_FUNCTION
TraceStoreGetTimerFunction(VOID)
{
    BOOL Status;
    PTIMER_FUNCTION TimerFunction;

    Status = InitOnceExecuteOnce(
        &InitOnceTimerFunction,
        TraceStoreGetTimerFunctionCallback,
        NULL,
        (LPVOID *)&TimerFunction
    );

    if (!Status) {
        return NULL;
    } else {
        return TimerFunction;
    }
}

_Use_decl_annotations_
BOOL
TraceStoreCallTimer(
    PFILETIME SystemTime,
    PPTIMER_FUNCTION ppTimerFunction
    )
{
    PTIMER_FUNCTION TimerFunction = NULL;

    if (ppTimerFunction) {
        if (*ppTimerFunction) {
            TimerFunction = *ppTimerFunction;
        } else {
            TimerFunction = TraceStoreGetTimerFunction();
            *ppTimerFunction = TimerFunction;
        }
    } else {
        TimerFunction = TraceStoreGetTimerFunction();
    }

    if (!TimerFunction) {
        return FALSE;
    }

    if (TimerFunction->GetSystemTimePreciseAsFileTime) {

        TimerFunction->GetSystemTimePreciseAsFileTime(SystemTime);

    } else if (TimerFunction->NtQuerySystemTime) {
        BOOL Success;

        Success = TimerFunction->NtQuerySystemTime((PLARGE_INTEGER)SystemTime);

        if (!Success) {
            return FALSE;
        }

    } else {
        return FALSE;
    }

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
