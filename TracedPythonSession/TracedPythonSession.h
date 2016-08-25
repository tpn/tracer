// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

typedef struct _TRACED_PYTHON_SESSION {

    //
    // Size of the entire structure, in bytes.
    //

    USHORT Size;

    //
    // Padding out to 8 bytes.
    //

    USHORT Unused1;
    ULONG  Unused2;

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
        PPWSTR ArgvW;
        PPWSTR argvw;
    };

    union {
        PPSTR ArgvA;
        PPSTR argv;
    };

    //
    // The argc/argv we pass to Python has the executable name skipped.
    //

    LONG PythonNumberOfArguments;
    PPSTR PythonArgvA;

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
    // Python DLL helper functions we need before we call InitializePython.
    //

    PPYSYS_SET_ARGV_EX PySys_SetArgvEx;
    PPY_SET_PROGRAM_NAME Py_SetProgramName;
    PPY_SET_PYTHON_HOME Py_SetPythonHome;
    PPY_INITIALIZE Py_Initialize;
    PPY_INITIALIZE_EX Py_InitializeEx;
    PPY_IS_INITIALIZED Py_IsInitialized;
    PPY_FINALIZE Py_Finalize;
    PPYEVAL_SETTRACE PyEval_SetProfile;
    PPYEVAL_SETTRACE PyEval_SetTrace;

    //
    // Path to the Python DLL and .exe found by FindPythonDllAndExe.
    //

    PUNICODE_STRING PythonDllPath;
    PUNICODE_STRING PythonExePath;

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
    // Python's main() entry point.
    //

    PPY_MAIN Py_Main;

} TRACED_PYTHON_SESSION, *PTRACED_PYTHON_SESSION, **PPTRACED_PYTHON_SESSION;

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
VOID (DESTROY_TRACED_PYTHON_SESSION)(
    _Inout_ PPTRACED_PYTHON_SESSION Session
    );
typedef DESTROY_TRACED_PYTHON_SESSION *PDESTROY_TRACED_PYTHON_SESSION;

typedef
_Success_(return != 0)
_Check_return_
__allocator
PVOID
(HEAP_ALLOCATION_ROUTINE)(
    _In_ PALLOCATION_CONTEXT AllocationContext,
    _In_ ULONG ByteSize
    );

typedef HEAP_ALLOCATION_ROUTINE *PHEAP_ALLOCATION_ROUTINE;

typedef
VOID
(HEAP_FREE_ROUTINE)(
    _In_ PFREE_CONTEXT FreeContext,
    __deallocate(Mem) _In_ PVOID Buffer
    );

typedef HEAP_FREE_ROUTINE *PHEAP_FREE_ROUTINE;

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
