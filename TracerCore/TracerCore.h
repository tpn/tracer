/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TracerCore.h

Abstract:

    This is the main header file for the TracerCore component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TRACER_CORE_CALL_CONV __stdcall

#ifdef _TRACER_CORE_INTERNAL_BUILD

//
// This is an internal build of the TracerCore component.
//

#define TRACER_CORE_API __declspec(dllexport)
#define TRACER_CORE_DATA extern __declspec(dllexport)

#include "stdafx.h"

#elif _TRACER_CORE_NO_API_EXPORT_IMPORT

//
// We're being included by someone who doesn't want dllexport or dllimport.
// This is useful for creating new .exe-based projects for things like unit
// testing or performance testing/profiling.
//

#define TRACER_CORE_API
#define TRACER_CORE_DATA extern

#else

//
// We're being included by an external component.
//

#define TRACER_CORE_API __declspec(dllimport)
#define TRACER_CORE_DATA extern __declspec(dllimport)

#include "../Rtl/Rtl.h"

#endif

//
// Define the TRACER_CORE structure.
//

typedef union _TRACER_CORE_FLAGS {
    struct {
        ULONG Unused:1;
    };
    ULONG AsLong;
} TRACER_CORE_FLAGS;

typedef TRACER_CORE_FLAGS *PTRACER_CORE_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACER_CORE {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACER_CORE)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    TRACER_CORE_FLAGS Flags;

    PRTL Rtl;
    PALLOCATOR Allocator;
    struct _TRACER_CONFIG *TracerConfig;
    struct _DEBUG_ENGINE_SESSION *DebugEngineSession;

} TRACER_CORE, *PTRACER_CORE, **PPTRACER_CORE;

typedef union _TRACER_CORE_INIT_FLAGS {
    struct {
        ULONG Unused:1;
    };
    ULONG AsLong;
} TRACER_CORE_INIT_FLAGS;
typedef TRACER_CORE_INIT_FLAGS *PTRACER_CORE_INIT_FLAGS;
C_ASSERT(sizeof(TRACER_CORE_INIT_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOL
(TRACER_CORE_CALL_CONV INITIALIZE_TRACER_CORE)(
    _In_opt_ PTRACER_CORE TracerCore,
    _Inout_ PULONG SizeInBytes
    );
typedef INITIALIZE_TRACER_CORE *PINITIALIZE_TRACER_CORE;

typedef
_Success_(return != 0)
ULONG
(CDECL TRACER_EXE_MAIN)(
    VOID
    );
typedef TRACER_EXE_MAIN *PTRACER_EXE_MAIN;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK INITIALIZE_TRACER_INJECTION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ struct _TRACER_CONFIG *TracerConfig,
    _In_ struct _DEBUG_ENGINE_SESSION *DebugEngineSession
    );
typedef INITIALIZE_TRACER_INJECTION *PINITIALIZE_TRACER_INJECTION;
typedef INITIALIZE_TRACER_INJECTION **PPINITIALIZE_TRACER_INJECTION;

//
// Public function declarations..
//

#pragma component(browser, off)
TRACER_CORE_API INITIALIZE_TRACER_CORE InitializeTracerCore;
TRACER_CORE_API TRACER_EXE_MAIN TracerExeMain;
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
