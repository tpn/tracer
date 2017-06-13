/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineEventLoop.c

Abstract:

    This module implements an event loop for the debug engine session component.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineSessionEventLoopRunOnce(
    PDEBUG_ENGINE_SESSION Session,
    PBOOL TerminateLoop
    )
/*++

Routine Description:

    Runs one iteration of the debug engine session event loop.

Arguments:

    Session - Supplies a pointer to the primary DEBUG_ENGINE_SESSION structure
        for which the event loop is to be run.

    TerminateLoop - Supplies the address of a variable that receives a TRUE
        indication if the loop should terminate.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    PAPC Apc;
    BOOL Success = FALSE;
    BOOL Alertable = TRUE;
    BOOL NotAlertable = FALSE;
    HRESULT Result;
    ULONG WaitResult;

    //
    // Clear the loop terminator up front.
    //

    *TerminateLoop = FALSE;

    Result = Session->EventDispatch(Session, 0, INFINITE);

    if (Result != S_OK && Result != S_FALSE) {
        goto Error;
    }

    //
    // Allow for APCs to be delivered.
    //

    if (Session->NumberOfPendingApcs) {

        while (Session->NumberOfPendingApcs > 0) {

            ResetEvent(Session->WaitForApcEvent);

            WaitResult = SignalObjectAndWait(Session->ReadyForApcEvent,
                                             Session->WaitForApcEvent,
                                             INFINITE,
                                             NotAlertable);

            if (WaitResult != WAIT_OBJECT_0) {
                __debugbreak();
                break;
            }

            Apc = &Session->Apc;
            Apc->Routine(Apc->Argument1, Apc->Argument2, Apc->Argument3);
            ZeroStructPointer(Apc);
        }
    }

    //
    // See if a shutdown event has been requested.
    //

    WaitResult = WaitForSingleObjectEx(Session->ShutdownEvent,
                                       0,
                                       NotAlertable);

    if (WaitResult == WAIT_OBJECT_0) {
        *TerminateLoop = TRUE;
        Success = TRUE;
    } else if (WaitResult == WAIT_TIMEOUT) {
        Success = TRUE;
    } else {
        __debugbreak();
        goto Error;
    }

    //
    // Invariant check: we should be indicating success if we hit this point.
    //

    if (!Success) {
        __debugbreak();
    }

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

_Use_decl_annotations_
BOOL
DebugEngineSessionEventLoop(
    PDEBUG_ENGINE_SESSION Session
    )
/*++

Routine Description:

    Enters the debug engine session event loop.

Arguments:

    Session - Supplies a pointer to the primary DEBUG_ENGINE_SESSION structure
        for which the event loop is to be entered.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    BOOL TerminateLoop;

    do {
        Success = Session->EventLoopRunOnce(Session, &TerminateLoop);
    } while (Success && !TerminateLoop);

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
