/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEventCallbacks.c

Abstract:

    Implements debug engine callbacks for Python tracing.

--*/

#include "stdafx.h"


////////////////////////////////////////////////////////////////////////////////
// IDebugEventCallbacks
////////////////////////////////////////////////////////////////////////////////

//
// Forward declaration for all IDebugEventCallbacks.
//

DEBUG_EVENT_QUERY_INTERFACE DebugEventQueryInterface;
DEBUG_EVENT_ADD_REF DebugEventAddRef;
DEBUG_EVENT_RELEASE DebugEventRelease;
DEBUG_EVENT_GET_INTEREST_MASK_CALLBACK DebugEventGetInterestMaskCallback;
DEBUG_EVENT_BREAKPOINT_CALLBACK DebugEventBreakpointCallback;
DEBUG_EVENT_EXCEPTION_CALLBACK DebugEventExceptionCallback;
DEBUG_EVENT_CREATE_THREAD_CALLBACK DebugEventCreateThreadCallback;
DEBUG_EVENT_EXIT_THREAD_CALLBACK DebugEventExitThreadCallback;
DEBUG_EVENT_CREATE_PROCESS_CALLBACK DebugEventCreateProcessCallback;
DEBUG_EVENT_EXIT_PROCESS_CALLBACK DebugEventExitProcessCallback;
DEBUG_EVENT_LOAD_MODULE_CALLBACK DebugEventLoadModuleCallback;
DEBUG_EVENT_UNLOAD_MODULE_CALLBACK DebugEventUnloadModuleCallback;
DEBUG_EVENT_SYSTEM_ERROR_CALLBACK DebugEventSystemErrorCallback;
DEBUG_EVENT_SESSION_STATUS_CALLBACK DebugEventSessionStatusCallback;
DEBUG_EVENT_CHANGE_DEBUGGEE_STATE_CALLBACK DebugEventChangeDebuggeeStateCallback;
DEBUG_EVENT_CHANGE_ENGINE_STATE_CALLBACK DebugEventChangeEngineStateCallback;
DEBUG_EVENT_CHANGE_SYMBOL_STATE_CALLBACK DebugEventChangeSymbolStateCallback;

//
// Helper macros.
//

#define DEBUG_EVENT_CALLBACK_PROLOGUE() \
    DEBUG_CALLBACK_PROLOGUE(EventCallbacks)

#define DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE() \
    DEBUG_SESSION_CALLBACK_PROLOGUE(EventCallbacks)

