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
    BOOL IsReturnBreakpoint = FALSE;
    HRESULT Result;
    HRESULT ReturnResult;
    ULONGLONG StackOffset;
    ULONGLONG ReturnOffset;
    PDEBUGCONTROL Control;
    PIDEBUGCONTROL IControl;
    PDEBUGREGISTERS Registers;
    PIDEBUGREGISTERS IRegisters;
    PDEBUGBREAKPOINT Breakpoint;
    PIDEBUGBREAKPOINT IBreakpoint;
    PDEBUGBREAKPOINT ReturnBreakpoint;
    PIDEBUGBREAKPOINT IReturnBreakpoint;
    PDEBUGSYSTEMOBJECTS SystemObjects;
    PIDEBUGSYSTEMOBJECTS ISystemObjects;
    PTRACER_INJECTION_CONTEXT InjectionContext;
    PTRACER_INJECTION_BREAKPOINT First;
    PTRACER_INJECTION_BREAKPOINT Last;
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint;
    PTRACER_INJECTION_HANDLE_BREAKPOINT HandleBreakpoint;
    PTRACER_INJECTION_HANDLE_BREAKPOINT HandleReturnBreakpoint;
    CHILD_DEBUG_EVENT_SESSION_CALLBACK_PROLOGUE();

    IBreakpoint = (PIDEBUGBREAKPOINT)Breakpoint2;
    Breakpoint = IBreakpoint->lpVtbl;

    Result = Breakpoint->GetId(IBreakpoint, &Id);
    if (Result != S_OK) {
        return DEBUG_STATUS_BREAK;
    }

    First = &Context->InitialBreakpoints.First;
    Last = &Context->InitialBreakpoints.First + NUM_INITIAL_BREAKPOINTS() - 1;
    FirstId = First->BreakpointId;
    LastId = Last->ReturnBreakpointId;

    if (Id < FirstId && Id > LastId) {
        goto SlowLookup;
    }

    InjectionBreakpoint = First + (Id - FirstId);
    IsReturnBreakpoint = (InjectionBreakpoint->ReturnBreakpointId == Id);

    if (InjectionBreakpoint->BreakpointId != Id && !IsReturnBreakpoint) {
        InjectionBreakpoint = NULL;
    }

    if (!InjectionBreakpoint) {
SlowLookup:
        Found = FALSE;

        //
        // We multiply the number of initial breakpoints by 2 to account for the
        // fact each breakpoint is potentially two breakpoints -- one for the
        // initial offset expression, then one for the return address
        // breakpoint.
        //

        for (Index = 0; Index < (NUM_INITIAL_BREAKPOINTS() << 1); Index++) {
            InjectionBreakpoint = First + Index;
            IsReturnBreakpoint = InjectionBreakpoint->ReturnBreakpointId == Id;
            if (InjectionBreakpoint->BreakpointId == Id || IsReturnBreakpoint) {
                Found = TRUE;
                break;
            }
        }

        if (!Found) {
            __debugbreak();
            return DEBUG_STATUS_BREAK;
        }
    }

    //
    // Initialize remaining aliases.
    //

    Control = Engine->Control;
    IControl = Engine->IControl;
    Breakpoint = InjectionBreakpoint->Breakpoint;
    IBreakpoint = InjectionBreakpoint->IBreakpoint;
    ReturnBreakpoint = InjectionBreakpoint->Breakpoint;
    IReturnBreakpoint = InjectionBreakpoint->IReturnBreakpoint;
    SystemObjects = Engine->SystemObjects;
    ISystemObjects = Engine->ISystemObjects;

    InjectionContext = &Context->InjectionContext;

    HandleBreakpoint = InjectionBreakpoint->HandleBreakpoint;
    HandleReturnBreakpoint = InjectionBreakpoint->HandleReturnBreakpoint;

    //
    // Get the current process and thread IDs and handles.
    //

    Result = SystemObjects->GetCurrentProcessId(
        ISystemObjects,
        &InjectionBreakpoint->CurrentProcessId
    );

    if (FAILED(Result)) {
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    Result = SystemObjects->GetCurrentProcessHandle(
        ISystemObjects,
        (PULONG64)&InjectionBreakpoint->CurrentProcessHandle
    );

    if (FAILED(Result)) {
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    Result = SystemObjects->GetCurrentThreadId(
        ISystemObjects,
        &InjectionBreakpoint->CurrentThreadId
    );

    if (FAILED(Result)) {
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    Result = SystemObjects->GetCurrentThreadHandle(
        ISystemObjects,
        (PULONG64)&InjectionBreakpoint->CurrentThreadHandle
    );

    if (FAILED(Result)) {
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    if (!IsReturnBreakpoint) {

        //
        // This is the first breakpoint.  Get the stack offset, then call the
        // handler and, if successful, enable the return breakpoint if one is
        // present.
        //

        Registers = Engine->Registers;
        IRegisters = Engine->IRegisters;
        Result = Registers->GetStackOffset(IRegisters, &StackOffset);
        if (FAILED(Result)) {
            InjectionBreakpoint->Error.GetStackOffsetFailed = TRUE;
            __debugbreak();
            return DEBUG_STATUS_BREAK;
        }

        InjectionBreakpoint->StackOffset = StackOffset;

        //
        // Get the return offset.
        //

        Result = Control->GetReturnOffset(IControl, &ReturnOffset);
        if (FAILED(Result)) {
            InjectionBreakpoint->Error.GetReturnOffsetFailed = TRUE;
            __debugbreak();
            return DEBUG_STATUS_BREAK;
        }

        InjectionBreakpoint->ReturnOffset = ReturnOffset;

        ReturnResult = HandleBreakpoint(InjectionContext, InjectionBreakpoint);

        if (FAILED(ReturnResult) || !HandleReturnBreakpoint) {
            goto End;
        }

        //
        // Enable the return breakpoint.
        //

        Result = ReturnBreakpoint->SetOffset(IReturnBreakpoint, ReturnOffset);
        if (FAILED(Result)) {
            InjectionBreakpoint->Error.SetOffsetFailed = TRUE;
            __debugbreak();
            return DEBUG_STATUS_BREAK;
        }

        Result = ReturnBreakpoint->AddFlags(IReturnBreakpoint,
                                            DEBUG_BREAKPOINT_ENABLED);
        if (FAILED(Result)) {
            InjectionBreakpoint->Error.AddReturnFlagsEnabledFailed = TRUE;
            __debugbreak();
            return DEBUG_STATUS_BREAK;
        }

        InjectionBreakpoint->Flags.ReturnBreakpointEnabled = TRUE;

        if (InjectionBreakpoint->Flags.IsOnceOff) {
            Result = Breakpoint->RemoveFlags(IBreakpoint,
                                             DEBUG_BREAKPOINT_ENABLED);
            if (FAILED(Result)) {
                InjectionBreakpoint->Error.RemoveFlagsEnabledFailed = TRUE;
                __debugbreak();
                return DEBUG_STATUS_BREAK;
            }

            InjectionBreakpoint->Flags.BreakpointEnabled = FALSE;
        }

    } else {

        //
        // This is the return breakpoint.  Call the handler then disable the
        // breakpoint.
        //

        ReturnResult = HandleReturnBreakpoint(InjectionContext,
                                              InjectionBreakpoint);

        Result = ReturnBreakpoint->RemoveFlags(IReturnBreakpoint,
                                               DEBUG_BREAKPOINT_ENABLED);
        if (FAILED(Result)) {
            InjectionBreakpoint->Error.RemoveReturnFlagsEnabledFailed = TRUE;
            __debugbreak();
            return DEBUG_STATUS_BREAK;
        }
    }


End:

    return ReturnResult;
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

    //
    // Disable injection at the CreateProcess level for now (as we're using
    // the Py_Initialize() breakpoint instead).
    //

    if (IsPython && FALSE) {

        //
        // This is definitely a Python program, perform a remote injection.
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
    const UNICODE_STRING PythonModule = RTL_CONSTANT_STRING(L"python");

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
