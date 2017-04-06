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

#pragma data_seg(".shared")
#pragma data_seg()
#pragma comment(linker, "/section:.shared,rws")

BOOL
CALLBACK
RtlpInitializeInjectionCallback(
    PINIT_ONCE InitOnce,
    PVOID Parameter,
    PVOID *lpContext
    )
{
    //PRTL_INJECTION_SHARED Context;

    return FALSE;

}

BOOL
RtlInitializeInjection(
    VOID
    )
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
RtlCreateInjectionPacket(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PRTL_CREATE_INJECTION_PACKET_FLAGS CreateInjectionFlags,
    PCUNICODE_STRING ModulePath,
    PCSTRING CallbackFunctionName,
    PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteCallback,
    ULONG TargetProcessId,
    ULONG TargetThreadId,
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

    CreateFlags - Supplies a pointer to injection packet flags.  At minimum,
        either CreateFlags.InjectModule or CreateFlags.InjectCode must be set,
        indicating which type of injection method should be used.

    ModulePath - Supplies a pointer to a fully-qualified UNICODE_STRING
        structure representing the module path to load into the remote process.
        If CreateFlags.InjectModule is set, both this value and the parameter
        CallbackFunctionName must be valid.

    CallbackFunctionName - Supplies a pointer to a STRING structure representing
        the name of an exported symbol to resolve once the module indicated by
        the ModulePath parameter is loaded.  This must be non-NULL and point to
        a valid string if CreateFlags.InjectModule is TRUE, otherwise it must
        be NULL.

    InjectionCompleteCallback - Supplies a pointer to a buffer of code to be
        injected into the process and then executed by the target thread.  This
        must be NULL if Flags.InjectModule is set, otherwise, it should point to
        a valid buffer that contains AMD64 byte code.

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
    BOOL SuspendedThread = FALSE;
    PBYTE OriginalCode;
    PBYTE FinalCode;
    FARPROC Proc;
    HMODULE Module = NULL;
    LONG ThreadSuspensionCount;
    LONG_INTEGER AllocSize;
    HANDLE ProcessHandle = NULL;
    HANDLE ThreadHandle = NULL;
    HANDLE ThreadsSnapshotHandle = NULL;
    HANDLE RemoteThreadHandle = NULL;
    LPVOID RemoteThreadStartAddress = NULL;
    RTL_INJECTION_ERROR Error;
    RTL_CREATE_INJECTION_PACKET_FLAGS CreateFlags;
    RTL_INJECTION_CONTEXT LocalContext;
    PRTL_INJECTION_PACKET Packet;
    PRTL_INJECTION_CONTEXT Context;
    PRTL_INJECTION_COMPLETE_CALLBACK OriginalCallback = NULL;
    PRTL_INJECTION_COMPLETE_CALLBACK FinalCallback = NULL;

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

    if (!ARGUMENT_PRESENT(CreateInjectionFlags)) {
        goto Error;
    } else {

        //
        // Initialize local copy of the flags.
        //

        CreateFlags.AsULong = CreateInjectionFlags->AsULong;

        if (!(CreateFlags.InjectModule & CreateFlags.InjectCode)) {

            //
            // One of InjectModule or InjectCode needs to be set.
            //

            goto Error;
        }

        //
        // If CreateFlags.HijackExistingThread is set, TargetThreadId must be
        // non-zero, else, it must be zero.
        //

        if (CreateFlags.HijackExistingThread) {
            if (!TargetThreadId) {
                Error.InvalidTargetThreadId = TRUE;
                goto Error;
            }
        } else {
            if (TargetThreadId) {
                Error.InvalidTargetThreadId = TRUE;
                goto Error;
            }
        }
    }

    if (CreateFlags.InjectModule) {

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

        if (!CreateFlags.InjectCode) {
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
    // Invariant check: InjectionCompleteCallback should be non-NULL here, and
    // Success should be set to TRUE.
    //

    if (!Success || !OriginalCallback) {
        __debugbreak();
        Error.InternalError = TRUE;
        goto Error;
    }

    //
    // Skip any initial jumps present in the code.
    //

    OriginalCode = (PBYTE)OriginalCallback;
    FinalCode = SkipJumps(OriginalCode);
    FinalCallback = (PRTL_INJECTION_COMPLETE_CALLBACK)FinalCode;

    //
    // Wire up context and packet pointers, then zero the entire context.
    //

    Context = &LocalContext;
    Packet = &LocalContext.Packet;

    SecureZeroMemory(Context, sizeof(*Context));

    //
    // Wire up callback information.
    //

    Context->OriginalCode = OriginalCode;
    Context->Code = FinalCode;

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
        // We won't be able to open processes or handles for which we don't have
        // appropriate access.
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
    // A handle was opened successfully.  If a thread ID was provided, attempt
    // to open a handle to it, too.
    //

    if (CreateFlags.HijackExistingThread) {

        ThreadHandle = OpenThread(PROCESS_ALL_ACCESS, FALSE, TargetThreadId);
        if (!ThreadHandle || ThreadHandle == INVALID_HANDLE_VALUE) {
            Rtl->LastError = GetLastError();
            Error.OpenTargetThreadFailed = TRUE;
            goto Error;
        }

        //
        // We successfully opened a handle to the thread.  Suspend it now.
        //

        ThreadSuspensionCount = SuspendThread(ThreadHandle);
        if (ThreadSuspensionCount == -1) {
            Rtl->LastError = GetLastError();
            Error.SuspendTargetThreadFailed = TRUE;
            goto Error;
        }

        //
        // Suspension was successful.  Make a note of that now, such that if
        // we encounter an error later in this routine, we can unsuspended the
        // thread if need be.
        //

        SuspendedThread = TRUE;

    } else {

        //
        // We need to create a new remote thread in the process, but we haven't
        // prepared the target memory to use as the start address yet, so punt
        // for now.
        //

        NOTHING;
    }

    //
    // All the parameters have been validated at this point, so clear the
    // invalid parameters flag we set earlier.
    //

    Error.InvalidParameters = FALSE;

    //
    // Attempt to create our initial chunks of memory in the remote process.
    //

    Success = RtlpPreInjectionAllocateRemoteMemory(Rtl, Context, &Error);

    if (!Success) {
        goto Error;
    }

    //
    // We now have enough information to commit to allocating and initializing
    // an injection context (which will contain the packet returned to the
    // user).
    //

    //
    // Calculate the required allocation size.
    //

    AllocSize.LongPart = (

        //
        // Account for the Context structure.
        //

        sizeof(RTL_INJECTION_CONTEXT) //+

        //
        // Account for a copy of ... string.
        //

        //sizeof(UNICODE_STRING) +

    );

    Context = (PRTL_INJECTION_CONTEXT)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(RTL_INJECTION_CONTEXT)
        )
    );

    if (!Context) {
        Error.InternalAllocationFailure = TRUE;
        goto Error;
    }

    //
    // Initialize scalar fields.
    //

    Context->SizeOfStruct = sizeof(*Context);
    Packet = &Context->Packet;
    Packet->SizeOfStruct = sizeof(*Packet);

    //
    // XXX TODO: current implementation point.
    //

    __debugbreak();

    //
    // Indicate success and return.
    //

    Success = TRUE;
    goto End;

Error:

    if (Module) {
        FreeLibrary(Module);
        Module = NULL;
    }

    if (ProcessHandle) {
        CloseHandle(ProcessHandle);
        ProcessHandle = NULL;
    }

    if (ThreadHandle) {
        if (SuspendedThread) {
            ResumeThread(ThreadHandle);
            SuspendedThread = FALSE;
        }
        CloseHandle(ThreadHandle);
        ThreadHandle = NULL;
    } else if (SuspendedThread) {

        //
        // Invariant check: the SuspendedThread boolean shouldn't be set if
        // there's no thread handle.
        //

        __debugbreak();
        return FALSE;
    }

    if (Context) {
        Allocator->FreePointer(Allocator->Context, &Context);
        Packet = NULL;
    }

    //
    // Update the caller's injection error pointer and clear the packet pointer.
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
            __debugbreak();
            return FALSE;
        }

        if (!Packet || Packet != &Context->Packet) {
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

_Use_decl_annotations_
BOOL
RtlDestroyInjectionPacket(
    PRTL Rtl,
    PRTL_INJECTION_PACKET *PacketPointer,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Destroys an injection packet previously created by RtlCreateInjectionPacket.
    This routine should only be called if a created injection packet needs to
    be abandoned before injecting it via RtlInject().  If RtlInject() is called
    against the packet, the packet will be destroyed at the appropriate time.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    PacketPointer - Supplies a pointer to a variable containing the address of
        the injection packet to destroy.  The pointer will be set to NULL by
        this routine.

    InjectionError - Supplies a pointer to a variable that receives information
        about the error, if one occurs (which will be indicated by the routine
        returning FALSE).

Return Value:

    If the routine completes successfully, TRUE is returned.  If a failure
    occurs, FALSE is returned and InjectionError is set with the relevant
    error code.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
RtlAddInjectionPayload(
    PRTL Rtl,
    PRTL_INJECTION_PACKET Packet,
    PRTL_INJECTION_PAYLOAD Payload,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

Arguments:

Return Value:

    If the routine completes successfully, TRUE is returned.  If a failure
    occurs, FALSE is returned and InjectionError is set with the relevant
    error code.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
RtlAddInjectionSymbols(
    PRTL Rtl,
    PRTL_INJECTION_PACKET Packet,
    PRTL_INJECTION_SYMBOLS Symbols,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

Arguments:

Return Value:

    If the routine completes successfully, TRUE is returned.  If a failure
    occurs, FALSE is returned and InjectionError is set with the relevant
    error code.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
RtlInject(
    PRTL Rtl,
    PCRTL_INJECTION_PACKET Packet,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

Arguments:

Return Value:

    If the routine completes successfully, TRUE is returned.  If a failure
    occurs, FALSE is returned and InjectionError is set with the relevant
    error code.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
BOOL
RtlIsInjectionCompleteCallbackTest(
    PCRTL_INJECTION_PACKET Packet,
    PULONG MagicNumber
    )
{
    return RtlIsInjectionCompleteCallbackTestInline(Packet, MagicNumber);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
