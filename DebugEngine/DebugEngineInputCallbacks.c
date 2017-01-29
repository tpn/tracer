/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineInputCallbacks.c

Abstract:

    This module implements the necessary COM glue required for debug input
    callbacks.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineSetInputCallbacks(
    PDEBUG_ENGINE Engine,
    PDEBUGINPUTCALLBACKS InputCallbacks,
    PCGUID InterfaceId
    )
/*++

Routine Description:

    TBD

Arguments:

    Engine - Supplies a pointer to a DEBUG_ENGINE structure to initialize.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    //
    // Not implemented yet.
    //

    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// IDebugInputCallbacks
////////////////////////////////////////////////////////////////////////////////

#define DEBUG_INPUT_CALLBACK_PROLOGUE() \
    DEBUG_CALLBACK_PROLOGUE(InputCallbacks)

#define DEBUG_INPUT_QUERY_INTERFACE() \
    DEBUG_QUERY_INTERFACE(InputCallbacks, INPUT_CALLBACKS)

#define DEBUG_INPUT_ADD_REF() DEBUG_ADD_REF(InputCallbacks)
#define DEBUG_INPUT_RELEASE() DEBUG_RELEASE(InputCallbacks)

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugInputQueryInterface(
    PIDEBUGINPUTCALLBACKS This,
    REFIID InterfaceId,
    PPVOID Interface
    )
{
    DEBUG_INPUT_CALLBACK_PROLOGUE();
    DEBUG_INPUT_QUERY_INTERFACE();
    return E_NOINTERFACE;
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugInputAddRef(
    PIDEBUGINPUTCALLBACKS This
    )
{
    DEBUG_INPUT_CALLBACK_PROLOGUE();
    return DEBUG_INPUT_ADD_REF();
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugInputRelease(
    PIDEBUGINPUTCALLBACKS This
    )
{
    DEBUG_INPUT_CALLBACK_PROLOGUE();
    return DEBUG_INPUT_RELEASE();
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugInputStartInputCallback(
    PIDEBUGINPUTCALLBACKS This,
    ULONG BufferSize
    )
{
    OutputDebugStringA("Entered DebugStartInputCallback().\n");
    return S_OK;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugInputEndInputCallback(
    PIDEBUGINPUTCALLBACKS This
    )
{
    OutputDebugStringA("Entered DebugEndInputCallback().\n");
    return S_OK;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
