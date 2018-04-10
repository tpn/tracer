/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracedPythonSessionPrivate.h

Abstract:

    This is the private header file for the TracedPythonSession component.
    It defines function typedefs and function declarations for all major
    (i.e. not local to the module) functions available for use by individual
    modules within this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

#define TRACER_MODULE_NAMES_ENV_VAR_A  "TRACER_MODULE_NAMES"
#define TRACER_MODULE_NAMES_ENV_VAR_W L"TRACER_MODULE_NAMES"
#define TRACER_MODULE_NAMES_DELIM ';'

FORCEINLINE
_Success_(return != 0)
BOOL
CreateStringTableForTracerModuleNamesEnvironmentVariable(
    _In_ PTRACED_PYTHON_SESSION Session
    )
/*++

Routine Description:

    Creates a STRING_TABLE structure from the environment variable
    TRACER_MODULE_NAMES if it has a value.

Arguments:

    Session - Supplies a pointer to a TRACED_PYTHON_SESSION structure to create
        the string table for.

Return Value:

    If no environment variable is set, or it does not have a valid value, a
    NULL pointer will be returned.  If a string table could be created, a
    pointer to the structure will be returned.

--*/
{
    PSTRING_TABLE_API Api;

    Api = &Session->StringTableApi;

    Session->ModuleNamesStringTable = (
        Api->CreateStringTableFromDelimitedEnvironmentVariable(
            Session->Rtl,
            Session->Allocator,
            Session->StringTableAllocator,
            Session->StringArrayAllocator,
            TRACER_MODULE_NAMES_ENV_VAR_A,
            TRACER_MODULE_NAMES_DELIM
        )
    );

    return (Session->ModuleNamesStringTable != NULL);
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
