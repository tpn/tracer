/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngine.h

Abstract:

    This is the header file for the DebugEngine module, which is a helper
    module that exposes a subset of functionality implemented by the Windows
    Debug Engine component (DbgEng.dll).

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma component(browser, off)
#include <DbgEng.h>
#pragma component(browser, on)

//
// Declare IIDs of the DbgEng COM classes we use.
//

typedef const GUID *PCGUID;

RTL_DATA CONST GUID IID_IDEBUG_CLIENT;
RTL_DATA CONST GUID IID_IDEBUG_CONTROL;

//
// Define typedefs for the COM interfaces we want to use.
//

typedef struct IDebugClient7 IDEBUGCLIENT7;
typedef IDEBUGCLIENT7 *PIDEBUGCLIENT7;

typedef struct IDebugClient7Vtbl IDEBUGCLIENT7VTBL;
typedef IDEBUGCLIENT7VTBL *PIDEBUGCLIENT7VTBL;

typedef struct IDebugControl7 IDEBUGCONTROL7;
typedef IDEBUGCONTROL7 *PIDEBUGCONTROL7;

typedef struct IDebugControl7Vtbl IDEBUGCONTROL7VTBL;
typedef IDEBUGCONTROL7VTBL *PIDEBUGCONTROL7VTBL;

typedef IDEBUGCLIENT7     IDEBUGCLIENT;
typedef IDEBUGCLIENT   *PIDEBUGCLIENT;
typedef IDEBUGCLIENT **PPIDEBUGCLIENT;

typedef IDEBUGCLIENT7VTBL   DEBUGCLIENT;
typedef DEBUGCLIENT      *PDEBUGCLIENT;
typedef DEBUGCLIENT    **PPDEBUGCLIENT;

typedef IDEBUGCONTROL7     IDEBUGCONTROL;
typedef IDEBUGCONTROL   *PIDEBUGCONTROL;
typedef IDEBUGCONTROL **PPIDEBUGCONTROL;

typedef IDEBUGCONTROL7VTBL   DEBUGCONTROL;
typedef DEBUGCONTROL      *PDEBUGCONTROL;
typedef DEBUGCONTROL    **PPDEBUGCONTROL;

//
// Our DebugEngine-related structures and function pointer typedefs.
//

typedef union _DEBUG_ENGINE_FLAGS {
    ULONG AsLong;
    struct {
        ULONG Unused:1;
    };
} DEBUG_ENGINE_FLAGS; *PDEBUG_ENGINE_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _DEBUG_ENGINE {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _DEBUG_ENGINE)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    DEBUG_ENGINE_FLAGS Flags;

    //
    // Internal fields.
    //

    PRTL Rtl;
    PALLOCATOR Allocator;

    //
    // IIDs, interfaces and vtables.
    //

    PCGUID IID_Client;
    PIDEBUGCLIENT IClient;
    PDEBUGCLIENT Client;

    PCGUID IID_Control;
    PIDEBUGCONTROL IControl;
    PDEBUGCONTROL Control;

} DEBUG_ENGINE, *PDEBUG_ENGINE, **PPDEBUG_ENGINE;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_DEBUG_ENGINE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PPDEBUG_ENGINE DebugEnginePointer
    );
typedef CREATE_DEBUG_ENGINE *PCREATE_DEBUG_ENGINE;
RTL_API CREATE_DEBUG_ENGINE CreateDebugEngine;


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
