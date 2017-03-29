/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineEventCallbacks.c

Abstract:

    This module implements the necessary COM glue required for debug event
    callbacks.

--*/

#include "stdafx.h"

_Use_decl_annotations_
HRESULT
DebugEngineSessionWaitForEvent(
    PDEBUG_ENGINE_SESSION Session,
    ULONG Flags,
    ULONG TimeoutInMilliseconds
    )
/*++

Routine Description:

    This routine is a thin wrapper around the debug engine's WaitForEvent()
    method.

Arguments:

    Session - Supplies a pointer to a DEBUG_ENGINE_SESSION.

    Flags - Unused.

    TimeoutInMilliseconds - Supplies an optional timeout, in milliseconds.

Return Value:

    Returns the HRESULT returned by IDebugControl's WaitForEvent() routine.

--*/
{
    HRESULT Result;
    PDEBUG_ENGINE Engine;

    Engine = Session->Engine;

    Result = Engine->Control->WaitForEvent(Engine->IControl,
                                           Flags,
                                           TimeoutInMilliseconds);

    return Result;
}


_Use_decl_annotations_
BOOL
DebugEngineSetEventCallbacks(
    PDEBUG_ENGINE Engine,
    PCDEBUGEVENTCALLBACKS EventCallbacks,
    PCGUID InterfaceId,
    DEBUG_EVENT_CALLBACKS_INTEREST_MASK InterestMask
    )
/*++

Routine Description:

    This routine initializes callbacks.

Arguments:

    Session - Supplies a pointer to a DEBUG_ENGINE_SESSION.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    HRESULT Result;

    //
    // Acquire the engine lock.
    //

    AcquireDebugEngineLock(Engine);

    if (InlineIsEqualGUID(&Engine->IID_CurrentEventCallbacks, InterfaceId)) {

        //
        // The event callbacks are already set.
        //

        Success = TRUE;
        goto End;
    }

    //
    // Copy the interest mask.
    //

    Engine->EventCallbacksInterestMask.AsULong = InterestMask.AsULong;

    //
    // Copy the debug event callbacks.
    //

    CopyIDebugEventCallbacks(Engine, &Engine->EventCallbacks, EventCallbacks);

    //
    // Set the callbacks.
    //

    CHECKED_HRESULT_MSG(
        Engine->Client->SetEventCallbacksWide(
            Engine->IClient,
            &Engine->IEventCallbacks
        ),
        "Client->SetEventCallbacks()"
    );

    //
    // Copy the interface ID.
    //

    CopyInterfaceId(&Engine->IID_CurrentEventCallbacks, InterfaceId);

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

End:
    ReleaseDebugEngineLock(Engine);

    return Success;
}

////////////////////////////////////////////////////////////////////////////////
// IDebugEventCallbacks
////////////////////////////////////////////////////////////////////////////////

#define DEBUG_EVENT_CALLBACK_PROLOGUE() \
    DEBUG_CALLBACK_PROLOGUE(EventCallbacks)

#define DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE() \
    DEBUG_SESSION_CALLBACK_PROLOGUE(EventCallbacks)

#define DEBUG_EVENT_QUERY_INTERFACE() \
    DEBUG_QUERY_INTERFACE(EventCallbacks, EVENT_CALLBACKS)

#define DEBUG_EVENT_ADD_REF() DEBUG_ADD_REF(EventCallbacks)
#define DEBUG_EVENT_RELEASE() DEBUG_RELEASE(EventCallbacks)

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventQueryInterface(
    PIDEBUGEVENTCALLBACKS This,
    REFIID InterfaceId,
    PPVOID Interface
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();
    DEBUG_EVENT_QUERY_INTERFACE();
    return E_NOINTERFACE;
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugEventAddRef(
    PIDEBUGEVENTCALLBACKS This
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();
    return DEBUG_EVENT_ADD_REF();
}

_Use_decl_annotations_
ULONG
STDAPICALLTYPE
DebugEventRelease(
    PIDEBUGEVENTCALLBACKS This
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();
    return DEBUG_EVENT_RELEASE();
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->GetInterestMask()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventGetInterestMaskCallback(
    PIDEBUGEVENTCALLBACKS This,
    PULONG Mask
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    *Mask = Engine->EventCallbacksInterestMask.AsULong;
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->Breakpoint()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventBreakpointCallback(
    PIDEBUGEVENTCALLBACKS This,
    PDEBUG_BREAKPOINT2 Breakpoint
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->Exception()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventExceptionCallback(
    PIDEBUGEVENTCALLBACKS This,
    PEXCEPTION_RECORD64 Exception,
    ULONG FirstChance
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->CreateThread()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventCreateThreadCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG64 Handle,
    ULONG64 DataOffset,
    ULONG64 StartOffset
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->ExitThread()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventExitThreadCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG ExitCode
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->CreateProcess()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
DebugEventCreateProcessCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG64 ImageFileHandle,
    ULONG64 Handle,
    ULONG64 BaseOffset,
    ULONG ModuleSize,
    PCWSTR ModuleName,
    PCWSTR ImageName,
    ULONG CheckSum,
    ULONG TimeDateStamp,
    ULONG64 InitialThreadHandle,
    ULONG64 ThreadDataOffset,
    ULONG64 StartOffset
    )
{
    DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

    Session->TargetMainThreadHandle = (HANDLE)InitialThreadHandle;

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->ExitProcess()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventExitProcessCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG ExitCode
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->LoadModule()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventLoadModuleCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG64 ImageFileHandle,
    ULONG64 BaseOffset,
    ULONG ModuleSize,
    PCWSTR ModuleName,
    PCWSTR ImageName,
    ULONG CheckSum,
    ULONG TimeDateStamp
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->UnloadModule()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
DebugEventUnloadModuleCallback(
    PIDEBUGEVENTCALLBACKS This,
    PCWSTR ImageBaseName,
    ULONG64 BaseOffset
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}


////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->SystemError()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventSystemErrorCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG Error,
    ULONG Level
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->SessionStatus()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventSessionStatusCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG Status
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    Engine->SessionStatus = Status;

    return DEBUG_STATUS_NO_CHANGE;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->ChangeDebuggeeState()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventChangeDebuggeeStateCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG Flags,
    ULONG64 Argument
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    Engine->DebuggeeState = Flags;
    Engine->DebuggeeStateArg = Argument;

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->ChangeEngineState()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventChangeEngineStateCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG Flags,
    ULONG64 Argument
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    Engine->ChangeEngineState.AsULong = Flags;

    if (Engine->ChangeEngineState.ExecutionStatus) {
        Engine->ExecutionStatus.AsULongLong = Argument;
        if (Engine->ExecutionStatus.InsideWait) {
            //OutputDebugStringA("ChangeEngineState->ExecutionStatus->InWait");
        }
    }

    Engine->EngineState = Flags;
    Engine->EngineStateArg = Argument;

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks->ChangeSymbolState()
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEventChangeSymbolStateCallback(
    PIDEBUGEVENTCALLBACKS This,
    ULONG Flags,
    ULONG64 Argument
    )
{
    DEBUG_EVENT_CALLBACK_PROLOGUE();

    Engine->SymbolState = Flags;
    Engine->SymbolStateArg = Argument;

    return S_OK;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
