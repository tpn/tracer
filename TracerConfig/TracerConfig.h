/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TracerConfig.h

Abstract:

    This is the main header file for the TracerConfig component.  It provides
    a runtime interface to tracer configuration settings mostly derived from
    the registry.

--*/

#pragma once

#ifdef _TRACER_CONFIG_INTERNAL_BUILD

//
// This is an internal build of the TracerConfig component.
//

#ifdef _TRACER_CONFIG_DLL_BUILD

//
// This is the DLL build.
//

#define TRACER_CONFIG_API __declspec(dllexport)
#define TRACER_CONFIG_DATA extern __declspec(dllexport)

#else

//
// This is the static library build.
//

#define TRACER_CONFIG_API
#define TRACER_CONFIG_DATA extern

#endif

#include "stdafx.h"

#else

//
// We're being included by another project.
//

#ifdef _TRACER_CONFIG_STATIC_LIB

//
// We're being included by another project as a static library.
//

#define TRACER_CONFIG_API
#define TRACER_CONFIG_DATA extern

#else

//
// We're being included by another project that wants to use us as a DLL.
//

#define TRACER_CONFIG_API __declspec(dllimport)
#define TRACER_CONFIG_DATA extern __declspec(dllimport)

#endif

#include "../Rtl/Rtl.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma once

//
// Constants.
//

static CONST UNICODE_STRING TracerRegistryPath = \
    RTL_CONSTANT_STRING(L"Software\\Tracer");

//
// Function typedefs.
//

typedef
_Check_return_
BOOLEAN
(CREATE_TRACE_SESSION_DIRECTORY)(
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _Out_ PPUNICODE_STRING Directory
    );

typedef CREATE_TRACE_SESSION_DIRECTORY *PCREATE_TRACE_SESSION_DIRECTORY;
TRACER_CONFIG_API CREATE_TRACE_SESSION_DIRECTORY CreateTraceSessionDirectory;

typedef
_Check_return_
BOOLEAN
(CREATE_GLOBAL_TRACE_SESSION_DIRECTORY)(
    _Out_ PPUNICODE_STRING Directory
    );

typedef CREATE_GLOBAL_TRACE_SESSION_DIRECTORY \
    *PCREATE_GLOBAL_TRACE_SESSION_DIRECTORY;

typedef _Struct_size_bytes_(Size) struct _TRACER_PATHS {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACER_PATHS)) USHORT Size;

    //
    // Padding out to 8-bytes.
    //

    USHORT Padding[3];

    //
    // Installation directory and the base trace data directory
    // (where trace session directories are created).  These values
    // are read from the registry.
    //

    UNICODE_STRING InstallationDirectory;
    UNICODE_STRING BaseTraceDirectory;

    //
    // Default Python directory to use when initializing a traced Python
    // session if the directory cannot be otherwise identified.  Also read
    // from the registry.
    //

    UNICODE_STRING DefaultPythonDirectory;

    //
    // Fully-qualified paths to relevant DLLs.  The paths are built
    // relative to the InstallationDirectory above.  If the flag
    // "LoadDebugLibraries" was set, the DLL paths will represent
    // debug versions of the libraries.
    //

    UNICODE_STRING RtlDllPath;
    UNICODE_STRING PythonDllPath;
    UNICODE_STRING TracerHeapDllPath;
    UNICODE_STRING TraceStoreDllPath;
    UNICODE_STRING StringTableDllPath;
    UNICODE_STRING PythonTracerDllPath;
    UNICODE_STRING TlsTracerHeapDllPath;
    UNICODE_STRING TracedPythonSessionDllPath;

} TRACER_PATHS, *PTRACER_PATHS;

//
// Bitmask of supported runtimes.  (Currently only Python and
// C are supported.)
//

typedef _Struct_size_bytes_(sizeof(ULONG)) struct _TRACER_SUPPORTED_RUNTIMES {
    ULONG Python:1;
    ULONG C:1;
    ULONG CPlusPlus:1;
    ULONG CSharp:1;
    ULONG Ruby:1;
} TRACER_SUPPORTED_RUNTIMES, *PTRACER_SUPPORTED_RUNTIMES;


//
// Tracer configuration flags.  Map to REG_DWORD entries of the same name.
//

