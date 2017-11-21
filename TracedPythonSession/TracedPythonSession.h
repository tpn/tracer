/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracedPythonSession.h

Abstract:

    This is the main header file for the TracedPythonSession component.  This
    component is responsible for managing the runtime configuration required to
    trace a Python interpreter using the trace store functionality.

    It depends on the following components: Rtl, Python, PythonTracer,
    TracerConfig, StringTable and TraceStore.

    It is currently used by the TracedPythonExe component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _TRACED_PYTHON_SESSION_INTERNAL_BUILD

//
// This is an internal build of the TracedPythonSession component.
//

#ifdef _TRACED_PYTHON_SESSION_DLL_BUILD

//
// This is the DLL build.
//

#define TRACED_PYTHON_SESSION_API __declspec(dllexport)
#define TRACED_PYTHON_SESSION_DATA extern __declspec(dllexport)

#else

//
// This is the static library build.
//

#define TRACED_PYTHON_SESSION_API
#define TRACED_PYTHON_SESSION_DATA extern

#endif

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define TRACED_PYTHON_SESSION_API __declspec(dllimport)
#define TRACED_PYTHON_SESSION_DATA extern __declspec(dllimport)

#include <Windows.h>
#include "../Rtl/Rtl.h"
#include "../Python/Python.h"
#include "../TraceStore/TraceStore.h"
#include "../StringTable/StringTable.h"
#include "../PythonTracer/PythonTracer.h"
#include "../TracerConfig/TracerConfig.h"

#endif

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _TRACED_PYTHON_SESSION_FLAGS {

    //
    // When set, indicates that this is a readonly trace session.
    //

    ULONG IsReadonly:1;

    //
    // Unused bits.
    //

    ULONG Unused:31;

} TRACED_PYTHON_SESSION_FLAGS;
typedef TRACED_PYTHON_SESSION_FLAGS *PTRACED_PYTHON_SESSION_FLAGS;
C_ASSERT(sizeof(TRACED_PYTHON_SESSION_FLAGS) == sizeof(ULONG));

//
// Forward declaration of the destroy function such that it can be included
// in the TRACED_PYTHON_SESSION struct.
//

typedef
VOID
(DESTROY_TRACED_PYTHON_SESSION)(
    _Pre_notnull_ _Post_null_ struct _TRACED_PYTHON_SESSION **Session
    );
typedef DESTROY_TRACED_PYTHON_SESSION   *PDESTROY_TRACED_PYTHON_SESSION;
typedef DESTROY_TRACED_PYTHON_SESSION **PPDESTROY_TRACED_PYTHON_SESSION;
TRACED_PYTHON_SESSION_API DESTROY_TRACED_PYTHON_SESSION \
                          DestroyTracedPythonSession;

