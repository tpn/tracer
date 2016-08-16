#pragma once

//#include "stdafx.h"
#include <ntddk.h>


//
// Function typedefs.
//

typedef
_Check_return_
BOOLEAN
(CREATE_TRACE_SESSION_DIRECTORY)(
    _In_ PUNICODE_STRING Directory   
    );

typedef CREATE_TRACE_SESSION_DIRECTORY *PCREATE_TRACE_SESSION_DIRECTORY;

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
        // When set, indicates that this structure is valid; a heap was
        // created and all strings were successfully primed.
        //

        ULONG IsValid:1;

        //
        // When set, indicates we're a debug build.  This should be used
        // to determine if the Debug versions of the DLLs should be loaded
        // instead of the Release versions.
        //

        ULONG IsDebug:1;

        //
        // N.B.: the remaining fields typically map 1-to-1 with corresponding
        // DWORD registry entries by the same name.
        // 

        //
        // When set, indicates that trace session directories will be created
        // with compression enabled.
        //

        ULONG EnableTraceSessionDirectoryCompression:1;

        //
        // When set, indicates that the trace store mechanism should pre-fault
        // pages ahead of time in a separate thread pool.
        //

        ULONG PrefaultPages:1;

    } Flags;

    //
    // Function pointer to the trace session directory initialization function.
    //

    PCREATE_TRACE_SESSION_DIRECTORY CreateTraceSessionDirectory;

    //
    // Fully-qualified paths.
    //

    UNICODE_STRING InstallationDir;
    UNICODE_STRING BaseTraceDirectory;
    UNICODE_STRING TracerDllPath;
    UNICODE_STRING TracerDllDebugPath;
    UNICODE_STRING RtlDllPath;
    UNICODE_STRING RtlDllDebugPath;
    UNICODE_STRING PythonDllPath;
    UNICODE_STRING PythonDllDebugPath;
    UNICODE_STRING PythonTracerDllPath;
    UNICODE_STRING PythonTracerDllDebugPath;

} TRACER_CONFIG, *PTRACER_CONFIG, **PPTRACER_CONFIG;

typedef CONST TRACER_CONFIG CTRACER_CONFIG;
typedef CONST PTRACER_CONFIG PCTRACER_CONFIG;
typedef PCTRACER_CONFIG *PPCTRACER_CONFIG;

typedef
_Check_return_
BOOLEAN
(INITIALIZE_TRACER_CONFIG)(
    _Out_ PPCTRACER_CONFIG TracerConfig
    );

#define TRACER_CONFIG_POOL_TAG ((ULONG)'pCrT')
#define TRACER_CONFIG_POOL_PRIORITY LowPoolPriority

typedef INITIALIZE_TRACER_CONFIG *PINITIALIZE_TRACER_CONFIG;