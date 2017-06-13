/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineExecuteCommand.c

Abstract:

    This module implements helper functionality for executing command strings
    that will be executed by the debug engine via the IDebugControl->Execute()
    interface.  Routines are provided to execute a built command string, as
    well as output callbacks that marshal the debugger output back to the
    upstream client in a consistent manner using the DEBUG_ENGINE_OUTPUT
    structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineExecuteCommand(
    PDEBUG_ENGINE_OUTPUT Output
    )
/*++

Routine Description:

    Execute a command in the debugger.

Arguments:

    Output - Supplies a pointer to an initialized DEBUG_ENGINE_OUTPUT structure
        to be used for the command.  DebugEngineBuildCommand() must have
        already been called on this structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    ULONG LastError;
    HRESULT Result;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session;
    DEBUG_OUTPUT_MASK OutputMask = { 0 };
    DEBUG_OUTPUT_MASK OldOutputMask = { 0 };

    //
    // Initialize aliases.
    //

    Session = Output->Session;
    Engine = Session->Engine;

    //
    // Initialize our output mask and get the current one.
    //

    OutputMask.Normal = TRUE;

    CHECKED_HRESULT_MSG(
        Engine->Client->GetOutputMask(
            Engine->IClient,
            &OldOutputMask.AsULong
        ),
        "Client->GetOutputMask()"
    );

    //
    // If our output mask differs from the current one, set it.
    //

    if (OutputMask.AsULong != OldOutputMask.AsULong) {
        CHECKED_HRESULT_MSG(
            Engine->Client->SetOutputMask(
                Engine->IClient,
                OutputMask.AsULong
            ),
            "Client->SetOutputMask()"
        );
    }

    if (Output->Flags.EnableLineTextAndCustomStructureAllocators ||
        Output->Flags.EnableLineOutputCallbacks) {
        Output->Flags.DispatchOutputLineCallbacks = TRUE;
    }

    //
    // Set our output callbacks and execute the command.
    //

    Engine->OutputCallback = DebugEngineOutputCallback;
    CHECKED_MSG(
        DebugEngineSetOutputCallbacks(Engine),
        "DebugEngineSetOutputCallbacks()"
    );

    Output->State.CommandExecuting = TRUE;
    Result = Output->LastResult = Engine->Control->ExecuteWide(
        Engine->IControl,
        DEBUG_OUTCTL_ALL_CLIENTS,
        Output->Command->Buffer,
        DEBUG_EXECUTE_ECHO
    );

    //
    // XXX: Do we need a Client->FlushCallbacks() here?
    //

    Output->State.CommandExecuting = FALSE;

    if (Result != S_OK) {
        Success = FALSE;
        LastError = GetLastError();
        OutputDebugStringA("Control->ExecuteWide() failed: ");
        OutputDebugStringW(Output->Command->Buffer);
        Output->State.ExecuteCommandFailed = TRUE;
    } else {
        Success = TRUE;
        Output->State.ExecuteCommandSucceeded = TRUE;
    }

    //
    // Restore the old output mask if we changed it earlier.
    //

    if (OutputMask.AsULong != OldOutputMask.AsULong) {
        CHECKED_HRESULT_MSG(
            Engine->Client->SetOutputMask(
                Engine->IClient,
                OldOutputMask.AsULong
            ),
            "Client->SetOutputMask(Old)"
        );
    }

    Output->State.CommandComplete = TRUE;

    if (Output->State.ExecuteCommandFailed) {
        goto Error;
    }

    //
    // Dispatch output completion callbacks.
    //

    Success = DispatchOutputCompleteCallbacks(Output);
    if (!Success) {
        goto Error;
    }

    goto End;

Error:
    Success = FALSE;

    Output->State.Failed = TRUE;

    //
    // Intentional follow-on to End.
    //

End:
    if (Success) {
        Output->State.Succeeded = TRUE;
    }

    //
    // Clear the OutputCallback handler such that subsequent output from the
    // debugger is ignored.  This could be improved by returning to a 'default'
    // output handler that uses a heap allocated DEBUG_ENGINE_OUTPUT structure
    // (most callers of this function use stack-allocated structures, and thus,
    //  the structure is invalid after the call returns).
    //

    Engine->OutputCallback = NULL;

    return Success;
}

_Use_decl_annotations_
BOOL
DebugEngineSessionExecuteStaticCommand(
    PDEBUG_ENGINE_SESSION Session,
    PCUNICODE_STRING Command,
    PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK LineOutputCallback
    )
/*++

Routine Description:

    Execute a static UNICODE_STRING command in the debugger.  Static in this
    context refers to a pre-composed command string sent verbatim to the
    debugger's ExecuteWide() routine.

Arguments:

    Engine - Supplies a pointer to the DEBUG_ENGINE to use.

    Command - Supplies a pointer to a UNICODE_STRING structure containing the
        command to send to the debugger.

    LineOutputCallback - Optionally supplies a pointer to a line output callback
        that will be set for the duration of the command's execution.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    PDEBUG_ENGINE Engine;
    DEBUG_ENGINE_OUTPUT Output;
    PDEBUG_ENGINE_LINE_OUTPUT_CALLBACK Callback;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Session)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Command)) {
        return FALSE;
    }

    //
    // If the caller has provided a line output callback, use it.  Otherwise,
    // default to the default DebugStream line output callback.
    //

    if (ARGUMENT_PRESENT(LineOutputCallback)) {
        Callback = LineOutputCallback;
    } else {
        Callback = DebugStreamLineOutputCallback;
    }

    //
    // Initialize aliases.
    //

    Engine = Session->Engine;

    //
    // Initialize the Output structure.
    //

    Output.SizeOfStruct = sizeof(Output);

    Success = (
        Session->InitializeDebugEngineOutput(
            &Output,
            Session,
            Session->Allocator,
            NULL,       // LineAllocator
            NULL,       // TextAllocator
            NULL,       // CustomStructureAllocator
            NULL,       // CustomStructureSecondaryAllocator
            Callback,   // LineOutputCallback
            NULL,       // PartialOutputCallback
            NULL,       // OutputCompleteCallback
            NULL,       // Context
            NULL        // ModulePath
        )
    );

    if (!Success) {
        return FALSE;
    }

    //
    // Acquire the engine lock and set the current output.
    //

    AcquireDebugEngineLock(Engine);
    AcquiredLock = TRUE;
    Engine->CurrentOutput = &Output;

    //
    // Enable line output.
    //

    Output.Flags.AsULong = 0;
    Output.Flags.EnableLineOutputCallbacks = TRUE;

    //
    // Set the command on the Output struct.
    //

    Output.Command = Command;

    //
    // Execute the given command string.
    //

    QueryPerformanceCounter(&Output.Timestamp.CommandStart);

    Success = DebugEngineExecuteCommand(&Output);

    QueryPerformanceCounter(&Output.Timestamp.CommandEnd);

    //
    // Release the engine lock if applicable and return success.
    //

    if (AcquiredLock) {
        ReleaseDebugEngineLock(Engine);
    }

    return Success;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
