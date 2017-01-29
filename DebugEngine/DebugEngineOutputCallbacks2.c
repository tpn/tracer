/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineOutputCallbacks2.c

Abstract:

    This module implements the necessary COM glue required for debug output
    callbacks.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineSetOutputCallbacks2(
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

    InterfaceId = &IID_IDEBUG_OUTPUT_CALLBACKS2;

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

    CopyIDebugOutputCallbacks2(Engine,
                               &Engine->OutputCallbacks2,
                               &DebugOutputCallbacks2);

    //
    // Set the callbacks.
    //

    CHECKED_HRESULT_MSG(
        Engine->Client->SetOutputCallbacks(
            Engine->IClient,
            (PDEBUG_OUTPUT_CALLBACKS)&Engine->IOutputCallbacks2
        ),
        "Client->SetOutputCallbacks2()"
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
// IDebugOutputCallbacks2
////////////////////////////////////////////////////////////////////////////////

#define DEBUG_OUTPUT_CALLBACKS2_PROLOGUE() \
    DEBUG_CALLBACK_PROLOGUE(OutputCallbacks2)

#define DEBUG_OUTPUT_CALLBACKS2_QUERY_INTERFACE() \
    DEBUG_QUERY_INTERFACE(OutputCallbacks2, OUTPUT_CALLBACKS2)

#define DEBUG_OUTPUT_CALLBACKS2_ADD_REF() DEBUG_ADD_REF(OutputCallbacks2)
#define DEBUG_OUTPUT_CALLBACKS2_RELEASE() DEBUG_RELEASE(OutputCallbacks2)

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugOutputCallbacks2QueryInterface(
    PIDEBUGOUTPUTCALLBACKS2 This,
    REFIID InterfaceId,
    PPVOID Interface
    )
{
    DEBUG_OUTPUT_CALLBACKS2_PROLOGUE();
    DEBUG_OUTPUT_CALLBACKS2_QUERY_INTERFACE();
    return E_NOINTERFACE;
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugOutputCallbacks2AddRef(
    PIDEBUGOUTPUTCALLBACKS2 This
    )
{
    DEBUG_OUTPUT_CALLBACKS2_PROLOGUE();
    return DEBUG_OUTPUT_CALLBACKS2_ADD_REF();
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugOutputCallbacks2Release(
    PIDEBUGOUTPUTCALLBACKS2 This
    )
{
    DEBUG_OUTPUT_CALLBACKS2_PROLOGUE();
    return DEBUG_OUTPUT_CALLBACKS2_RELEASE();
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugOutputCallbacks2Output(
    PIDEBUGOUTPUTCALLBACKS2 This,
    ULONG Mask,
    PCSTR String
    )
{
    OutputDebugStringA(String);
    return S_OK;
}

_Use_decl_annotations_
HRESULT
DebugOutputCallbacks2GetInterestMask(
    PIDEBUGOUTPUTCALLBACKS2 This,
    PULONG Mask
    )
{
    DEBUG_OUTPUT_CALLBACKS2_PROLOGUE();

    *Mask = Engine->OutputCallbacks2InterestMask.AsULong;
    return S_OK;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugOutputCallbacks2Output2(
    PIDEBUGOUTPUTCALLBACKS2 This,
    ULONG Which,
    ULONG Flags,
    ULONG64 Args,
    PCWSTR String
    )
{
    return S_OK;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
