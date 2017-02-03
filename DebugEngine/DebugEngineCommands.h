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

CONST UNICODE_STRING Asterisk;
CONST UNICODE_STRING ExclamationPoint;

//
// ExamineSymbols
//

STRING ExamineSymbolsCommandName;
UNICODE_STRING ExamineSymbolsCommandNameWide;
STRING ExamineSymbolsCommandDisplayName;
UNICODE_STRING ExamineSymbolsCommandOptions[];
DEBUG_ENGINE_COMMAND_TEMPLATE ExamineSymbolsCommandTemplate;

//
// UnassembleFunction.
//

STRING UnassembleFunctionCommandName;
UNICODE_STRING UnassembleFunctionCommandNameWide;
STRING UnassembleFunctionCommandDisplayName;
UNICODE_STRING UnassembleFunctionCommandOptions[];
DEBUG_ENGINE_COMMAND_TEMPLATE UnassembleFunctionCommandTemplate;

//
// DisplayType.
//

STRING DisplayTypeCommandName;
UNICODE_STRING DisplayTypeCommandNameWide;
STRING DisplayTypeCommandDisplayName;
UNICODE_STRING DisplayTypeCommandOptions[];
DEBUG_ENGINE_COMMAND_TEMPLATE DisplayTypeCommandTemplate;

//
// SettingsMeta.
//

STRING SettingsMetaCommandName;
UNICODE_STRING SettingsMetaCommandNameWide;
STRING SettingsMetaCommandDisplayName;
UNICODE_STRING SettingsMetaCommandOptions[];
DEBUG_ENGINE_COMMAND_TEMPLATE SettingsMetaCommandTemplate;

//
// Array of templates.
//

PDEBUG_ENGINE_COMMAND_TEMPLATE DebugEngineCommandTemplates[];

//
// Inline helpers.
//

FORCEINLINE
PDEBUG_ENGINE_COMMAND_TEMPLATE
CommandIdToCommandTemplate(
    _In_ DEBUG_ENGINE_COMMAND_ID CommandId
    )
{
    LONG ArrayIndex;

    ArrayIndex = CommandIdToArrayIndex(CommandId);
    if (ArrayIndex < 0) {
        return NULL;
    }

    return DebugEngineCommandTemplates[ArrayIndex];
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
