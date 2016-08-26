
#include "stdafx.h"

INITIALIZE_TRACED_PYTHON_SESSION InitializeTracedPythonSession;
DESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;

ULONG
Main(VOID)
{
    BOOL Success;
    ULONG ExitCode = 1;
    ALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PTRACED_PYTHON_SESSION Session;
    PPYTHON Python;
    PPYTHON_TRACE_CONTEXT PythonTraceContext;

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        goto Error;
    }

    TracerConfig = InitializeTracerConfig(
        &Allocator,
        (PUNICODE_STRING)&TracerRegistryPath
    );

    if (!TracerConfig) {
        goto Error;
    }

    Success = InitializeTracedPythonSession(
        &Session,
        TracerConfig,
        &Allocator,
        NULL
    );

    if (!Success) {
        goto Error;
    }

    Python = Session->Python;
    PythonTraceContext = Session->PythonTraceContext;

    //
    // Do any hooking here.
    //

    PythonTraceContext->StartTracing(PythonTraceContext);

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

    //
    // N.B.: there's no `PythonTraceContext->StopTracing(PythonTraceContext)`
    //       call here to match the StartTracing() one above because Py_Main()
    //       tears down any attached tracers before it returns.
    //

    //
    // Intentional follow-on to Error.
    //

Error:

    if (Session) {
        DestroyTracedPythonSession(&Session);
    }

    return ExitCode;
}


VOID
WINAPI
mainCRTStartup()
{
    ExitProcess(Main());
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
