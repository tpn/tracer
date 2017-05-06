/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineDisplayType.c

Abstract:

    This module implements functionality related to displaying types via the
    debugger's `dt` (display type) command.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineDisplayType(
    PDEBUG_ENGINE_OUTPUT Output,
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS CommandOptions,
    PCUNICODE_STRING SymbolName
    )
/*++

Routine Description:

    Display a type via the debugger's `dt` (display type) command.

Arguments:

    Output - Supplies a pointer to an initialized DEBUG_ENGINE_OUTPUT
        structure.

    OutputFlags - Supplies flags customizing the command output.

    CommandOptions - Supplies options for the display type command.

    SymbolName - Supplies the name of the symbol for which the type will be
        displayed.

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
                                      DisplayTypeCommandId,
                                      CommandOptions.AsULong,
                                      SymbolName,
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

_Use_decl_annotations_
BOOL
DisplayTypeParseLine(
    PDEBUG_ENGINE_OUTPUT Output,
    PSTRING Line
    )
/*++

Routine Description:

    Parses a line of output from the `dt` (display type) command.

Arguments:

    Output - Supplies a pointer to the DEBUG_ENGINE_OUTPUT structure in use
        for the display type command.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    return TRUE;
}

_Use_decl_annotations_
BOOL
DisplayTypeParseLinesIntoCustomStructureCallback(
    PDEBUG_ENGINE_OUTPUT Output
    )
/*++

Routine Description:

    This routine is called once the display type command has completed
    executing and all output lines have been saved.

Arguments:

    Output - Supplies a pointer to the DEBUG_ENGINE_OUTPUT structure in use
        for the display type command.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    BOOL Started = FALSE;
    PSTRING Line;
    ULONG TotalLines;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEntry;
    PLINKED_LINE LinkedLine;

    //
    // For each line, call parse lines.
    //

    Output->NumberOfParsedLines = 0;
    ListHead = &Output->SavedLinesListHead;

    FOR_EACH_LIST_ENTRY(ListHead, ListEntry) {
        LinkedLine = CONTAINING_RECORD(ListEntry, LINKED_LINE, ListEntry);
        Line = &LinkedLine->String;
        Success = DisplayTypeParseLine(Output, Line);
        if (!Success) {
            OutputDebugStringA("Failed line!\n");
            PrintStringToDebugStream(Line);
            RemoveEntryList(&LinkedLine->ListEntry);
            AppendTailList(&Output->FailedLinesListHead,
                           &LinkedLine->ListEntry);
            Output->NumberOfFailedLines++;
        } else {
            Output->NumberOfParsedLines++;
        }
    }

    TotalLines = (
        Output->NumberOfParsedLines +
        Output->NumberOfFailedLines
    );

    if (TotalLines != Output->NumberOfSavedLines) {
        __debugbreak();
        Success = FALSE;
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
