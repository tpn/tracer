/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerInjection.c

Abstract:

    This module implements the necessary routines to facilitate tracer
    injection of Python modules.

--*/

#include "stdafx.h"

DEFINE_GUID_EX(IID_IDEBUG_EVENT_CALLBACKS, 0x0690e046, 0x9c23, 0x45ac,
               0xa0, 0x4f, 0x98, 0x7a, 0xc2, 0x9a, 0xd0, 0xd3);

_Use_decl_annotations_
ULONG
PythonTracerInjectionThreadEntry(
    PTRACER_INJECTION_CONTEXT InjectionContext
    )
/*++

Routine Description:

    This is the thread entry point for the PythonTracerInjection child
    debug engine session thread.

Arguments:

    InjectionContext - Supplies a pointer to an injection context.

Return Value:

    0 on Success, !0 if error.

--*/
{
    BOOL Success;
    ULONG ExitCode;
    HRESULT Result;
    PDEBUG_ENGINE Engine;
    PDEBUGCLIENT Client;
    PIDEBUGCLIENT IClient;
    PDEBUG_ENGINE_SESSION Session;
    PPYTHON_TRACER_INJECTION_CONTEXT Context;

    Context = CONTAINING_RECORD(InjectionContext,
                                PYTHON_TRACER_INJECTION_CONTEXT,
                                InjectionContext);

    Success = CompletePythonTracerInjection(Context);
    if (!Success) {
        goto Error;
    }

    Session = Context->InjectionContext.DebugEngineSession;
    Engine = Session->Engine;
    Client = Engine->Client;
    IClient = Engine->IClient;

    while (TRUE) {
        Result = Client->DispatchCallbacks(IClient, INFINITE);

        if (Result != S_OK && Result != S_FALSE) {
            goto Error;
        }
    }

    ExitCode = 0;
    goto End;

Error:

    ExitCode = 1;

End:

    return ExitCode;
}

_Use_decl_annotations_
BOOL
InitializePythonTracerInjection(
    PDEBUG_ENGINE_SESSION Parent
    )
/*++

Routine Description:

    This function initializes tracer injection for Python.  It creates a new
    PYTHON_TRACER_INJECTION_CONTEXT structure, initializes a minimum set of
    fields, then creates a new thread to continue the initialization.  This
    ensures the debug engine client isn't created with the same thread that
    the parent session was created in.

Arguments:

    DebugEngineSession - Supplies a pointer to a parent debug engine session
        that was initialized with injection intent.

Return Value:

    TRUE on Success, FALSE if an error occurred.

--*/
{
    BOOL Success;
    ULONG HeapFlags;
    PALLOCATOR Allocator;
    HANDLE ThreadHandle;
    ULONG ThreadId;
    PPYTHON_TRACER_INJECTION_CONTEXT Context;
    PTRACER_INJECTION_CONTEXT InjectionContext;
    LPTHREAD_START_ROUTINE StartRoutine;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Parent)) {
        return FALSE;
    }

    //
    // Allocate space for an ALLOCATOR structure.
    //

    Allocator = (PALLOCATOR)(
        Parent->Allocator->Calloc(
            Parent->Allocator->Context,
            1,
            sizeof(*Allocator)
        )
    );

    if (!Allocator) {
        goto Error;
    }

    //
    // Initialize it.
    //

    HeapFlags = HEAP_NO_SERIALIZE;
    Success = InitializeHeapAllocatorExInline(Allocator, HeapFlags, 0, 0);
    if (!Success) {
        goto Error;
    }

    //
    // Allocate our Python tracer injection context.
    //

    Context = (PPYTHON_TRACER_INJECTION_CONTEXT)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(*Context)
        )
    );

    if (!Context) {
        goto Error;
    }

    //
    // Initialize the core injection context.
    //

    InjectionContext = &Context->InjectionContext;
    InjectionContext->Rtl = Parent->Rtl;
    InjectionContext->Allocator = Allocator;
    InjectionContext->TracerConfig = Parent->TracerConfig;
    InjectionContext->SizeOfStruct = sizeof(*InjectionContext);
    InjectionContext->ParentDebugEngineSession = Parent;

    //
    // Create a thread to continue initialization.
    //

    StartRoutine = (LPTHREAD_START_ROUTINE)(
        PythonTracerInjectionThreadEntry
    );

    ThreadHandle = CreateThread(NULL, 0, StartRoutine, Context, 0, &ThreadId);
    if (!ThreadHandle || ThreadHandle == INVALID_HANDLE_VALUE) {
        goto Error;
    }

    InjectionContext->DebugEngineThreadHandle = ThreadHandle;
    InjectionContext->DebugEngineThreadId = ThreadId;

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    if (Allocator) {
        DestroyHeapAllocatorInline(Allocator);
        Parent->Allocator->FreePointer(Parent->Allocator->Context, &Allocator);
    }

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

