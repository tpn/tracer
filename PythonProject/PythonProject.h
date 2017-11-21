/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonProject.h

Abstract:

    This is the main header file for the PythonProject component, which is
    responsible for capturing information about a Python project such as the
    Python environment (e.g. conda environment), OS environment strings, root
    directory, etc.

    This module defines the PYTHON_PROJECT structure, as well as function
    type definitions for creating and destroying such structures.

    The PythonProject component plays an important role in facilitating the
    tracing of Python projects during their development.  When tracing is
    first being configured for a project, a normal (non-traced) run will be
    used to prime the first PYTHON_PROJECT structure -- i.e. create a structure
    that captures all the information pertaining to a successful run of the
    project.

    This information can then be persisted and re-loaded at a later time by
    the tracing components in order to re-establish the exact same environment
    conditions before starting execution.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _PYTHON_PROJECT_INTERNAL_BUILD

//
// This is an internal build of the PythonProject component.
//

#ifdef _PYTHON_PROJECT_DLL_BUILD

//
// This is the DLL build.
//

#define PYTHON_PROJECT_API __declspec(dllexport)
#define PYTHON_PROJECT_DATA extern __declspec(dllexport)

#else

//
// This is the static library build.
//

#define PYTHON_PROJECT_API
#define PYTHON_PROJECT_DATA extern

#endif


#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define PYTHON_PROJECT_API __declspec(dllimport)
#define PYTHON_PROJECT_DATA extern __declspec(dllimport)

#include <Windows.h>
#include "../Rtl/Rtl.h"

#endif

//
// Forward declaration of the destroy function such that it can be included
// in the PYTHON_PROJECT struct.
//

typedef
VOID
(DESTROY_PYTHON_PROJECT)(
    _Pre_notnull_ _Post_null_ struct _PYTHON_PROJECT **Session
    );
typedef DESTROY_PYTHON_PROJECT   *PDESTROY_PYTHON_PROJECT;
typedef DESTROY_PYTHON_PROJECT **PPDESTROY_PYTHON_PROJECT;
PYTHON_PROJECT_API DESTROY_PYTHON_PROJECT \
                          DestroyPythonProject;

typedef struct _Struct_size_bytes_(StructSize) _PYTHON_PROJECT {

    //
    // Size of the entire structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PYTHON_PROJECT)) USHORT StructSize;

    //
    // Python major version ('2' or '3') and minor version, as derived from the
    // DLL path name.
    //

    CHAR PythonMajorVersion;
    CHAR PythonMinorVersion;


} PYTHON_PROJECT;
typedef PYTHON_PROJECT *PPYTHON_PROJECT;
typedef PYTHON_PROJECT **PPPYTHON_PROJECT;

////////////////////////////////////////////////////////////////////////////////
// Function Type Definitions
////////////////////////////////////////////////////////////////////////////////

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_PYTHON_PROJECT)(
    _Outptr_opt_result_maybenull_ PPPYTHON_PROJECT PythonProject,
    _In_     PUNICODE_STRING Directory
    );
typedef INITIALIZE_PYTHON_PROJECT *PINITIALIZE_PYTHON_PROJECT;
PYTHON_PROJECT_API INITIALIZE_PYTHON_PROJECT InitializePythonProject;


////////////////////////////////////////////////////////////////////////////////
// Inline Functions
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
