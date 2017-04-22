/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    InjectedTracedPythonSessionRemoteThreadEntry.c

Abstract:

    TBD.

--*/

#include "stdafx.h"

#include "../PythonTracerInjection/PythonTracerInjectionPrivate.h"

_Use_decl_annotations_
LONG
InjectedTracedPythonSessionRemoteThreadEntry(
    PPYTHON_TRACER_INJECTED_CONTEXT InjectedContext
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
    ALLOCATOR Allocator;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    PTRACED_PYTHON_SESSION Session;
    PDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;
    PUNICODE_STRING TraceSessionDirectory;

    //
    // Initialize the default heap allocator.  This is a thin wrapper around
    // the generic Win32 Heap functions.
    //

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
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

    Success = LoadAndInitializeTracedPythonSession(
        Rtl,
        TracerConfig,
        &Allocator,
        NULL,
        InjectedContext->PythonDllModule,
        &TraceSessionDirectory,
        &Session,
        &DestroyTracedPythonSession
    );

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
    // Start tracing/profiling.
    //

    //
    // XXX: this should be done in the context of the main thread...
    //

    PythonTraceContext->Start(PythonTraceContext);

    while (TRUE) {
        SleepEx(INFINITE, TRUE);
    }

    //
    // We're finally done.
    //

    Success = TRUE;

    goto End;

Error:
    Success = FALSE;

    //DestroyTracedPythonSession(&Session);

End:

    return (Success ? 0 : 1);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