typedef struct _Struct_size_bytes_(Size) _TRACED_PYTHON_SESSION {

    //
    // Size of the entire structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACED_PYTHON_SESSION)) USHORT Size;

    //
    // Python major version ('2' or '3') and minor version, as derived from the
    // DLL path name.
    //

    CHAR PythonMajorVersion;
    CHAR PythonMinorVersion;

    //
    // Number of paths pointed to by the PathEntries array below.
    //

    USHORT NumberOfPathEntries;

    //
    // Padding out to 8 bytes.
    //

    USHORT Unused1;

    //
    // This will be set to the system time used in order to create the trace
    // session directory.
    //

    SYSTEMTIME SystemTime;

    //
    // Array of PUNICODE_STRING paths to add to the DLL path.  Size is governed
    // by NumberOfPathEntries.
    //

    PUNICODE_STRING PathEntries;

    //
    // Array of DLL_DIRECTORY_COOKIE entries for each of the paths above.
    //

    PDLL_DIRECTORY_COOKIE PathDirectoryCookies;

    //
    // TRACER_CONFIG structure.
    //

    PTRACER_CONFIG TracerConfig;

    //
    // Memory allocator.
    //

    PALLOCATOR Allocator;

    //
    // Rundown list entry.
    //

    LIST_ENTRY RundownListEntry;

    //
    // Generic list entry for general use.
    //

    LIST_ENTRY ListEntry;

    ////////////////////////////////////////////////////////////////////////////
    // Modules
    ////////////////////////////////////////////////////////////////////////////

    //
    // Owning module.
    //

    HMODULE OwningModule;
    PRTL_PATH OwningModulePath;

    //
    // System modules.
    //

    HMODULE Kernel32Module;
    HMODULE Shell32Module;
    HMODULE User32Module;
    HMODULE Advapi32Module;
    HMODULE Winsock2Module;

    //
    // Msvc modules.
    //

    HMODULE Msvcr90Module;

    //
    // The target Python modules.
    //

    HMODULE PythonDllModule;
    HMODULE Python2Module;
    HMODULE Python3Module;

    //
    // Our core tracing/support modules.
    //

    HMODULE RtlModule;
    HMODULE HookModule;
    HMODULE TracerHeapModule;
    HMODULE TraceStoreModule;
    HMODULE StringTableModule;

    //
    // Python-specific tracer modules.
    //

    HMODULE PythonModule;
    HMODULE PythonTracerModule;

    //
    // If we've been built as a DLL and someone has loaded us via the inline
    // LoadAndInitializeTracedPythonSession(), this will be set to the module
    // representing the DLL.  If we've been built as a static lib, it won't
    // have a value.
    //

    HMODULE TracedPythonSessionModule;

    ////////////////////////////////////////////////////////////////////////////
    // End of modules.
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Tracer control driver.
    ////////////////////////////////////////////////////////////////////////////

    //
    // Name of the device.
    //

    PUNICODE_STRING TracerControlDeviceName;

    //
    // Handle to the device.
    //

    HANDLE TracerControlDevice;

    ////////////////////////////////////////////////////////////////////////////
    // End of tracer control driver.
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // Functions and data structure pointers.
    ////////////////////////////////////////////////////////////////////////////

    //
    // Shell32's CommandLineToArgvW function and command line data.
    //

    PCOMMAND_LINE_TO_ARGVW CommandLineToArgvW;
    PSTR CommandLineA;
    PWSTR CommandLineW;

    union {
        LONG NumberOfArguments;
        LONG argc;
    };

    union {
        PPSTR ArgvA;
        PPSTR argv;
    };

    union {
        PPWSTR ArgvW;
        PPWSTR argvw;
    };

    //
    // The argc/argv we pass to Python has the executable name skipped.
    //

    LONG PythonNumberOfArguments;
    PPSTR PythonArgvA;
    PPWSTR PythonArgvW;

    //
    // Rtl-specific functions/data.
    //

    PINITIALIZE_RTL InitializeRtl;
    PRTL Rtl;

    //
    // StringTable-specific functions.
    //

    PCREATE_STRING_TABLE CreateStringTable;
    PDESTROY_STRING_TABLE DestroyStringTable;

    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE
        CreateStringTableFromDelimitedEnvironmentVariable;

    PALLOCATOR StringTableAllocator;
    PALLOCATOR StringArrayAllocator;
    PSTRING_TABLE ModuleNamesStringTable;

    //
    // TraceStore-specific initializers.
    //

    PINITIALIZE_TRACE_STORES InitializeTraceStores;
    PINITIALIZE_TRACE_CONTEXT InitializeTraceContext;
    PINITIALIZE_TRACE_SESSION InitializeTraceSession;
    PCLOSE_TRACE_STORES CloseTraceStores;
    PCLOSE_TRACE_CONTEXT CloseTraceContext;
    PINITIALIZE_ALLOCATOR_FROM_TRACE_STORE InitializeAllocatorFromTraceStore;

    //
    // Pointers to tracer-specific data structures initialized by the routines
    // above.
    //

    PTRACE_STORES TraceStores;
    PTRACE_SESSION TraceSession;
    PTRACE_CONTEXT TraceContext;

    //
    // Our trace directory.  The TracerConfig struct owns this.
    //

    PUNICODE_STRING TraceSessionDirectory;

    //
    // Threadpool and callback environment.
    //

    ULONG MaximumProcessorCount;
    PTP_POOL Threadpool;
    TP_CALLBACK_ENVIRON ThreadpoolCallbackEnviron;

    //
    // A threadpool with 1 thread limited to cancellations.
    //

    PTP_POOL CancellationThreadpool;
    TP_CALLBACK_ENVIRON CancellationThreadpoolCallbackEnviron;

    //
    // Python-specific initializers.
    //

    PFIND_PYTHON_DLL_AND_EXE FindPythonDllAndExe;
    PINITIALIZE_PYTHON InitializePython;
    PINITIALIZE_PYTHON_TRACE_CONTEXT InitializePythonTraceContext;
    PCLOSE_PYTHON_TRACE_CONTEXT ClosePythonTraceContext;

    //
    // Pointer to our own destroy function.  This will be filled out if we've
    // been loaded via a DLL by LoadAndInitializeTracedPythonSession().
    //

    PDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;

    //
    // Path to the Python DLL and .exe found by FindPythonDllAndExe.
    //

    PUNICODE_STRING PythonDllPath;
    PUNICODE_STRING PythonExePath;

    //
    // Python home directory.  We don't own this.
    //

    PUNICODE_STRING PythonHomePath;

    //
    // Original directory we started out in.
    //

    PUNICODE_STRING OriginalDirectory;

    //
    // A UTF-8 encoded version of the PythonExePath above (used for setting
    // argv[0]).
    //

    PSTRING PythonExePathA;

    //
    // A UTF-8 encoded version of the directory containing Python DLL and the
    // python.exe file.  This is used to set the Python HOME directory via
    // Py_SetPythonHome
    //

    PSTRING PythonHomePathA;

    //
    // Pointers to Python-specific data structures initialized by the routines
    // above.
    //

    PPYTHON Python;
    PPYTHON_TRACE_CONTEXT PythonTraceContext;

    //
    // Relocation information.
    //

    PTRACE_STORE_FIELD_RELOCS PythonTracerTraceStoreRelocations;

    //
    // Buffers for general purpose use.
    //

    ULONGLONG NumberOfBuffers;
    PVOID Buffers[2];

} TRACED_PYTHON_SESSION, *PTRACED_PYTHON_SESSION, **PPTRACED_PYTHON_SESSION;

