/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineBuildCommand.c

Abstract:

    This module implements helper functionality for building command strings
    that will be executed by the debug engine via the IDebugControl->Execute()
    interface.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineBuildCommand(
    PDEBUG_ENGINE_OUTPUT Output,
    DEBUG_ENGINE_COMMAND_ID CommandId,
    ULONG CommandOptions,
    PCUNICODE_STRING Argument,
    PUNICODE_STRING CommandBuffer
    )
/*++

Routine Description:

    Build a command suitable for executing via the debugger.

Arguments:

    Output - Supplies a pointer to an initialized DEBUG_ENGINE_OUTPUT structure
        to be used for the command.

    CommandId - Supplies the command ID for which to build the command.

    CommandOptions - Supplies command options for this command.

    OutputFlags - Supplies output flags for this command.

    Argument - Optionally supplies a const UNICODE_STRING as an argument to
        append to the command.

    CommandBuffer - Optionally supplies a pointer to a temporary buffer that
        can be used by this routine in order to avoid more expensive allocations
        via the registered Allocator.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    PRTL Rtl;
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    NTSTATUS Status;
    UNICODE_STRING ModuleName;
    PCUNICODE_STRING CommandName;
    PRTL_PATH ModulePath;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session;
    PDEBUG_ENGINE_COMMAND_TEMPLATE CommandTemplate;

    //
    // Initialize aliases.
    //

    Session = Output->Session;
    Engine = Session->Engine;
    Rtl = Session->Rtl;
    ModulePath = Output->ModulePath;

    //
    // Load the command template from the command ID.
    //

    CommandTemplate = CommandIdToCommandTemplate(CommandId);
    if (!CommandTemplate) {
        return FALSE;
    }

    Output->CommandTemplate = CommandTemplate;

    //
    // Define a helper macro for appending to the command buffer.
    //

#define APPEND_TO_COMMAND(String)                           \
    CHECKED_NTSTATUS_MSG(                                   \
        Rtl->RtlAppendUnicodeStringToString(                \
            CommandBuffer,                                  \
            (String)                                        \
        ),                                                  \
        "Rtl->RtlAppendUnicodeStringToString(" #String ")"  \
    )

    //
    // Write the command name to the command buffer.
    //

    CommandName = CommandTemplate->CommandNameWide;
    APPEND_TO_COMMAND(CommandName);

    if (CommandTemplate->Flags.HasOptions) {

        ULONG Count;
        ULONG Index;
        ULONG Shift = 0;
        ULONG Bitmap;
        ULONG NumberOfTrailingZeros;
        ULONG NumberOfOptions = CommandTemplate->NumberOfOptions;
        PUNICODE_STRING Options = CommandTemplate->Options;
        PUNICODE_STRING Option;

        //
        // The command supports options.  Determine if the user has requested
        // any by doing a population count on the command options bitmap.
        //
        // N.B. We need to explicitly zero bits we don't care about when using
        //      the command options as a bitmap, hence the _bzhi_u32() call.
        //

        Bitmap = _bzhi_u32(CommandOptions, NumberOfOptions);

        Count = PopulationCount32(Bitmap);

        if (!Count) {

            //
            // No options were requested.
            //

            goto ProcessModuleName;
        }

        //
        // At least one option was requested by the user.
        //

        do {

            //
            // Extract the next index by counting the number of trailing zeros
            // left in the bitmap and adding the amount we've already shifted
            // by.
            //

            NumberOfTrailingZeros = TrailingZeros(Bitmap);
            Index = NumberOfTrailingZeros + Shift;

            if (Index > NumberOfOptions - 1) {
                __debugbreak();
                goto Error;
            }

            //
            // Get the option string from the command template at this index.
            //

            Option = (Options + Index);

            //
            // Append it to the current command buffer.
            //

            APPEND_TO_COMMAND(Option);

            //
            // Shift the bitmap right, past the zeros and the 1 that was just
            // found, such that it's positioned correctly for the next loop's
            // tzcnt. Update the shift count accordingly.
            //

            Bitmap >>= (NumberOfTrailingZeros + 1);
            Shift = Index + 1;

        } while (--Count);
    }

ProcessModuleName:

    if (CommandTemplate->Flags.HasModuleName) {
        PUNICODE_STRING pModuleName = &ModuleName;

        //
        // Append the module name to the command.
        //

        ModuleName.Length = ModulePath->Name.Length;
        ModuleName.Buffer = ModulePath->Name.Buffer;

        //
        // If the module has an extension, which it almost certainly will
        // (e.g. ".dll"), omit its length.  The debugger expects the module
        // name without any extension, e.g. "python27!".
        //

        if (ModulePath->Extension.Length) {

            //
            // Add a sizeof(WCHAR) to account for the '.', then subtract from
            // the current module length.
            //

            ModuleName.Length -= (
                ModulePath->Extension.Length + sizeof(WCHAR)
            );
        }

        ModuleName.MaximumLength = ModuleName.Length;

        APPEND_TO_COMMAND(pModuleName);
    }

    if (CommandTemplate->Flags.HasExclamationPoint) {
        PCUNICODE_STRING pExclamationPoint = &ExclamationPoint;

        APPEND_TO_COMMAND(pExclamationPoint);
    }

    if (CommandTemplate->Flags.MandatoryArgument) {

        if (!Argument || !Argument->Length) {
            __debugbreak();
            goto Error;
        }

        APPEND_TO_COMMAND(Argument);

    } else if (CommandTemplate->Flags.OptionalArgument &&
               (Argument && Argument->Length)) {

        APPEND_TO_COMMAND(Argument);

    } else if (CommandTemplate->Flags.ArgumentDefaultsToAsterisk) {
        PCUNICODE_STRING pAsterisk = &Asterisk;

        APPEND_TO_COMMAND(pAsterisk);
    }

    //
    // Terminate the string.
    //

    if (CommandBuffer->Length == CommandBuffer->MaximumLength) {
        __debugbreak();
        goto Error;
    }

    CommandBuffer->Buffer[CommandBuffer->Length >> 1] = L'\0';

    //
    // Command was successfully built.  Update state and return success.
    //

    Output->State.CommandBuilt = TRUE;
    Success = TRUE;

    Output->Command = CommandBuffer;

    goto End;

Error:
    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
