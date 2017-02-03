/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineSettingsMeta.c

Abstract:

    This module implements functionality related to the debugger's `.settings`
    meta-command.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineSettingsMeta(
    PDEBUG_ENGINE_OUTPUT Output,
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS CommandOptions,
    PCUNICODE_STRING Arguments
    )
/*++

Routine Description:

    Run the debugger meta-command `.settings`.

Arguments:

    Output - Supplies a pointer to an initialized DEBUG_ENGINE_OUTPUT
        structure.

    OutputFlags - Supplies flags customizing the command output.

    CommandOptions - Supplies options for the `.settings` meta-command.

    Arguments - Unused.

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
                                      SettingsMetaCommandId,
                                      CommandOptions.AsULong,
                                      Arguments,
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
DebugEngineListSettings(
    PDEBUG_ENGINE_SESSION Session
    )
/*++

Routine Description:

    Executes the `.settings list` meta-command and prints output to the
    debug output stream.

Arguments:

    Session - Supplies a pointer to a DEBUG_ENGINE_SESSION structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    DEBUG_ENGINE_OUTPUT Output;
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags;
    DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS SettingsMetaOptions;

    //
    // Initialize the DEBUG_ENGINE_OUTPUT structure.  This is a stack allocated
    // structure that will persist for the lifetime of this routine and is used
    // to communicate partial output state across multiple callbacks.
    //

    SecureZeroMemory(&Output, sizeof(DEBUG_ENGINE_OUTPUT));
    Output.SizeOfStruct = sizeof(DEBUG_ENGINE_OUTPUT);

    Success = InitializeDebugEngineOutputSimple(&Output, Session);

    if (!Success) {
        return FALSE;
    }

    OutputFlags.AsULong = 0;
    OutputFlags.EnableLineOutputCallbacks = TRUE;

    SettingsMetaOptions.AsULong = 0;
    SettingsMetaOptions.List = TRUE;

    Success = Session->SettingsMeta(&Output,
                                    OutputFlags,
                                    SettingsMetaOptions,
                                    NULL);

    return Success;
}

_Use_decl_annotations_
BOOL
DebugEngineLoadSettings(
    PDEBUG_ENGINE_SESSION Session,
    PCUNICODE_STRING DebugSettingsXmlPath
    )
/*++

Routine Description:

    Executes the `.settings list` meta-command and prints output to the
    debug output stream.

Arguments:

    Session - Supplies a pointer to a DEBUG_ENGINE_SESSION structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    DEBUG_ENGINE_OUTPUT Output;
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags;
    DEBUG_ENGINE_SETTINGS_META_COMMAND_OPTIONS SettingsMetaOptions;

    //
    // Initialize the DEBUG_ENGINE_OUTPUT structure.  This is a stack allocated
    // structure that will persist for the lifetime of this routine and is used
    // to communicate partial output state across multiple callbacks.
    //

    SecureZeroMemory(&Output, sizeof(DEBUG_ENGINE_OUTPUT));
    Output.SizeOfStruct = sizeof(DEBUG_ENGINE_OUTPUT);

    Success = InitializeDebugEngineOutputSimple(&Output, Session);

    if (!Success) {
        return FALSE;
    }

    OutputFlags.AsULong = 0;
    OutputFlags.EnableLineOutputCallbacks = TRUE;

    SettingsMetaOptions.AsULong = 0;
    SettingsMetaOptions.Load = TRUE;

    Success = Session->SettingsMeta(&Output,
                                    OutputFlags,
                                    SettingsMetaOptions,
                                    DebugSettingsXmlPath);

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