_Use_decl_annotations_
BOOL
CompletePythonTracerInjection(
    PPYTHON_TRACER_INJECTION_CONTEXT Context
    )
{
    BOOL Success;
    PDEBUG_ENGINE_SESSION Parent;
    PDEBUG_ENGINE_SESSION Session;
    DEBUG_EVENT_CALLBACKS_INTEREST_MASK InterestMask;

    //
    // Initialize interest mask.
    //

    InterestMask.AsULong = (
        DEBUG_EVENT_BREAKPOINT          |
        DEBUG_EVENT_EXCEPTION           |
        DEBUG_EVENT_CREATE_THREAD       |
        DEBUG_EVENT_EXIT_THREAD         |
        DEBUG_EVENT_CREATE_PROCESS      |
        DEBUG_EVENT_EXIT_PROCESS        |
        DEBUG_EVENT_LOAD_MODULE         |
        DEBUG_EVENT_UNLOAD_MODULE       |
        DEBUG_EVENT_SYSTEM_ERROR        |
        DEBUG_EVENT_CHANGE_SYMBOL_STATE
    );

    //
    // The following block can be useful during debugging to quickly
    // toggle interest masks on and off.
    //

    if (0) {
        InterestMask.AsULong = 0;
        InterestMask.Breakpoint = TRUE;
        InterestMask.Exception = TRUE;
        InterestMask.CreateThread = TRUE;
        InterestMask.ExitThread = TRUE;
        InterestMask.CreateProcess = TRUE;
        InterestMask.ExitProcess = TRUE;
        InterestMask.LoadModule = TRUE;
        InterestMask.UnloadModule = TRUE;
        InterestMask.SystemError = TRUE;
        InterestMask.SessionStatus = TRUE;
        InterestMask.ChangeDebuggeeState = TRUE;
        InterestMask.ChangeEngineState = TRUE;
        InterestMask.ChangeSymbolState = TRUE;
    }

    //
    // Initialize our child debug engine session.
    //

    Parent = Context->InjectionContext.ParentDebugEngineSession;

    Success = (
        Parent->InitializeChildDebugEngineSession(
            Parent,
            &PythonTracerInjectionDebugEventCallbacks,
            &IID_IDEBUG_EVENT_CALLBACKS,
            InterestMask,
            &Session
        )
    );

    if (!Success) {
        goto Error;
    }

    //
    // Wire up the new session pointer to our context.
    //

    Context->InjectionContext.DebugEngineSession = Session;

    //
    // Set our context and return success;
    //

    Session->ChildContext = Context;
    Context->InjectionContext.DebugEngineSession = Session;

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Indicate to our parent that we've completed initialization.
    //

    InjectionInitializationComplete(Parent, Session);

    return Success;
}

//
// The following code is a work in progress and will be moved into a more
// appropriate final resting place later.  For now, it lives here to facilitate
// quicker iteration.
//

