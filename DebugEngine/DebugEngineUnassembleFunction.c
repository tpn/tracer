/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineUnassembleFunction.c

Abstract:

    This module implements functionality related to unassembling instruction
    codes in a function to their corresponding assembly mnemonics using the
    debugger command `uf` (unassemble function).
    via DbgEng.dll.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineUnassembleFunction(
    PDEBUG_ENGINE_OUTPUT Output,
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS CommandOptions,
    PUNICODE_STRING FunctionName
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
    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
