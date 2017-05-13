/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracerConfigPrivate.c

Abstract:

    This module implements private routines used by the TracerConfig component.

--*/

#include "stdafx.h"


_Use_decl_annotations_
BOOL
LoadAllTracerModules(
    PTRACER_CONFIG TracerConfig
    )
/*++

Routine Description:

    Call LoadLibraryW() against every fully-qualified path represented in the
    TRACER_PATHS structure of a TRACER_CONFIG structure.

    This is currently only used by the LoadModulesExe component to assist in
    symbol introspection.  It has minimal error handling and will effectively
    leak all loaded modules (in that they'll stay resident for the entire
    duration of the program as FreeLibrary() is not called, nor is the module
    handle even captured).

Arguments:

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure
        whose DLL paths will be loaded.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    USHORT Index;
    USHORT NumberOfDllPaths;
    HMODULE Module;
    PTRACER_PATHS Paths;
    PUNICODE_STRING Path;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    //
    // Initialize aliases and local variables.
    //

    Paths = &TracerConfig->Paths;
    NumberOfDllPaths = TracerConfig->Paths.NumberOfDllPaths;

    for (Index = 0; Index < NumberOfDllPaths; Index++) {
        Path = &Paths->FirstDllPath + Index;
        Module = LoadLibraryW(Path->Buffer);
        if (!Module || Module == INVALID_HANDLE_VALUE) {
            NOTHING;
        }
    }

    return TRUE;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
