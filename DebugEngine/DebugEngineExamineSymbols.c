/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineExamineSymbols.c

Abstract:

    This module implements functionality related to examining symbols via
    DbgEng.dll's `x` (examine symbols) command.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineExamineSymbols(
    PDEBUG_ENGINE_OUTPUT Output,
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags,
    DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS CommandOptions
    )
/*++

Routine Description:

    Examine symbols in a module via DbgEng.dll's `x` (examine symbols) command.
    Capture output and feed it back to the caller via the DEBUG_ENGINE_OUTPUT
    structure.

Arguments:

    Output - Supplies a pointer to an initialized DEBUG_ENGINE_OUTPUT
        structure.

    OutputFlags - Supplies flags customizing the command output.

    CommandOptions - Supplies options for the examine symbols command.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session;
    WCHAR WideStackBuffer[256];
    UNICODE_STRING StackBuffer;

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
    // Initialize the stack buffer.
    //

    StackBuffer.Length = 0;
    StackBuffer.MaximumLength = sizeof(WideStackBuffer);
    StackBuffer.Buffer = WideStackBuffer;

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
                                      ExamineSymbolsCommandId,
                                      CommandOptions.AsULong,
                                      NULL,
                                      &StackBuffer);

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
ExamineSymbolsParseLine(
    PDEBUG_ENGINE_OUTPUT Output,
    PDEBUG_ENGINE_EXAMINED_SYMBOL Symbol,
    PSTRING Line
    )
/*++

Routine Description:

    Parses the output of the examine symbols `x /v /t` command.

Arguments:

    Output - Supplies a pointer to the DEBUG_ENGINE_OUTPUT structure in use
        for the examine symbols command.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    PRTL Rtl;
    CHAR Temp;
    BOOL Success;
    BOOL IsFunctionPointer;
    USHORT Length;
    ULONG Size;
    SHORT MatchIndex;
    SHORT MatchOffset;
    PCHAR Char;
    PCHAR End;
    PSTRING Scope;
    STRING SymbolSize;
    STRING Address;
    STRING BasicType;
    STRING Function;
    STRING Parameters;
    //PSTRING Parameter;
    HANDLE HeapHandle;
    STRING_MATCH Match;
    ULARGE_INTEGER Addr;
    USHORT NumberOfParameters;
    USHORT BasicTypeMatchAttempts;
    USHORT NumberOfBasicTypeStringTables;
    PALLOCATOR ExamineSymbolAllocator;
    PALLOCATOR ExamineSymbolSecondaryAllocator;
    PSTRING_TABLE StringTable;
    PDEBUG_ENGINE_SESSION Session;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE SymbolType;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE SymbolScope;
    PRTL_CHAR_TO_INTEGER RtlCharToInteger;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;


    CHAR StackBitmapBuffer[32];
    RTL_BITMAP Bitmap = { 32 << 3, (PULONG)&StackBitmapBuffer };
    PRTL_BITMAP BitmapPointer = &Bitmap;

    //
    // Initialize aliases.
    //

    Session = Output->Session;
    HeapHandle = Output->Allocator->HeapHandle;
    ExamineSymbolAllocator = Output->CustomStructureAllocator;
    ExamineSymbolSecondaryAllocator = Output->CustomStructureSecondaryAllocator;

    End = Line->Buffer + Line->Length;

    StringTable = Session->ExamineSymbolsPrefixStringTable;
    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;

    MatchIndex = IsPrefixOfStringInTable(StringTable,
                                         Line,
                                         &Match);

    if (MatchIndex == NO_MATCH_FOUND) {
        __debugbreak();
        return TRUE;
    }

    Rtl = Session->Rtl;
    RtlCharToInteger = Rtl->RtlCharToInteger;

    SymbolScope = MatchIndex;
    Scope = Match.String;

    Length = Match.NumberOfMatchedCharacters;
    Char = (Line->Buffer + Length);

    while (*(++Char) == ' ');

    Address.Length = sizeof("00000000`00000000")-1;
    Address.MaximumLength = Address.Length;
    Address.Buffer = Char;

    if (Address.Buffer[8] != '`') {
        __debugbreak();
        return TRUE;
    }

    Address.Buffer[8] = '\0';
    if (FAILED(RtlCharToInteger(Address.Buffer, 16, &Addr.HighPart))) {
        __debugbreak();
        NOTHING;
    }
    Address.Buffer[8] = '`';

    Address.Buffer[17] = '\0';
    if (FAILED(RtlCharToInteger(Address.Buffer+9, 16, &Addr.LowPart))) {
        __debugbreak();
        NOTHING;
    }
    Address.Buffer[17] = ' ';

    Char = Address.Buffer + Address.Length;

    while (*(++Char) == ' ');

    SymbolSize.Buffer = Char;

    while (*Char++ != ' ');

    SymbolSize.Length = (USHORT)(Char - SymbolSize.Buffer) - 1;
    SymbolSize.MaximumLength = SymbolSize.Length;

    BasicType.Buffer = Char;

    Temp = SymbolSize.Buffer[SymbolSize.Length];
    SymbolSize.Buffer[SymbolSize.Length] = '\0';
    if (FAILED(Rtl->RtlCharToInteger(SymbolSize.Buffer, 0, &Size))) {
        __debugbreak();
        NOTHING;
    }

    SymbolSize.Buffer[SymbolSize.Length] = Temp;

    BasicType.Length = (USHORT)(End - BasicType.Buffer) - 1;
    BasicType.MaximumLength = BasicType.Length;

    StringTable = Session->ExamineSymbolsBasicTypeStringTable1;
    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;
    MatchOffset = 0;
    BasicTypeMatchAttempts = 0;
    NumberOfBasicTypeStringTables = Session->NumberOfBasicTypeStringTables;

RetryBasicTypeMatch:

    MatchIndex = IsPrefixOfStringInTable(StringTable,
                                         &BasicType,
                                         &Match);

    if (MatchIndex == NO_MATCH_FOUND) {
        if (++BasicTypeMatchAttempts > NumberOfBasicTypeStringTables) {
            __debugbreak();
            UnknownBasicType(&BasicType);
            return TRUE;
        }
        StringTable++;
        MatchOffset += MAX_STRING_TABLE_ENTRIES;
        goto RetryBasicTypeMatch;
    }

    SymbolType = MatchIndex + MatchOffset;

    switch (SymbolType) {

        case FunctionType:

            //
            // Advance to the function name.
            //

            Char += Match.NumberOfMatchedCharacters + 1;

            IsFunctionPointer = (*Char == '*');

            if (IsFunctionPointer) {
                __debugbreak();
                return TRUE;
            }

            while (*(Char++) != '!');

            Function.Buffer = Char;

            while (*(Char++) != ' ');

            Function.Length = (USHORT)(Char - Function.Buffer) - 1;
            Function.MaximumLength = Function.Length;

            while (*(Char++) != '(');

            Parameters.Buffer = Char;
            Parameters.Length = (USHORT)(End - Parameters.Buffer) - 2;
            Parameters.MaximumLength = Parameters.Length;

            Success = Rtl->CreateBitmapIndexForString(Rtl,
                                                      &Parameters,
                                                      ',',
                                                      &HeapHandle,
                                                      &BitmapPointer,
                                                      FALSE,
                                                      NULL);

            NumberOfParameters = (USHORT)Rtl->RtlNumberOfSetBits(BitmapPointer);

            if (NumberOfParameters == 0) {
                PULONG ULongBuffer = (PULONG)Parameters.Buffer;
                PULONG Void = (PULONG)"void";
                BOOL IsVoid = (*(ULongBuffer) == *(Void));

                if (!IsVoid) {
                    NumberOfParameters = 1;
                }
            }

            break;

        default:
            break;
    }

    if (SymbolType != FunctionType) {
        return TRUE;
    }


//End:
    if (HeapHandle) {

        if ((ULONG_PTR)Bitmap.Buffer == (ULONG_PTR)StackBitmapBuffer) {
            __debugbreak();
        }

        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
ExamineSymbolsParseLinesIntoCustomStructureCallback(
    PDEBUG_ENGINE_OUTPUT Output
    )
/*++

Routine Description:

    Parses the output of the examine symbols `x /v /t` command.

Arguments:

    Output - Supplies a pointer to the DEBUG_ENGINE_OUTPUT structure in use
        for the examine symbols command.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    //
    // For each line, call parse lines.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
