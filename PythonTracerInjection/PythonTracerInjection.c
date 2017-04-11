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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
