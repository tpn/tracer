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
    _Out_ PPUNICODE_STRING Directory,
    _In_ PSYSTEMTIME SystemTime
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

//
// Define an enumeration for capturing the types of paths we expose.
//

typedef enum _TRACER_PATH_TYPE {
    TracerNullPathType = -1,
    TracerDllPathType = 0,
    TracerPtxPathType,
    TracerInvalidPathType = TracerPtxPathType + 1
} TRACER_PATH_TYPE;
typedef TRACER_PATH_TYPE *PTRACER_PATH_TYPE;

FORCEINLINE
BOOL
IsValidTracerPathTypeInline(
    _In_ TRACER_PATH_TYPE Type
    )
{
    return (
        Type > TracerNullPathType &&
        Type < TracerInvalidPathType
    );
}

typedef
BOOL
(IS_VALID_TRACER_PATH_TYPE_AND_INDEX)(
    _In_ TRACER_PATH_TYPE PathType,
    _In_ USHORT Index
    );
typedef IS_VALID_TRACER_PATH_TYPE_AND_INDEX
      *PIS_VALID_TRACER_PATH_TYPE_AND_INDEX;

//
// Define DLL path enumerations and bitmaps.
//

typedef enum _Enum_is_bitflag_ _TRACER_DLL_PATH_ID {
    TracerNullDllPathId                   = 0,

    AsmDllPathId                          =        1,
    RtlDllPathId                          =  1 <<  1,
    TracerCoreDllPathId                   =  1 <<  2,
    PythonDllPathId                       =  1 <<  3,
    TracerHeapDllPathId                   =  1 <<  4,
    TraceStoreDllPathId                   =  1 <<  5,
    DebugEngineDllPathId                  =  1 <<  6,
    StringTableDllPathId                  =  1 <<  7,
    PythonTracerDllPathId                 =  1 <<  8,
    TlsTracerHeapDllPathId                =  1 <<  9,
    InjectionThunkDllPathId               =  1 << 10,
    TracedPythonSessionDllPathId          =  1 << 11,
    TraceStoreSqlite3ExtDllPathId         =  1 << 12,
    PythonTracerInjectionDllPathId        =  1 << 13,

    //
    // Make sure the right shift value matches the last value in the line above.
    //

    TracerInvalidDllPathId                = (1 << 13) + 1,

} TRACER_DLL_PATH_ID;
typedef TRACER_DLL_PATH_ID *PTRACER_DLL_PATH_ID;
C_ASSERT(sizeof(TRACER_DLL_PATH_ID) == sizeof(ULONG));

#define MAX_TRACER_DLL_PATH_ID (TrailingZeros(TracerInvalidDllPathId-1)+1)

typedef union _TRACER_DLL_PATH_TYPE {
    struct _Struct_size_bytes_(sizeof(ULONGLONG)) {
        ULONGLONG AsmDllPath:1;
        ULONGLONG RtlDllPath:1;
        ULONGLONG TracerCoreDllPath:1;
        ULONGLONG PythonDllPath:1;
        ULONGLONG TracerHeapDllPath:1;
        ULONGLONG TraceStoreDllPath:1;
        ULONGLONG DebugEngineDllPath:1;
        ULONGLONG StringTableDllPath:1;
        ULONGLONG PythonTracerDllPath:1;
        ULONGLONG TlsTracerHeapDllPath:1;
        ULONGLONG InjectionThunkDllPathId:1;
        ULONGLONG TracedPythonSessionDllPath:1;
        ULONGLONG TraceStoreSqlite3ExtDllPath:1;
        ULONGLONG PythonTracerInjectionDllPath:1;

        ULONGLONG Unused:50;
    };
    LONGLONG AsLongLong;
    ULONGLONG AsULongLong;
    TRACER_DLL_PATH_ID AsPathId;
} TRACER_DLL_PATH_TYPE;
C_ASSERT(sizeof(TRACER_DLL_PATH_TYPE) == sizeof(ULONGLONG));

//
// Define PTX path enumerations and bitmaps.
//

typedef enum _Enum_is_bitflag_ _TRACER_PTX_PATH_ID {
    TracerNullPtxPathId                   =        0,

    TraceStoreKernelsPtxPathId            =        1,

    //
    // Make sure the right shift value matches the last value in the line above.
    //

    TracerInvalidPtxPathId                =        1 + 1,

} TRACER_PTX_PATH_ID;
typedef TRACER_PTX_PATH_ID *PTRACER_PTX_PATH_ID;
C_ASSERT(sizeof(TRACER_PTX_PATH_ID) == sizeof(ULONG));

