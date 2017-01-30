/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineExecuteCommand.c

Abstract:

    This module implements helper functionality for executing command strings
    that will be executed by the debug engine via the IDebugControl->Execute()
    interface.  Routines are provided to execute a build command string, as
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
    // Initialize our output mask and get the current output mask.  If they
    // differ, update the current one.
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
    // XXX: temporarily inherit the existing mask.
    //

    OutputMask.AsULong = OldOutputMask.AsULong;

    if (OutputMask.AsULong != OldOutputMask.AsULong) {
        CHECKED_HRESULT_MSG(
            Engine->Client->SetOutputMask(
                Engine->IClient,
                OutputMask.AsULong
            ),
            "Client->SetOutputMask()"
        );
    }

    //
    // Set our output callbacks and execute the command.
    //

    Output->State.CommandExecuting = TRUE;

    //
    // XXX: temporarily force everything to go through ExecuteWide() whilst
    // evaluating the debugger's behavior.
    //

    if (1) {

        Engine->OutputCallback = DebugEngineOutputCallback;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks(Engine),
            "DebugEngineSetOutputCallbacks()"
        );

        Result = Output->LastResult = Engine->Control->ExecuteWide(
            Engine->IControl,
            DEBUG_OUTCTL_ALL_CLIENTS,
            Output->Command->Buffer,
            DEBUG_EXECUTE_ECHO
        );

        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Control->ExecuteWide() failed: ");
            OutputDebugStringW(Output->Command->Buffer);
            Output->State.Failed = TRUE;
        } else {
            Success = TRUE;
            Output->State.Succeeded = TRUE;
        }

    } else if (Output->OutputFlags.WideCharacterOutput) {

        //
        // We don't support wide output at the moment.
        //

        __debugbreak();
        Engine->OutputCallback2 = DebugEngineOutputCallback2;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks2(Engine),
            "DebugEngineSetOutputCallbacks2()"
        );

        Result = Output->LastResult = Engine->Control->ExecuteWide(
            Engine->IControl,
            DEBUG_OUTCTL_THIS_CLIENT,
            Output->Command->Buffer,
            DEBUG_EXECUTE_NOT_LOGGED
        );

        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Control->ExecuteWide() failed: ");
            OutputDebugStringW(Output->Command->Buffer);
            Output->State.Failed = TRUE;
        } else {
            Success = TRUE;
            Output->State.Succeeded = TRUE;
        }

    } else {

        PSTRING Command;
        PALLOCATOR Allocator;

        Allocator = Output->Allocator;

        if (!ConvertUtf16StringToUtf8String(Output->Command,
                                            &Command,
                                            Allocator)) {
            goto Error;
        }

        Engine->OutputCallback = DebugEngineOutputCallback;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks(Engine),
            "DebugEngineSetOutputCallbacks()"
        );

        Result = Output->LastResult = Engine->Control->Execute(
            Engine->IControl,
            DEBUG_OUTCTL_ALL_CLIENTS,
            Command->Buffer,
            DEBUG_EXECUTE_ECHO
        );

        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Control->Execute() failed: ");
            OutputDebugStringA(Command->Buffer);
            Output->State.Failed = TRUE;
        } else {
            Success = TRUE;
            Output->State.Succeeded = TRUE;
        }

        Allocator->Free(Allocator->Context, Command);
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

    //
    // Call the output completion callback.
    //

    Output->State.CommandComplete = TRUE;
    Success = Output->OutputCompleteCallback(Output);

    goto End;

Error:
    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:
    Output->State.CommandExecuting = FALSE;

    return Success;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEngineOutputCallback(
    PDEBUG_ENGINE DebugEngine,
    DEBUG_OUTPUT_MASK OutputMask,
    PCSTR Text
    )
{
    BOOL Success;
    HRESULT Result;
    LONG_INTEGER TextSizeInBytes;
    PDEBUG_ENGINE_OUTPUT Output;

    //
    // If this isn't normal output, just print it to the debug stream and
    // return.
    //

    if (!OutputMask.Normal) {
        if (Text) {
            OutputDebugStringA(Text);
        }
        return S_OK;
    }

    Output = DebugEngine->CurrentOutput;
    Output->State.InPartialOutputCallback = TRUE;

    TextSizeInBytes.LongPart = (LONG)strlen(Text);

    if (TextSizeInBytes.HighPart) {
        __debugbreak();
        goto Error;
    }

    Output->ThisChunk.Length = TextSizeInBytes.LowPart;
    Output->ThisChunk.MaximumLength = TextSizeInBytes.LowPart;
    Output->ThisChunk.Buffer = (PSTR)Text;

    Output->NumberOfPartialCallbacks++;
    Output->TotalBufferLengthInChars += TextSizeInBytes.LowPart;
    Output->TotalBufferSizeInBytes += TextSizeInBytes.LowPart;

    Success = Output->PartialOutputCallback(Output);
    if (!Success) {
        goto Error;
    }

    Result = S_OK;
    goto End;

Error:
    Result = E_FAIL;

End:
    Output->State.InPartialOutputCallback = FALSE;
    return Result;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEngineOutputCallback2(
    PDEBUG_ENGINE DebugEngine,
    DEBUG_OUTPUT_TYPE OutputType,
    DEBUG_OUTPUT_CALLBACK_FLAGS OutputFlags,
    ULONG64 Arg,
    PCWSTR IncomingText
    )
{
    return E_FAIL;
}



// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