typedef struct _PYTHON_HOME {

    //
    // Size of the entire structure, in bytes.
    //

    USHORT Size;

    //
    // Python major version ('2' or '3') and minor version, as derived from the
    // DLL path name.
    //

    CHAR PythonMajorVersion;
    CHAR PythonMinorVersion;

    //
    // Number of paths pointed to by the PathEntries array below.
    //

    USHORT NumberOfPathEntries;

    //
    // Padding out to 8 bytes.
    //

    USHORT   Unused1;

    //
    // Array of PUNICODE_STRING paths to add to the DLL path.  Size is governed
    // by NumberOfPathEntries.
    //

    PUNICODE_STRING PathEntries;

    UNICODE_STRING Directory;

    PUNICODE_STRING DllPath;
    PUNICODE_STRING ExePath;

} PYTHON_HOME, *PPYTHON_HOME;

////////////////////////////////////////////////////////////////////////////////
// Function Type Definitions
////////////////////////////////////////////////////////////////////////////////

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACED_PYTHON_SESSION)(
    _In_     PRTL Rtl,
    _In_     PTRACER_CONFIG TracerConfig,
    _In_opt_ PALLOCATOR Allocator,
    _In_opt_ HMODULE OwningModule,
    _Inout_  PPUNICODE_STRING TraceSessionDirectoryPointer,
    _Outptr_opt_result_maybenull_ PPTRACED_PYTHON_SESSION Session
    );
typedef INITIALIZE_TRACED_PYTHON_SESSION *PINITIALIZE_TRACED_PYTHON_SESSION;
TRACED_PYTHON_SESSION_API INITIALIZE_TRACED_PYTHON_SESSION \
                          InitializeTracedPythonSession;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACED_PYTHON_SESSION_FROM_PYTHON_DLL_MODULE)(
    _In_     PRTL Rtl,
    _In_     PTRACER_CONFIG TracerConfig,
    _In_opt_ PALLOCATOR Allocator,
    _In_opt_ HMODULE PythonDllModule,
    _Inout_  PPUNICODE_STRING TraceSessionDirectoryPointer,
    _Outptr_opt_result_maybenull_ PPTRACED_PYTHON_SESSION Session
    );
