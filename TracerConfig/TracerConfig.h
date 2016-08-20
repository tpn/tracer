#pragma once

//#include "stdafx.h"
//#include <ntddk.h>
//#include <ntifs.h>
#include "../Rtl/Rtl.h"

//
// Function typedefs.
//

typedef
_Check_return_
BOOLEAN
(CREATE_TRACE_SESSION_DIRECTORY)(
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _Out_ PUNICODE_STRING Directory   
    );

typedef CREATE_TRACE_SESSION_DIRECTORY *PCREATE_TRACE_SESSION_DIRECTORY;

typedef
_Check_return_
BOOLEAN
(CREATE_GLOBAL_TRACE_SESSION_DIRECTORY)(
    _Out_ PUNICODE_STRING Directory   
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
    // Fully-qualified UNICODE path names.  If the registry key
    // "LoadDebugLibraries" was set, the DLL paths will represent
    // debug versions of the libraries.
    //
    
    UNICODE_STRING InstallationDirectory;
    UNICODE_STRING BaseTraceDirectory;
    UNICODE_STRING TracerDllPath;
    UNICODE_STRING RtlDllPath;
    UNICODE_STRING PythonDllPath;
    UNICODE_STRING PythonTracerDllPath;

} TRACER_PATHS, *PTRACER_PATHS;

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
    // The minimum Directory->Length value that should be passed to the
    // CreateTraceSessionDirectory(_In_ PUNICODE_STRING Directory) function.
    // (Note that this is a UNICODE_STRING, so Length represents number of
    //  WCHAR characters; the number of bytes will be double this value.)
    // This is provided as a convenience so that users can allocate correctly-
    // sized UNICODE_STRING buffers before calling the creation function.
    // 

    USHORT TraceSessionDirectoryLength;

    //
    // Bitmask of supported runtimes.  (Currently only Python and
    // C are supported.)
    //

    _Struct_size_bytes_(sizeof(ULONG)) struct {
        ULONG Python:1;
        ULONG C:1;
        ULONG CPlusPlus:1;
        ULONG CSharp:1;
        ULONG Ruby:1;
    } SupportedRuntimes;

    //
    // Global configuration flags.
    //

    _Struct_size_bytes_(sizeof(ULONG)) struct {

        //
        // When set, entry routines such as InitializeTracerConfig() and
        // DestroyTracerConfig() should call __debugbreak() as soon as
        // possible during startup.
        //

        ULONG DebugBreakOnEntry:1;

        //
        // N.B.: the remaining fields typically map 1-to-1 with corresponding
        // DWORD registry entries by the same name.
        // 

        //
        // When set, indicates that debug versions of the DLLs should be
        // loaded instead of the release versions.  Defaults to FALSE.
        //

        ULONG LoadDebugLibraries:1;

        //
        // When set, indicates that trace session directories will be created
        // with compression enabled.  Defaults to TRUE.
        //

        ULONG EnableTraceSessionDirectoryCompression:1;

        //
        // When set, indicates that the trace store mechanism should pre-fault
        // pages ahead of time in a separate thread pool.  Defaults to TRUE.
        //

        ULONG PrefaultPages:1;

    } Flags;

    //
    // Function pointer to the allocator structure being used by this instance.
    //

    PALLOCATOR Allocator;

    //
    // Generic LIST_ENTRY that can be used to link multiple configs together.
    // This is not used by the default global tracer configuration.
    //

    LIST_ENTRY ListEntry;

    //
    // Paths loaded by the registry.
    //

    TRACER_PATHS Paths;

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
    _In_ PUNICODE_STRING RegistryPath
    );
typedef INITIALIZE_TRACER_CONFIG *PINITIALIZE_TRACER_CONFIG;

typedef
_Success_(return != 0)
_Check_return_
BOOLEAN
(INITIALIZE_GLOBAL_TRACER_CONFIG)(VOID);
typedef INITIALIZE_GLOBAL_TRACER_CONFIG *PINITIALIZE_GLOBAL_TRACER_CONFIG;

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


#define TRACER_CONFIG_POOL_TAG ((ULONG)'pCrT')
#define TRACER_CONFIG_POOL_PRIORITY LowPoolPriority
