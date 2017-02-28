/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DestroyDebugEngine.c

Abstract:

    This module implements the DestroyDebugEngine() routine.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
DestroyDebugEngine(
    PPDEBUG_ENGINE EnginePointer,
    PBOOL IsProcessTerminating
    )
/*++

Routine Description:

    This routine destroys a previously created DEBUG_ENGINE structure.  It is
    responsible for releasing internal COM interfaces.

Arguments:

    EnginePointer - Supplies the address of a variable that contains the
        address of the DEBUG_ENGINE structure to destroy.  This pointer
        will be cleared by this routine.

    IsProcessTerminating - Optionally supplies a pointer to a boolean variable
        that indicates whether or not the process is terminating.

Return Value:

    None.

--*/
{
    PDEBUG_ENGINE Engine;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(EnginePointer)) {
        return;
    }

    //
    // Initialize the local engine alias then clean the caller's pointer.
    //

    Engine = *EnginePointer;
    *EnginePointer = NULL;

    //
    // Release any interfaces we've created.
    //

#define RELEASE_INTERFACE(Name)                   \
    if (Engine->I##Name) {                        \
        Engine->##Name->Release(Engine->I##Name); \
        Engine->I##Name = NULL;                   \
    }

    RELEASE_INTERFACE(DataSpaces);
    RELEASE_INTERFACE(Advanced);
    RELEASE_INTERFACE(Symbols);
    RELEASE_INTERFACE(Control);
    RELEASE_INTERFACE(Client);

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