#define CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE() \
    CHILD_DEBUG_SESSION_CALLBACK_PROLOGUE(            \
        PPYTHON_TRACER_INJECTION_CONTEXT,             \
        EventCallbacks                                \
    )

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
    PDEBUG_BREAKPOINT2 Breakpoint2
    )
{
    ULONG Id;
    ULONG FirstId;
    ULONG LastId;
    ULONG Index;
    BOOL Found;
    HRESULT Result;
    PDEBUGBREAKPOINT Breakpoint;
    PIDEBUGBREAKPOINT IBreakpoint;
    PTRACER_INJECTION_BREAKPOINT First;
    PTRACER_INJECTION_BREAKPOINT Last;
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint;
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

    IBreakpoint = (PIDEBUGBREAKPOINT)Breakpoint2;
    Breakpoint = IBreakpoint->lpVtbl;

    Result = Breakpoint->GetId(IBreakpoint, &Id);
    if (Result != S_OK) {
        return DEBUG_STATUS_BREAK;
    }

    First = &Context->InitialBreakpoints.First;
    Last = &Context->InitialBreakpoints.First + NUM_INITIAL_BREAKPOINTS() - 1;
    FirstId = First->Id;
    LastId = Last->Id;

    if (Id < FirstId && Id > LastId) {
        goto SlowLookup;
    }

    InjectionBreakpoint = First + (Id - FirstId);
    if (InjectionBreakpoint->Id != Id) {
        InjectionBreakpoint = NULL;
    }

    if (!InjectionBreakpoint) {
SlowLookup:
        Found = FALSE;
        for (Index = 0; Index < NUM_INITIAL_BREAKPOINTS(); Index++) {
            InjectionBreakpoint = First + Index;
            if (InjectionBreakpoint->Id == Id) {
                Found = TRUE;
                break;
            }
        }
        if (!Found) {
            __debugbreak();
            return DEBUG_STATUS_BREAK;
        }
    }

    Result = InjectionBreakpoint->HandleBreakpoint(&Context->InjectionContext,
                                                   IBreakpoint);

    return Result;
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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    PRTL Rtl;
    BOOL Success;
    BOOL IsPython;
    ULONG ProcessId;
    ULONG ThreadId;
    UNICODE_STRING Module;
    HANDLE ProcessHandle;
    HANDLE ThreadHandle;
    PTRACER_INJECTION_CONTEXT InjectionContext;
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();
    const UNICODE_STRING PythonModule = RTL_CONSTANT_STRING(L"python");

    //
    // Initialize aliases.
    //

    Rtl = Session->Rtl;
    InjectionContext = &Context->InjectionContext;

    ProcessHandle = (HANDLE)Handle;
    ThreadHandle = (HANDLE)InitialThreadHandle;

    ProcessId = GetProcessId(ProcessHandle);
    ThreadId = GetThreadId(ThreadHandle);

    Module.Length = (USHORT)(wcslen(ModuleName) << 1);
    Module.MaximumLength = Module.Length;
    Module.Buffer = (PWSTR)ModuleName;

    if (!Session->InitialProcessId) {
        Session->InitialProcessId = ProcessId;
        Session->InitialThreadId = ThreadId;

        Success = AllocateAndCopyUnicodeString(
            Session->Allocator,
            &Module,
            &Session->InitialModuleNameW
        );

        if (!Success) {
            __debugbreak();
            goto End;
        }
    }

    //
    // Initialize breakpoints we're interested in (regardless of whether or not
    // this was a Python process).
    //

    Success = InitializePythonTracerInjectionBreakpoints(InjectionContext);
    if (!Success) {
        __debugbreak();
        goto End;
    }

    IsPython = Rtl->RtlPrefixUnicodeString(&Module, &PythonModule, TRUE);

    if (IsPython) {

        //
        // This is definitely a Python program, perform a remote injection.
        //

        RTL_INJECTION_FLAGS Flags;
        PRTL_INJECTION_PACKET Packet;
        PRTL_INJECTION_COMPLETE_CALLBACK Callback;
        RTL_INJECTION_ERROR Error;

        Flags.AsULong = 0;
        Flags.InjectCode = TRUE;

        Callback = PythonTracerInjectionCompleteCallback;

        Success = Rtl->Inject(Rtl,
                              Session->Allocator,
                              &Flags,
                              NULL,
                              NULL,
                              Callback,
                              NULL,
                              ProcessId,
                              &Packet,
                              &Error);

        if (!Success) {
            OutputDebugStringA("Rtl->Inject() failed.");
            goto End;
        }

        //
        // XXX: how does the signal/wait logic work here if we're within a
        // debug callback?  We probably need to spawn a new thread such that
        // we can freely communicate with the remote thread without impeding
        // the general debug engine event loop/callback dispatching.
        //
    }

    //
    // If an initial module name hasn't been set, do it now.
    //

End:

    return DEBUG_STATUS_GO;
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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

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
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

    Engine->SymbolState = Flags;
    Engine->SymbolStateArg = Argument;

    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
//      IDebugEventCallbacks Vtbl
////////////////////////////////////////////////////////////////////////////////

CONST DEBUGEVENTCALLBACKS PythonTracerInjectionDebugEventCallbacks = {
    DebugEventQueryInterface,
    DebugEventAddRef,
    DebugEventRelease,
    DebugEventGetInterestMaskCallback,
    DebugEventBreakpointCallback,
    DebugEventExceptionCallback,
    DebugEventCreateThreadCallback,
    DebugEventExitThreadCallback,
    DebugEventCreateProcessCallback,
    DebugEventExitProcessCallback,
    DebugEventLoadModuleCallback,
    DebugEventUnloadModuleCallback,
    DebugEventSystemErrorCallback,
    DebugEventSessionStatusCallback,
    DebugEventChangeDebuggeeStateCallback,
    DebugEventChangeEngineStateCallback,
    DebugEventChangeSymbolStateCallback,
};

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
