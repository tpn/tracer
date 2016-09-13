#pragma once

#include "../TracerConfig/TracerConfig.h"

typedef
_Success_(return != 0)
BOOL
(TLS_TRACER_HEAP_SET_TRACER_CONFIG)(
    _In_ PTRACER_CONFIG TracerConfig
    );
typedef TLS_TRACER_HEAP_SET_TRACER_CONFIG *PTLS_TRACER_HEAP_SET_TRACER_CONFIG;

FORCEINLINE
BOOL
LoadTlsTracerHeapAndSetTracerConfig(
    _In_ PTRACER_CONFIG TracerConfig
    )
{
    BOOL Success;
    HMODULE Module;
    PTLS_TRACER_HEAP_SET_TRACER_CONFIG TlsTracerHeapSetTracerConfig;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    //
    // Attempt to load the library.  This will trigger the TlsTracerHeap's
    // DLL_PROCESS_ATTACH function, which will reserve a TlsIndex slot.
    //

    Module = LoadLibraryW(TracerConfig->Paths.TlsTracerHeapDllPath.Buffer);

    if (!Module) {
        OutputDebugStringA("Failed to load TlsTracerHeapDllPath.\n");
        return FALSE;
    }

    //
    // Resolve the function we want.
    //

    TlsTracerHeapSetTracerConfig = (PTLS_TRACER_HEAP_SET_TRACER_CONFIG)(
        GetProcAddress(Module, "TlsTracerHeapSetTracerConfig")
    );

    if (!TlsTracerHeapSetTracerConfig) {
        OutputDebugStringA("Failed to resolve TlsTracerHeapSetTracerConfig\n");
        goto Error;
    }

    //
    // Set the TracerConfig for the TlsTracerHeap.  This will set the global
    // variable within the DLL and wire up the TracerConfig allocator with
    // the necessary Tls heap information.
    //

    Success = TlsTracerHeapSetTracerConfig(TracerConfig);

    if (!Success) {
        goto Error;
    }

    //
    // We're done.  Record the module in the TracerConfig struct and return
    // success.
    //

    TracerConfig->TlsTracerHeapModule = Module;

    return TRUE;

Error:

    //
    // An error occurred, make sure we free the library we just loaded.
    //

    if (Module) {
        FreeLibrary(Module);
    }

    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