#define MAX_TRACER_PTX_PATH_ID (TrailingZeros(TracerInvalidPtxPathId-1)+1)

typedef union _TRACER_PTX_PATH_TYPE {
    struct _Struct_size_bytes_(sizeof(ULONGLONG)) {
        ULONGLONG TraceStoreKernelsPtxPathId:1;
        ULONGLONG Unused:63;
    };
    LONGLONG AsLongLong;
    ULONGLONG AsULongLong;
    TRACER_PTX_PATH_ID AsPathId;
} TRACER_PTX_PATH_TYPE;
C_ASSERT(sizeof(TRACER_PTX_PATH_TYPE) == sizeof(ULONGLONG));


typedef struct _Struct_size_bytes_(Size) _TRACER_PATHS {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACER_PATHS)) USHORT SizeOfStruct;

    //
    // Number of DLL paths.  This field can be used in conjunction with the
    // FirstDllPath field below to iterate over the paths using pointer
    // arithmetic.
    //

    _Field_range_(==, ARRAYSIZE(struct _TRACER_DLL_PATH_UNICODE_STRINGS))
    USHORT NumberOfDllPaths;

    //
    // Number of PTX paths.  This field can be used in conjunction with the
    // FirstPtxPath field below to iterate over the paths using pointer
    // arithmetic.
    //

    _Field_range_(==, ARRAYSIZE(struct _TRACER_PTX_PATH_UNICODE_STRINGS))
    USHORT NumberOfPtxPaths;

    //
    // Pad out to 8 bytes.
    //

    USHORT Padding;

    //
    // Installation directory and the base trace data directory
    // (where trace session directories are created).  These values
    // are read from the registry.
    //

    UNICODE_STRING InstallationDirectory;
    UNICODE_STRING BaseTraceDirectory;

    //
    // Optional debugger settings.
    //

    UNICODE_STRING DebuggerSettingsXmlPath;

    //
    // Fully-qualified paths to relevant DLLs.  The paths are built relative to
    // the InstallationDirectory above.  If the flag "LoadDebugLibraries" was
    // set, the DLL paths will represent debug versions of the libraries.
    // Ordering is important; make sure to match the ordering of the path type
    // enum/bitfields.
    //

    union {
        struct _TRACER_DLL_PATH_UNICODE_STRINGS {
            UNICODE_STRING AsmDllPath;
            UNICODE_STRING RtlDllPath;
            UNICODE_STRING TracerCoreDllPath;
            UNICODE_STRING PythonDllPath;
            UNICODE_STRING TracerHeapDllPath;
            UNICODE_STRING TraceStoreDllPath;
            UNICODE_STRING DebugEngineDllPath;
            UNICODE_STRING StringTableDllPath;
            UNICODE_STRING PythonTracerDllPath;
            UNICODE_STRING TlsTracerHeapDllPath;
            UNICODE_STRING InjectionThunkDllPath;
            UNICODE_STRING TracedPythonSessionDllPath;
            UNICODE_STRING TraceStoreSqlite3ExtDllPath;
            UNICODE_STRING PythonTracerInjectionDllPath;
        };

        //
        // Provide a convenient way to access the first DLL path name member
        // without having to know its name.
        //

        UNICODE_STRING FirstDllPath;
    };

    //
    // Fully-qualified paths to PTX files.  The paths are built relative to
    // InstallationDirectory above, using the same binary type semantics as the
    // DLL paths above for selecting the intermediate part of the path name
    // (e.g. x64\Debug vs x64\Release).
    //

    union {
        struct _TRACER_PTX_PATH_UNICODE_STRINGS {
            UNICODE_STRING TraceStoreKernelsPtxPath;
        };

        //
        // Provide a convenient way to access the first DLL path name member
        // without having to know its name.
        //

        UNICODE_STRING FirstPtxPath;
    };

    //
    // A bitmap indicating DLLs that support the tracer injection interface.
    // The bitmap buffer will be wired up directly to the subsequent
    // TRACER_DLL_PATH_TYPE structure.
    //

    RTL_BITMAP TracerInjectionDllsBitmap;
    TRACER_DLL_PATH_TYPE TracerInjectionDlls;

} TRACER_PATHS;
typedef TRACER_PATHS *PTRACER_PATHS;

#define CopyTracerDllPathType(Dest, Source) \
    Dest.AsULongLong = Source.AsULongLong;

FORCEINLINE
PUNICODE_STRING
TracerDllPathIdToUnicodeString(
    _In_ PTRACER_PATHS Paths,
    _In_ TRACER_DLL_PATH_ID PathId
    )
{
    if (PathId <= TracerNullDllPathId || PathId >= TracerInvalidDllPathId) {
        return NULL;
    }

    return &Paths->FirstDllPath + (PathId - 1);
}

