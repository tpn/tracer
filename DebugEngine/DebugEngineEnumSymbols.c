/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    EnumSymbols.c

Abstract:

    This module implements functionality related to enumerating symbols via
    DbgEng.dll's `x` (examine symbols) command.

--*/

#include "stdafx.h"

//
// Define output line prefix related variables and inline functions.  We
// set an 8 byte prefix, which allows us to cast the first 8 bytes of the
// incoming text as a ULARGE_INTEGER and compare as two ULONGs.
//

CONST STRING EnumSymbolsPrefix = RTL_CONSTANT_STRING(":mySmunE");
CONST UNICODE_STRING EnumSymbolsPrefixWide = RTL_CONSTANT_STRING(L"mySE");

FORCEINLINE
BOOL
IsEnumSymbolsPrefix(
    _In_ PCSTR String
    )
{
    ULARGE_INTEGER ExpectedPrefix;
    ULARGE_INTEGER ActualPrefix;
    PSTR Buffer;

    Buffer = EnumSymbolsPrefix.Buffer;

    ActualPrefix.LowPart = *((PULONG)String);
    ActualPrefix.HighPart = *((PULONG)(String + sizeof(ULONG)));

    ExpectedPrefix.LowPart = *((PULONG)Buffer);
    ExpectedPrefix.HighPart = *((PULONG)(Buffer + sizeof(ULONG)));

    return (ExpectedPrefix.QuadPart == ActualPrefix.QuadPart);
}

FORCEINLINE
BOOL
IsEnumSymbolsPrefixWide(
    _In_ PCWSTR String
    )
{
    ULARGE_INTEGER ExpectedPrefix;
    ULARGE_INTEGER ActualPrefix;
    PWSTR Buffer;

    Buffer = EnumSymbolsPrefixWide.Buffer;

    ActualPrefix.LowPart = *((PULONG)String);
    ActualPrefix.HighPart = *((PULONG)(String + (sizeof(ULONG) >> 1)));

    ExpectedPrefix.LowPart = *((PULONG)Buffer);
    ExpectedPrefix.HighPart = *((PULONG)(Buffer + (sizeof(ULONG) >> 1)));

    return (ExpectedPrefix.QuadPart == ActualPrefix.QuadPart);
}

//
// Functions.
//

_Use_decl_annotations_
BOOL
DebugEngineEnumSymbols(
    PDEBUG_ENGINE_SESSION DebugEngineSession,
    PVOID Context,
    PALLOCATOR Allocator,
    PDEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK Callback,
    DEBUG_ENGINE_ENUM_SYMBOLS_FLAGS Flags,
    PRTL_PATH ModulePath
    )
