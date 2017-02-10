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

#define MAYBE_BREAK()                                 \
    if (Output->Flags.DebugBreakOnLineParsingError) { \
        __debugbreak();                               \
    }

_Use_decl_annotations_
BOOL
ExamineSymbolsParseLine(
    PDEBUG_ENGINE_OUTPUT Output,
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
    BOOL Found;
    BOOL Success = FALSE;
    BOOL IsFunctionPointer;
    USHORT Length;
    ULONG Size;
    PCHAR End;
    PCHAR Char;
    PCHAR Marker;
    PCHAR Lookback;
    PCHAR NextChar;
    USHORT Index;
    SHORT MatchIndex;
    SHORT MatchOffset;
    STRING SymbolSize;
    STRING Address;
    STRING BasicType;
    PSTRING Array;
    PSTRING Scope;
    PSTRING TypeName;
    PSTRING Arguments;
    PSTRING SymbolName;
    PSTRING ModuleName;
    PSTRING Remaining;
    HANDLE HeapHandle;
    STRING_MATCH Match;
    ULARGE_INTEGER Addr;
    LONG BytesRemaining;
    USHORT NumberOfArguments;
    USHORT MatchAttempts;
    USHORT NumberOfStringTables;
    PALLOCATOR ExaminedSymbolAllocator;
    PALLOCATOR ExaminedSymbolSecondaryAllocator;
    PSTRING_TABLE StringTable;
    PDEBUG_ENGINE_SESSION Session;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE SymbolType;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE SymbolScope;
    PDEBUG_ENGINE_EXAMINED_SYMBOL Symbol;
    PRTL_CHAR_TO_INTEGER RtlCharToInteger;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    CHAR StackBitmapBuffer[32];
    RTL_BITMAP Bitmap = { 32 << 3, (PULONG)&StackBitmapBuffer };
    PRTL_BITMAP BitmapPointer = &Bitmap;

    //
    // Initialize aliases.
    //

    Session = Output->Session;
    StringTable = Session->ExamineSymbolsPrefixStringTable;
    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;

    MatchIndex = IsPrefixOfStringInTable(StringTable, Line, &Match);

    if (MatchIndex == NO_MATCH_FOUND) {

        //
        // Ignore any lines that don't have an expected prefix.  This will
        // typically be the first line which is an echo of the examine symbols
        // command that was just executed (assuming ExecuteWide() was called
        // with the DEBUG_EXECUTE_ECHO flag set).
        //

        return TRUE;
    }

    //
    // Continue initializing aliases.
    //

    HeapHandle = Output->Allocator->HeapHandle;
    ExaminedSymbolAllocator = Output->CustomStructureAllocator;
    ExaminedSymbolSecondaryAllocator = (
        Output->CustomStructureSecondaryAllocator
    );

    End = Line->Buffer + (Line->Length - 1);
    BytesRemaining = Line->Length - 1;

    //
    // The end of the line should be a line feed character.
    //

    if (*End != '\n') {
        MAYBE_BREAK();
        goto Error;
    }

    Rtl = Session->Rtl;
    RtlCharToInteger = Rtl->RtlCharToInteger;

    SymbolScope = MatchIndex;
    Scope = Match.String;

    Length = Match.NumberOfMatchedCharacters;
    Char = (Line->Buffer + Length);
    BytesRemaining -= Length;

    //
    // Skip over spaces after the scope name.
    //

    while (BytesRemaining > 0 && *Char == ' ') {
        BytesRemaining--;
        Char++;
    }

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // Extract the address, then convert from hex to integer in two parts.
    //

    Address.Length = sizeof("00000000`00000000")-1;
    Address.MaximumLength = Address.Length;
    Address.Buffer = Char;

    if (Address.Buffer[8] != '`') {
        MAYBE_BREAK();
        goto Error;
    }

    Address.Buffer[8] = '\0';
    if (FAILED(RtlCharToInteger(Address.Buffer, 16, &Addr.HighPart))) {
        MAYBE_BREAK();
        goto Error;
    }
    Address.Buffer[8] = '`';

    Address.Buffer[17] = '\0';
    if (FAILED(RtlCharToInteger(Address.Buffer+9, 16, &Addr.LowPart))) {
        MAYBE_BREAK();
        goto Error;
    }
    Address.Buffer[17] = ' ';

    //
    // Advance the character pointer to the first space after the address.
    //

    Char = Address.Buffer + Address.Length;
    BytesRemaining -= Address.Length;

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // Skip over spaces after the address.
    //

    while (BytesRemaining > 0 && *Char == ' ') {
        BytesRemaining--;
        Char++;
    }

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // Symbol size is next.
    //

    if (--BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    SymbolSize.Buffer = Char;

    Char++;

    //
    // Advance to the first space after the symbol size.
    //

    while (BytesRemaining > 0 && *Char != ' ') {
        BytesRemaining--;
        Char++;
    }

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    if (--BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    Char++;

    //
    // Set symbol size lengths, temporarily NUL-terminate the string, convert
    // to an integer, then restore the character replaced.
    //

    SymbolSize.Length = (USHORT)((Char - 1) - SymbolSize.Buffer);
    SymbolSize.MaximumLength = SymbolSize.Length;

    Temp = SymbolSize.Buffer[SymbolSize.Length];

    if (Temp != ' ') {

        //
        // It should always be a space.
        //

        MAYBE_BREAK();
        goto Error;
    }

    SymbolSize.Buffer[SymbolSize.Length] = '\0';
    if (FAILED(Rtl->RtlCharToInteger(SymbolSize.Buffer, 16, &Size))) {
        MAYBE_BREAK();
        goto Error;
    }

    SymbolSize.Buffer[SymbolSize.Length] = Temp;

    //
    // The basic type will be next.  Set up the variable then search the string
    // table for a match.  Set the length to the BytesRemaining for now; as long
    // as it's >= or equal to the basic type length (which it should always be),
    // that will be fine.
    //

    BasicType.Buffer = Char;
    BasicType.Length = (USHORT)BytesRemaining;
    BasicType.MaximumLength = (USHORT)BytesRemaining;

    StringTable = Session->ExamineSymbolsBasicTypeStringTable1;
    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;
    MatchOffset = 0;
    MatchAttempts = 0;
    NumberOfStringTables = Session->NumberOfBasicTypeStringTables;

RetryBasicTypeMatch:

    MatchIndex = IsPrefixOfStringInTable(StringTable, &BasicType, &Match);

    if (MatchIndex == NO_MATCH_FOUND) {
        if (++MatchAttempts >= NumberOfStringTables) {
            UnknownBasicType(&BasicType);
            goto Error;
        }
        StringTable++;
        MatchOffset += MAX_STRING_TABLE_ENTRIES;
        goto RetryBasicTypeMatch;
    }

    //
    // We found a known type.  Proceed with allocation of an examined symbol
    // record.
    //

    Symbol = (PDEBUG_ENGINE_EXAMINED_SYMBOL)(
        ExaminedSymbolAllocator->CallocWithTimestamp(
            ExaminedSymbolAllocator->Context,
            1,
            sizeof(*Symbol),
            &Output->Timestamp.CommandStart
        )
    );

    if (!Symbol) {
        goto Error;
    }

    //
    // Initialize common parts of the structure before doing type-specific
    // processing.
    //

    Symbol->SizeOfStruct = sizeof(*Symbol);

    Symbol->Type = SymbolType = MatchIndex + MatchOffset;
    Symbol->Size = Size;
    Symbol->Scope = SymbolScope;
    Symbol->Output = Output;

    //
    // Define some helper macros.
    //

#define COPY_PSTRING_EX(Name, Source)                                  \
    Symbol->String.##Name##.Length = ##Source##->Length;               \
    Symbol->String.##Name##.MaximumLength = ##Source##->MaximumLength; \
    Symbol->String.##Name##.Buffer = ##Source##->Buffer

#define COPY_PSTRING(Name) COPY_PSTRING_EX(Name, Name)

#define COPY_STRING_EX(Name, Source)                                  \
    Symbol->String.##Name##.Length = ##Source##.Length;               \
    Symbol->String.##Name##.MaximumLength = ##Source##.MaximumLength; \
    Symbol->String.##Name##.Buffer = ##Source##.Buffer

#define COPY_STRING(Name) COPY_STRING_EX(Name, Name)

    COPY_PSTRING(Line);
    COPY_PSTRING(Scope);
    COPY_STRING(Address);

    COPY_STRING_EX(Size, SymbolSize);

    //
    // Copy the address.
    //

    Symbol->Address.QuadPart = Addr.QuadPart;

    //
    // Update our Char pointer past the length of the matched string (the basic
    // type name).
    //

    Char += Match.NumberOfMatchedCharacters;
    BytesRemaining -= Match.NumberOfMatchedCharacters;

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    BasicType.Length = (USHORT)(Char - BasicType.Buffer);
    BasicType.MaximumLength = BasicType.Length;

    COPY_STRING(BasicType);

    if (*Char != ' ') {

        //
        // The basic type name should always be followed by a space.
        //

        MAYBE_BREAK();
        goto Error;
    }

    //
    // Skip past the space.
    //

    if (--BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    Char++;

    //
    // If this is a class, struct or union type, we need to advance the char
    // pointer again to the next space.
    //

    if (SymbolType == ClassType  || SymbolType == UnionType ||
        SymbolType == StructType) {

        TypeName = &Symbol->String.TypeName;
        TypeName->Buffer = Char;

        while (BytesRemaining > 0 && *Char != ' ') {
            BytesRemaining--;
            Char++;
        }

        if (BytesRemaining <= 0) {
            MAYBE_BREAK();
            goto Error;
        }

        TypeName->Length = (USHORT)(Char - TypeName->Buffer);
        TypeName->MaximumLength = TypeName->Length;

        //
        // Skip past the space.
        //

        if (--BytesRemaining <= 0) {
            MAYBE_BREAK();
            goto Error;
        }

        Char++;
    }

    //
    // Pointer types will have an asterisk at this point.  They should be
    // followed by spaces.
    //

    IsFunctionPointer = FALSE;

    if (*Char == '*') {
        Symbol->Flags.IsPointer = TRUE;

        //
        // If we're a function, make a note that this is a function pointer,
        // as we'll handle it differently in the type-specific logic below.
        //

        if (SymbolType == FunctionType) {
            IsFunctionPointer = TRUE;
        }

        //
        // Advance the pointer.
        //

        if (--BytesRemaining <= 0) {
            MAYBE_BREAK();
            goto Error;
        }

        Char++;
    }

    //
    // Advance over any spaces.
    //

    while (BytesRemaining > 0 && *Char == ' ') {
        BytesRemaining--;
        Char++;
    }

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // Make a note of the current buffer position, then advance the pointer
    // to the exclamation point, which delineates the module name from the
    // symbol name.  Once we've found this point, we can scan backwards for
    // the space, which will allow us to extract both the module name and any
    // array information that comes after the symbol type, e.g.:
    //
    //      double [5] python27!bigtens = ...
    //      struct _UNICODE_STRING *[95] Python!ApiSetFilesW ...
    //

    Marker = Char;

    while (BytesRemaining > 0 && *Char != '!') {
        BytesRemaining--;
        Char++;
    }

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // Scan backwards for the space.
    //

    Found = FALSE;

    for (Lookback = Char; Lookback >= Marker - 1; Lookback--) {
        if (*Lookback == ' ') {
            Found = TRUE;
            break;
        }
    }

    if (!Found) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // Invariant check: the next character after *Lookback should not be a
    // space.  (Note that this is an internal invariant check, not the normal
    // aggressive bounds checking we do elsewhere in the routine.)
    //

    NextChar = (Lookback + 1);
    if (*NextChar == ' ') {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // The module name will be from the next character (the character after the
    // space) to the character before our exclamation point, which is where
    // Char will currently be pointing.
    //

    ModuleName = &Symbol->String.ModuleName;
    ModuleName->Buffer = NextChar;
    ModuleName->Length = (USHORT)(Char - NextChar);
    ModuleName->MaximumLength = ModuleName->Length;

    //
    // Invariant check: if marker is past the next character, something has
    // gone wrong.
    //

    if (Marker > NextChar) {

        MAYBE_BREAK();
        goto Error;

    } else if (Marker == NextChar) {

        //
        // If the marker we took earlier matches the next character (the module
        // name), there was no array information present after the type name.
        //

        NOTHING;

    } else {

        //
        // If not, then there's array information present after the type name
        // but before the module name.  Capture that information now.
        //

        Array = &Symbol->String.Array;
        Array->Buffer = Marker;

        //
        // Lookback points to the space before the module name.  Marker points
        // to the first character of the array information.  Deduce the length
        // by subtracting the Marker from Lookback-1 (the -1 to account for the
        // space).
        //

        Array->Length = (USHORT)(Lookback - Marker);
        Array->MaximumLength = Array->Length;

        //
        // Invariant check: one past the array's buffer should match the
        // module name's buffer.
        //

        if (&Array->Buffer[Array->Length+1] != ModuleName->Buffer) {
            MAYBE_BREAK();
            goto Error;
        }

        //
        // Toggle the IsArray flag to true.
        //

        Symbol->Flags.IsArray = TRUE;
    }

    //
    // Char will be positioned on the exclamation point; advance it to the next
    // character, which will be the symbol name.
    //

    if (--BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    Char++;

    //
    // Capture the current position as the start of the symbol name buffer.
    //

    SymbolName = &Symbol->String.SymbolName;
    SymbolName->Buffer = Char;

    //
    // Advance through the symbol name until we get to the next space.  Whilst
    // doing this, check to see if any of the characters are colons; if they
    // are, we use this as an indicator that a C++ type is being referred to.
    //

    while (BytesRemaining > 0 && *Char != ' ') {
        BytesRemaining--;
        if (*(++Char) == ':') {
            Symbol->Flags.IsCpp = TRUE;
        }
    }

    if (BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // The character pointer is now positioned at the space trailing the symbol
    // name; use this to calculate the lengths by subtracting one from it (to
    // account for the space) then subtracting the address of the start of the
    // buffer (SymbolName->Buffer).
    //

    SymbolName->Length = (USHORT)(Char - SymbolName->Buffer);
    SymbolName->MaximumLength = SymbolName->Length;

    //
    // The remaining bytes in the string will be specific to the type of the
    // symbol.  The only types we care about at the moment are functions as
    // the remaining string will capture the function's arguments with types
    // (assuming this is a private function symbol and not a function pointer).
    //

    //
    // Set the remaining bytes string.  The length will be whatever is left in
    // the BytesRemaining counter we've been decrementing for each token we've
    // processed.
    //

    Remaining = &Symbol->String.Remaining;
    Remaining->Buffer = Char;
    Remaining->Length = (USHORT)BytesRemaining;
    Remaining->MaximumLength = (USHORT)BytesRemaining;

    //
    // Make sure the ends line up where we expect.
    //

    if ((Remaining->Buffer + Remaining->Length) != End) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // If this isn't a private function, we're done.
    //

    if (SymbolScope != PrivateFunctionScope) {
        Success = TRUE;
        goto End;
    }

    //
    // The SEH/CLR-type symbols get registered as private functions, too.
    // Skip them as well.
    //

    if (SymbolType == CLRType) {
        Success = TRUE;
        goto End;
    }

    //
    // Sanity check our symbol type is a function too.
    //

    if (SymbolType != FunctionType) {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // The character pointer will be positioned on a space.  The next character
    // should be an opening parenthesis.
    //

    if (--BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    Char++;

    if (*Char != '(') {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // The parenthesis was where we expected, advance the pointer.
    //

    if (--BytesRemaining <= 0) {
        MAYBE_BREAK();
        goto Error;
    }

    Char++;

    //
    // Make a note of the current buffer position, then advance the pointer to
    // the closing parenthesis.
    //

    Marker = Char;

    while (BytesRemaining > 0 && *Char != ')') {
        BytesRemaining--;
        Char++;
    }

    //
    // Invariant check: we should have one byte left here, and char should be
    // positioned on the final ')', which means that the following character
    // should be a line feed.
    //

    if (BytesRemaining != 1 || *(Char + 1) != '\n') {
        MAYBE_BREAK();
        goto Error;
    }

    //
    // Update the function arguments string with the buffer offsets.
    //

    Arguments = &Symbol->String.FunctionArguments;
    Arguments->Buffer = Marker;
    Arguments->Length = (USHORT)((Char - 1) - Arguments->Buffer);
    Arguments->MaximumLength = Arguments->Length;

    //
    // Create a bitmap index of all the commas in the arguments string.
    //

    Success = Rtl->CreateBitmapIndexForString(Rtl,
                                              Arguments,
                                              ',',
                                              &HeapHandle,
                                              &BitmapPointer,
                                              FALSE,
                                              NULL);

    NumberOfArguments = (USHORT)Rtl->RtlNumberOfSetBits(BitmapPointer) + 1;

    if (NumberOfArguments == 1) {

        //
        // No commas were detected.  This is either because there was only one
        // argument, or there were no arguments.  If it's the latter, we'll see
        // a "void" string value.
        //

        PULONG ULongBuffer = (PULONG)Arguments->Buffer;
        PULONG Void = (PULONG)"void";
        BOOL IsVoid = (*(ULongBuffer) == *(Void));

        if (IsVoid) {
            NumberOfArguments = 0;
        }
    }

    //
    // Set the number of arguments and initialize the list head.
    //

    Symbol->Function.NumberOfArguments = NumberOfArguments;
    InitializeListHead(&Symbol->Function.ArgumentsListHead);

    if (NumberOfArguments == 0) {

        //
        // There were no arguments; we're done.
        //

        Success = TRUE;
        goto End;
    }

    //
    // Create a new argument record for each argument.
    //

    Char = Arguments->Buffer;
    Marker = Arguments->Buffer;

    for (Index = 0; Index < NumberOfArguments; Index++) {

        PSTRING ArgumentType;
        PSTRING ArgumentTypeName;
        PDEBUG_ENGINE_FUNCTION_ARGUMENT Argument;

        //
        // Allocate an argument record.
        //

        Argument = (PDEBUG_ENGINE_FUNCTION_ARGUMENT)(
            ExaminedSymbolSecondaryAllocator->CallocWithTimestamp(
                ExaminedSymbolSecondaryAllocator->Context,
                1,
                sizeof(*Argument),
                &Output->Timestamp.CommandStart
            )
        );

        if (!Argument) {
            goto Error;
        }

        //
        // Initialize the argument.
        //

        Argument->ArgumentNumber = Index + 1;
        Argument->Atom = (USHORT)HashAnsiStringToAtom(&Argument->Name);

        //
        // If the argument number is four or less, set register information.
        //

        if (Index + 1 <= 4) {
            Argument->Register.x64 = Index;
            Argument->Flags.InRegister = TRUE;
        }

        //
        // Wire the argument type string structure up to the relevant buffer
        // offset.  We use the Marker to delineate the start of each argument.
        //

        ArgumentType = &Argument->String.ArgumentType;
        ArgumentType->Buffer = Marker;

        //
        // If this is the last element, set our character pointer to the end
        // of the arguments buffer.  Otherwise, advance to the next comma.
        //

        if (Index == NumberOfArguments - 1) {
            Char = Arguments->Buffer + (Arguments->Length + 1);
        } else {
            while (*Char != ',') {
                Char++;
            }
        }

        ArgumentType->Length = (USHORT)(Char - Marker);
        ArgumentType->MaximumLength = ArgumentType->Length;

        //
        // Search for the argument in the function argument string tables.
        // This uses the same algorith as the string table match earlier,
        // slightly tweaked for function arguments.
        //

        MatchOffset = 0;
        MatchAttempts = 0;
        StringTable = Session->FunctionArgumentTypeStringTable1;
        IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;
        NumberOfStringTables = Session->NumberOfFunctionArgumentStringTables;

RetryArgumentMatch:

        MatchIndex = IsPrefixOfStringInTable(StringTable, ArgumentType, &Match);

        if (MatchIndex == NO_MATCH_FOUND) {
            if (++MatchAttempts >= NumberOfStringTables) {
                UnknownArgumentType(ArgumentType);
            } else {
                StringTable++;
                MatchOffset += MAX_STRING_TABLE_ENTRIES;
                goto RetryArgumentMatch;
            }
        }

        InitializeListHead(&Argument->ListEntry);

        AppendTailList(&Symbol->Function.ArgumentsListHead,
                       &Argument->ListEntry);

        //
        // Adjust the character pointer by 2 such that it points to the next
        // symbol name, then set the marker to the same value.
        //

        Char += 2;
        Marker = Char;
    }

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    if (Success) {
        InitializeListHead(&Symbol->ListEntry);
        AppendTailList(&Output->CustomStructureListHead,
                       &Symbol->ListEntry);

        if (++Output->NumberOfCustomStructures == 1) {
            Output->FirstCustomStructure = Symbol;
        }
    }

    if ((ULONG_PTR)Bitmap.Buffer != (ULONG_PTR)StackBitmapBuffer) {
        HeapFree(HeapHandle, 0, Bitmap.Buffer);
    }

    return Success;
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
    BOOL Success;
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
        Success = ExamineSymbolsParseLine(Output, Line);
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
