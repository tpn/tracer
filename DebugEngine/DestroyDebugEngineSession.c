/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineDestroy.c

Abstract:

    This module implements the DestroyDebugEngineSession() routine.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
DestroyDebugEngineSession(
    PPDEBUG_ENGINE_SESSION SessionPointer
    )
/*++

Routine Description:

    This routine destroys a previously created debug engine session.

Arguments:

    SessionPointer - Supplies the address of a variable that contains the
        address of the DEBUG_ENGINE_SESSION structure to destroy.  This pointer
        will be cleared by this routine.

Return Value:

    None.

--*/
{
    //
    // Currently unimplemented.
    //

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