typedef INITIALIZE_TRACED_PYTHON_SESSION_FROM_PYTHON_DLL_MODULE
      *PINITIALIZE_TRACED_PYTHON_SESSION_FROM_PYTHON_DLL_MODULE;
TRACED_PYTHON_SESSION_API
    INITIALIZE_TRACED_PYTHON_SESSION_FROM_PYTHON_DLL_MODULE
    InitializeTracedPythonSessionFromPythonDllModule;


typedef
_Success_(return != 0)
BOOL
(SANITIZE_PATH_ENVIRONMENT_VARIABLE_FOR_PYTHON)(
    _In_ PTRACED_PYTHON_SESSION Session
    );

typedef  SANITIZE_PATH_ENVIRONMENT_VARIABLE_FOR_PYTHON \
        *PSANITIZE_PATH_ENVIRONMENT_VARIABLE_FOR_PYTHON;
TRACED_PYTHON_SESSION_API SANITIZE_PATH_ENVIRONMENT_VARIABLE_FOR_PYTHON \
                          SanitizePathEnvironmentVariableForPython;

typedef
_Success_(return != 0)
BOOL
(CHANGE_INTO_PYTHON_HOME_DIRECTORY)(
    _In_ PTRACED_PYTHON_SESSION Session
    );
typedef CHANGE_INTO_PYTHON_HOME_DIRECTORY  \
      *PCHANGE_INTO_PYTHON_HOME_DIRECTORY, \
    **PPCHANGE_INTO_PYTHON_HOME_DIRECTORY;
TRACED_PYTHON_SESSION_API CHANGE_INTO_PYTHON_HOME_DIRECTORY \
                          ChangeIntoPythonHomeDirectory;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(LOAD_AND_INITIALIZE_TRACED_PYTHON_SESSION)(
    _In_ PRTL Rtl,
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PALLOCATOR Allocator,
    _In_opt_ HMODULE OwningModule,
    _In_opt_ HMODULE PythonDllModule,
    _Inout_ PPUNICODE_STRING TraceSessionDirectoryPointer,
    _Out_ PPTRACED_PYTHON_SESSION Session,
    _Out_ PPDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSessionPointer
    );
typedef LOAD_AND_INITIALIZE_TRACED_PYTHON_SESSION
      *PLOAD_AND_INITIALIZE_TRACED_PYTHON_SESSION;
TRACED_PYTHON_SESSION_API LOAD_AND_INITIALIZE_TRACED_PYTHON_SESSION
                          LoadAndInitializeTracedPythonSession;

////////////////////////////////////////////////////////////////////////////////
// Inline Functions
////////////////////////////////////////////////////////////////////////////////

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
LoadAndInitializeTracedPythonSessionInline(
    _In_ PRTL Rtl,
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PALLOCATOR Allocator,
    _In_opt_ HMODULE OwningModule,
    _In_opt_ HMODULE PythonDllModule,
    _Inout_ PPUNICODE_STRING TraceSessionDirectoryPointer,
    _Out_ PPTRACED_PYTHON_SESSION Session,
    _Out_ PPDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSessionPointer
    )
