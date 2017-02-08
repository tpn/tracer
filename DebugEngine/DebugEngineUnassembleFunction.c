/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineUnassembleFunction.c

Abstract:

    This module implements functionality related to unassembling instruction
    codes in a function to their corresponding assembly mnemonics using the
    debugger command `uf` (unassemble function).

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineUnassembleFunction(
    PDEBUG_ENGINE_OUTPUT Output,
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS CommandOptions,
    PCUNICODE_STRING FunctionName
    )
/*++

Routine Description:

    Unassemble a function via DbgEng's `uf` (unassemble function) command.

Arguments:

    Output - Supplies a pointer to an initialized DEBUG_ENGINE_OUTPUT
        structure.

    OutputFlags - Supplies flags customizing the command output.

    CommandOptions - Supplies options for the examine symbols command.

    FunctionName - Name of the function to unassemble.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session;
    WCHAR CommandBuffer[256];
    UNICODE_STRING Command;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Output)) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    Session = Output->Session;
    Engine = Session->Engine;

    //
    // Initialize the stack-allocated command buffer.
    //

    Command.Length = 0;
    Command.MaximumLength = sizeof(CommandBuffer);
    Command.Buffer = CommandBuffer;

    //
    // Acquire the engine lock and set the current output.
    //

    AcquireDebugEngineLock(Engine);
    AcquiredLock = TRUE;
    Engine->CurrentOutput = Output;

    Output->Flags.AsULong = OutputFlags.AsULong;

    //
    // Create the command string.
    //

    Success = DebugEngineBuildCommand(Output,
                                      UnassembleFunctionCommandId,
                                      CommandOptions.AsULong,
                                      FunctionName,
                                      &Command);

    if (!Success) {
        goto Error;
    }

    //
    // Execute the command.
    //

    QueryPerformanceCounter(&Output->Timestamp.CommandStart);

    Success = DebugEngineExecuteCommand(Output);

    QueryPerformanceCounter(&Output->Timestamp.CommandEnd);

    if (Success) {
        goto End;
    }

    //
    // Intentional follow-on to Error.
    //

Error:
    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    if (AcquiredLock) {
        ReleaseDebugEngineLock(Engine);
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
