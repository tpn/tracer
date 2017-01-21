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
CreateIDebugControl(
    PDEBUG_ENGINE_SESSION Session
    )
/*++

Routine Description:

    Creates an IDebugControl object.

Arguments:

    Session - Supplies a pointer to a DEBUG_ENGINE_SESSION structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    PDEBUG_ENGINE Engine;
    PDEBUGCLIENT Client;
    PIDEBUGCLIENT IClient;
    PDEBUGCONTROL Control;
    PIDEBUGCONTROL IControl;

    //
    // Create the IDebugControl interface.
    //

    Engine = &Session->Engine;
    Client = Engine->Client;
    IClient = Engine->IClient;

    CHECKED_HRESULT_MSG(
        Client->QueryInterface(
            IClient,
            (REFIID)&IID_IDEBUG_CONTROL,
            &IControl
        ),
        "Client->QueryInterface(&IID_IDEBUG_CONTROL)"
    );

    Control = (PDEBUGCONTROL)IControl->lpVtbl;

    Engine->IID_Control = &IID_IDEBUG_CONTROL;
    Engine->IControl = IControl;
    Engine->Control = Control;

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

End:
    return Success;
}

BOOL
InitializeDebugEngine(
    PRTL Rtl,
    PDEBUG_ENGINE Engine
    )
/*++

Routine Description:

    This routine initializes a DEBUG_ENGINE structure.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Engine - Supplies a pointer to a DEBUG_ENGINE structure to initialize.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    PDEBUGCLIENT Client;
    PIDEBUGCLIENT IClient;

    //
    // Initialize COM.
    //

    CHECKED_HRESULT_MSG(Rtl->CoInitializeEx(NULL, COINIT_APARTMENTTHREADED),
                        "Rtl->CoInitializeEx()");

    //
    // Create the IDebugClient interface.
    //

    CHECKED_HRESULT_MSG(Rtl->DebugCreate(&IID_IDEBUG_CLIENT, &IClient),
                        "Rtl->DebugCreate(IID_IDEBUG_CLIENT)");

    Client = (PDEBUGCLIENT)IClient->lpVtbl;

    Engine->SizeOfStruct = sizeof(*Engine);

    Engine->IID_Client = &IID_IDEBUG_CLIENT;
    Engine->IClient = IClient;
    Engine->Client = Client;

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;


End:
    return Success;
}

_Use_decl_annotations_
BOOL
CreateAndInitializeDebugEngineSession(
    PRTL Rtl,
    PALLOCATOR Allocator,
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    PPDEBUG_ENGINE_SESSION SessionPointer
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
    HMODULE Shell32Module = NULL;
    PDEBUGCLIENT Client;
    PDEBUGCONTROL Control;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session = NULL;
    PCOMMAND_LINE_TO_ARGVW CommandLineToArgvW;

    *SessionPointer = NULL;

    CHECKED_MSG(InitFlags.InitializeFromCommandLine,
                "CreateAndInitializeDebugEngineSession(): InitFlags "
                "missing InitializeFromCommandLine bit set");

    CHECKED_MSG(Rtl->LoadDbgEng(Rtl), "Rtl!LoadDbgEng()");
    ALLOCATE_TYPE(Session, DEBUG_ENGINE_SESSION, Allocator);

    CHECKED_MSG(InitializeDebugEngine(Rtl, &Session->Engine),
                "InitializeDebugEngine()");

    LOAD_LIBRARY_A(Shell32Module, Shell32);

    RESOLVE_FUNCTION(CommandLineToArgvW,
                     Shell32Module,
                     PCOMMAND_LINE_TO_ARGVW,
                     CommandLineToArgvW);

    CHECKED_MSG(Session->CommandLineA = GetCommandLineA(), "GetCommandLineA()");
    CHECKED_MSG(Session->CommandLineW = GetCommandLineW(), "GetCommandLineW()");

    Session->ArgvW = CommandLineToArgvW(Session->CommandLineW,
                                        &Session->NumberOfArguments);

    CHECKED_MSG(Session->ArgvW, "Shell32!CommandLineToArgvW()");

    CHECKED_MSG(
        Rtl->ArgvWToArgvA(
            Session->ArgvW,
            Session->NumberOfArguments,
            &Session->ArgvA,
            NULL,
            Allocator
        ),
        "Rtl!ArgvWToArgA"
    );

    CHECKED_MSG(Session->NumberOfArguments == 2,
                "Invalid usage.  Usage: TracerDebugEngine <pid>");

    //
    // Extract the process ID to attach to.
    //

    CHECKED_NTSTATUS_MSG(
        Rtl->RtlCharToInteger(
            Session->ArgvA[1],
            10,
            &Session->TargetProcessId
        ),
        "Rtl->RtlCharToInteger(ArgvA[1])"
    );

    //
    // Try open a handle to the process with all access.
    //

    Session->TargetProcessHandle = OpenProcess(PROCESS_ALL_ACCESS,
                                               TRUE,
                                               Session->TargetProcessId);
    if (!Session->TargetProcessHandle) {
        OutputDebugStringA("DbgEng:OpenProcess(PROCESS_ALL_ACCESS) failed.");
    }

    //
    // Attach to the process with the debug client.
    //

    Engine = &Session->Engine;
    Client = Engine->Client;

    CHECKED_HRESULT_MSG(
        Client->AttachProcess(
            (PIDEBUGCLIENT)Client,
            0,
            Session->TargetProcessId,
            DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND
        ),
        "IDebugClient->AttachProcess()"
    );

    //
    // AttachProcess work, create a debug control object.
    //

    CHECKED_MSG(CreateIDebugControl(Session), "CreateIDebugControl(Session)");

    Control = Engine->Control;

    //
    // Set the start pointer.
    //

    Session->Start = StartDebugEngineSession;

    //
    // Try disassemble a known address.
    //

    {
        //ULONG Flags = DEBUG_DISASM_EFFECTIVE_ADDRESS;
        ULONG Flags = 0;
        ULONG64 Address = (ULONG64)0x000000001e0c5450;
        CHAR Buffer[256];
        ULONG BufferSize = sizeof(Buffer);
        ULONG DisassemblySize;
        ULONG64 EndOffset;

        SecureZeroMemory(&Buffer, sizeof(Buffer));

        CHECKED_HRESULT_MSG(
            Control->Disassemble(
                (PIDEBUGCONTROL)Control,
                Address,
                Flags,
                (PSTR)&Buffer,
                BufferSize,
                &DisassemblySize,
                &EndOffset
            ),
            "Control->Disassemble()"
        );
    }

    //
    // Update the caller's pointer.
    //

    *SessionPointer = Session;

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    if (Session) {
        MAYBE_CLOSE_HANDLE(Session->TargetProcessHandle);
    }

    MAYBE_FREE_POINTER(Session, Allocator);

End:
    MAYBE_FREE_LIBRARY(Shell32Module);

    return Success;
}

_Use_decl_annotations_
BOOL
StartDebugEngineSession(
    PDEBUG_ENGINE_SESSION Session
    )
/*++

Routine Description:

    This routine starts a debug engine session.

Arguments:

    Session - Supplies a pointer to a DEBUG_ENGINE_SESSION.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
