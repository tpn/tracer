/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Injection.c

Abstract:

    This module implements the Rtl component's remote thread and code injection
    component.  Functions are provided for creating injection packets, adding
    injection payloads to packets, adding symbol requests to packets, destroying
    packets, and performing the actual injection.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
RtlInject(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PRTL_INJECTION_FLAGS InjectionFlags,
    PCUNICODE_STRING ModulePath,
    PCSTRING CallbackFunctionName,
    PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteCallback,
    PRTL_INJECTION_PAYLOAD InjectionPayload,
    ULONG TargetProcessId,
    PRTL_INJECTION_PACKET *InjectionPacketPointer,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Creates an injection packet as part of the first step of code injection
    into a remote process.  The caller must indicate what type of injection
    they want via the Flags parameter.  Specifically, either Flags.InjectCode
    or Flags.InjectModule must be set.  This affects the parameter requirements
    as discussed in the Arguments section below.

    Once an injection packet has been successfully created, a caller can add
    payloads to the packet via RtlAddInjectionPayload(), and request symbols
    to be made available to the remote thread via RtlAddInjectionSymbols().

    The actual injection will be performed when the packet is passed to the
    RtlInject() routine.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure to use for all
        memory allocations for this injection.

    InjectionFlags - Supplies a pointer to injection flags.  At minimum, either
        InjectionFlags.InjectModule or InjectionFlags.InjectCode must be set,
        indicating which type of injection method should be used.

    ModulePath - Supplies a pointer to a fully-qualified UNICODE_STRING
        structure representing the module path to load into the remote process.
        If InjectionFlags.InjectModule is set, both this value and the parameter
        CallbackFunctionName must be valid.

    CallbackFunctionName - Supplies a pointer to a STRING structure representing
        the name of an exported symbol to resolve once the module indicated by
        the ModulePath parameter is loaded.  This must be non-NULL and point to
        a valid string if InjectionFlags.InjectModule is TRUE, otherwise it must
        be NULL.

    InjectionCompleteCallback - Supplies a pointer to a buffer of code to be
        injected into the process and then executed by the target thread.  This
        must be NULL if Flags.InjectModule is set, otherwise, it should point to
        a valid buffer that contains AMD64 byte code.

    InjectionPayload - Supplies a pointer to an RTL_INJECTION_PAYLOAD structure
        to include in the remote process injection.  This must be non-NULL and
        point to a valid structure if InjectionFlags.InjectPayload is TRUE, NULL
        otherwise..

    TargetProcessId - Supplies the ID of the target process to perform the
        injection.

    TargetThreadId - Optionally supplies the ID of an existing thread in the
        target process to hijack instead of creating a new remote thread.  If
        Flags.HijackExistingThread is set, this parameter must be non-zero,
        otherwise, it must be zero.

    InjectionPacketPointer - Supplies a pointer to a variable that will receive
        the address of the newly-created RTL_INJECTION_PACKET if this routine
        completes successfully.  The packet can then have payload or symbols
        added to it before being injected.

    InjectionError - Supplies a pointer to a variable that receives information
        about the error, if one occurred (as indicated by this routine returning
        FALSE).

Return Value:

    If the routine completes successfully, TRUE is returned.  If a failure
    occurs, FALSE is returned and InjectionError is set with the relevant
    error code.

--*/
{
    BOOL Success;
    BOOL SuspendedThread;
    PBYTE OriginalCode;
    PBYTE FinalCode;
    FARPROC Proc;
    HMODULE Module;
    HANDLE ProcessHandle;
    RTL_INJECTION_ERROR Error;
    RTL_INJECTION_FLAGS Flags;
    RTL_INJECTION_CONTEXT LocalContext;
    PRTL_INJECTION_PACKET Packet;
    PRTL_INJECTION_PAYLOAD Payload;
    PRTL_INJECTION_CONTEXT Context;
    PRTL_INJECTION_COMPLETE_CALLBACK OriginalCallback;
    PRTL_INJECTION_COMPLETE_CALLBACK FinalCallback;

    //
    // Validate arguments.  Start with the simple non-NULL checks, then verify
    // the optional parameters against the given flags.
    //

    if (!ARGUMENT_PRESENT(InjectionError)) {
        return FALSE;
    } else {

        //
        // Initialize local error variable.  The next handful of tests all exit
        // early with the InvalidParameters flag set to TRUE, so set it now and
        // clear it later once all parameters have been validated.
        //

        Error.ErrorCode = 0;
        Error.InvalidParameters = TRUE;

        //
        // Clear local variables.
        //

        Module = NULL;
        ProcessHandle = NULL;
        FinalCallback = NULL;
        OriginalCallback = NULL;
        SuspendedThread = FALSE;
    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(InjectionPacketPointer)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(TargetProcessId)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(InjectionFlags)) {

        goto Error;

    } else {

        //
        // Initialize local copy of the flags.
        //

        Flags.AsULong = InjectionFlags->AsULong;

        if ((!Flags.InjectModule && !Flags.InjectCode) ||
            (Flags.InjectModule && Flags.InjectCode)) {

            //
            // One (and only one) of InjectModule or InjectCode needs to be set.
            //

            goto Error;
        }

    }

    if (Flags.InjectModule) {

        //
        // Caller wants to inject a module and call the named exported routine.
        // Verify the strings are sane, then try load the module and resolve the
        // name.
        //

        if (!IsValidNullTerminatedUnicodeString(ModulePath)) {
            Error.InvalidModuleName = TRUE;
            goto Error;
        }

        if (!IsValidNullTerminatedString(CallbackFunctionName)) {
            Error.InvalidCallbackFunctionName = TRUE;
            goto Error;
        }

        Module = LoadLibraryW(ModulePath->Buffer);
        if (!Module || Module == INVALID_HANDLE_VALUE) {
            Error.LoadLibraryFailed = TRUE;
            goto Error;
        }

        //
        // We were able to load the library requested by the caller.  Attempt
        // to resolve the routine they've requested.
        //

        Proc = GetProcAddress(Module, CallbackFunctionName->Buffer);
        if (!Proc) {
            Error.GetProcAddressFailed = TRUE;
            goto Error;
        }

        //
        // Everything checks out, save Proc as our Callback pointer.
        //

        OriginalCallback = (PRTL_INJECTION_COMPLETE_CALLBACK)Proc;

    } else {

        //
        // Invariant check: InjectCode should be set at this point.
        //

        if (!Flags.InjectCode) {
            __debugbreak();
            Error.InternalError = TRUE;
            goto Error;
        }

        //
        // Caller has requested code injection.  Verify Code is non-NULL, and
        // then dereference the first byte and make sure it isn't NULL either
        // (it's highly unlikely the caller wants to inject 0x00 as the first
        //  code byte).
        //

        if (!ARGUMENT_PRESENT(InjectionCompleteCallback) ||
            !ARGUMENT_PRESENT(*InjectionCompleteCallback)) {
            Error.InvalidCodeParameter = TRUE;
            goto Error;
        }

        //
        // Save the initial caller provided value as the original callback.
        //

        OriginalCallback = InjectionCompleteCallback;
    }

    //
    // Invariant check: OriginalCallback should be non-NULL here.
    //

    if (!OriginalCallback) {
        __debugbreak();
        Error.InternalError = TRUE;
        goto Error;
    }

    //
    // Initialize local payload alias.
    //

    Payload = InjectionPayload;

    if (Flags.InjectPayload) {
        ULONG ValidPages;

        //
        // Payload should be non-NULL here.
        //

        if (!ARGUMENT_PRESENT(Payload)) {
            Error.InvalidPayloadParameter = TRUE;
            goto Error;
        }

        //
        // Verify payload details.  Buffer should be non-NULL, buffer size
        // should be greater than 0 and less than or equal to 2GB, and the
        // SizeOfStruct field should match what we're expecting.
        //

        Error.InvalidPayload = (
            (Payload->Buffer == NULL)                   |
            (Payload->SizeOfBufferInBytes == 0)         |
            (Payload->SizeOfBufferInBytes > (1 << 31))  |
            (Payload->SizeOfStruct != sizeof(*Payload))
        );

        if (Error.InvalidPayload) {
            goto Error;
        }

        //
        // Probe all of the pages within the payload buffer for read access.
        //

        Success = Rtl->ProbeForRead(Rtl,
                                    Payload->Buffer,
                                    Payload->SizeOfBufferInBytes,
                                    &ValidPages);

        if (!Success) {
            Error.InvalidPayload = TRUE;
            Error.PayloadReadProbeFailed = TRUE;
            goto Error;
        }

    } else {

        //
        // Payload should be NULL here.
        //

        if (ARGUMENT_PRESENT(Payload)) {
            Error.InvalidPayloadParameter = TRUE;
            goto Error;
        }
    }

    //
    // Skip any initial jumps present in the code.
    //

    OriginalCode = (PBYTE)OriginalCallback;
    FinalCode = SkipJumps(OriginalCode);
    FinalCallback = (PRTL_INJECTION_COMPLETE_CALLBACK)FinalCode;

    //
    // Wire up context, packet and payload pointers, then zero everything (via
    // the context, which embed the other two structures).
    //

    Context = &LocalContext;
    Packet = &LocalContext.Packet;
    Payload = &Packet->Payload;

    SecureZeroMemory(Context, sizeof(*Context));

    //
    // Initialize structure sizes.
    //

    Context->SizeOfStruct = sizeof(*Context);
    Packet->SizeOfStruct = sizeof(*Packet);
    Payload->SizeOfStruct = sizeof(*Payload);

    //
    // Wire up callback information and copy injection flags over.
    //

    Context->OriginalCode = OriginalCode;
    Context->CallerCode = (PAPC_ROUTINE)FinalCode;
    Packet->InjectionFlags.AsULong = Flags.AsULong;

    //
    // Mark this context as being stack-allocated.
    //

    Context->Flags.IsStackAllocated = TRUE;

    //
    // Wire up the module path and function name strings if this was a module
    // injection.
    //

    if (Flags.InjectModule) {
        PSTRING String;
        PUNICODE_STRING Unicode;

        Unicode = &Packet->ModulePath;
        Unicode->Length = ModulePath->Length;
        Unicode->MaximumLength = ModulePath->MaximumLength;
        Unicode->Buffer = ModulePath->Buffer;

        String = &Packet->CallbackFunctionName;
        String->Length = CallbackFunctionName->Length;
        String->MaximumLength = CallbackFunctionName->MaximumLength;
        String->Buffer = CallbackFunctionName->Buffer;
    }

    //
    // Copy the payload details if applicable.
    //

    if (Flags.InjectPayload) {
        Payload->Flags.AsULong = InjectionPayload->Flags.AsULong;
        Payload->SizeOfBufferInBytes = InjectionPayload->SizeOfBufferInBytes;
        Payload->OriginalBufferAddress = (ULONG_PTR)InjectionPayload->Buffer;
    }

    //
    // Verify the callback.  This performs the magic number test, then obtains
    // the code size of the callback function and saves it in Context.
    //

    Success = RtlpVerifyInjectionCallback(Rtl, Context, &Error);

    if (!Success) {
        goto Error;
    }

    //
    // We now need to validate the process ID and then attempt to open a handle
    // to it with all access.  Try enable the SeDebugPrivilege, as it'll allow
    // us to bypass all access checks.
    //

    Success = Rtl->EnableDebugPrivilege();

    if (!Success) {

        //
        // Not having debug privileges is not fatal, it just limits what we'll
        // be able to open.
        //

        NOTHING;
    }

    //
    // Open a handle to the process.
    //

    ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, TargetProcessId);

    if (!ProcessHandle || ProcessHandle == INVALID_HANDLE_VALUE) {
        Rtl->LastError = GetLastError();
        Error.OpenTargetProcessFailed = TRUE;
        goto Error;
    }

    //
    // All the parameters have been validated at this point, so clear the
    // invalid parameters flag we set earlier.
    //

    Error.InvalidParameters = FALSE;

    //
    // Save the target process ID as it's used in event name generation.
    //

    Packet->TargetProcessId = TargetProcessId;

    //
    // We now have enough information to commit to allocating and initializing
    // an injection context (which will contain the packet returned to the
    // user).  Assign handles to the context and clear our local stack-allocated
    // pointers; the context (via RtlpDestroyInjectionContext()) is responsible
    // for closing these handles now.
    //

    Context->ModuleHandle = Module;
    Context->TargetProcessHandle = ProcessHandle;
    Module = NULL;
    ProcessHandle = NULL;

    Success = RtlpInjectContext(Rtl,
                                Allocator,
                                &Context,
                                &LocalContext,
                                &Error);

    if (Success) {

        //
        // We've successfully created the final injection context and injected
        // it into the remote process.  We're done.
        //

        goto End;
    }

    //
    // Intentional follow-on to Error.
    //

Error:

    if (Module) {
        FreeLibrary(Module);
        Module = NULL;
    }

    if (ProcessHandle) {
        CloseHandle(ProcessHandle);
        ProcessHandle = NULL;
    }

    if (Context) {
        RtlpDestroyInjectionContext(&Context);
        Packet = NULL;
        Payload = NULL;
    }

    //
    // Update the caller's injection error pointer.
    //

    InjectionError->ErrorCode = Error.ErrorCode;

    //
    // Indicate failure.
    //

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    if (Success) {

        //
        // Some final invariant tests prior to updating the caller's injection
        // packet pointer.
        //

        if (!Context) {

            //
            // Context should be non-NULL.
            //

            __debugbreak();
            return FALSE;

        } else if (Context == &LocalContext) {

            //
            // Context shouldn't be pointing at the stack-allocated local
            // context.
            //

            __debugbreak();
            return FALSE;
        }

        if (!Packet || Packet != &Context->Packet) {

            //
            // Packet should be non-NULL and wired up to the Context.
            //

            __debugbreak();
            return FALSE;
        }

        //
        // Update the caller's pointer.
        //

        *InjectionPacketPointer = Packet;
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
