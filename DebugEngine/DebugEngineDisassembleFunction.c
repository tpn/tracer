/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DisassembleFunction.c

Abstract:

    This module implements functionlity related to disassembling functions
    via DbgEng.dll.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineDisassembleFunction(
    PDEBUG_ENGINE_SESSION DebugEngineSession,
    PALLOCATOR Allocator,
    DEBUG_ENGINE_DISASSEMBLE_FUNCTION_FLAGS Flags,
    PRTL_PATH ModulePath,
    PSTRING FunctionName,
    PUNICODE_STRING FunctionNameWide,
    PPDEBUG_ENGINE_DISASSEMBLED_FUNCTION FunctionPointer
    )
/*++

Routine Description:

    Disassemble a function via DbgEng's `uf` (unassemble function) command.

Arguments:

    DebugEngineSession - Supplies a pointer to a DEBUG_ENGINE_SESSION structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure to be used for any
        memory allocation required by the routine.

    Flags - Supplies an instance of DEBUG_ENGINE_DISASSEMBLE_FUNCTION_FLAGS that
        can be used to customize the disassembly options.

    ModulePath - Supplies a pointer to an RTL_PATH structure for the module
        where the function to be disassembles resides.

    FunctionName - Supplies a pointer to a STRING structure representing the
        function name to disassemble.  Either FunctionName or FunctionNameWide
        must be provided.

    FunctionNameWide - Supplies a pointer to a UNICODE_STRING structure
        representing the function name to disassemble.  Either FunctionNameWide
        or FunctionName must be provided.

            N.B. If both are provided, FunctionNameWide takes precendence.

    FunctionPointer - Supplies an address to a variable that receives the
        address of a DEBUG_ENGINE_DISASSEMBLE_FUNCTION structure that represents
        the disassembled function.  This will be set to NULL on error.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
