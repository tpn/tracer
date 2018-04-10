/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This is the main module for the TracedPythonExe component.  It implements
    a Main() function and provides a mainCRTStartup() entry point which calls
    Main().

--*/

#include "stdafx.h"

DECLSPEC_DLLEXPORT
ULONG
Main(VOID)
{
    BOOL Success;
    ULONG ExitCode = 1;
    PRTL Rtl;
    ALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PTRACED_PYTHON_SESSION Session = NULL;
    PPYTHON Python;
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    PUNICODE_STRING RegistryPath;
    PUNICODE_STRING TraceSessionDirectory = NULL;
    PDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;
    PRUNDOWN_GLOBAL_ATEXIT_FUNCTIONS RundownGlobalAtExitFunctions;

    //
    // Initialize the default heap allocator.  This is a thin wrapper around
    // the generic Win32 Heap functions.
    //

    if (!RtlHeapAllocatorInitialize(&Allocator)) {
        goto Error;
    }

    //
    // Initialize TracerConfig and Rtl.
    //

    RegistryPath = (PUNICODE_STRING)&TracerRegistryPath;

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            &Allocator,
            RegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    //
    // Initialize the TracedPythonSession.  This is the main workhorse that
    // loads all the relevant libraries and preps our Python runtime environment
    // with tracing stores created and enabled.
    //

    Success = LoadAndInitializeTracedPythonSessionInline(
        Rtl,
        TracerConfig,
        &Allocator,
        NULL,
        NULL,
        &TraceSessionDirectory,
        &Session,
        &DestroyTracedPythonSession
    );

    if (!Success) {
        goto Error;
    }

    Python = Session->Python;
    PythonTraceContext = Session->PythonTraceContext;
    RundownGlobalAtExitFunctions = Rtl->RundownGlobalAtExitFunctions;

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

    PythonTraceContext->Start(PythonTraceContext);

    __try {

        if (Python->MajorVersion == 2) {

            ExitCode = Python->Py_MainA(
                Session->NumberOfArguments,
                Session->ArgvA
            );

        } else {

            ExitCode = Python->Py_MainW(
                Session->NumberOfArguments,
                Session->ArgvW
            );

        }

    } __finally {

        RundownGlobalAtExitFunctions(TRUE);
        DestroyTracedPythonSession(&Session);
    }

    //
    // N.B.: there's no `PythonTraceContext->Stop(PythonTraceContext)` call
    //       here to match the Start() one above because Py_Main() tears down
    //       any attached tracers before it returns.
    //

    //
    // Intentional follow-on to Error.
    //

Error:

    RundownGlobalAtExitFunctions(TRUE);

    if (Session) {
        DestroyTracedPythonSession(&Session);
    }

    return ExitCode;
}


DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    ExitProcess(Main());
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
