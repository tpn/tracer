/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DestroyDebugEngineSession.c

Abstract:

    This module implements the DestroyDebugEngineSession() routine.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
DestroyDebugEngineSession(
    PPDEBUG_ENGINE_SESSION SessionPointer,
    PBOOL IsProcessTerminating
    )
/*++

Routine Description:

    This routine destroys a previously created debug engine session.

    N.B. This routine is a work-in-progress.

Arguments:

    SessionPointer - Supplies the address of a variable that contains the
        address of the DEBUG_ENGINE_SESSION structure to destroy.  The pointer
        will be cleared (set to NULL) by this routine.

    IsProcessTerminating - Optionally supplies a pointer to a boolean variable
        that indicates whether or not the process is terminating.

Return Value:

    None.

--*/
{
    PDEBUG_ENGINE_SESSION Session;
    PDEBUG_ENGINE Engine;
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(SessionPointer)) {
        return;
    }

    //
    // Initialize the local session alias then clear the caller's pointer.
    //

    Session = *SessionPointer;
    *SessionPointer = NULL;

    //
    // Initialize other local aliases.
    //

    Engine = Session->Engine;
    Allocator = Session->Allocator;

    //
    // Close the process handle if one was opened.
    //

    if (Session->TargetProcessHandle) {
        CloseHandle(Session->TargetProcessHandle);
        Session->TargetProcessHandle = NULL;
    }

    if (Engine) {

        if (Engine->Client) {
            Engine->Client->DetachProcesses(Engine->IClient);
        }

        DestroyDebugEngine(&Engine, IsProcessTerminating);
    }

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
