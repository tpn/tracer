/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Injection.c

Abstract:

    This module implements the Rtl component's remote thread and code injection
    component.  Functions are provided for creating injection packets, adding
    injection payloads to packets, adding symbol requests to packets, destroying
    packets, and performing the actual injection.

--*/

#include "stdafx.h"

#pragma data_seg(".shared")
RTL_INJECTION_SHARED InjectionShared = { 0 };
#pragma data_seg()
#pragma comment(linker, "/section:.shared,rws")

BOOL
CALLBACK
RtlpInitializeInjectionCallback(
    PINIT_ONCE InitOnce,
    PVOID Parameter,
    PVOID *lpContext
    )
{
    return TRUE;
}

BOOL
RtlInitializeInjection(
    VOID
    )
{
    BOOL Status;

    Status = InitOnceExecuteOnce(&InjectionShared.InitOnce,
                                 RtlpInitializeInjectionCallback,
                                 NULL,
                                 NULL);

    return Status;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