typedef _Struct_size_bytes_(sizeof(ULONG)) struct _TRACER_FLAGS {

    //
    // When set, entry routines such as InitializeTracerConfig() and
    // DestroyTracerConfig() should call __debugbreak() as soon as
    // possible during startup.
    //

    ULONG DebugBreakOnEntry:1;

    //
    // When set, indicates that debug versions of the DLLs should be
    // loaded instead of the release versions.  Defaults to FALSE.
    //

    ULONG LoadDebugLibraries:1;

    //
    // When set, disable compression on trace session directories.
    // Defaults to FALSE.
    //

    ULONG DisableTraceSessionDirectoryCompression:1;

    //
    // When set, disable the trace store mechanism that pre-faults
    // pages ahead of time in a separate thread pool.  Defaults to FALSE.
    //

    ULONG DisablePrefaultPages:1;

    //
    // When set, virtual memory stats will be tracked during tracing.
    // Defaults to FALSE.
    //

    ULONG EnableMemoryTracing:1;

    //
    // When set, I/O counters will be tracked during tracing.
    // Defaults to FALSE.
    //

    ULONG EnableIoCounterTracing:1;

    //
    // When set, handle counts will be tracked during tracing.
    // Defaults to FALSE.
    //

    ULONG EnableHandleCountTracing:1;

    //
    // The following flags relate to the dwFlagsAndAttributes parameter passed
    // to CreateFile() when opening a trace store.
    //

    //
    // When set, do not set the FILE_FLAG_OVERLAPPED flag.
    //

    ULONG DisableFileFlagOverlapped:1;

    //
    // When set, do not set the FILE_FLAG_SEQUENTIAL_SCAN flag.  This will
    // automatically be set if EnableFileFlagRandomAccess is set.
    //

    ULONG DisableFileFlagSequentialScan:1;

    //
    // When set, sets the FILE_FLAG_RANDOM_ACCESS flag.  This will also set
    // DisableFileFlagSequentialScan (or rather, ensure that the final flags
    // passed to CreateFile() do not contain FILE_FLAG_SEQUENTIAL_SCAN).
    //

    ULONG EnableFileFlagRandomAccess:1;

    //
    // When set, sets the FILE_FLAG_WRITE_THROUGH flag.
    //

    ULONG EnableFileFlagWriteThrough:1;

    //
    // When set, the tracer will start off profiling instead of tracing.
    //

    ULONG ProfileOnly:1;

    //
    // When set, tracks the maximum reference count values observed for
    // _Py_NoneStruct, _Py_TrueStruct, _Py_ZeroStruct, and _Py_FalseStruct if
    // it's available.
    //

    ULONG TrackMaxRefCounts:1;

    //
    // When set, enables tracing of the process's working set.
    //

    ULONG EnableWorkingSetTracing:1;

    //
    // Unused.  Use these before digging into TraceEventType's bits.
    //

    ULONG UnusedBits:2;

    //
    // The remaining bits are used for indicating which trace event type is
    // being used.  To add a new bit flag, decrement this bit field width by
    // one and add the flag *above* this flag.  That allows us to always keep
    // the event type at the end of the bitmask.
    //

    ULONG TraceEventType:16;

} TRACER_FLAGS, *PTRACER_FLAGS;
C_ASSERT(sizeof(TRACER_FLAGS) == sizeof(ULONG));

typedef struct _TRACE_SESSION_DIRECTORY {
    LIST_ENTRY ListEntry;
    UNICODE_STRING Directory;
    PVOID Context;
} TRACE_SESSION_DIRECTORY, *PTRACE_SESSION_DIRECTORY;

typedef struct _TRACE_SESSION_DIRECTORIES {
    LIST_ENTRY ListHead;
    SRWLOCK Lock;
    volatile ULONG Count;
} TRACE_SESSION_DIRECTORIES, *PTRACE_SESSION_DIRECTORIES;

//
// The tracer configuration structure.  The main purpose of this structure
// is to expose fully-qualified path names of the tracer installation
// directory, base trace directory (where trace output is stored), and all
// known DLLs we may load.  It is a very shallow reflection of whatever the
// registry has set in HKCU\Software\Tracer.
//

