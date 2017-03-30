/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InitializeDebugEngine.c

Abstract:

    This module implements the InitializeDebugEngine() routine.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeDebugEngine(
    PRTL Rtl,
    PDEBUG_ENGINE Engine
    )
/*++

Routine Description:

    This routine initializes a DEBUG_ENGINE structure.  CoInitializeEx() is
    called with COINIT_APARTMENTTHREADED, and then COM interfaces are created
    for IDebugClient, IDebugControl, IDebugSymbols, IDebugAdvanced,
    IDebugRegisters and IDebugDataSpaces.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Engine - Supplies a pointer to a DEBUG_ENGINE structure to initialize.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    HRESULT Result;

    //
    // Initialize COM.
    //

    CHECKED_HRESULT_MSG(
        Rtl->CoInitializeEx(
            NULL,
            COINIT_APARTMENTTHREADED
        ),
        "Rtl->CoInitializeEx()"
    );

#define CREATE_INTERFACE(Name, Upper)                        \
    CHECKED_HRESULT_MSG(                                     \
        Rtl->DebugCreate(                                    \
            &IID_IDEBUG_##Upper,                             \
            &Engine->I##Name                                 \
        ),                                                   \
        "Rtl->DebugCreate(IID_IDEBUG_" #Upper ")"            \
    );                                                       \
    Engine->##Name = (PDEBUG##Upper)Engine->I##Name->lpVtbl; \
    Engine->IID_##Name = &IID_IDEBUG_##Upper

    CREATE_INTERFACE(Client, CLIENT);
    CREATE_INTERFACE(Control, CONTROL);
    CREATE_INTERFACE(Symbols, SYMBOLS);
    CREATE_INTERFACE(Advanced, ADVANCED);
    CREATE_INTERFACE(Registers, REGISTERS);
    CREATE_INTERFACE(DataSpaces, DATASPACES);

    //
    // Wire up the vtable pointers.
    //

    Engine->IEventCallbacks.lpVtbl = &Engine->EventCallbacks;
    Engine->IInputCallbacks.lpVtbl = &Engine->InputCallbacks;
    Engine->IOutputCallbacks.lpVtbl = &Engine->OutputCallbacks;
    Engine->IOutputCallbacks2.lpVtbl = &Engine->OutputCallbacks2;

    //
    // Save the Rtl pointer and return success.
    //

    Engine->Rtl = Rtl;

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

End:
    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
