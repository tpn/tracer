/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineConstants.c

Abstract:

    This module declares constants used by the DebugEngine component.

--*/

#include "stdafx.h"

//
// Instances of callback structures primed with relevant function pointers.
//

CONST DEBUGEVENTCALLBACKS DebugEventCallbacks = {
    DebugEventQueryInterface,
    DebugEventAddRef,
    DebugEventRelease,
    DebugEventGetInterestMaskCallback,
    DebugEventBreakpointCallback,
    DebugEventExceptionCallback,
    DebugEventCreateThreadCallback,
    DebugEventExitThreadCallback,
    DebugEventCreateProcessCallback,
    DebugEventExitProcessCallback,
    DebugEventLoadModuleCallback,
    DebugEventUnloadModuleCallback,
    DebugEventSystemErrorCallback,
    DebugEventSessionStatusCallback,
    DebugEventChangeDebuggeeStateCallback,
    DebugEventChangeEngineStateCallback,
    DebugEventChangeSymbolStateCallback,
};

CONST DEBUGINPUTCALLBACKS DebugInputCallbacks = {
    DebugInputQueryInterface,
    DebugInputAddRef,
    DebugInputRelease,
    DebugInputStartInputCallback,
    DebugInputEndInputCallback,
};

CONST DEBUGOUTPUTCALLBACKS DebugOutputCallbacks = {
    DebugOutputCallbacksQueryInterface,
    DebugOutputCallbacksAddRef,
    DebugOutputCallbacksRelease,
    DebugOutputCallbacksOutput,
};

CONST DEBUGOUTPUTCALLBACKS2 DebugOutputCallbacks2 = {
    DebugOutputCallbacks2QueryInterface,
    DebugOutputCallbacks2AddRef,
    DebugOutputCallbacks2Release,
    DebugOutputCallbacks2Output,
    DebugOutputCallbacks2GetInterestMask,
    DebugOutputCallbacks2Output2,
};

//
// Custom structure names.  The C_ASSERT() ensures the type name passed in is
// actually a valid structure name.
//

#define DEFINE_CUSTOM_STRUCTURE_NAME(Name, Type)     \
    C_ASSERT(sizeof(Type) == sizeof(Type));          \
    CONST STRING Name = RTL_CONSTANT_STRING(#Type)

DEFINE_CUSTOM_STRUCTURE_NAME(ExamineSymbolCustomStructureName,
                             DEBUG_ENGINE_EXAMINED_SYMBOL);

DEFINE_CUSTOM_STRUCTURE_NAME(UnassembledFunctionCustomStructureName,
                             DEBUG_ENGINE_UNASSEMBLED_FUNCTION);

DEFINE_CUSTOM_STRUCTURE_NAME(DisplayedTypeCustomStructureName,
                             DEBUG_ENGINE_DISPLAYED_TYPE);

//
// StringTable-related constants.  Each table consists of a maximum of 16
// strings, as this is the limit of an individual string table.
//

#undef DSTR
#define DSTR(String) String ";"

CONST CHAR StringTableDelimiter = ';';

//
// N.B. The order of these string constants must match the *exact* order of the
//      the corresponding enumeration symbol defined in DebugEngine.h.
//
//      Additionally, when a name overlaps, the longer name must come first.
//      E.g. "int64" needs to come before "int".
//

//
// ExamineSymbolPrefixes
//

CONST STRING ExamineSymbolsPrefixes = RTL_CONSTANT_STRING(
    DSTR("prv global")
    DSTR("prv inline")
    DSTR("prv func")
    DSTR("pub global")
    DSTR("pub func")
);

//
// ExamineSymbolsBasicTypes
//

CONST STRING ExamineSymbolsBasicTypes1 = RTL_CONSTANT_STRING(
    DSTR("<NoType>")
    DSTR("<function>")
    DSTR("char")
    DSTR("wchar_t")
    DSTR("short")
    DSTR("long")
    DSTR("int64")
    DSTR("int")
    DSTR("unsigned char")
    DSTR("unsigned wchar_t")
    DSTR("unsigned short")
    DSTR("unsigned long")
    DSTR("unsigned int64")
    DSTR("unsigned int")
    DSTR("union")
    DSTR("struct")
);

CONST STRING ExamineSymbolsBasicTypes2 = RTL_CONSTANT_STRING(
    DSTR("<CLR type>")
    DSTR("bool")
    DSTR("void")
    DSTR("class")
    DSTR("float")
    DSTR("double")
    DSTR("_SAL_ExecutionContext")
    DSTR("__enative_startup_state")
);

//
// FunctionArgumentTypes
//

CONST STRING FunctionArgumentTypes1 = RTL_CONSTANT_STRING(
    DSTR("char")
    DSTR("wchar_t")
    DSTR("short")
    DSTR("long")
    DSTR("int64")
    DSTR("int")
    DSTR("unsigned char")
    DSTR("unsigned wchar_t")
    DSTR("unsigned short")
    DSTR("unsigned long")
    DSTR("unsigned int64")
    DSTR("unsigned int")
    DSTR("union")
    DSTR("struct")
    DSTR("float")
    DSTR("double")
);

CONST STRING FunctionArgumentTypes2 = RTL_CONSTANT_STRING(
    DSTR("bool")
    DSTR("void")
    DSTR("class")
    DSTR("<function>")
);

//
// FunctionArgumentVectorTypes
//

CONST STRING FunctionArgumentVectorTypes1 = RTL_CONSTANT_STRING(
    DSTR("__m64")
    DSTR("__m128")
    DSTR("__m256")
);

//
// Command line-related strings.
//

CONST STRING CommandLineOptions = RTL_CONSTANT_STRING(
    DSTR("-p")
    DSTR("--pid")
    DSTR("-c")
    DSTR("--commandline")
);

CONST SHORT CommandLineMatchIndexToOption[] = {
    PidCommandLineOption,
    PidCommandLineOption,
    CommandLineCommandLineOption,
    CommandLineCommandLineOption,
};

CONST USHORT NumberOfCommandLineMatchIndexOptions =
    ARRAYSIZE(CommandLineMatchIndexToOption);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
