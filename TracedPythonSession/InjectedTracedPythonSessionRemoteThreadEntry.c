/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    InjectedTracedPythonSessionRemoteThreadEntry.c

Abstract:

    TBD.

--*/

#include "stdafx.h"

#include "../PythonTracerInjection/PythonTracerInjectionPrivate.h"

typedef struct _WORK_ITEM {

    union {
        SLIST_ENTRY ListEntry;
        struct {
            PSLIST_ENTRY NextEntry;
            struct {
                ULONG WorkId;
                ULONG Unused1;
            };
        };
    };

    PVOID Param1;
    PVOID Param2;
    PVOID Param3;
    PVOID Param4;
    PVOID Param5;
    PVOID Param6;

} WORK_ITEM;
typedef WORK_ITEM *PWORK_ITEM;
C_ASSERT(sizeof(WORK_ITEM) == 64);

typedef struct _TRACED_PYTHON_SESSION_INJECTED_THREAD_CONTEXT {
    SLIST_HEADER FreeList;
    SLIST_HEADER WorkList;

    ULONG Flags;
    ULONG State;

    ULONG MainThreadId;
    ULONG NumberOfWorkItems;

    PWORK_ITEM BaseWorkItem;

    HANDLE LocalShutdownEvent;
    HANDLE LocalWorkAvailableEvent;

    HANDLE RemoteShutdownEvent;
    HANDLE RemoteWorkAvailableEvent;

} TRACED_PYTHON_SESSION_INJECTED_THREAD_CONTEXT;
typedef TRACED_PYTHON_SESSION_INJECTED_THREAD_CONTEXT
      *PTRACED_PYTHON_SESSION_INJECTED_THREAD_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(TRACED_PYTHON_SESSION_INJECTED_THREAD_EVENT_LOOP)(
    _In_ PTRACED_PYTHON_SESSION Session,
    _In_ PPYTHON_TRACER_INJECTED_CONTEXT InjectedContext,
    _In_ PTRACED_PYTHON_SESSION_INJECTED_THREAD_CONTEXT Context,
    _Out_ PULONG ExitCode
    );
typedef TRACED_PYTHON_SESSION_INJECTED_THREAD_EVENT_LOOP
      *PTRACED_PYTHON_SESSION_INJECTED_THREAD_EVENT_LOOP;

BOOL
TracedPythonSessionInjectedThreadEventLoop(
    PTRACED_PYTHON_SESSION Session,
    PPYTHON_TRACER_INJECTED_CONTEXT InjectedContext,
    PTRACED_PYTHON_SESSION_INJECTED_THREAD_CONTEXT Context,
    PULONG ExitCode
    )
/*++

Routine Description:

    TBD.

Arguments:

    TBD.

Return Value:

    TBD.

--*/
{
    BOOL Success = TRUE;

    return Success;
}


DECLSPEC_DLLEXPORT
_Use_decl_annotations_
LONG
InjectedTracedPythonSessionRemoteThreadEntry(
    PPYTHON_TRACER_INJECTED_CONTEXT InjectedContext,
    PINJECTION_OBJECTS InjectionObjects,
    PINJECTION_FUNCTIONS Functions
    )
/*++

Routine Description:

    TBD.

Arguments:

    TBD.

Return Value:

    TBD.

--*/
{
    BOOL Success;
    PRTL Rtl;
    PPYTHON Python;
    LONG ExitCode = -1;
    volatile ULONG *Status;
    PALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    PTRACED_PYTHON_SESSION Session;
    PDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;
    PUNICODE_STRING TraceSessionDirectory = NULL;
    PINJECTION_OBJECT Object;
    PINJECTION_OBJECT_EVENT Event1;
    PINJECTION_OBJECT_EVENT Event2;
    PINJECTION_OBJECT_FILE_MAPPING Shared1;

    Object = InjectionObjects->Objects;
    Event1 = &Object->AsEvent;
    Event2 = &(Object + 1)->AsEvent;
    Shared1 = &(Object + 2)->AsFileMapping;

    Status = (PULONG)Shared1->BaseAddress;

    OutputDebugStringA("InjectedTracedPythonSessionRemoteThreadEntry()!\n");

    Allocator = (PALLOCATOR)(
        HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(ALLOCATOR)
        )
    );

    if (!Allocator) {
        goto Error;
    }

    if (!RtlHeapAllocatorInitialize(Allocator)) {
        goto Error;
    }

    //
    // Initialize TracerConfig and Rtl.
    //

    RegistryPath = (PUNICODE_STRING)&TracerRegistryPath;

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            Allocator,
            RegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    OutputDebugStringA("CreateAndInitializeTracerConfigAndRtl()\n");

    Success = LoadAndInitializeTracedPythonSession(
        Rtl,
        TracerConfig,
        Allocator,
        NULL,
        InjectedContext->PythonDllModule,
        &TraceSessionDirectory,
        &Session,
        &DestroyTracedPythonSession
    );

    OutputDebugStringA("LoadAndInitializeTracedPythonSession()\n");

    if (!Success) {
        goto Error;
    }

    Python = Session->Python;
    PythonTraceContext = Session->PythonTraceContext;

    //
    // Initialize the __C_specific_handler from Rtl.
    //

    __C_specific_handler_impl = Session->Rtl->__C_specific_handler;

    //
    // Do any hooking here.
    //

    //
    // Start tracing/profiling.
    //

    Session->PythonTraceContext->Start(Session->PythonTraceContext);

    //
    // Set the event indicating we've completed initialization.
    //

    SetEvent(Event1->Handle);

    //
    // Enter the event loop.
    //

    *Status = 1;

    Success = (
        TracedPythonSessionInjectedThreadEventLoop(
            Session,
            InjectedContext,
            NULL,
            &ExitCode
        )
    );

    if (Success) {
        ExitCode = 0;
        goto End;
    }

Error:

    ExitCode = -1;
    *Status = ExitCode;

End:

    OutputDebugStringA("Injected thread returning\n");

    return ExitCode;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