/*++

Routine Description:

    This routine loads the TracedPythonSession DLL based on the path specified
    by the TracerConfig->Paths.TracedPythonSessionDllPath variable, resolves
    the InitializeTracedPythonSession() function, then calls it with the same
    arguments as passed in.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    Allocator - Optionally supplies a pointer to an alternate ALLOCATOR to use.
        If not present, TracerConfig->Allocator will be used.

    OwningModule - Optionally supplies the owning module handle.

    PythonDllModule - Optionally supplies a handle for the target Python module
        (i.e. python27.dll) for which the traced session is to be initialized.

    TraceSessionDirectoryPointer - Supplies a pointer to a variable that either
        provides the address to a UNICODE_STRING structure that represents the
        trace session directory to open in a read-only context, or, if the
        pointer is NULL, this will receive the address of the newly-created
        UNICODE_STRING structure that matches the trace session directory.

    SessionPointer - Supplies a pointer that will receive the address of the
        TRACED_PYTHON_SESSION structure allocated.  This pointer is immediately
        cleared (that is, '*SessionPointer = NULL;' is performed once
        SessionPointer is deemed non-NULL), and a value will only be set if
        initialization was successful.

    DestroyTracedPythonSessionPointer - Supplies a pointer to the address of a
        variable that will receive the address of the DLL export by the same
        name.  This should be called in order to destroy a successfully
        initialized session.

Return Value:

    TRUE on Success, FALSE if an error occurred.  *SessionPointer will be
    updated with the value of the newly created TRACED_PYTHON_SESSION
    structure.

See Also:

    InitializeTracedPythonSession().

--*/
{
    BOOL Success;
    HMODULE Module;
    PDESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;
    PINITIALIZE_TRACED_PYTHON_SESSION InitializeTracedPythonSession;
    PINITIALIZE_TRACED_PYTHON_SESSION_FROM_PYTHON_DLL_MODULE
        InitializeTracedPythonSessionFromPythonDllModule;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceSessionDirectoryPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Session)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(DestroyTracedPythonSessionPointer)) {
        return FALSE;
    }

    //
    // Attempt to load the library.
    //

    Module = LoadLibraryW(
        TracerConfig->Paths.TracedPythonSessionDllPath.Buffer
    );

    if (!Module) {
        OutputDebugStringA("Failed to load TracedPythonSessionDllPath.\n");
        return FALSE;
    }

    //
    // Resolve the initialize and destroy functions.
    //

    DestroyTracedPythonSession = (PDESTROY_TRACED_PYTHON_SESSION)(
        GetProcAddress(
            Module,
            "DestroyTracedPythonSession"
        )
    );

    if (!DestroyTracedPythonSession) {
        OutputDebugStringA("Failed to resolve DestroyTracedPythonSession\n");
        goto Error;
    }

    //
    // Call the initialization function with the same arguments we were passed.
    //

    if (PythonDllModule) {

        InitializeTracedPythonSessionFromPythonDllModule = (
            (PINITIALIZE_TRACED_PYTHON_SESSION_FROM_PYTHON_DLL_MODULE)(
                GetProcAddress(
                    Module,
                    "InitializeTracedPythonSessionFromPythonDllModule"
                )
            )
        );

        if (!InitializeTracedPythonSessionFromPythonDllModule) {
            OutputDebugStringA(
                "InitializeTracedPythonSessionFromPythonDllModule: "
                "not found.\n"
            );
            goto Error;
        }

        Success = (
            InitializeTracedPythonSessionFromPythonDllModule(
                Rtl,
                TracerConfig,
                Allocator,
                PythonDllModule,
                TraceSessionDirectoryPointer,
                Session
            )
        );

    } else {

        InitializeTracedPythonSession = (PINITIALIZE_TRACED_PYTHON_SESSION)(
            GetProcAddress(
                Module,
                "InitializeTracedPythonSession"
            )
        );

        if (!InitializeTracedPythonSession) {
            OutputDebugStringA("InitializeTracedPythonSession: not found.\n");
            goto Error;
        }


        Success = InitializeTracedPythonSession(Rtl,
                                                TracerConfig,
                                                Allocator,
                                                OwningModule,
                                                TraceSessionDirectoryPointer,
                                                Session);
    }

    if (!Success) {
        goto Error;
    }

    //
    // Update the caller's DestroyTracedPythonSession function pointer and the
    // session pointer to the same function.
    //

    *DestroyTracedPythonSessionPointer = DestroyTracedPythonSession;
    (*Session)->DestroyTracedPythonSession = DestroyTracedPythonSession;

    //
    // Make a note of the session's own module.
    //

    (*Session)->TracedPythonSessionModule = Module;

    //
    // Return success.
    //

    return TRUE;

Error:

    //
    // Attempt to destroy the session.
    //

    if (Session && *Session && DestroyTracedPythonSession) {

        //
        // This will also clear the caller's session pointer.
        //

        DestroyTracedPythonSession(Session);
    }

    //
    // Attempt to free the module.
    //

    if (Module) {
        FreeLibrary(Module);
    }

    return FALSE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