FORCEINLINE
PUNICODE_STRING
TracerPtxPathIdToUnicodeString(
    _In_ PTRACER_PATHS Paths,
    _In_ TRACER_PTX_PATH_ID PathId
    )
{
    if (PathId <= TracerNullPtxPathId || PathId >= TracerInvalidPtxPathId) {
        return NULL;
    }

    return &Paths->FirstPtxPath + (PathId - 1);
}


FORCEINLINE
BOOL
GetNextTracerInjectionDll(
    _In_ PRTL Rtl,
    _In_ PTRACER_PATHS Paths,
    _Inout_ PULONG Index,
    _Out_ PPUNICODE_STRING DllPath
    )
{
    ULONG BitIndex;
    ULONG LastIndex;
    PUNICODE_STRING Path;
    PRTL_BITMAP Bitmap = &Paths->TracerInjectionDllsBitmap;

    LastIndex = *Index;
    BitIndex = Rtl->RtlFindSetBits(Bitmap, 1, LastIndex);
    if (BitIndex == BITS_NOT_FOUND || BitIndex < LastIndex) {
        *DllPath = NULL;
        return FALSE;
    }

    Path = &Paths->FirstDllPath + BitIndex;

    *Index = BitIndex + 1;
    *DllPath = Path;
    return TRUE;
}

//
// Tracer configuration flags.  Map to REG_DWORD entries of the same name.
//

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _TRACER_FLAGS {

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
    // When set, indicates that the PGInstrument versions of binaries should
    // be loaded instead of the release or debug versions.  Defaults to FALSE.
    //

    ULONG LoadPGInstrumentedLibraries:1;

    //
    // When set, indicates that the PGOptimize versions of binaries should
    // be loaded instead of the release, debug or PGInstrument versions.
    // Defaults to FALSE.
    //

    ULONG LoadPGOptimizedLibraries:1;

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
    // When set, enables tracing of the process's working set.
    //

    ULONG EnableWorkingSetTracing:1;

    //
    // When set, enables tracing of performance information related to the
    // process.  This covers memory, I/O and handle counts.
    //

    ULONG EnablePerformanceTracing:1;

    //
    // When set, enables tracing of process DLL load/unload activity.
    //

    ULONG EnableLoaderTracing:1;

    //
    // When set, enables tracing of module (exe/DLL) symbol information.
    //

    ULONG EnableSymbolTracing:1;

    //
    // When set, enables tracing of module (exe/DLL) type information via the
    // DebugEngine component.
    //

    ULONG EnableTypeInfoTracing:1;

    //
    // When set, enables tracing of function assembly.
    //

    ULONG EnableAssemblyTracing:1;

    //
    // If symbol, type info or assembly tracing is enabled, the following bits
    // control whether or not modules residing in Windows directories are
    // ignored.
    //

    ULONG IgnoreModulesInWindowsSystemDirectory:1;
    ULONG IgnoreModulesInWindowsSxSDirectory:1;

    //
    // When set, disables asynchronous initialization of trace contexts.  This
    // applies to the InitializeTraceContext() call.  When enabled, asynchronous
    // initialization will submit the relevant preparation required to bind the
    // trace stores to the trace context in a threadpool, and then return
    // immediately.  If a caller attempts to allocate records against a store
    // prior to that store being available, they will trigger the suspended
    // allocation code path and will simply wait on the resume allocations
    // event.
    //

    ULONG DisableAsynchronousInitialization:1;

    //
    // When set, the injection thunk will be configured to __debugbreak() as
    // early as possible in the thunk routine.
    //

    ULONG InjectionThunkDebugBreakOnEntry:1;

    //
    // When set, the trace store sqlite3 module will __debugbreak() as early as
    // possible (during module load).
    //

    ULONG TraceStoreSqlite3ModuleDebugBreakOnEntry:1;

    //
    // When set, allocation timestamps are not recorded for trace stores.
    //

    ULONG DisableAllocationTimestamps:1;

    //
    // Unused bits.
    //

    ULONG Unused:10;
} TRACER_FLAGS;
C_ASSERT(sizeof(TRACER_FLAGS) == sizeof(ULONG));
typedef TRACER_FLAGS *PTRACER_FLAGS;

//
// This enum can be used as an array index for the IntermediatePaths[] array
// defined in TraceConfigConstants.h.
//

