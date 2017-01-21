/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEnginePrivate.h

Abstract:

    This is the private header file for the DebugEngine module.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Private function typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_DEBUG_ENGINE)(
    _In_ PRTL Rtl,
    _In_ PDEBUG_ENGINE DebugEngine
    );
typedef INITIALIZE_DEBUG_ENGINE *PINITIALIZE_DEBUG_ENGINE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(DEBUG_ENGINE_DISASSEMBLE_ADDRESS)(
    _In_ PDEBUG_ENGINE_SESSION Session,
    _In_ ULONG64 Offset,
    _In_ PSTRING Buffer
    );
typedef DEBUG_ENGINE_DISASSEMBLE_ADDRESS *PDEBUG_ENGINE_DISASSEMBLE_ADDRESS;

//
// Private function declarations.
//

#pragma component(browser, off)
INITIALIZE_DEBUG_ENGINE InitializeDebugEngine;
START_DEBUG_ENGINE_SESSION StartDebugEngineSession;
#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
