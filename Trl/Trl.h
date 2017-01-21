/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Trl.h

Abstract:

    This is the main header file for the Trl (Tracer Runtime Library) component.
    This component has been created such that non-NT specific parts of the Trl
    library can be moved here.

    The Trl component will provide the OS-dependent platform facilities; the
    Trl component implements OS-independent facilities, leveraging Trl where
    necessary.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define TRL_CALL_CONV __stdcall

#ifdef _TRL_INTERNAL_BUILD

//
// This is an internal build of the Trl component.
//

#define TRL_API __declspec(dllexport)
#define TRL_DATA extern __declspec(dllexport)

#include "stdafx.h"

#elif _TRL_NO_API_EXPORT_IMPORT

//
// We're being included by someone who doesn't want dllexport or dllimport.
// This is useful for creating new .exe-based projects for things like unit
// testing or performance testing/profiling.
//

#define TRL_API
#define TRL_DATA extern

#else

//
// We're being included by an external component.
//

#define TRL_API __declspec(dllimport)
#define TRL_DATA extern __declspec(dllimport)

#include "../Rtl/Rtl.h"

#endif

//
// Define the TRL structure.
//

typedef union _TRL_FLAGS {
    ULONG AsLong;
    struct {
        ULONG Unused:1;
    };
} TRL_FLAGS; *PTRL_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRL {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRL)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    TRL_FLAGS Flags;

    PRTL Rtl;
    PALLOCATOR Allocator;
    struct _TRACER_CONFIG *TracerConfig;

} TRL, *PTRL, **PPTRL;

typedef union _TRL_INIT_FLAGS {
    ULONG AsLong;
    struct {
        ULONG Unused:1;
    };
} TRL_INIT_FLAGS; *PTRL_INIT_FLAGS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(TRL_CALL_CONV INITIALIZE_TRL)(
    _In_opt_ PTRL Trl,
    _Inout_ PULONG SizeInBytes
    );
typedef INITIALIZE_TRL *PINITIALIZE_TRL;

//
// Public function declarations..
//

#pragma component(browser, off)
TRL_API INITIALIZE_TRL InitializeTrl;
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
