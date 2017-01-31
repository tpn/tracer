/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    DebugEngineExecuteCommand.c

Abstract:

    This module implements helper functionality for executing command strings
    that will be executed by the debug engine via the IDebugControl->Execute()
    interface.  Routines are provided to execute a build command string, as
    well as output callbacks that marshal the debugger output back to the
    upstream client in a consistent manner using the DEBUG_ENGINE_OUTPUT
    structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
DebugEngineExecuteCommand(
    PDEBUG_ENGINE_OUTPUT Output
    )
/*++

Routine Description:

    Execute a command in the debugger.

Arguments:

    Output - Supplies a pointer to an initialized DEBUG_ENGINE_OUTPUT structure
        to be used for the command.  DebugEngineBuildCommand() must have
        already been called on this structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    HRESULT Result;
    PDEBUG_ENGINE Engine;
    PDEBUG_ENGINE_SESSION Session;
    DEBUG_OUTPUT_MASK OutputMask = { 0 };
    DEBUG_OUTPUT_MASK OldOutputMask = { 0 };

    //
    // Initialize aliases.
    //

    Session = Output->Session;
    Engine = Session->Engine;

    //
    // Initialize our output mask and get the current output mask.  If they
    // differ, update the current one.
    //

    OutputMask.Normal = TRUE;

    CHECKED_HRESULT_MSG(
        Engine->Client->GetOutputMask(
            Engine->IClient,
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
            Engine->Client->SetOutputMask(
                Engine->IClient,
                OutputMask.AsULong
            ),
            "Client->SetOutputMask()"
        );
    }

    //
    // Set our output callbacks and execute the command.
    //

    Output->State.CommandExecuting = TRUE;

    //
    // XXX: temporarily force everything to go through ExecuteWide() whilst
    // evaluating the debugger's behavior.
    //

    if (1) {

        Engine->OutputCallback = DebugEngineOutputCallback;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks(Engine),
            "DebugEngineSetOutputCallbacks()"
        );

        Result = Output->LastResult = Engine->Control->ExecuteWide(
            Engine->IControl,
            DEBUG_OUTCTL_ALL_CLIENTS,
            Output->Command->Buffer,
            DEBUG_EXECUTE_ECHO
        );

        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Control->ExecuteWide() failed: ");
            OutputDebugStringW(Output->Command->Buffer);
            Output->State.Failed = TRUE;
        } else {
            Success = TRUE;
            Output->State.Succeeded = TRUE;
        }

    } else if (Output->OutputFlags.WideCharacterOutput) {

        //
        // We don't support wide output at the moment.
        //

        __debugbreak();
        Engine->OutputCallback2 = DebugEngineOutputCallback2;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks2(Engine),
            "DebugEngineSetOutputCallbacks2()"
        );

        Result = Output->LastResult = Engine->Control->ExecuteWide(
            Engine->IControl,
            DEBUG_OUTCTL_THIS_CLIENT,
            Output->Command->Buffer,
            DEBUG_EXECUTE_NOT_LOGGED
        );

        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Control->ExecuteWide() failed: ");
            OutputDebugStringW(Output->Command->Buffer);
            Output->State.Failed = TRUE;
        } else {
            Success = TRUE;
            Output->State.Succeeded = TRUE;
        }

    } else {

        PSTRING Command;
        PALLOCATOR Allocator;

        Allocator = Output->Allocator;

        if (!ConvertUtf16StringToUtf8String(Output->Command,
                                            &Command,
                                            Allocator)) {
            goto Error;
        }

        Engine->OutputCallback = DebugEngineOutputCallback;
        CHECKED_MSG(
            DebugEngineSetOutputCallbacks(Engine),
            "DebugEngineSetOutputCallbacks()"
        );

        Result = Output->LastResult = Engine->Control->Execute(
            Engine->IControl,
            DEBUG_OUTCTL_ALL_CLIENTS,
            Command->Buffer,
            DEBUG_EXECUTE_ECHO
        );

        if (Result != S_OK) {
            Success = FALSE;
            OutputDebugStringA("Control->Execute() failed: ");
            OutputDebugStringA(Command->Buffer);
            Output->State.Failed = TRUE;
        } else {
            Success = TRUE;
            Output->State.Succeeded = TRUE;
        }

        Allocator->Free(Allocator->Context, Command);
    }

    //
    // Restore the old output mask if we changed it earlier.
    //

    if (OutputMask.AsULong != OldOutputMask.AsULong) {
        CHECKED_HRESULT_MSG(
            Engine->Client->SetOutputMask(
                Engine->IClient,
                OldOutputMask.AsULong
            ),
            "Client->SetOutputMask(Old)"
        );
    }

    //
    // Call the output completion callback.
    //

    Output->State.CommandComplete = TRUE;
    Success = Output->OutputCompleteCallback(Output);

    goto End;

Error:
    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:
    Output->State.CommandExecuting = FALSE;

    return Success;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEngineOutputCallback(
    PDEBUG_ENGINE DebugEngine,
    DEBUG_OUTPUT_MASK OutputMask,
    PCSTR Text
    )
{
    PRTL Rtl;
    ULONG Index;
    BOOL Success;
    PCHAR Dest;
    PSTRING Line;
    PSTRING Chunk;
    STRING NewLine;
    HRESULT Result;
    ULONG BitIndex;
    ULONG LastBitIndex;
    ULONG NumberOfLines;
    PSTRING PartialLine;
    PALLOCATOR Allocator;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEntry;
    HANDLE HeapHandle = NULL;
    PDEBUG_ENGINE_OUTPUT Output;
    ULONG_INTEGER TextSizeInBytes;
    ULONG_INTEGER AllocSizeInBytes;
    ULONG_INTEGER NewLineLengthInBytes;
    PRTL_FIND_SET_BITS FindSetBits;
    PLINKED_PARTIAL_LINE LinkedPartialLine;

    //
    // Reserve a 256 byte (2048 bit/char) stack-allocated bitmap buffer.
    //

    CHAR StackBitmapBuffer[256];
    RTL_BITMAP Bitmap = { 256 << 3, (PULONG)&StackBitmapBuffer };
    PRTL_BITMAP BitmapPointer = &Bitmap;
    PRTL_BITMAP LineEndings;

    //
    // If this isn't normal output, just print it to the debug stream and
    // return.  (We should probably attempt to grok the warning/error messages
    // in the future -- like those involving symbol loading issues.)
    //

    if (!OutputMask.Normal) {
        if (Text) {
            OutputDebugStringA(Text);
        }
        return S_OK;
    }

    //
    // Resolve the Output structure and set our state to indicate we're in a
    // partial callback.
    //

    Output = DebugEngine->CurrentOutput;
    Output->State.InPartialOutputCallback = TRUE;

    //
    // Calculate the size in bytes of the incoming text.
    //

    TextSizeInBytes.LongPart = (LONG)strlen(Text);

    //
    // Sanity check it's not over MAX_USHORT (which would prevent it from
    // fitting in our STRING structure).
    //

    if (TextSizeInBytes.HighPart) {
        __debugbreak();
        goto Error;
    }

    //
    // Do we ever get empty lines?
    //

    if (!TextSizeInBytes.LowPart) {
        __debugbreak();
        goto Error;
    }

    //
    // Capture the details about this chunk.
    //

    Chunk = &Output->Chunk;
    Chunk->Length = TextSizeInBytes.LowPart;
    Chunk->MaximumLength = TextSizeInBytes.LowPart;
    Chunk->Buffer = (PSTR)Text;

    if (TextSizeInBytes.LongPart > Output->LargestChunkSizeInBytes) {

        //
        // This new chunk is the largest we've seen.  Make a note.
        //

        Output->LargestChunkSizeInBytes = TextSizeInBytes.LongPart;
    }

    //
    // Update our partial callback counter and totals counters.
    //

    Output->NumberOfPartialCallbacks++;
    Output->TotalBufferLengthInChars += TextSizeInBytes.LowPart;
    Output->TotalBufferSizeInBytes += TextSizeInBytes.LowPart;

    //
    // Invoke the relevant callbacks based on the caller's output flags.
    //

    Result = S_OK;

    if (Output->Flags.EnablePartialOutputCallbacks) {
        Success = Output->PartialOutputCallback(Output);
        if (!Success) {
            goto Error;
        }
    }

    if (!Output->Flags.EnableLineOutputCallbacks) {
        goto End;
    }

    //
    // The caller wants line-oriented callbacks.
    //

    //
    // Initialize aliases.
    //

    Rtl = DebugEngine->Rtl;
    FindSetBits = Rtl->RtlFindSetBits;
    Allocator = Output->Allocator;

    //
    // Create a bitmap of line endings.
    //

    Success = Rtl->CreateBitmapIndexForString(Rtl,
                                              Chunk,
                                              '\n',
                                              &HeapHandle,
                                              &BitmapPointer,
                                              FALSE,
                                              NULL);

    if (!Success) {
        goto Error;
    }

    //
    // Determine the number of lines in the output based on the number of
    // bits in the bitmap.
    //

    NumberOfLines = Rtl->RtlNumberOfSetBits(BitmapPointer);
    if (NumberOfLines == 0) {

        //
        // There were no lines in this chunk.  Save this partial line and
        // finish up.
        //

        AllocSizeInBytes.LongPart = (

            //
            // Account for the size of the STRING structure.
            //

            sizeof(LINKED_PARTIAL_LINE) +

            //
            // Account for the actual backing string buffer.
            //

            TextSizeInBytes.LongPart

        );

        //
        // Sanity check we're under MAX_USHORT.
        //

        if (AllocSizeInBytes.HighPart) {
            __debugbreak();
            goto Error;
        }

        //
        // Attempt to allocate space for the STRING + buffer.
        //

        LinkedPartialLine = (PLINKED_PARTIAL_LINE)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                AllocSizeInBytes.LongPart
            )
        );

        if (!LinkedPartialLine) {
            goto Error;
        }

        //
        // Initialize the list entry.
        //

        InitializeListHead(&LinkedPartialLine->ListEntry);

        //
        // Carve out the line and then buffer pointer from allocated memory.
        //

        Line = &LinkedPartialLine->PartialLine;
        Line->Buffer = (PCHAR)(
            RtlOffsetToPointer(
                LinkedPartialLine,
                sizeof(LINKED_PARTIAL_LINE)
            )
        );

        //
        // Set the lengths.
        //

        Line->Length = TextSizeInBytes.LowPart;
        Line->MaximumLength = TextSizeInBytes.LowPart;

        //
        // Append the linked line to the output list head and increment the
        // partial line counter.
        //

        AppendTailList(&Output->PartialLinesListHead,
                       &LinkedPartialLine->ListEntry);

        Output->NumberOfPartialLines++;

        goto End;
    }

    //
    // We have at least one line to process if we reach this point.
    //

    //
    // Initialize variables prior to the loop.
    //

    BitIndex = 0;
    LastBitIndex = 0;
    Line = &Output->Line;
    LineEndings = BitmapPointer;

    //
    // Extract the first line.
    //

    BitIndex = FindSetBits(LineEndings, 1, LastBitIndex);
    if (BitIndex == BITS_NOT_FOUND || BitIndex < LastBitIndex) {

        //
        // We shouldn't ever hit this on the first line.
        //

        __debugbreak();
    }

    //
    // Fill in the first line details.
    //

    Line->Length = (USHORT)(BitIndex - (LastBitIndex-1));
    Line->MaximumLength = Line->Length;
    Line->Buffer = (PCHAR)(Text + LastBitIndex);

    //
    // If there are no partial lines recorded, dispatch this line output
    // callback then jump to the processing of remaining lines.
    //

    if (!Output->NumberOfPartialLines) {
        Success = Output->LineOutputCallback(Output);
        if (!Success) {
            goto Error;
        }
        goto ProcessLines;
    }

    //
    // Calculate the size of all the partial lines, allocate a new buffer,
    // copy all of them over, freeing the strings + buffers as we go, call
    // the caller's line output callback with the final line, then free it
    // too.
    //

    NewLineLengthInBytes.LongPart = 0;

    ListHead = &Output->PartialLinesListHead;

    FOR_EACH_LIST_ENTRY(ListHead, ListEntry) {

        LinkedPartialLine = CONTAINING_RECORD(ListEntry,
                                              LINKED_PARTIAL_LINE,
                                              ListEntry);

        NewLineLengthInBytes.LongPart += LinkedPartialLine->PartialLine.Length;
    }

    //
    // Add the length of this most recent line.
    //

    NewLineLengthInBytes.LongPart += Line->Length;

    //
    // Align up to a pointer boundary.
    //

    AllocSizeInBytes.LongPart = ALIGN_UP_POINTER(NewLineLengthInBytes.LongPart);

    //
    // Sanity check the size doesn't exceed MAX_USHORT.
    //

    if (AllocSizeInBytes.HighPart) {
        __debugbreak();
        goto Error;
    }

    //
    // Attempt to allocate a buffer.
    //

    NewLine.Buffer = (PCHAR)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSizeInBytes.LongPart
        )
    );

    if (!NewLine.Buffer) {
        goto Error;
    }

    //
    // Allocation succeeded, copy each partial string, then free it.
    //

    Dest = NewLine.Buffer;

    while (!IsListEmpty(ListHead)) {

        ListEntry = RemoveHeadList(ListHead);

        LinkedPartialLine = CONTAINING_RECORD(ListEntry,
                                              LINKED_PARTIAL_LINE,
                                              ListEntry);

        PartialLine = &LinkedPartialLine->PartialLine;
        __movsb(Dest, PartialLine->Buffer, PartialLine->Length);
        Dest += PartialLine->Length;
        Allocator->Free(Allocator->Context, LinkedPartialLine);
        Output->NumberOfPartialLines--;
    }

    //
    // Sanity check invariants: number of partial lines should be 0 and our
    // list head should be empty.
    //

    if (Output->NumberOfPartialLines) {
        __debugbreak();
    }

    if (!IsListEmpty(ListHead)) {
        __debugbreak();
    }

    //
    // Copy the contents of the latest line.
    //

    __movsb(Dest, Line->Buffer, Line->Length);

    //
    // Update lengths and invoke the caller's line output callback.
    //

    NewLine.Length = NewLineLengthInBytes.LowPart;
    NewLine.MaximumLength = AllocSizeInBytes.LowPart;

    Success = Output->LineOutputCallback(Output);

    //
    // Free the temporary string buffer we allocated.
    //

    Allocator->Free(Allocator->Context, NewLine.Buffer);
    if (!Success) {
        goto Error;
    }

    //
    // Intentional follow-on to ProcessLines.
    //

ProcessLines:

    //
    // Enumerate the line ending bitmap and isolate each line.  Note that
    // the Index starts at 1 as we've already processed the first line
    // ending.
    //

    for (Index = 1; Index < NumberOfLines; Index++) {
        BitIndex = FindSetBits(LineEndings, 1, LastBitIndex);
        if (BitIndex == BITS_NOT_FOUND || BitIndex < LastBitIndex) {
            break;
        }
        Line->Length = (USHORT)(BitIndex - (LastBitIndex-1));
        Line->MaximumLength = Line->Length;
        Line->Buffer = (PCHAR)(Text + LastBitIndex);
        LastBitIndex = BitIndex + 1;

        //
        // Invoke the caller's line output callback.
        //

        if (!Output->LineOutputCallback(Output)) {
            goto Error;
        }
    }

    goto End;

Error:
    Result = E_FAIL;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Perform heap/bitmap cleanup.
    //

    if (HeapHandle) {

        if ((ULONG_PTR)Bitmap.Buffer == (ULONG_PTR)BitmapPointer->Buffer) {
            __debugbreak();
        }

        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);
    }

    Output->State.InPartialOutputCallback = FALSE;
    return Result;
}

_Use_decl_annotations_
HRESULT
STDAPICALLTYPE
DebugEngineOutputCallback2(
    PDEBUG_ENGINE DebugEngine,
    DEBUG_OUTPUT_TYPE OutputType,
    DEBUG_OUTPUT_CALLBACK_FLAGS OutputFlags,
    ULONG64 Arg,
    PCWSTR IncomingText
    )
{
    return E_FAIL;
}



// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
