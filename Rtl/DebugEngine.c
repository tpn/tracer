/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngine.c

Abstract:

    This module implements functionality related to the Windows Debugger
    Engine COM facilities exposed by the DbgEng.dll library.

--*/

#include "stdafx.h"

//
// Locally define the IIDs of the interfaces we want in order to avoid the
// EXTERN_C linkage we'll pick up if we use the values directly from DbgEng.h.
//

#define DEFINE_GUID_EX(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID DECLSPEC_SELECTANY name                                  \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

//
// IID_IDebugClient7: 13586be3-542e-481e-b1f2-8497ba74f9a9
//

DEFINE_GUID_EX(IID_IDEBUG_CLIENT, 0x27fe5639, 0x8407, 0x4f47,
               0x83, 0x64, 0xee, 0x11, 0x8f, 0xb0, 0x8a, 0xc8);


//
// IID_IDebugControl7: b86fb3b1-80d4-475b-aea3-cf06539cf63a
//

DEFINE_GUID_EX(IID_IDEBUG_CONTROL, 0xb86fb3b1, 0x80d4, 0x475b,
               0xae, 0xa3, 0xcf, 0x06, 0x53, 0x9c, 0xf6, 0x3a);

//
// Functions.
//

BOOL
CreateDebugEngine(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PPDEBUG_ENGINE EnginePointer
    )
/*++

Routine Description:

    This routine creates a debug client object.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    EnginePointer - Supplies an address to a variable that will receive the
        address of the newly created debug client.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    HRESULT Result;
    PIDEBUGCLIENT IClient = NULL;
    PIDEBUGCONTROL IControl = NULL;
    PDEBUGCLIENT Client = NULL;
    PDEBUGCONTROL Control = NULL;
    PDEBUG_ENGINE Engine = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(EnginePointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *EnginePointer = NULL;

    //
    // Initialize COM.
    //

    Result = Rtl->CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(Result)) {
        OutputDebugStringA("CoInitializeEx() failed.\n");
        return FALSE;
    }

    //
    // Create the IDebugClient interface.
    //

    Result = Rtl->DebugCreate(&IID_IDEBUG_CLIENT, &IClient);
    if (FAILED(Result)) {
        OutputDebugStringA("DebugCreate(Client1) failed.\n");
        goto Error;
    }

    Client = (PDEBUGCLIENT)IClient->lpVtbl;

    Result = Rtl->DebugCreate(&IID_IDEBUG_CONTROL, &IControl);
    if (FAILED(Result)) {
        OutputDebugStringA("DebugCreate(Control7) failed.\n");
        goto Error;
    }

    Control = (PDEBUGCONTROL)IControl->lpVtbl;

    Engine = (PDEBUG_ENGINE)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(*Engine)
        )
    );

    if (!Engine) {
        goto Error;
    }

    Engine->SizeOfStruct = sizeof(*Engine);
    Engine->Rtl = Rtl;
    Engine->Allocator = Allocator;

    Engine->IID_Client = &IID_IDEBUG_CLIENT;
    Engine->IClient = IClient;
    Engine->Client = Client;

    Engine->IID_Control = &IID_IDEBUG_CONTROL;
    Engine->IControl = IControl;
    Engine->Control = Control;

    //
    // Update the caller's pointer.
    //

    *EnginePointer = Engine;

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    if (Client) {
        Client->Release(IClient);
        Client = NULL;
    }

    if (Control) {
        Control->Release(IControl);
        Control = NULL;
    }

    if (Engine) {
        Allocator->FreePointer(Allocator->Context, &Engine);
    }

End:

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
