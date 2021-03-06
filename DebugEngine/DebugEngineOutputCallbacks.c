/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineOutputCallbacks.c

Abstract:

    This module implements the necessary COM glue required for debug output
    callbacks.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineSetOutputCallbacks(
    PDEBUG_ENGINE Engine
    )
/*++

Routine Description:

    TBD.

Arguments:

    Engine - Supplies a pointer to a DEBUG_ENGINE structure to initialize.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    HRESULT Result;
    PCGUID InterfaceId;

    InterfaceId = &IID_IDEBUG_OUTPUT_CALLBACKS;

    if (InlineIsEqualGUID(&Engine->IID_CurrentOutputCallbacks, InterfaceId)) {

        //
        // The output callbacks are already set.
        //

        Success = TRUE;
        goto End;
    }

    //
    // Copy the debug output callbacks.
    //

    CopyIDebugOutputCallbacks(Engine,
                              &Engine->OutputCallbacks,
                              &DebugOutputCallbacks);

    //
    // Set the callbacks.
    //

    CHECKED_HRESULT_MSG(
        Engine->Client->SetOutputCallbacks(
            Engine->IClient,
            &Engine->IOutputCallbacks
        ),
        "Client->SetOutputCallbacks()"
    );

    //
    // Copy the interface ID.
    //

    CopyInterfaceId(&Engine->IID_CurrentOutputCallbacks, InterfaceId);

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

End:

    return Success;
}

////////////////////////////////////////////////////////////////////////////////
// IDebugOutputCallbacks
////////////////////////////////////////////////////////////////////////////////

#define DEBUG_OUTPUT_CALLBACKS_PROLOGUE() \
    DEBUG_CALLBACK_PROLOGUE(OutputCallbacks)

#define DEBUG_OUTPUT_CALLBACKS_QUERY_INTERFACE() \
    DEBUG_QUERY_INTERFACE(OutputCallbacks, OUTPUT_CALLBACKS)

#define DEBUG_OUTPUT_CALLBACKS_ADD_REF() DEBUG_ADD_REF(OutputCallbacks)
#define DEBUG_OUTPUT_CALLBACKS_RELEASE() DEBUG_RELEASE(OutputCallbacks)

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugOutputCallbacksQueryInterface(
    PIDEBUGOUTPUTCALLBACKS This,
    REFIID InterfaceId,
    PPVOID Interface
    )
{
    DEBUG_OUTPUT_CALLBACKS_PROLOGUE();
    DEBUG_OUTPUT_CALLBACKS_QUERY_INTERFACE();
    return E_NOINTERFACE;
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugOutputCallbacksAddRef(
    PIDEBUGOUTPUTCALLBACKS This
    )
{
    DEBUG_OUTPUT_CALLBACKS_PROLOGUE();
    return DEBUG_OUTPUT_CALLBACKS_ADD_REF();
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugOutputCallbacksRelease(
    PIDEBUGOUTPUTCALLBACKS This
    )
{
    DEBUG_OUTPUT_CALLBACKS_PROLOGUE();
    return DEBUG_OUTPUT_CALLBACKS_RELEASE();
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugOutputCallbacksOutput(
    PIDEBUGOUTPUTCALLBACKS This,
    ULONG Mask,
    PCSTR String
    )
{
    DEBUG_OUTPUT_CALLBACKS_PROLOGUE();

    if (Engine->OutputCallback) {
        DEBUG_OUTPUT_MASK OutputMask = { Mask };
        return Engine->OutputCallback(Engine, OutputMask, String);
    } else {
        OutputDebugStringA(String);
    }

    return S_OK;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
