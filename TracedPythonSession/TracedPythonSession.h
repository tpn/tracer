// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

static CONST PWSTR PATH_ENV_NAME = L"Path";

typedef struct _TRACED_PYTHON_SESSION {

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

    ////////////////////////////////////////////////////////////////////////////
    // Modules
    ////////////////////////////////////////////////////////////////////////////

    //
    // Owning module.
    //

    HMODULE OwningModule;
    PPATH OwningModulePath;

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
    HMODULE TracerModule;

    //
    // Python-specific tracer modules.
    //

    HMODULE PythonModule;
    HMODULE PythonTracerModule;

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
    // Hook-specific functions.
    //

    PHOOK Hook;
    PUNHOOK Unhook;
    PINITIALIZE_HOOKED_FUNCTION InitializeHookedFunction;

    //
    // Tracer-specific initializers.
    //

    PINITIALIZE_TRACE_STORES InitializeTraceStores;
    PINITIALIZE_TRACE_CONTEXT InitializeTraceContext;
    PINITIALIZE_TRACE_SESSION InitializeTraceSession;
    PCLOSE_TRACE_STORES CloseTraceStores;

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
    // Python-specific initializers.
    //

    PFIND_PYTHON_DLL_AND_EXE FindPythonDllAndExe;
    PINITIALIZE_PYTHON InitializePython;
    PINITIALIZE_PYTHON_TRACE_CONTEXT InitializePythonTraceContext;

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

} TRACED_PYTHON_SESSION, *PTRACED_PYTHON_SESSION, **PPTRACED_PYTHON_SESSION;

typedef _Struct_size_bytes_(Size) struct _PATH_ENV_VAR {

    _Field_range_(==, sizeof(struct _PATH_ENV_VAR)) USHORT StructSize;

    USHORT FirstAlignedAllocSizeInBytes;
    USHORT SecondAlignedAllocSizeInBytes;

    USHORT NumberOfElements;
    USHORT ReservedUnicodeBufferSizeInBytes;

    //
    // Pad out to an 8-byte boundary.
    //

    USHORT Padding[3];

    //
    // The allocator used to create this structure.
    //

    PALLOCATOR Allocator;

    //
    // Bitmap for directory end points.
    //

    RTL_BITMAP Bitmap;

    UNICODE_STRING Paths;
    UNICODE_STRING NewPaths;
    UNICODE_PREFIX_TABLE PathsPrefixTable;
    UNICODE_PREFIX_TABLE PathsToRemovePrefixTable;

    //
    // Pointer to the first element of an array of UNICODE_STRING structs used
    // to capture each directory in the path.
    //

    PUNICODE_STRING Directories;

    //
    // Pointer to the first element of an array of UNICODE_PREFIX_TABLE_ENTRY
    // structs used for the PathsPrefixTable.
    //

    PUNICODE_PREFIX_TABLE_ENTRY PathsPrefixTableEntries;

    //
    // Pointer to the first element of an array of UNICODE_PREFIX_TABLE_ENTRY
    // structs used for the PathsToRemovePrefixTable.
    //

    PUNICODE_PREFIX_TABLE_ENTRY PathsToRemovePrefixTableEntries;

} PATH_ENV_VAR, *PPATH_ENV_VAR, **PPPATH_ENV_VAR;

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

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACED_PYTHON_SESSION)(
    _Out_       PPTRACED_PYTHON_SESSION Session,
    _In_        PTRACER_CONFIG TracerConfig,
    _In_opt_    PALLOCATOR Allocator,
    _In_opt_    HMODULE OwningModule
    );
typedef INITIALIZE_TRACED_PYTHON_SESSION *PINITIALIZE_TRACED_PYTHON_SESSION;

typedef
_Success_(return != 0)
BOOL
(REMOVE_CONFLICTING_PYTHON_PATHS_FROM_PATH_ENVIRONMENT_VARIABLE)(
    _In_ PTRACED_PYTHON_SESSION Session
    );

typedef  REMOVE_CONFLICTING_PYTHON_PATHS_FROM_PATH_ENVIRONMENT_VARIABLE \
        *PREMOVE_CONFLICTING_PYTHON_PATHS_FROM_PATH_ENVIRONMENT_VARIABLE;

typedef
VOID (DESTROY_TRACED_PYTHON_SESSION)(
    _Inout_ PPTRACED_PYTHON_SESSION Session
    );
typedef DESTROY_TRACED_PYTHON_SESSION *PDESTROY_TRACED_PYTHON_SESSION;

typedef
_Success_(return != 0)
PPATH_ENV_VAR
(LOAD_PATH_ENVIRONMENT_VARIABLE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ USHORT ReservedUnicodeBufferSizeInBytes
    );
typedef LOAD_PATH_ENVIRONMENT_VARIABLE *PLOAD_PATH_ENVIRONMENT_VARIABLE;

typedef
VOID
(DESTROY_PATH_ENVIRONMENT_VARIABLE)(
    _Inout_ PPPATH_ENV_VAR PathPointer
    );
typedef DESTROY_PATH_ENVIRONMENT_VARIABLE *PDESTROY_PATH_ENVIRONMENT_VARIABLE;

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
