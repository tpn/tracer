/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineSessionInitialize.c

Abstract:

    This module implements functionality related to initializing a debug engine
    session.  Routines are provided for initializing via the command line,
    initializing from the current process, and initializing the structure
    itself.

--*/

#include "stdafx.h"


_Use_decl_annotations_
BOOL
InitializeFromCommandLine(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PDEBUG_ENGINE_SESSION Session
    )
/*++

Routine Description:

    Initialize a debug engine session via command line parameters.

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
    NTSTATUS Status;
    HMODULE Shell32Module = NULL;
    PCOMMAND_LINE_TO_ARGVW CommandLineToArgvW;

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

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    if (Session) {
        MAYBE_CLOSE_HANDLE(Session->TargetProcessHandle);
    }


End:
    MAYBE_FREE_LIBRARY(Shell32Module);

    return Success;
}


_Use_decl_annotations_
BOOL
InitializeDebugEngineSession(
    PRTL Rtl,
    PALLOCATOR Allocator,
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags,
    PPDEBUG_ENGINE_SESSION SessionPointer
    )
/*++

Routine Description:

    This routine initializes a debug engine session.  This involves loading
    DbgEng.dll, initializing COM, creating an IDebugClient COM object,
    attaching to the process indicated in InitFlags, then creating the
    remaining COM objects and setting relevant client callbacks.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    InitFlags - Supplies initialization flags that are used to control how
        this routine attaches to the initial process.

    SessionPointer - Supplies an address to a variable that will receive the
        address of the newly DEBUG_ENGINE_SESSION structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    HRESULT Result;
    ULONG ExecutionStatus;
    LONG_INTEGER AllocSizeInBytes;
    PDEBUGCLIENT Client;
    PIDEBUGCLIENT IClient;
    PDEBUG_ENGINE Engine;
    ULONG ModuleIndex;
    ULONGLONG ModuleBaseAddress;
    PDEBUG_ENGINE_SESSION Session = NULL;

    //
    // Clear the caller's session pointer up-front.
    //

    *SessionPointer = NULL;

    //
    // Load DbgEng.dll.
    //

    CHECKED_MSG(Rtl->LoadDbgEng(Rtl), "Rtl!LoadDbgEng()");

    //
    // Calculate the required allocation size.
    //

    AllocSizeInBytes.LongPart = (

        //
        // Account for the DEBUG_ENGINE_SESSION structure.
        //

        sizeof(DEBUG_ENGINE_SESSION) +

        //
        // Account for the DEBUG_ENGINE structure, which follows the session
        // in memory.
        //

        sizeof(DEBUG_ENGINE)
    );

    //
    // Sanity check our size isn't over MAX_USHORT.
    //

    if (AllocSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    Session = (PDEBUG_ENGINE_SESSION)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSizeInBytes.LowPart
        )
    );

    if (!Session) {
        goto Error;
    }

    //
    // Memory was allocated successfully.  Carve out the DEBUG_ENGINE pointer.
    //

    Engine = Session->Engine = (PDEBUG_ENGINE)(
        RtlOffsetToPointer(
            Session,
            sizeof(DEBUG_ENGINE_SESSION)
        )
    );

    //
    // Initialize and acquire the debug engine lock.
    //

    InitializeSRWLock(&Engine->Lock);
    AcquireDebugEngineLock(Engine);
    AcquiredLock = TRUE;

    Engine->SizeOfStruct = sizeof(*Engine);

    //
    // Initialize the debug engine and create the COM interfaces.
    //

    CHECKED_MSG(InitializeDebugEngine(Rtl, Engine), "InitializeDebugEngine()");

    if (InitFlags.InitializeFromCommandLine) {
        CHECKED_MSG(
            InitializeFromCommandLine(
                Rtl,
                Allocator,
                Session
            ),
            "InitializeDebugEngine()->FromCommandLine"
        );
    } else if (InitFlags.InitializeFromCurrentProcess) {
        Session->TargetProcessId = FastGetCurrentProcessId();
    }

    Session->TargetProcessHandle = OpenProcess(PROCESS_ALL_ACCESS,
                                               TRUE,
                                               Session->TargetProcessId);
    if (!Session->TargetProcessHandle) {
        OutputDebugStringA("DbgEng:OpenProcess(PROCESS_ALL_ACCESS) failed.");
    }

    //
    // Attach to the process with the debug client.
    //

    Client = Engine->Client;
    IClient = Engine->IClient;

    CHECKED_HRESULT_MSG(
        Client->AttachProcess(
            IClient,
            0,
            Session->TargetProcessId,
            DEBUG_ATTACH_NONINVASIVE |
            DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND
        ),
        "IDebugClient->AttachProcess()"
    );

    CHECKED_HRESULT_MSG(
        Engine->Control->WaitForEvent(
            Engine->IControl,
            0,
            0
        ),
        "Control->WaitForEvent()"
    );

    CHECKED_HRESULT_MSG(
        Engine->Control->GetExecutionStatus(
            Engine->IControl,
            &ExecutionStatus
        ),
        "Control->GetExecutionStatus()"
    );

    //
    // Attempt to resolve Rtl.
    //

    CHECKED_HRESULT_MSG(
        Engine->Symbols->GetModuleByModuleName(
            Engine->ISymbols,
            "Rtl",
            0,
            &ModuleIndex,
            &ModuleBaseAddress
        ),
        "Symbols->GetModuleByModuleName('Rtl')"
    );

    Session->Rtl = Rtl;

    //
    // Set the function pointers.
    //

    Session->Destroy = DestroyDebugEngineSession;
    Session->DisplayType = DebugEngineDisplayType;
    Session->ExamineSymbols = DebugEngineExamineSymbols;
    Session->UnassembleFunction = DebugEngineUnassembleFunction;
    Session->InitializeDebugEngineOutput = InitializeDebugEngineOutput;

    //
    // Update the caller's pointer.

    *SessionPointer = Session;

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    if (Session) {
        MAYBE_CLOSE_HANDLE(Session->TargetProcessHandle);
    }

    //MAYBE_FREE_POINTER(Session, Allocator);

End:

    if (AcquiredLock) {
        ReleaseDebugEngineLock(Engine);
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
