#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "TracedPythonSession.h"

INITIALIZE_TRACED_PYTHON_SESSION InitializeTracedPythonSession;
DESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;

LOAD_PATH_ENVIRONMENT_VARIABLE LoadPathEnvironmentVariable;
DESTROY_PATH_ENVIRONMENT_VARIABLE DestroyPathEnvironmentVariable;

SANITIZE_PATH_ENVIRONMENT_VARIABLE_FOR_PYTHON \
    SanitizePathEnvironmentVariableForPython;


#ifdef __cpp
} // extern "C"
#endif


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
