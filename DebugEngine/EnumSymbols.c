/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    EnumSymbols.c

Abstract:

    This module implements functionlity related to enumerating symbols via
    DbgEng.dll's `x` (examine symbols) command.

--*/

#include "stdafx.h"

//
// IDebugOutputClients: 4bf58045-d654-4c40-b0af-683090f356dc
//

DEFINE_GUID_EX(IID_IDebugOutputCallbacksEx, 0x4bf58045, 0xd654, 0x4c40,
               0xb0, 0xaf, 0x68, 0x30, 0x90, 0xf3, 0x56, 0xdc);

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
    BOOL OversizedCommand;
    USHORT StackBufferSizeInBytes;
    USHORT ModuleNameLengthInChars;
    ULONG_INTEGER CommandLengthInChars;
    ULONG_INTEGER CommandLengthInBytes;
    ULONG_INTEGER AllocSizeInBytes;
    PDEBUG_ENGINE DebugEngine;
    UNICODE_STRING CommandWide;
    WCHAR CommandWideStackBuffer[256];
    STRING Command;
    CHAR CommandStackBuffer[256];
    PWSTR Dest;
    PWSTR Source;
    PWSTR Buffer;
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

    DebugEngine = &DebugEngineSession->Engine;

    //
    // Calculate the module name length in characters by shifting the Length
    // (which is in bytes) right once.
    //

    ModuleNameLengthInChars = (ModulePath->Name.Length >> 1);

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

    Dest += ModuleNameLengthInChars;

    //
    // Append the final '!*' and trailing NULL.
    //

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
        USHORT Length;
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
        // characters in their name part.)
        //

        for (Index = 0; Index < CommandLengthInChars.Length; Index++) {
            WideChar = CommandWide.Buffer[Index];
            if (WideChar.HighPart) {
                __debugbreak();
            }
            Command.Buffer[Index] = WideChar.LowPart;
        }

        Command.Length = Index;
        Command.MaximumLength = Index + 1;

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

    DebugEngine->OutputCallbackContext.EnumSymbols = OutputContext;

    //
    // Execute the command.
    //

    Result = DebugEngine->Control->ExecuteWide(
        DebugEngine->IControl,
        DEBUG_OUTCTL_THIS_CLIENT,
        Command.Buffer,
        DEBUG_EXECUTE_NOT_LOGGED
    );

    ReleaseDebugEngineLock(DebugEngine);

    if (Result != S_OK) {
        __debugbreak();
        goto Error;
    }

    Success = TRUE;
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

    return Success;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEngineEnumSymbolsOutputCallback(
    PDEBUG_ENGINE DebugEngine,
    DEBUG_OUTPUT_TYPE OutputType,
    DEBUG_OUTPUT_CALLBACK_FLAGS OutputFlags,
    ULONG64 Arg,
    PCWSTR Text
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

    if (!Text) {
        Result = S_OK;
        goto End;
    }

    Context = DebugEngine->OutputCallbackContext.EnumSymbols;
    Callback = Context->CallerCallback;
    Allocator = Context->Allocator;

    TextLengthInChars.LongPart = wcslen(Text);
    TextSizeInBytes.LongPart = TextLengthInChars.LongPart << 1;

    if (TextSizeInBytes.HighPart) {
        __debugbreak();
        goto Error;
    }

    Symbol.SizeOfStruct = sizeof(Symbol);
    Symbol.Raw.Length = TextSizeInBytes.LowPart;
    Symbol.Raw.MaximumLength = TextSizeInBytes.LowPart;
    Symbol.Raw.Buffer = Text;

    Success = Callback(&Symbol, Context->CallerContext);
    if (!Success) {
        goto Error;
    }

Error:
    Result = E_FAIL;

End:
    return Result;
}



// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
