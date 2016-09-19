#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "TracedPythonSession.h"

#define TRACER_MODULE_NAMES_ENV_VAR "TRACER_MODULE_NAMES"
#define TRACER_MODULE_NAMES_DELIM ';'

FORCEINLINE
_Success_(return != 0)
PSTRING_TABLE
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
    PSTRING_TABLE StringTable;
    PALLOCATOR StringTableAllocator;
    PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE \
        CreateStringTableFromDelimitedEnvironmentVariable;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Session)) {
        return NULL;
    }

    //
    // Make sure we've got a string table allocator initialized.
    //

    StringTableAllocator = Session->pStringTableAllocator;

    if (!StringTableAllocator) {
        return NULL;
    }

    //
    // Make sure we've got the function available.
    //

    CreateStringTableFromDelimitedEnvironmentVariable = (
        Session->CreateStringTableFromDelimitedEnvironmentVariable
    );

    if (!CreateStringTableFromDelimitedEnvironmentVariable) {
        return NULL;
    }

    StringTable = CreateStringTableFromDelimitedEnvironmentVariable(
        Session->Rtl,
        Session->Allocator,
        StringTableAllocator,
        TRACER_MODULE_NAMES_ENV_VAR,
        TRACER_MODULE_NAMES_DELIM
    );

    return StringTable;
}



#ifdef __cplusplus
}; // extern "C"
#endif


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
