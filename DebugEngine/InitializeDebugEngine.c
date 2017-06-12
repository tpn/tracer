/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InitializeDebugEngine.c

Abstract:

    This module implements the InitializeDebugEngine() routine.

--*/

#include "stdafx.h"

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_DEBUG_ENGINE_PRIVATE)(
    _In_ PRTL Rtl,
    _In_ PDEBUG_ENGINE Engine,
    _In_opt_ PDEBUG_ENGINE Parent
    );
typedef INITIALIZE_DEBUG_ENGINE_PRIVATE *PINITIALIZE_DEBUG_ENGINE_PRIVATE;
INITIALIZE_DEBUG_ENGINE_PRIVATE InitializeDebugEnginePrivate;

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
    return InitializeDebugEnginePrivate(Rtl, Engine, NULL);
}

_Use_decl_annotations_
BOOL
InitializeChildDebugEngine(
    PRTL Rtl,
    PDEBUG_ENGINE Engine,
    PDEBUG_ENGINE Parent
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
    return InitializeDebugEnginePrivate(Rtl, Engine, Parent);
}

_Use_decl_annotations_
BOOL
InitializeDebugEnginePrivate(
    PRTL Rtl,
    PDEBUG_ENGINE Engine,
    PDEBUG_ENGINE Parent
    )
/*++

Routine Description:

    This routine initializes a DEBUG_ENGINE structure.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Engine - Supplies a pointer to a DEBUG_ENGINE structure to initialize.

    Parent - Supplies an optional pointer to a DEBUG_ENGINE structure from which
        the initial IDebugClient is to be created.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    HRESULT Result;

    if (!ARGUMENT_PRESENT(Parent)) {

        //
        // Initialize COM.
        //

        CHECKED_HRESULT_MSG(
            Rtl->CoInitializeEx(NULL, COINIT_APARTMENTTHREADED),
            "Rtl->CoInitializeEx()"
        );

        //
        // Create the initial client interface.
        //

        CHECKED_HRESULT_MSG(
            Rtl->DebugCreate(&IID_IDEBUG_CLIENT, &Engine->IClient),
            "Rtl->DebugCreate(IID_IDEBUG_CLIENT)"
        );
        Engine->Client = Engine->IClient->lpVtbl;
        Engine->IID_Client = &IID_IDEBUG_CLIENT;

    } else {

        CHECKED_HRESULT_MSG(
            Parent->Client->CreateClient(
                Parent->IClient,
                (PDEBUG_CLIENT *)&Engine->IClient
            ),
            "Parent->Client->CreateClient()"
        );
        Engine->Client = Engine->IClient->lpVtbl;
        Engine->IID_Client = &IID_IDEBUG_CLIENT;
    }

    //
    // Create remaining interfaces.
    //

#define CREATE_INTERFACE(Name, Upper)                           \
    CHECKED_HRESULT_MSG(                                        \
        Engine->Client->QueryInterface(                         \
            Engine->IClient,                                    \
            &IID_IDEBUG_##Upper,                                \
            &Engine->I##Name                                    \
        ),                                                      \
        "Engine->Client->QueryInterface(IID_IDEBUG_" #Upper ")" \
    );                                                          \
    Engine->##Name = (PDEBUG##Upper)Engine->I##Name->lpVtbl;    \
    Engine->IID_##Name = &IID_IDEBUG_##Upper

    CREATE_INTERFACE(Control, CONTROL);
    CREATE_INTERFACE(Symbols, SYMBOLS);
    CREATE_INTERFACE(Advanced, ADVANCED);
    CREATE_INTERFACE(Registers, REGISTERS);
    CREATE_INTERFACE(DataSpaces, DATASPACES);
    CREATE_INTERFACE(SystemObjects, SYSTEMOBJECTS);

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
