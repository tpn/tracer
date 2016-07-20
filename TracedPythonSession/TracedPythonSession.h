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
    // Memory allocation routines and contexts.
    //

    PALLOCATION_ROUTINE AllocationRoutine;
    PALLOCATION_CONTEXT AllocationContext;
    PFREE_ROUTINE FreeRoutine;
    PFREE_CONTEXT FreeContext;

    //
    // Rundown list entry.
    //

    LIST_ENTRY RundownListEntry;

    ////////////////////////////////////////////////////////////////////////////
    // Modules
    ////////////////////////////////////////////////////////////////////////////

    //
    // System modules.
    //

    HMODULE Kernel32Module;
    HMODULE Shell32Module;
    HMODULE User32Module;
    HMODULE Advapi32Module;

    //
    // Msvc modules.
    //

    HMODULE Msvcr90Module;

    //
    // The target Python modules.
    //

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
    // Shell32's CommandLineToArgvW function.
    //

    PCOMMAND_LINE_TO_ARGV CommandLineToArgvW;

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

    //
    // Pointers to tracer-specific data structures initialized by the routines
    // above.
    //

    PTRACE_STORES TraceStores;
    PTRACE_SESSION TraceSession;
    PTRACE_CONTEXT TraceContext;

    //
    // Python-specific initializers.
    //

    PINITIALIZE_PYTHON InitializePython;
    PINITIALIZE_PYTHON_TRACE_CONTEXT InitializePythonTraceContext;

    //
    // Pointers to Python-specific data structures initialized by the routines
    // above.
    //

    PPYTHON Python;
    PPYTHON_TRACE_CONTEXT PythonTraceContext;

} TRACED_PYTHON_SESSION, *PTRACED_PYTHON_SESSION, **PPTRACED_PYTHON_SESSION;

typedef
_Success_(return != 0)
BOOL (INITIALIZE_TRACED_PYTHON_SESSION)(
    _Out_    PPTRACED_PYTHON_SESSION Session,
    _In_opt_ PALLOCATION_ROUTINE AllocationRoutine,
    _In_opt_ PALLOCATION_CONTEXT AllocationContext,
    _In_opt_ PFREE_ROUTINE FreeRoutine,
    _In_opt_ PFREE_CONTEXT FreeContext
    );

typedef  INITIALIZE_TRACED_PYTHON_SESSION \
        *PINITIALIZE_TRACED_PYTHON_SESSION;

typedef
VOID (DESTROY_TRACED_PYTHON_SESSION)(
    _Inout_ PPTRACED_PYTHON_SESSION Session
    );

typedef  DESTROY_TRACED_PYTHON_SESSION \
        *PDESTROY_TRACED_PYTHON_SESSION;

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