typedef enum _TRACER_BINARY_TYPE_INDEX {
    TracerReleaseBinaries = 0,
    TracerDebugBinaries,
    TracerPGInstrumentedBinaries,
    TracerPGOptimizedBinaries,
} TRACER_BINARY_TYPE_INDEX, *PTRACER_BINARY_TYPE_INDEX;

FORCEINLINE
TRACER_BINARY_TYPE_INDEX
ExtractTracerBinaryTypeIndexFromFlags(
    _In_ TRACER_FLAGS Flags
    )
{
    if (Flags.LoadPGOptimizedLibraries) {
        return TracerPGOptimizedBinaries;
    } else if (Flags.LoadPGInstrumentedLibraries) {
        return TracerPGInstrumentedBinaries;
    } else if (Flags.LoadDebugLibraries) {
        return TracerDebugBinaries;
    } else {
        return TracerReleaseBinaries;
    }
}

//
// Tracer runtime parameters.  Map to REG_DWORD values of the same name.
//

typedef struct _TRACER_RUNTIME_PARAMETERS {

    //
    // The timer interval used by the GetWorkingSetChanges() routine, which is
    // responsible for periodically calling GetWsChangesEx() and flushing the
    // kernel working set changes buffer to our corresponding working set trace
    // stores (assuming working set tracing has been enabled).
    //
    // This value is passed as the msPeriod parameter to SetThreadpoolTimer().
    //

    ULONG GetWorkingSetChangesIntervalInMilliseconds;

    //
    // This value is passed as the msWindowLength to SetThreadpoolTimer(), and
    // indicates to the kernel the amount of slack we're willing to tolerate
    // with regards to our GetWorkingSetChanges() timer callback being fired.
    // This allows the kernel to potentially batch timer callbacks, improving
    // efficiency.
    //

    ULONG GetWorkingSetChangesWindowLengthInMilliseconds;

    //
    // This value controls the number of elements we size our temporary working
    // set buffers to accommodate. It needs to be large enough to ensure records
    // aren't dropped for a given timer interval.  (Interval, window length and
    // number of elements are all related -- i.e. making interval or window
    // length longer will require a greater number of elements and vice versa.)
    //
    // N.B. The trace store working set machinery will start out with buffers
    //      sized based on this value.  If it receives an indication that the
    //      buffer was too small and that records were dropped, it will double
    //      the number of elements at runtime and then realloc appropriately
    //      sized buffers based on the new number of elements.
    //

    ULONG WsWatchInfoExInitialBufferNumberOfElements;

    //
    // The interval used for the threadpool timer that captures performance
    // information, if applicable.
    //

    ULONG CapturePerformanceMetricsIntervalInMilliseconds;

    //
    // The window length for the timer above.
    //

    ULONG CapturePerformanceMetricsWindowLengthInMilliseconds;

    //
    // If a trace store indicates concurrent allocations via traits, a critical
    // section will be used to serialize access to allocations.  The following
    // field determines the spin count used to initialize the critical section;
    // after this count, the thread stops spinning and goes into an alertable
    // wait state.
    //

    ULONG ConcurrentAllocationsCriticalSectionSpinCount;

    //
    // The number of frames per second to aim for when doing interval-based
    // slicing and aggregation of trace store data.
    //

    ULONG IntervalFramesPerSecond;

    //
    // Pad out to 32 bytes.
    //

    ULONG Unused;

} TRACER_RUNTIME_PARAMETERS;
typedef TRACER_RUNTIME_PARAMETERS *PTRACER_RUNTIME_PARAMETERS;
C_ASSERT(sizeof(TRACER_RUNTIME_PARAMETERS) == 32);

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

typedef struct _Struct_size_bytes_(Size) _TRACER_CONFIG {

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
    // Runtime parameters.
    //

    TRACER_RUNTIME_PARAMETERS RuntimeParameters;

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

    //
    // The CUDA device ordinal to use for any CUDA-related work.
    //

    ULONG CuDeviceOrdinal;
    ULONG Padding2;

} TRACER_CONFIG, *PTRACER_CONFIG, **PPTRACER_CONFIG;
typedef CONST TRACER_CONFIG CTRACER_CONFIG;
typedef CONST PTRACER_CONFIG PCTRACER_CONFIG;
typedef PCTRACER_CONFIG *PPCTRACER_CONFIG;

//
// Modules that support the tracer injection protocol are referred to generally
// as "tracer injection modules".  The set of modules are represented by the
// structure TRACER_INJECTION_MODULES.
//

typedef union _TRACER_INJECTION_MODULES_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACER_INJECTION_MODULES_FLAGS;
typedef TRACER_INJECTION_MODULES_FLAGS *PTRACER_INJECTION_MODULES_FLAGS;