typedef _Struct_size_bytes_(Size) struct _TRACER_CONFIG {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACER_CONFIG)) USHORT Size;

    //
    // Number of trace store elements per trace store.  That is, the count of
    // both the trace store and all metadata stores.
    //

    USHORT NumberOfElementsPerTraceStore;

    //
    // Size of the individual TRACE_STORE structure.
    //

    USHORT SizeOfTraceStoreStructure;

    //
    // Pad to 8 bytes.
    //

    USHORT Padding;

    //
    // The maximum value of the TRACE_STORE_ID enum.
    //

    ULONG MaximumTraceStoreId;

    //
    // The maximum value of the TRACE_STORE_INDEX enum.  As indexes are
    // contiguous, this also indicates the size of the TRACE_STORES Stores[]
    // array.
    //

    ULONG MaximumTraceStoreIndex;

    //
    // Global configuration flags.
    //

    TRACER_FLAGS Flags;

    //
    // Function pointer to the allocator structure being used by this instance.
    //

    PALLOCATOR Allocator;

    //
    // HMODULE for the TlsTracerHeap DLL, if it has been loaded.
    //

    HMODULE TlsTracerHeapModule;

    //
    // Generic LIST_ENTRY that can be used to link multiple configs together.
    // This is not used by the default global tracer configuration.
    //

    LIST_ENTRY ListEntry;

    //
    // Paths loaded by the registry.
    //

    TRACER_PATHS Paths;

    //
    // List of trace session directories created by this config instance.
    //

    TRACE_SESSION_DIRECTORIES TraceSessionDirectories;

} TRACER_CONFIG, *PTRACER_CONFIG, **PPTRACER_CONFIG;

typedef CONST TRACER_CONFIG CTRACER_CONFIG;
typedef CONST PTRACER_CONFIG PCTRACER_CONFIG;
typedef PCTRACER_CONFIG *PPCTRACER_CONFIG;

typedef
_Success_(return != 0)
_Check_return_
PTRACER_CONFIG
(INITIALIZE_TRACER_CONFIG)(
    _In_ PALLOCATOR Allocator,
    _In_opt_ PUNICODE_STRING RegistryPath
    );
typedef INITIALIZE_TRACER_CONFIG *PINITIALIZE_TRACER_CONFIG;
TRACER_CONFIG_API INITIALIZE_TRACER_CONFIG InitializeTracerConfig;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_GLOBAL_TRACER_CONFIG)(VOID);
typedef INITIALIZE_GLOBAL_TRACER_CONFIG *PINITIALIZE_GLOBAL_TRACER_CONFIG;

typedef
_Success_(return != 0)
BOOL
(GET_GLOBAL_TRACER_CONFIG)(
    _Out_ PPTRACER_CONFIG TracerConfig
    );
typedef GET_GLOBAL_TRACER_CONFIG *PGET_GLOBAL_TRACER_CONFIG;

typedef
VOID
(DESTROY_TRACER_CONFIG)(
    _In_opt_ PTRACER_CONFIG TracerConfig
    );
typedef DESTROY_TRACER_CONFIG *PDESTROY_TRACER_CONFIG;
TRACER_CONFIG_API DESTROY_TRACER_CONFIG DestroyTracerConfig;

typedef
VOID
(DESTROY_GLOBAL_TRACER_CONFIG)(VOID);
typedef DESTROY_GLOBAL_TRACER_CONFIG *PDESTROY_GLOBAL_TRACER_CONFIG;

typedef
VOID
(DEBUG_BREAK)(
    VOID
    );
typedef DEBUG_BREAK *PDEBUG_BREAK;
TRACER_CONFIG_API DEBUG_BREAK _DebugBreak;

//
// TracerHeap-related functions.  We export these so that modules don't have to
// load TracerHeap first in order to create/initialize an allocator to pass to
// InitializeTracerConfig().
//

TRACER_CONFIG_API CREATE_AND_INITIALIZE_ALLOCATOR \
    CreateAndInitializeDefaultHeapAllocator;

TRACER_CONFIG_API GET_OR_CREATE_GLOBAL_ALLOCATOR GetOrCreateGlobalAllocator;

#ifdef __cplusplus
}; // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