TRACER_INJECTION_HANDLE_BREAKPOINT Py_Main_HandleBreakpoint;
TRACER_INJECTION_HANDLE_BREAKPOINT Py_InitializeEx_HandleBreakpoint;

_Use_decl_annotations_
HRESULT
Py_Main_HandleBreakpoint(
    PTRACER_INJECTION_CONTEXT Context,
    PIDEBUGBREAKPOINT IBreakpoint
    )
{
    OutputDebugStringA("Caught Py_Main.\n");
    return DEBUG_STATUS_NO_CHANGE;
}

_Use_decl_annotations_
HRESULT
Py_InitializeEx_HandleBreakpoint(
    PTRACER_INJECTION_CONTEXT Context,
    PIDEBUGBREAKPOINT IBreakpoint
    )
{
    OutputDebugStringA("Caught Py_InitializeEx.\n");
    return DEBUG_STATUS_NO_CHANGE;
}

CONST TRACER_INJECTION_BREAKPOINT_SPEC BreakpointSpecs[] = {
    { "python27!Py_Main", Py_Main_HandleBreakpoint },
    { "python30!Py_Main", Py_Main_HandleBreakpoint },
    { "python31!Py_Main", Py_Main_HandleBreakpoint },
    { "python32!Py_Main", Py_Main_HandleBreakpoint },
    { "python33!Py_Main", Py_Main_HandleBreakpoint },
    { "python34!Py_Main", Py_Main_HandleBreakpoint },
    { "python35!Py_Main", Py_Main_HandleBreakpoint },
    { "python36!Py_Main", Py_Main_HandleBreakpoint },

    { "python27!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
    { "python30!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
    { "python31!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
    { "python32!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
    { "python33!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
    { "python34!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
    { "python35!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
    { "python36!Py_InitializeEx", Py_InitializeEx_HandleBreakpoint },
};

C_ASSERT(ARRAYSIZE(BreakpointSpecs) == NUM_INITIAL_BREAKPOINTS());

BOOL
InitializeBreakpoint(
    PDEBUGCONTROL Control,
    PIDEBUGCONTROL IControl,
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint,
    PCSTR OffsetExpression,
    PTRACER_INJECTION_HANDLE_BREAKPOINT HandleBreakpoint
    )
{
    BOOL Success;
    ULONG Id;
    HRESULT Result;
    PDEBUGBREAKPOINT Breakpoint;
    PIDEBUGBREAKPOINT IBreakpoint;

    TRACER_INJECTION_BREAKPOINT_ERROR Error;
    TRACER_INJECTION_BREAKPOINT_FLAGS Flags;

    Flags.AsULong = 0;
    Error.AsULong = 0;
    Error.InitializationFailed = TRUE;

    Result = Control->AddBreakpoint(IControl,
                                    DEBUG_BREAKPOINT_CODE,
                                    DEBUG_ANY_ID,
                                    (PDEBUG_BREAKPOINT *)&IBreakpoint);

    if (Result != S_OK) {
        Error.AddBreakpointFailed = TRUE;
        goto Error;
    }

    Breakpoint = IBreakpoint->lpVtbl;

    Result = Breakpoint->GetId(IBreakpoint, &Id);
    if (Result != S_OK) {
        Id = DEBUG_ANY_ID;
        Error.GetIdFailed = TRUE;
        goto Error;
    }

    Result = Breakpoint->SetOffsetExpression(IBreakpoint, OffsetExpression);
    if (Result != S_OK) {
        Error.SetOffsetExpressionFailed = TRUE;
        goto Error;
    }

    Result = Breakpoint->AddFlags(IBreakpoint, DEBUG_BREAKPOINT_ENABLED);
    if (Result != S_OK) {
        Error.AddFlagsEnabledFailed = TRUE;
        goto Error;
    }

    Error.InitializationFailed = FALSE;
    Flags.Initialized = TRUE;
    Flags.Enabled = TRUE;
    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    InjectionBreakpoint->Id = Id;
    InjectionBreakpoint->Breakpoint = Breakpoint;
    InjectionBreakpoint->IBreakpoint = IBreakpoint;
    InjectionBreakpoint->Error.AsULong = Error.AsULong;
    InjectionBreakpoint->Flags.AsULong = Flags.AsULong;
    InjectionBreakpoint->HandleBreakpoint = HandleBreakpoint;
    InjectionBreakpoint->SizeOfStruct = sizeof(*InjectionBreakpoint);
    InjectionBreakpoint->OffsetExpression = OffsetExpression;

    return Success;
}

_Use_decl_annotations_
BOOL
InitializePythonTracerInjectionBreakpoints(
    PTRACER_INJECTION_CONTEXT InjectionContext
    )
{
    BOOL Success;
    USHORT Index;
    USHORT NumberOfBreakpoints;

    PDEBUGCONTROL Control;
    PIDEBUGCONTROL IControl;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session;
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint;
    TRACER_INJECTION_CONTEXT_ERROR Error;
    TRACER_INJECTION_CONTEXT_FLAGS Flags;
    PPYTHON_TRACER_INJECTION_CONTEXT Context;
    PCTRACER_INJECTION_BREAKPOINT_SPEC BreakpointSpec;

    //
    // Resolve our context.
    //

    Context = CONTAINING_RECORD(InjectionContext,
                                PYTHON_TRACER_INJECTION_CONTEXT,
                                InjectionContext);

    //
    // Initialize local flags and error variables.
    //

    Flags.AsULong = 0;
    Error.AsULong = 0;
    Flags.BreakpointsInitialized = FALSE;
    Error.BreakpointInitializationFailed = TRUE;

    //
    // Initialize aliases.
    //

    Session = Context->InjectionContext.DebugEngineSession;
    Engine = Session->Engine;

    Control = Engine->Control;
    IControl = Engine->IControl;

    NumberOfBreakpoints = ARRAYSIZE(BreakpointSpecs);

    InjectionBreakpoint = &Context->InitialBreakpoints.First;
    InjectionContext = &Context->InjectionContext;
    InjectionContext->InjectionBreakpoints = InjectionBreakpoint;
    InjectionContext->NumberOfBreakpoints = NumberOfBreakpoints;

    for (Index = 0; Index < NumberOfBreakpoints; Index++) {
        BreakpointSpec = &BreakpointSpecs[Index];

        Success = InitializeBreakpoint(Control,
                                       IControl,
                                       InjectionBreakpoint++,
                                       BreakpointSpec->OffsetExpression,
                                       BreakpointSpec->HandleBreakpoint);

        if (!Success) {
            InjectionContext->NumberOfBreakpoints = Index;
            goto Error;
        }
    }

    Error.BreakpointInitializationFailed = FALSE;
    Flags.BreakpointsInitialized = TRUE;
    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    InjectionContext->Error.AsULong |= Error.AsULong;
    InjectionContext->Flags.AsULong |= Flags.AsULong;

    return Success;
}

#pragma optimize("", off)
_Use_decl_annotations_
ULONGLONG
CALLBACK
PythonTracerInjectionCompleteCallback(
    PRTL_INJECTION_PACKET Packet
    )
{
    ULONG WaitResult;
    ULONGLONG Token;
    POUTPUT_DEBUG_STRING_A OutputDebugStringA;
    PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;

    if (Packet->IsInjectionProtocolCallback(Packet, &Token)) {
        return Token;
    }

    OutputDebugStringA = Packet->Functions->OutputDebugStringA;
    SignalObjectAndWait = Packet->Functions->SignalObjectAndWait;

    OutputDebugStringA("PythonTracerInjectionCompleteCallback!\n");

    WaitResult = SignalObjectAndWait(Packet->SignalEventHandle,
                                     Packet->WaitEventHandle,
                                     INFINITE,
                                     TRUE);

    return WaitResult;
}
#pragma optimize("", on)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
