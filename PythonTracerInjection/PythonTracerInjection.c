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

    if (!Session->EventLoop(Session)) {
        goto Error;
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
    DEBUGEVENTCALLBACKS EventCallbacks;

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
        DEBUG_EVENT_CHANGE_ENGINE_STATE |
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

    if (1) {
        InterestMask.AsULong = 0;
        InterestMask.Breakpoint = TRUE;
        InterestMask.CreateProcess = TRUE;
        //InterestMask.ExitProcess = TRUE;
        //InterestMask.SessionStatus = TRUE;
        //InterestMask.ChangeDebuggeeState = TRUE;
        //InterestMask.ChangeEngineState = TRUE;
        //InterestMask.ChangeSymbolState = TRUE;
    }

    //
    // Copy our debug events over, then override the ChangeEngineStatus one.
    //


    CopyMemory((PBYTE)&EventCallbacks,
               (PCBYTE)&PythonTracerInjectionDebugEventCallbacks,
               sizeof(EventCallbacks));

    Parent = Context->InjectionContext.ParentDebugEngineSession;

    //EventCallbacks.ChangeEngineState = (
    //    Parent->Engine->EventCallbacks.ChangeEngineState
    //);

    //
    // Initialize our child debug engine session.
    //

    Success = (
        Parent->InitializeChildDebugEngineSession(
            Parent,
            &EventCallbacks,
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
    // Wire ourselves up to the parent.
    //

    Session->Parent = Parent;

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
TRACER_INJECTION_HANDLE_BREAKPOINT Py_Main_HandleReturnBreakpoint;
TRACER_INJECTION_HANDLE_BREAKPOINT Py_InitializeEx_HandleBreakpoint;
TRACER_INJECTION_HANDLE_BREAKPOINT Py_InitializeEx_HandleReturnBreakpoint;

#define PYTHON_BREAKPOINT(Version, Name) \
    {                                    \
        "python" # Version "!" # Name,   \
        Name##_HandleBreakpoint,         \
        Name##_HandleReturnBreakpoint    \
    }

CONST TRACER_INJECTION_BREAKPOINT_SPEC BreakpointSpecs[] = {
    PYTHON_BREAKPOINT(27, Py_Main),
    PYTHON_BREAKPOINT(30, Py_Main),
    PYTHON_BREAKPOINT(31, Py_Main),
    PYTHON_BREAKPOINT(32, Py_Main),
    PYTHON_BREAKPOINT(33, Py_Main),
    PYTHON_BREAKPOINT(34, Py_Main),
    PYTHON_BREAKPOINT(35, Py_Main),
    PYTHON_BREAKPOINT(36, Py_Main),

    PYTHON_BREAKPOINT(27, Py_InitializeEx),
    PYTHON_BREAKPOINT(30, Py_InitializeEx),
    PYTHON_BREAKPOINT(31, Py_InitializeEx),
    PYTHON_BREAKPOINT(32, Py_InitializeEx),
    PYTHON_BREAKPOINT(33, Py_InitializeEx),
    PYTHON_BREAKPOINT(34, Py_InitializeEx),
    PYTHON_BREAKPOINT(35, Py_InitializeEx),
    PYTHON_BREAKPOINT(36, Py_InitializeEx),
};

_Use_decl_annotations_
HRESULT
Py_Main_HandleBreakpoint(
    PTRACER_INJECTION_CONTEXT InjectionContext,
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint
    )
{
    PPYTHON_TRACER_INJECTION_CONTEXT Context;

    Context = CONTAINING_RECORD(InjectionContext,
                                PYTHON_TRACER_INJECTION_CONTEXT,
                                InjectionContext);

    OutputDebugStringA("Caught Py_Main.\n");
    return DEBUG_STATUS_NO_CHANGE;
}

_Use_decl_annotations_
HRESULT
Py_Main_HandleReturnBreakpoint(
    PTRACER_INJECTION_CONTEXT InjectionContext,
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint
    )
{
    PPYTHON_TRACER_INJECTION_CONTEXT Context;

    Context = CONTAINING_RECORD(InjectionContext,
                                PYTHON_TRACER_INJECTION_CONTEXT,
                                InjectionContext);

    OutputDebugStringA("Caught return of Py_Main.\n");
    return DEBUG_STATUS_NO_CHANGE;
}

_Use_decl_annotations_
HRESULT
Py_InitializeEx_HandleBreakpoint(
    PTRACER_INJECTION_CONTEXT InjectionContext,
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint
    )
{
    PPYTHON_TRACER_INJECTION_CONTEXT Context;

    Context = CONTAINING_RECORD(InjectionContext,
                                PYTHON_TRACER_INJECTION_CONTEXT,
                                InjectionContext);

    OutputDebugStringA("Caught Py_InitializeEx.\n");
    return DEBUG_STATUS_NO_CHANGE;
}

VOID
UnfreezeThreadApc(
    PPYTHON_TRACER_INJECTION_CONTEXT Context,
    ULONG SuspendedThreadId
    )
{
    PRTL Rtl;
    BOOL Success;
    NTSTATUS Status;
    USHORT NumberOfDigits;
    PDEBUG_ENGINE_SESSION Session;
    PCUNICODE_STRING Command;
    UNICODE_STRING UnfreezeThreadCommand;
    UNICODE_STRING Suffix = RTL_CONSTANT_STRING(L" u");
    WCHAR Buffer[] = L"~~[            u";
    //                    ^^^^^^^^^^
    //                    Thread ID will be inserted here (up to 10 digits).

    OutputDebugStringA("Entered UnfreezeThreadApc()\n");

    Rtl = Context->InjectionContext.Rtl;
    Session = Context->InjectionContext.DebugEngineSession;

    //
    // Set the length to the third character, so that we append the thread ID
    // after the open bracket (character position 4).  (Shifting left by to
    // account for wide characters.)
    //

    UnfreezeThreadCommand.Length = 3 << 1;
    UnfreezeThreadCommand.MaximumLength = ARRAYSIZE(Buffer) << 1;
    UnfreezeThreadCommand.Buffer = Buffer;

    NumberOfDigits = CountNumberOfDigitsInline(SuspendedThreadId);

    Success = AppendIntegerToUnicodeString(&UnfreezeThreadCommand,
                                           SuspendedThreadId,
                                           NumberOfDigits,
                                           L']');
    if (!Success) {
        __debugbreak();
        return;
    }

    Status = Rtl->RtlAppendUnicodeStringToString(&UnfreezeThreadCommand,
                                                 (PCUNICODE_STRING)&Suffix);
    if (!SUCCEEDED(Status)) {
        __debugbreak();
        return;
    }

    OutputDebugStringA("Unfreeze thread command: ");
    PrintUnicodeStringToDebugStream(&UnfreezeThreadCommand);

    Command = (PCUNICODE_STRING)&UnfreezeThreadCommand;
    Success = Session->ExecuteStaticCommand(Session, Command, NULL);
    if (!Success) {
        __debugbreak();
    }

    OutputDebugStringA("Leaving apc...\n");

    InterlockedDecrement64(&Session->NumberOfPendingApcs);

    return;
}

VOID
DetachProcessesApc(
    PPYTHON_TRACER_INJECTION_CONTEXT Context
    )
{
    PRTL Rtl;
    HRESULT Result;
    PDEBUG_ENGINE Engine;
    PDEBUGCLIENT Client;
    PIDEBUGCLIENT IClient;
    PDEBUG_ENGINE_SESSION Session;

    OutputDebugStringA("Entered DetachProcessesApc()\n");

    Rtl = Context->InjectionContext.Rtl;
    Session = Context->InjectionContext.DebugEngineSession;
    Engine = Session->Engine;
    Client = Engine->Client;
    IClient = Engine->IClient;

    Result = Client->DetachProcesses(IClient);

    if (!SUCCEEDED(Result)) {
        OutputDebugStringA("DetachProcesses() failed.\n");
        __debugbreak();
    }

    OutputDebugStringA("Leaving apc...\n");

    //InterlockedDecrement64(&Session->NumberOfPendingApcs);

    SetEvent(Session->ShutdownEvent);

    return;
}

LONG
ParentThreadEntry(
    PPYTHON_TRACER_INJECTION_CONTEXT Context
    )
{
    BOOL Success;
    PRTL Rtl;
    PALLOCATOR Allocator;
    HRESULT Result;
    NTSTATUS Status;
    LONG ExitCode;
    USHORT Index;
    USHORT MappingIndex;
    USHORT NumberOfObjects;
    USHORT NumberOfEvents;
    USHORT NumberOfFileMappings;
    ULONG RemoteThreadId;
    ULONG WaitResult;
    ULONG LastError;
    LONG SuspendedCount;
    ULONG SuspendedThreadId;
    ULONG SizeOfBuffer;
    PWSTR WideBuffer;
    HANDLE Handle;
    HANDLE ResumeEvent;
    HANDLE RemoteThreadHandle;
    HANDLE RemotePythonProcessHandle;
    HANDLE DebugEngineThreadHandle;
    HANDLE SuspendedThreadHandle;
    PVOID BaseAddress;
    PVOID RemoteBaseCodeAddress;
    PVOID RemoteUserBufferAddress;
    PAPC Apc;
    PDEBUGCLIENT ExitDispatchClient;
    PIDEBUGCLIENT IExitDispatchClient;
    INJECTION_THUNK_FLAGS Flags;
    PCUNICODE_STRING DllPath;
    PTRACER_INJECTION_CONTEXT InjectionContext;
    PYTHON_TRACER_INJECTED_CONTEXT InjectedContext;
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session;
    PDEBUG_ENGINE_SESSION Parent;
    PUNICODE_STRING Name;
    PINJECTION_OBJECT Object;
    INJECTION_OBJECT Objects[3];
    UNICODE_STRING ObjectNames[3];
    UNICODE_STRING ObjectPrefixes[] = {
        RTL_CONSTANT_STRING(L"Event1_"),
        RTL_CONSTANT_STRING(L"Event2_"),
        RTL_CONSTANT_STRING(L"SharedMem1_"),
    };
    PUNICODE_STRING Names[] = {
        &ObjectNames[0],
        &ObjectNames[1],
        &ObjectNames[2],
    };
    PUNICODE_STRING Prefixes[] ={
        &ObjectPrefixes[0],
        &ObjectPrefixes[1],
        &ObjectPrefixes[2],
    };
    ULONG FileMappingDesiredAccess[] = {
        FILE_MAP_READ | FILE_MAP_WRITE,
    };
    ULONG FileMappingPageProtection[] = {
        PAGE_READWRITE,
        PAGE_READWRITE,
    };
    LARGE_INTEGER MappingSizes[] = {
        {
            1 << 16,    // 64KB
            0,
        },
    };
    LARGE_INTEGER MappingSize;
    INJECTION_OBJECTS InjectionObjects;
    INJECTION_THUNK_FLAGS InjectionThunkFlags;
    PINJECTION_OBJECT_EVENT Event;
    PINJECTION_OBJECT_EVENT Event1;
    PINJECTION_OBJECT_EVENT Event2;
    PINJECTION_OBJECT_FILE_MAPPING Shared1;
    PINJECTION_OBJECT_FILE_MAPPING FileMapping;
    const STRING FunctionName =
        RTL_CONSTANT_STRING("InjectedTracedPythonSessionRemoteThreadEntry");

    ZeroStruct(Objects);
    ZeroStruct(InjectionObjects);
    ZeroStruct(InjectionContext);

    NumberOfObjects = ARRAYSIZE(ObjectNames);
    NumberOfEvents = 4;
    NumberOfFileMappings = 3;

    Rtl = Context->InjectionContext.Rtl;
    Allocator = Context->InjectionContext.Allocator;

    if (!Rtl->InitializeInjection(Rtl)) {
        goto Error;
    }

    InjectedContext.OutputDebugStringA = Rtl->OutputDebugStringA;
    InjectedContext.ParentProcessId = FastGetCurrentProcessId();
    InjectedContext.ParentThreadId = FastGetCurrentThreadId();

    Flags.AsULong = 0;
    Session = Context->InjectionContext.DebugEngineSession;
    Engine = Session->Engine;
    DllPath = &Session->TracerConfig->Paths.TracedPythonSessionDllPath;

    //
    // Take a copy of volatile details.  The values of these fields are
    // undefined as soon as we call SetEvent() on the resume event.
    //

    InjectionContext = &Context->InjectionContext;
    InjectionBreakpoint = InjectionContext->CurrentInjectionBreakpoint;
    RemotePythonProcessHandle = Context->RemotePythonProcessHandle;
    SuspendedThreadId = InjectionBreakpoint->CurrentThreadId;
    SuspendedThreadHandle = InjectionBreakpoint->SuspendedThreadHandle;
    DebugEngineThreadHandle = InjectionContext->DebugEngineThreadHandle;
    ResumeEvent = InjectionContext->ResumeEvent;

    OutputDebugStringA("About to call SetEvent(ResumeEvent)...\n");

    SetEvent(ResumeEvent);

    Success = Rtl->CreateRandomObjectNames(Rtl,
                                           Allocator,
                                           Allocator,
                                           ARRAYSIZE(ObjectNames),
                                           64,
                                           NULL,
                                           (PPUNICODE_STRING)&Names,
                                           (PPUNICODE_STRING)&Prefixes,
                                           &SizeOfBuffer,
                                           &WideBuffer);

    if (!Success) {
        __debugbreak();
    }

    NumberOfEvents = 2;
    NumberOfObjects = ARRAYSIZE(ObjectNames);

    for (Index = 0; Index < NumberOfEvents; Index++) {
        Name = Names[Index];
        Object = &Objects[Index];
        Event = &Object->AsEvent;
        InitializeUnicodeStringFromUnicodeString(&Event->Name, Name);
        Event->Type.AsId = EventInjectionObjectId;
        Event->Flags.ManualReset = FALSE;
        Event->DesiredAccess = EVENT_MODIFY_STATE | SYNCHRONIZE;

        Event->Handle = Rtl->CreateEventW(NULL,
                                          Event->Flags.ManualReset,
                                          FALSE,
                                          Name->Buffer);

        LastError = GetLastError();

        if (!Event->Handle || LastError == ERROR_ALREADY_EXISTS) {
            __debugbreak();
            goto Error;
        }
    }

    for (Index = NumberOfEvents, MappingIndex = 0;
         Index < NumberOfObjects;
         Index++, MappingIndex++) {

        Name = Names[Index];
        Object = &Objects[Index];
        FileMapping = &Object->AsFileMapping;
        InitializeUnicodeStringFromUnicodeString(&FileMapping->Name, Name);
        FileMapping->Type.AsId = FileMappingInjectionObjectId;
        FileMapping->DesiredAccess = FileMappingDesiredAccess[MappingIndex];
        FileMapping->PageProtection = FileMappingPageProtection[MappingIndex];
        MappingSize.QuadPart = MappingSizes[MappingIndex].QuadPart;
        FileMapping->MappingSize.QuadPart = MappingSize.QuadPart;

        if (FileMapping->Path.Buffer) {

            Handle = Rtl->CreateFileW(FileMapping->Path.Buffer,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_DELETE,
                                      NULL,
                                      OPEN_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

            LastError = GetLastError();

            if (!Handle || Handle == INVALID_HANDLE_VALUE) {
                __debugbreak();
                goto Error;
            }

            FileMapping->FileHandle = Handle;

            if (FALSE && LastError == ERROR_ALREADY_EXISTS) {
                FILE_STANDARD_INFO FileInfo;
                FILE_INFO_BY_HANDLE_CLASS Class;

                Class = (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo;

                Success = GetFileInformationByHandleEx(Handle,
                                                       Class,
                                                       &FileInfo,
                                                       sizeof(FileInfo));
                if (!Success) {
                    __debugbreak();
                    goto Error;
                }

                FileMapping->FileOffset.QuadPart = FileInfo.EndOfFile.QuadPart;
            }
        }

        Handle = Rtl->CreateFileMappingW(FileMapping->FileHandle,
                                         NULL,
                                         PAGE_READWRITE,
                                         FileMapping->MappingSize.HighPart,
                                         FileMapping->MappingSize.LowPart,
                                         FileMapping->Name.Buffer);

        LastError = GetLastError();

        if (!Handle || LastError == ERROR_ALREADY_EXISTS) {
            __debugbreak();
            goto Error;
        }

        FileMapping->Handle = Handle;

        BaseAddress = Rtl->MapViewOfFileEx(Handle,
                                           FILE_MAP_ALL_ACCESS,
                                           FileMapping->FileOffset.HighPart,
                                           FileMapping->FileOffset.LowPart,
                                           FileMapping->MappingSize.QuadPart,
                                           FileMapping->BaseAddress);

        if (!BaseAddress) {
            LastError = GetLastError();
            __debugbreak();
            goto Error;
        }

        FileMapping->BaseAddress = BaseAddress;
        FileMapping->PreferredBaseAddress = BaseAddress;
    }

    //
    // Initialize the INJECTION_OBJECTS container.
    //

    InjectionObjects.SizeOfStruct = sizeof(InjectionObjects);
    InjectionObjects.SizeOfInjectionObjectInBytes = sizeof(INJECTION_OBJECT);
    InjectionObjects.NumberOfObjects = NumberOfObjects;
    InjectionObjects.TotalAllocSizeInBytes = 0;
    InjectionObjects.Objects = Objects;

    //
    // Perform injection.
    //

    InjectionThunkFlags.AsULong = 0;
    InjectionThunkFlags.HasInjectionObjects = TRUE;
    InjectionThunkFlags.HasModuleAndFunction = TRUE;

    InjectionThunkFlags.DebugBreakOnEntry = (
        InjectionContext->TracerConfig->Flags.InjectionThunkDebugBreakOnEntry
    );

    Object = Objects;

    Event1 = &Object->AsEvent;
    Event2 = &(Object + 1)->AsEvent;

    Event1 = &Object++->AsEvent;
    Event2 = &Object++->AsEvent;
    Shared1 = &Object++->AsFileMapping;

    Event1->Flags.SetEventAfterOpening = FALSE;

    Event2->Flags.WaitOnEventAfterOpening = FALSE;
    Event2->Flags.DebugBreakAfterWaitSatisfied = FALSE;

    Parent = Context->InjectionContext.ParentDebugEngineSession;

    SetEvent(Parent->ShutdownEvent);
    SetEvent(Session->ShutdownEvent);

    Parent->Engine->State.ExitDispatchWhenAble = TRUE;

    Result = Parent->Engine->Control->SetInterrupt(
        Parent->Engine->IControl,
        DEBUG_INTERRUPT_ACTIVE
    );

    ExitDispatchClient = Session->ExitDispatchEngine->Client;
    IExitDispatchClient = Session->ExitDispatchEngine->IClient;

    Result = ExitDispatchClient->ExitDispatch(
        IExitDispatchClient,
        (PDEBUG_CLIENT)Session->Engine->IClient
    );
    if (FAILED(Result)) {
        __debugbreak();
    }

    WaitResult = WaitForSingleObject(Parent->ShutdownCompleteEvent, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
        __debugbreak();
    }

    Success = Rtl->Inject(Rtl,
                          Session->Allocator,
                          InjectionThunkFlags,
                          RemotePythonProcessHandle,
                          DllPath,
                          &FunctionName,
                          NULL,
                          (PBYTE)&InjectedContext,
                          sizeof(InjectedContext),
                          &InjectionObjects,
                          NULL,
                          &RemoteThreadHandle,
                          &RemoteThreadId,
                          &RemoteBaseCodeAddress,
                          &RemoteUserBufferAddress,
                          NULL,
                          NULL,
                          NULL);

    if (!Success) {
        __debugbreak();
    }

    WaitResult = WaitForSingleObject(Event1->Handle, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
        __debugbreak();
    }

    goto Resume;

    //
    // The injected thread has loaded.
    //

    //
    // Detaching processes.
    //

    //OutputDebugStringA("Queuing APC...\n");

    //InterlockedIncrement64(&Session->NumberOfPendingApcs);

#if 0
    Session->Engine.State.ExitDispatchWhenAble = TRUE;
    Result = Session->Engine->Control->SetInterrupt(
        Session->Engine->IControl,
        DEBUG_INTERRUPT_ACTIVE
    );

    if (FAILED(Result)) {
        __debugbreak();
        return -1;
    }
#endif

    //
    // Force the main debug thread to exit it's event dispatching.
    //

#if 0
    ExitDispatchClient = Session->ExitDispatchEngine->Client;
    IExitDispatchClient = Session->ExitDispatchEngine->IClient;

    Result = ExitDispatchClient->ExitDispatch(
        IExitDispatchClient,
        (PDEBUG_CLIENT)Session->Engine->IClient
    );

    if (FAILED(Result)) {
        __debugbreak();
        return -1;
    }
#endif

    WaitResult = WaitForSingleObject(Session->ReadyForApcEvent, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
        __debugbreak();
        ExitCode = -1;
        goto Error;
    }

    Apc = &Session->Apc;
    ZeroStructPointer(Apc);
    Apc->Routine = (PAPC_ROUTINE)DetachProcessesApc;
    Apc->Argument1 = Context;
    Apc->Argument2 = (PVOID)((ULONG_PTR)SuspendedThreadId);
    UNREFERENCED_PARAMETER(Status);

    SetEvent(Session->WaitForApcEvent);

#if 0
    OutputDebugStringA("Queuing APC.\n");

    Status = Rtl->NtQueueApcThread(DebugEngineThreadHandle,
                                   (PAPC_ROUTINE)DetachProcessesApc,
                                   Context,
                                   NULL,
                                   NULL);

    if (!SUCCEEDED(Status)) {
        __debugbreak();
    }
#endif

    //
    // Resume the original thread we suspended.
    //

Resume:

    do {
        SuspendedCount = (LONG)ResumeThread(SuspendedThreadHandle);
    } while (SuspendedCount > 0);

    if (SuspendedCount == -1) {
        __debugbreak();
        return -1;
    }

    return 0;

Error:

    return -1;

}


_Use_decl_annotations_
HRESULT
Py_InitializeEx_HandleReturnBreakpoint(
    PTRACER_INJECTION_CONTEXT InjectionContext,
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint
    )
{
    BOOL Success;
    ULONG WaitResult;
    ULONG LastError;
    HANDLE EventHandle;
    HANDLE CurrentProcessHandle;
    HANDLE DebugEngineThreadHandle;
    PPYTHON_TRACER_INJECTION_CONTEXT Context;
    PDEBUG_ENGINE_SESSION Session;
    UNICODE_STRING FreezeThreadCommand = RTL_CONSTANT_STRING(L"~# f");
    PCUNICODE_STRING Command = (PCUNICODE_STRING)&FreezeThreadCommand;

    OutputDebugStringA("Caught return of Py_InitializeEx, injecting...\n");

    Context = CONTAINING_RECORD(InjectionContext,
                                PYTHON_TRACER_INJECTION_CONTEXT,
                                InjectionContext);

    //
    // Duplicate the process handle such that we can use it independently of
    // the debug engine session.
    //

    CurrentProcessHandle = GetCurrentProcess();
    Success = DuplicateHandle(CurrentProcessHandle,
                              InjectionBreakpoint->CurrentProcessHandle,
                              CurrentProcessHandle,
                              &Context->RemotePythonProcessHandle,
                              0,
                              FALSE,
                              DUPLICATE_SAME_ACCESS);
    if (!Success) {
        LastError = GetLastError();
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    //
    // We need to freeze the faulting thread that triggered the Py_InitializeEx
    // breakpoint, such that it doesn't run when this method returns.
    //

    Session = InjectionContext->DebugEngineSession;

    Session->Parent->TargetProcessHandle = Context->RemotePythonProcessHandle;

#if 0
    Success = Session->ExecuteStaticCommand(Session, Command, NULL);
    if (!Success) {
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }
    //Session->Engine->OutputCallback = NULL;
#endif

    EventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!EventHandle) {
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    Success = DuplicateHandle(CurrentProcessHandle,
                              GetCurrentThread(),
                              CurrentProcessHandle,
                              &DebugEngineThreadHandle,
                              0,
                              FALSE,
                              DUPLICATE_SAME_ACCESS);
    if (!Success) {
        LastError = GetLastError();
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    InjectionContext->ResumeEvent = EventHandle;
    InjectionContext->DebugEngineThreadHandle = DebugEngineThreadHandle;

    Context->PythonThreadHandle = (
        CreateThread(NULL,
                     0,
                     (LPTHREAD_START_ROUTINE)ParentThreadEntry,
                     InjectionContext,
                     0,
                     &Context->PythonThreadId)
    );

    if (!Context->PythonThreadHandle) {
        __debugbreak();
        return DEBUG_STATUS_BREAK;
    }

    OutputDebugStringA("Created thread, waiting on resume event...\n");

    WaitResult = WaitForSingleObject(EventHandle, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
        __debugbreak();
    }

    OutputDebugStringA("Thread resumed... returning...\n");

    CloseHandle(EventHandle);
    InjectionContext->ResumeEvent = NULL;

    return DEBUG_STATUS_NO_CHANGE;
}

C_ASSERT(ARRAYSIZE(BreakpointSpecs) == NUM_INITIAL_BREAKPOINTS());

BOOL
InitializeBreakpoint(
    PDEBUG_ENGINE Engine,
    PTRACER_INJECTION_BREAKPOINT InjectionBreakpoint,
    PCTRACER_INJECTION_BREAKPOINT_SPEC BreakpointSpec
    )
{

    //
    // N.B. This method is generic enough to be moved to somewhere like
    //      TracerCore or DebugEngine.
    //

    BOOL Success;
    ULONG BreakpointId;
    ULONG ReturnBreakpointId;
    HRESULT Result;
    PDEBUGCONTROL Control;
    PIDEBUGCONTROL IControl;
    PDEBUGBREAKPOINT Breakpoint;
    PIDEBUGBREAKPOINT IBreakpoint;
    PDEBUGBREAKPOINT ReturnBreakpoint;
    PIDEBUGBREAKPOINT IReturnBreakpoint;
    PCSTR OffsetExpression;
    PTRACER_INJECTION_HANDLE_BREAKPOINT HandleBreakpoint;
    PTRACER_INJECTION_HANDLE_BREAKPOINT HandleReturnBreakpoint;

    TRACER_INJECTION_BREAKPOINT_ERROR Error;
    TRACER_INJECTION_BREAKPOINT_FLAGS Flags;

    Flags.AsULong = 0;
    Error.AsULong = 0;
    Error.InitializationFailed = TRUE;

    OffsetExpression = BreakpointSpec->OffsetExpression;
    HandleBreakpoint = BreakpointSpec->HandleBreakpoint;
    HandleReturnBreakpoint = BreakpointSpec->HandleReturnBreakpoint;

    Control = Engine->Control;
    IControl = Engine->IControl;

    Result = Control->AddBreakpoint(IControl,
                                    DEBUG_BREAKPOINT_CODE,
                                    DEBUG_ANY_ID,
                                    (PDEBUG_BREAKPOINT *)&IBreakpoint);

    if (Result != S_OK) {
        Error.AddBreakpointFailed = TRUE;
        goto Error;
    }

    Breakpoint = IBreakpoint->lpVtbl;

    Result = Breakpoint->GetId(IBreakpoint, &BreakpointId);
    if (Result != S_OK) {
        BreakpointId = DEBUG_ANY_ID;
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

    if (!HandleReturnBreakpoint) {
        Success = TRUE;
        goto End;
    }

    //
    // Add the return breakpoint in the same fashion, but leave it disabled.
    // (They are enabled in the breakpoint callback for the main handler.)
    //

    Result = Control->AddBreakpoint(IControl,
                                    DEBUG_BREAKPOINT_CODE,
                                    DEBUG_ANY_ID,
                                    (PDEBUG_BREAKPOINT *)&IReturnBreakpoint);

    if (Result != S_OK) {
        Error.AddReturnBreakpointFailed = TRUE;
        goto Error;
    }

    ReturnBreakpoint = IReturnBreakpoint->lpVtbl;

    Result = ReturnBreakpoint->GetId(IReturnBreakpoint, &ReturnBreakpointId);
    if (Result != S_OK) {
        ReturnBreakpointId = DEBUG_ANY_ID;
        Error.GetReturnIdFailed = TRUE;
        goto Error;
    }

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    if (Success) {
        Error.InitializationFailed = FALSE;
        Flags.Initialized = TRUE;
        Flags.BreakpointEnabled = TRUE;
        Flags.ReturnBreakpointEnabled = TRUE;
    }

    InjectionBreakpoint->BreakpointId = BreakpointId;
    InjectionBreakpoint->ReturnBreakpointId = ReturnBreakpointId;
    InjectionBreakpoint->Breakpoint = Breakpoint;
    InjectionBreakpoint->IBreakpoint = IBreakpoint;
    InjectionBreakpoint->ReturnBreakpoint = ReturnBreakpoint;
    InjectionBreakpoint->IReturnBreakpoint = IReturnBreakpoint;
    InjectionBreakpoint->Error.AsULong = Error.AsULong;
    InjectionBreakpoint->Flags.AsULong = Flags.AsULong;
    InjectionBreakpoint->HandleBreakpoint = HandleBreakpoint;
    InjectionBreakpoint->HandleReturnBreakpoint = HandleReturnBreakpoint;
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

    NumberOfBreakpoints = ARRAYSIZE(BreakpointSpecs);

    InjectionBreakpoint = &Context->InitialBreakpoints.First;
    InjectionContext = &Context->InjectionContext;
    InjectionContext->InjectionBreakpoints = InjectionBreakpoint;
    InjectionContext->NumberOfBreakpoints = NumberOfBreakpoints;

    for (Index = 0; Index < NumberOfBreakpoints; Index++) {
        BreakpointSpec = &BreakpointSpecs[Index];

        Success = InitializeBreakpoint(Engine,
                                       InjectionBreakpoint++,
                                       BreakpointSpec);

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
