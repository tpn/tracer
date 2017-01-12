/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreLoader.c

Abstract:

    This module implements functionality related to capturing library loading
    information whilst a process is being traced.  This is done by hooking into
    loader's DLL notification callbacks.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
CALLBACK
TraceStoreLoaderDllNotification(
    ULONG NotificationReason,
    PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    PVOID IncomingContext
    )
/*++

Routine Description:

    This routine is the callback function invoked automatically by the loader
    when a DLL has been loaded or unloaded.

Arguments:

    NotificationReason - Supplies the reason for the callback (DLL load or DLL
        unload).

    NotificationData - Supplies a pointer to notification data for this loader
        operation.

    IncomingContext - Supplies a pointer to the TRACE_CONTEXT structure that
        registered the loader DLL notifications.

Return Value:

    None.

--*/
{
    HRESULT Result;
    ULONG Length;
    ULONG FramesToSkip;
    ULONG FramesToCapture;
    PRTL Rtl;
    PTRACE_CONTEXT TraceContext;
    PROCESS_BASIC_INFORMATION ProcessBasicInfo;
    PPEB Peb;
    PPEB Peb1;
    PPEB Peb2;
    PCRITICAL_SECTION LoaderLock;
    HANDLE CurrentProcess;
    HANDLE CurrentThread;
    USHORT FramesCaptured;
    PVOID BackTrace;
    ULONG BackTraceHash;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(IncomingContext)) {
        return;
    }

    //
    // Load aliases.
    //

    TraceContext = (PTRACE_CONTEXT)IncomingContext;
    Rtl = TraceContext->Rtl;

    CurrentProcess = GetCurrentProcess();
    CurrentThread = GetCurrentThread();

    Result = Rtl->ZwQueryInformationProcess(CurrentProcess,
                                            ProcessBasicInformation,
                                            &ProcessBasicInfo,
                                            sizeof(ProcessBasicInfo),
                                            &Length);

    if (FAILED(Result)) {
        return;
    }

    Peb1 = ProcessBasicInfo.PebBaseAddress;
    Peb2 = NtCurrentPeb();
    Peb = Peb1;

    if (Peb1 != Peb2) {
        __debugbreak();
    }

    LoaderLock = Peb->LoaderLock;

    FramesToSkip = 0;
    FramesToCapture = 32;
    FramesCaptured = Rtl->RtlCaptureStackBackTrace(FramesToSkip,
                                                   FramesToCapture,
                                                   &BackTrace,
                                                   &BackTraceHash);

    if (FramesCaptured == 0) {
        __debugbreak();
    }

}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