/*++

Routine Description:

    Enumerate symbols in a module via DbgEng.dll's `x` (examine symbols)
    function.

Arguments:

    DebugEngineSession - Supplies a pointer to a DEBUG_ENGINE_SESSION structure.

    Context - Optionally supplies an opaque pointer to a context that will be
        passed back to the caller when the callback is invoked.

    Allocator - Supplies a pointer to an ALLOCATOR structure to be used for any
        memory allocation required by the routine.

    Callback - Supplies a function pointer to a callback that will be invoked
        for each symbol found.

    Flags - Supplies an instance of DEBUG_ENGINE_ENUM_SYMBOLS_FLAGS that can
        be used to customize the enumeration of symbols.

    ModulePath - Supplies a pointer to an RTL_PATH structure for the module
        for which symbols are to be enumerated.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    BOOL AcquiredLock = FALSE;
    BOOL OversizedCommand;
    HRESULT Result;
    ULONG ModuleStartIndex;
    ULONG ModuleIndex;
    ULONGLONG ModuleBaseAddress;
    USHORT StackBufferSizeInBytes;
    USHORT ModuleNameLengthInChars;
    ULONG_INTEGER CommandLengthInChars;
    ULONG_INTEGER CommandLengthInBytes;
    PDEBUG_ENGINE DebugEngine;
    UNICODE_STRING CommandWide;
    WCHAR CommandWideStackBuffer[256];
    STRING Command;
    CHAR CommandStackBuffer[256];
    PWSTR Dest;
    PWSTR Buffer;
    ULONG64 OutputLinePrefixHandle = 0;
    DEBUG_OUTPUT_MASK OutputMask = { 0 };
    DEBUG_OUTPUT_MASK OldOutputMask = { 0 };
    DEBUG_ENGINE_ENUM_SYMBOLS_OUTPUT_CALLBACK_CONTEXT OutputContext = { 0 };

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(DebugEngineSession)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Callback)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ModulePath)) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    DebugEngine = DebugEngineSession->Engine;

    //
    // Calculate the module name length in characters by shifting the Length
    // (which is in bytes) right once.
    //

    ModuleNameLengthInChars = (ModulePath->Name.Length >> 1);

    //
    // If the module has an extension (e.g. ".dll"), omit its length.
    //

    if (ModulePath->Extension.Length) {

        //
        // Add a sizeof(WCHAR) to account for the '.', then shift right to
        // to convert from bytes into character length and subtract from the
        // current module name length.
        //

        ModuleNameLengthInChars -= (
            (ModulePath->Extension.Length + sizeof(WCHAR)) >> 1
        );
    }

    //
    // Calculate the length in characters of the command string.
    //

    CommandLengthInChars.LongPart = (

        //
        // Account for the initial "x ".
        //

        2 +

        //
        // Account for the "/v " if applicable.
        //

        (Flags.Verbose ? 3 : 0) +

        //
        // Account for the "/t " if applicable.
        //

        (Flags.TypeInformation ? 3 : 0) +

        //
        // Account for the name of the module.
        //

        ModuleNameLengthInChars +

        //
        // Account for the final "!*" plus trailing NULL.
        //

        3
    );

    //
    // Convert to length in bytes by shifting left once.
    //

    CommandLengthInBytes.LongPart = CommandLengthInChars.LowPart << 1;

    //
    // Sanity check the size hasn't exceeded MAX_USHORT.
    //

    if (CommandLengthInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Clear the Command.Buffer pointer.
    //

    Command.Buffer = NULL;

    //
    // Determine if we can fit the string in our stack buffer.
    //

    StackBufferSizeInBytes = sizeof(CommandStackBuffer);

    if (CommandLengthInBytes.LowPart <= StackBufferSizeInBytes) {

        //
        // Command string fits inside our stack allocated buffer, so no separate
        // allocation is necessary.
        //

        CommandWide.Buffer = CommandWideStackBuffer;
        OversizedCommand = FALSE;

    } else {

        //
        // Command length exceeds the size of our stack buffer, so allocate
        // a new buffer using the provided allocator.
        //

        CommandWide.Buffer = (PWCHAR)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                CommandLengthInBytes.LowPart
            )
        );

        if (!CommandWide.Buffer) {
            goto Error;
        }

        OversizedCommand = TRUE;
    }

    //
    // Initialize lengths.
    //

    CommandWide.Length = 0;
    CommandWide.MaximumLength = CommandLengthInBytes.LowPart;

    //
    // Initialize the command buffer.
    //

    Dest = Buffer = CommandWide.Buffer;

    *Dest++ = L'x';
    *Dest++ = L' ';

    if (Flags.Verbose) {
        *Dest++ = L'/';
        *Dest++ = L'v';
        *Dest++ = L' ';
    }

    if (Flags.TypeInformation) {
        *Dest++ = L'/';
        *Dest++ = L't';
        *Dest++ = L' ';
    }

    //
    // Copy the module name over.
    //

    __movsw((PWORD)Dest,
            (PWORD)ModulePath->Name.Buffer,
            ModuleNameLengthInChars);

    //
    // Zero the word after the module name in order to NULL terminate the
    // string before we pass it to GetModuleByModuleNameWide().
    //

    *(Dest + ModuleNameLengthInChars) = L'\0';

    //
    // Attempt to get the index and base address for this module.
    //

    ModuleStartIndex = 0;
    Result = DebugEngine->Symbols->GetModuleByModuleNameWide(
        DebugEngine->ISymbols,
        Dest,
        ModuleStartIndex,
        &ModuleIndex,
        &ModuleBaseAddress
    );

    if (Result != S_OK) {
        OutputDebugStringA("Failed: Symbols->GetModuleByModuleNameWide().\n");
        goto Error;
    }

    //
    // Continue construction of the command; append the final '!*' and
    // trailing NULL.
    //

    Dest += ModuleNameLengthInChars;

    *Dest++ = L'!';
    *Dest++ = L'*';
    *Dest = L'\0';

    CommandWide.Length = (USHORT)(((ULONG_PTR)Dest) - ((ULONG_PTR)Buffer));

    //
    // Sanity check things are where they should be.
    //

    if (CommandWide.Buffer[(CommandWide.Length >> 1)-1] != L'*') {
        __debugbreak();
    }

    if (CommandWide.Buffer[CommandWide.Length >> 1] != L'\0') {
        __debugbreak();
    }

    //
    // Initialize our output callback context.
    //

    OutputContext.SizeOfStruct = sizeof(OutputContext);
    OutputContext.OutputCallback = DebugEngineEnumSymbolsOutputCallback;
    OutputContext.Allocator = Allocator;
    OutputContext.CallerContext = Context;
    OutputContext.CallerCallback = Callback;
    OutputContext.ModulePath = ModulePath;

    if (Flags.WideCharacter) {
        OutputContext.CommandWide = &CommandWide;
        OutputContext.Flags.WideCharacter = TRUE;
    } else {

        USHORT Index;
        WIDE_CHARACTER WideChar;

        //
        // The caller doesn't want wide character output, so convert the
        // command into a normal char representation so that we can call
        // Execute() instead of ExecuteWide().
        //

        if (!OversizedCommand) {

            //
            // We can use the stack-allocated buffer.
            //

            Command.Buffer = CommandStackBuffer;

        } else {

            //
            // We can't use the stack-allocated command buffer, so allocate
            // a new one.  We can use the command length in characters as the
            // actual byte size given that it's a normal character string.
            //

            Command.Buffer = (PCHAR)(
                Allocator->Calloc(
                    Allocator->Context,
                    1,
                    CommandLengthInChars.LowPart
                )
            );

            if (!Command.Buffer) {
                goto Error;
            }
        }

        //
        // Initialize lengths.  Subtract sizeof(CHAR) from MaximumLength to
        // account for the trailing NULL.
        //

        Command.Length = 0;
        Command.MaximumLength = CommandLengthInChars.LowPart;

        //
        // Do a poor man's wchar->char conversion.  (I don't know how Unicode
        // characters would be handled by DbgEng; or rather, I'm making an
        // assumption that our RTL_PATHs point to modules with no non-ANSI
        // characters in their name part.  Put a __debugbreak() in to trap
        // any actual wide characters (i.e. those with a non-zero high byte).)
        //

        for (Index = 0; Index < CommandLengthInChars.LowPart; Index++) {
            WideChar.WidePart = CommandWide.Buffer[Index];
            if (WideChar.HighPart) {
                __debugbreak();
            }
            Command.Buffer[Index] = WideChar.LowPart;
        }

        Command.Length = Index - 1;
        Command.MaximumLength = Index;

        //
        // Sanity check things are where they should be.
        //

        if (Command.Buffer[Command.Length-1] != '*') {
            __debugbreak();
        }

        if (Command.Buffer[Command.Length] != '\0') {
            __debugbreak();
        }

        OutputContext.Flags.WideCharacter = FALSE;
    }

    //
    // Acquire the engine lock and set our output callback and context.
    //

    AcquireDebugEngineLock(DebugEngine);
    AcquiredLock = TRUE;
    DebugEngine->OutputCallbackContext.EnumSymbols = &OutputContext;

    //
    // Initialize our output mask and get the current output mask.  If they
    // differ, update the current one.
    //

    OutputMask.Normal = TRUE;

    CHECKED_HRESULT_MSG(
        DebugEngine->Client->GetOutputMask(
            DebugEngine->IClient,
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
            DebugEngine->Client->SetOutputMask(
                DebugEngine->IClient,
                OutputMask.AsULong
            ),
            "Client->SetOutputMask()"
        );
    }

    //
    // Push our "EnumSym:" output prefix, execute the command, pop our prefix
    // and potentially restore the output mask.
    //

    if (Flags.WideCharacter) {

        DebugEngine->OutputCallback2 = DebugEngineEnumSymbolsOutputCallback2;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks2(DebugEngine),
            "DebugEngineSetOutputCallbacks2()"
        );

        CHECKED_HRESULT_MSG(
            DebugEngine->Client->PushOutputLinePrefixWide(
                DebugEngine->IClient,
                EnumSymbolsPrefixWide.Buffer,
                &OutputLinePrefixHandle
            ),
            "Client->PushOutputLinePrefixWide()"
        );

        CHECKED_HRESULT_MSG(
            DebugEngine->Control->ExecuteWide(
                DebugEngine->IControl,
                DEBUG_OUTCTL_THIS_CLIENT,
                CommandWide.Buffer,
                DEBUG_EXECUTE_NOT_LOGGED
            ),
            "Control->ExecuteWide()"
        );

    } else {

        DebugEngine->OutputCallback = DebugEngineEnumSymbolsOutputCallback;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks(DebugEngine),
            "DebugEngineSetOutputCallbacks()"
        );

        CHECKED_HRESULT_MSG(
            DebugEngine->Client->PushOutputLinePrefix(
                DebugEngine->IClient,
                EnumSymbolsPrefix.Buffer,
                &OutputLinePrefixHandle
            ),
            "Client->PushOutputLinePrefix()"
        );

        Result = DebugEngine->Control->Execute(
            DebugEngine->IControl,
            DEBUG_OUTCTL_ALL_CLIENTS,
            Command.Buffer,
            DEBUG_EXECUTE_NOT_LOGGED
        );

        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Control->Execute() failed: ");
            OutputDebugStringA(Command.Buffer);
        } else {
            Success = TRUE;
        }

    }

    //
    // Pop the output line prefix.
    //

    Result = DebugEngine->Client->PopOutputLinePrefix(
        DebugEngine->IClient,
        OutputLinePrefixHandle
    );

    if (Result != S_OK) {
        Success = FALSE;
        OutputDebugStringA("Client->PopOutputLinePrefix() failed.\n");
    } else {
        OutputLinePrefixHandle = 0;
    }

    //
    // Restore the old output mask if we changed it earlier.
    //

    if (OutputMask.AsULong != OldOutputMask.AsULong) {
        Result = DebugEngine->Client->SetOutputMask(
            DebugEngine->IClient,
            OldOutputMask.AsULong
        );
        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Client->SetOutputMask(Old) failed.\n");
        }
    }

    goto End;

Error:
    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // If the command's buffer doesn't match the address of our local stack
    // allocated buffer, it means we allocated it via the Allocator, so, free
    // it now.
    //

    if (CommandWide.Buffer && CommandWide.Buffer != CommandWideStackBuffer) {
        Allocator->Free(Allocator->Context, CommandWide.Buffer);
    }

    //
    // Ditto for the non-wide version.
    //

    if (Command.Buffer && Command.Buffer != CommandStackBuffer) {
        Allocator->Free(Allocator->Context, Command.Buffer);
    }

    if (AcquiredLock) {
        ReleaseDebugEngineLock(DebugEngine);
    }

    return Success;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEngineEnumSymbolsOutputCallback(
    PDEBUG_ENGINE DebugEngine,
    DEBUG_OUTPUT_MASK OutputMask,
    PCSTR IncomingText
    )
{
    BOOL Success;
    HRESULT Result;
    PALLOCATOR Allocator;
    LONG_INTEGER TextSizeInBytes;
    PDEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK Callback;
    PDEBUG_ENGINE_ENUM_SYMBOLS_OUTPUT_CALLBACK_CONTEXT Context;
    DEBUG_ENGINE_SYMBOL Symbol = { 0 };
    PSTR Text;

    Context = DebugEngine->OutputCallbackContext.EnumSymbols;
    Callback = Context->CallerCallback;
    Allocator = Context->Allocator;

    if (!OutputMask.Normal) {
        return S_OK;
    }

    if (0) {
        if (!IsEnumSymbolsPrefix(IncomingText)) {
            return S_OK;
        }
        Text = (PSTR)(IncomingText + sizeof(ULARGE_INTEGER));
    } else {
        Text = (PSTR)IncomingText;
    }

    TextSizeInBytes.LongPart = (LONG)strlen(Text);

    if (TextSizeInBytes.HighPart) {
        __debugbreak();
        goto Error;
    }

    Symbol.SizeOfStruct = sizeof(Symbol);
    Symbol.RawText.Length = TextSizeInBytes.LowPart;
    Symbol.RawText.MaximumLength = TextSizeInBytes.LowPart;
    Symbol.RawText.Buffer = (PSTR)Text;

    Success = Callback(&Symbol, Context->CallerContext);
    if (!Success) {
        goto Error;
    }

    Result = S_OK;
    goto End;

Error:
    Result = E_FAIL;

End:
    return Result;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEngineEnumSymbolsOutputCallback2(
    PDEBUG_ENGINE DebugEngine,
    DEBUG_OUTPUT_TYPE OutputType,
    DEBUG_OUTPUT_CALLBACK_FLAGS OutputFlags,
    ULONG64 Arg,
    PCWSTR IncomingText
    )
{
    BOOL Success;
    HRESULT Result;
    LONG_INTEGER TextLengthInChars;
    LONG_INTEGER TextSizeInBytes;
    PALLOCATOR Allocator;
    PDEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK Callback;
    PDEBUG_ENGINE_ENUM_SYMBOLS_OUTPUT_CALLBACK_CONTEXT Context;
    DEBUG_ENGINE_SYMBOL Symbol = { 0 };
    PWSTR Text;

    if (!IsTextOutput(OutputType)) {
        return S_OK;
    }

    if (!IsEnumSymbolsPrefixWide(IncomingText)) {
        return S_OK;
    }

    Text = (PWSTR)(IncomingText + (sizeof(ULARGE_INTEGER) >> 1));

    Context = DebugEngine->OutputCallbackContext.EnumSymbols;
    Callback = Context->CallerCallback;
    Allocator = Context->Allocator;

    TextLengthInChars.LongPart = (LONG)wcslen(Text);

    if (TextLengthInChars.HighPart) {
        __debugbreak();
    }

    TextSizeInBytes.LongPart = TextLengthInChars.LongPart << 1;

    if (TextSizeInBytes.HighPart) {
        __debugbreak();
        goto Error;
    }

    Symbol.SizeOfStruct = sizeof(Symbol);
    Symbol.RawTextWide.Length = TextSizeInBytes.LowPart;
    Symbol.RawTextWide.MaximumLength = TextSizeInBytes.LowPart;
    Symbol.RawTextWide.Buffer = (PWSTR)Text;

    Success = Callback(&Symbol, Context->CallerContext);
    if (!Success) {
        goto Error;
    }

    Result = S_OK;
    goto End;

Error:
    Result = E_FAIL;

End:
    return Result;
}



// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
