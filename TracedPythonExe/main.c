
#include "stdafx.h"


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
    PDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;

    //
    // Initialize the default heap allocator.  This is a thin wrapper around
    // the generic Win32 Heap functions.
    //

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        goto Error;
    }

    //
    // Initialize our TracerConfig from the given registry path.
    //

    TracerConfig = InitializeTracerConfig(
        &Allocator,
        (PUNICODE_STRING)&TracerRegistryPath
    );

    if (!TracerConfig) {
        goto Error;
    }

    //
    // Initialize the TlsHeap machinery, which attaches to the TracerConfig
    // and allocator.
    //

    Success = LoadTlsTracerHeapAndSetTracerConfig(TracerConfig);

    if (!Success) {
        goto Error;
    }

    //
    // Initialize the TracedPythonSession.  This is the main workhorse that
    // loads all the relevant libraries and preps our Python runtime environment
    // with tracing stores created and enabled.
    //

    Success = LoadAndInitializeTracedPythonSession(
        &Session,
        TracerConfig,
        &Allocator,
        NULL,
        &DestroyTracedPythonSession
    );

    if (!Success) {
        goto Error;
    }

    Python = Session->Python;
    PythonTraceContext = Session->PythonTraceContext;

    //
    // Do any hooking here.
    //

    //
    // Start tracing.
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