//
// Tracer injection modules export a function named InitializeTracerInjection
// which conforms to the PINITIALIZE_TRACER_INJECTION signature, defined below.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK INITIALIZE_TRACER_INJECTION)(
    _In_ struct _DEBUG_ENGINE_SESSION *ParentDebugEngineSession
    );
typedef INITIALIZE_TRACER_INJECTION *PINITIALIZE_TRACER_INJECTION;
typedef INITIALIZE_TRACER_INJECTION **PPINITIALIZE_TRACER_INJECTION;


typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACER_INJECTION_MODULES {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACER_PATHS)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    TRACER_INJECTION_MODULES_FLAGS Flags;

    //
    // Number of DLLs participating in this structure.  This governs the size
    // of the Paths and Modules arrays.
    //

    ULONG NumberOfModules;

    //
    // Total allocation size of this structure and all supporting arrays.
    //

    ULONG TotalAllocSizeInBytes;

    //
    // Base address of an array of pointers to UNICODE_STRING structures that
    // reflect the fully-qualified path names of each module.
    //

    PPUNICODE_STRING Paths;

    //
    // Base address of an array of module handles for each path.
    //

    PHMODULE Modules;

    //
    // Base address of an array of function pointers to each respective module's
    // exported InitializeTracerInjection function.
    //

    PPINITIALIZE_TRACER_INJECTION Initializers;

} TRACER_INJECTION_MODULES;
typedef TRACER_INJECTION_MODULES *PTRACER_INJECTION_MODULES;

typedef
_Check_return_
_Success_(return != 0)
PTRACER_CONFIG
(INITIALIZE_TRACER_CONFIG)(
    _In_ PALLOCATOR Allocator,
    _In_opt_ PUNICODE_STRING RegistryPath
    );
typedef INITIALIZE_TRACER_CONFIG *PINITIALIZE_TRACER_CONFIG;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL)(
    _In_ PALLOCATOR Allocator,
    _In_opt_ PUNICODE_STRING RegistryPath,
    _Outptr_result_nullonfailure_ PPTRACER_CONFIG TracerConfig,
    _Outptr_result_nullonfailure_ PPRTL Rtl
    );
typedef CREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL
      *PCREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_AND_INITIALIZE_TRACER_INJECTION_MODULES)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PTRACER_CONFIG TracerConfig,
    _Outptr_result_nullonfailure_
        PTRACER_INJECTION_MODULES *InjectionModulesPointer
    );
typedef CREATE_AND_INITIALIZE_TRACER_INJECTION_MODULES
      *PCREATE_AND_INITIALIZE_TRACER_INJECTION_MODULES;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(MAKE_TRACER_PATH)(
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PCUNICODE_STRING Filename,
    _In_opt_ PTRACER_BINARY_TYPE_INDEX BinaryTypeIndexPointer,
    _Inout_ PPUNICODE_STRING PathPointer
    );
typedef MAKE_TRACER_PATH *PMAKE_TRACER_PATH;

typedef
_Check_return_
_Success_(return != 0)
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

//
// TracerHeap-related functions.  We export these so that modules don't have to
// load TracerHeap first in order to create/initialize an allocator to pass to
// InitializeTracerConfig().
//


#pragma component(browser, off)
TRACER_CONFIG_API CREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL
                  CreateAndInitializeTracerConfigAndRtl;

TRACER_CONFIG_API CREATE_AND_INITIALIZE_ALLOCATOR
                  CreateAndInitializeDefaultHeapAllocator;

TRACER_CONFIG_API CREATE_AND_INITIALIZE_TRACER_CONFIG_AND_RTL
                  CreateAndInitializeTracerConfigAndRtl;

TRACER_CONFIG_API CREATE_AND_INITIALIZE_TRACER_INJECTION_MODULES
                  CreateAndInitializeTracerInjectionModules;

TRACER_CONFIG_API DESTROY_TRACER_CONFIG DestroyTracerConfig;
TRACER_CONFIG_API GET_OR_CREATE_GLOBAL_ALLOCATOR GetOrCreateGlobalAllocator;
TRACER_CONFIG_API INITIALIZE_TRACER_CONFIG InitializeTracerConfig;
TRACER_CONFIG_API DEBUG_BREAK _DebugBreak;
TRACER_CONFIG_API MAKE_TRACER_PATH MakeTracerPath;
TRACER_CONFIG_API IS_VALID_TRACER_PATH_TYPE_AND_INDEX
                  IsValidTracerPathTypeAndIndex;
#pragma component(browser, on)

#ifdef __cplusplus
}; // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
