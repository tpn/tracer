/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionRemoteThreadEntryThunk.c

Abstract:

    This module implements the thread entry routine that is used as the start
    address of injected threads.  This routine is copied directly into the
    target process's memory space, and thus, care must be taken regarding the
    types of instructions that can be used.

    Keeping the routine in a separate module increases flexibility with regards
    to potentially specifying different compiler (optimization) flags for this
    module only.

--*/

#include "stdafx.h"

#pragma optimize("", off)

_Use_decl_annotations_
ULONGLONG
RtlpInjectionRemoteThreadEntryThunk(
    PRTL_INJECTION_CONTEXT Context
    )
/*++

Routine Description:

    This is the main thread entry point for a remote thread created as part of
    injection.  It is responsible for calling the initial injection context
    protocol callback function, then loading the Rtl module and initializing an
    RTL structure (allocated on our stack) if the caller has requested Rtl
    injection.

    This method is somewhat unique in that it is copied into the remote address
    space of a different process; this is why we use function pointers stored in
    the Context structure for all routines we need to call.

Arguments:

    Context - Supplies a pointer to the RTL_INJECTION_CONTEXT structure that is
        being used for this injection.

Return Value:

    A LONG integer, the value of which will depend on the context.

--*/
{
    PRTL Rtl;
    RTL RtlLocal;
    ULONG SizeOfRtl;
    ULONG Token;
    ULONGLONG Result;
    HMODULE Module;
    PINITIALIZE_RTL InitRtl;
    POPEN_EVENT_W OpenEventW;
    PGET_PROC_ADDRESS GetProcAddress;
    PLOAD_LIBRARY_EX_W LoadLibraryExW;
    PRTL_INJECTION_PACKET Packet;
    PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteCallback;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Context)) {
        return InjectedRemoteThreadNullContext;
    }

    //
    // Adhere to the context callback protocol.
    //

    if (Context->IsInjectionContextProtocolCallback(Context, &Token)) {
        return Token;
    }

    //
    // Initialize local function pointers.
    //

    OpenEventW = Context->OpenEventW;
    GetProcAddress = Context->GetProcAddress;
    LoadLibraryExW = Context->LoadLibraryExW;

    //
    // Wire up a local packet alias.
    //

    Packet = &Context->Packet;

    //
    // Open our signal handle.
    //

    Context->SignalEventHandle = (
        OpenEventW(
            EVENT_ALL_ACCESS,
            FALSE,
            Context->SignalEventName.Buffer
        )
    );

    if (!Context->SignalEventHandle) {
        Result = InjectedRemoteThreadOpenSignalEventFailed;
        goto Error;
    }

    //
    // Open our wait handle.
    //

    Context->WaitEventHandle = (
        OpenEventW(
            EVENT_ALL_ACCESS,
            FALSE,
            Context->WaitEventName.Buffer
        )
    );

    if (!Context->WaitEventHandle) {
        Result = InjectedRemoteThreadOpenWaitEventFailed;
        goto Error;
    }

    //
    // Open caller's signal handle.
    //

    Packet->SignalEventHandle = Context->CallerSignalEventHandle = (
        OpenEventW(
            EVENT_ALL_ACCESS,
            FALSE,
            Context->CallerSignalEventName.Buffer
        )
    );

    if (!Context->CallerSignalEventHandle) {
        Result = InjectedRemoteThreadOpenCallerSignalEventFailed;
        goto Error;
    }

    //
    // Open caller's wait handle.
    //

    Packet->WaitEventHandle = Context->CallerWaitEventHandle = (
        OpenEventW(
            EVENT_ALL_ACCESS,
            FALSE,
            Context->CallerWaitEventName.Buffer
        )
    );

    if (!Context->CallerWaitEventHandle) {
        Result = InjectedRemoteThreadOpenCallerWaitEventFailed;
        goto Error;
    }

    Packet->SignalEventName = &Context->CallerSignalEventName;
    Packet->WaitEventName = &Context->CallerWaitEventName;

    //
    // If Rtl injection has been requested, do that now.
    //

    if (Context->InjectionFlags.InjectRtl) {

        //
        // Load the Rtl module.
        //

        Module = LoadLibraryExW(Context->RtlDllPath.Buffer, NULL, 0);

        if (!Module || Module == INVALID_HANDLE_VALUE) {
            Result = InjectedRemoteThreadLoadLibraryRtlFailed;
            goto Error;
        }

        //
        // Get the address of the InitializeRtl() function.
        //

        InitRtl = (PINITIALIZE_RTL)GetProcAddress(Module, "InitializeRtl");
        if (!InitRtl) {
            Result = InjectedRemoteThreadResolveInitializeRtlFailed;
            goto Error;
        }

        //
        // Initialize our local, stack-allocated RTL structure.
        //

        Rtl = &RtlLocal;
        SizeOfRtl = sizeof(*Rtl);

        if (!InitRtl(Rtl, &SizeOfRtl)) {
            Result = InjectedRemoteThreadInitializeRtlFailed;
            goto Error;
        }

        //
        // Save the Rtl pointer.
        //

        Packet->Rtl = Context->Rtl = Rtl;
    }

    Packet = &Context->Packet;
    InjectionCompleteCallback = Context->Callback;
    Packet->Flags.AsULong = 0;
    Packet->Flags.IsInjected = TRUE;

    Packet->Functions = &Context->Functions;

    if (Context->InjectionFlags.CatchExceptionsInRemoteThreadCallback) {

        BOOL EncounteredException;

        //
        // Wrap the callback in a SEH block.
        //

        //
        // N.B. We need to install runtime function table entries in order for
        //      this to work I believe.
        //

        __try {

            Result = InjectionCompleteCallback(Packet);

        } __except(
            GetExceptionCode() == STATUS_IN_PAGE_ERROR       ||
            GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
            GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ?
                EXCEPTION_EXECUTE_HANDLER :
                EXCEPTION_CONTINUE_SEARCH) {

            EncounteredException = TRUE;

            switch (GetExceptionCode()) {
                case STATUS_IN_PAGE_ERROR:
                    Result = InjectedRemoteThreadStatusInPageErrorInCallback;
                    break;
                case EXCEPTION_ACCESS_VIOLATION:
                    Result = InjectedRemoteThreadAccessViolationInCallback;
                    break;
                case EXCEPTION_ILLEGAL_INSTRUCTION:
                    Result = InjectedRemoteThreadIllegalInstructionInCallback;
                    break;
            }
        }

        if (EncounteredException) {
            goto Error;
        }

    } else {

        //
        // Invoke the callback without SEH protection.
        //

        Result = InjectionCompleteCallback(Packet);
    }

    Result = InjectedRemoteThreadNoError;
    goto End;

Error:

    //
    // Intentional follow-on to End for now.
    //

End:

    return Result;
}

#pragma optimize("", on)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
