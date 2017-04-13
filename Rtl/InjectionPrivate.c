/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionPrivate.c

Abstract:

    This module implements private routines pertaining to the injection of code
    into a remote process.  Routines are provided for getting an address of a
    currently executing function, obtaining an approximate code size of a
    function given an address within the function, skipping jumps when given
    a pointer to byte code.

--*/

#include "stdafx.h"

//
// The following routines are simple wrappers around their inline counterparts.
// The InjectionInline.h for docstrings.
//

_Use_decl_annotations_
BOOL
IsJump(
    PBYTE Code
    )
{
    return IsJumpInline(Code);
}

_Use_decl_annotations_
PBYTE
SkipJumps(
    PBYTE Code
    )
{
    return SkipJumpsInline(Code);
}

_Use_decl_annotations_
BOOL
GetApproximateFunctionBoundaries(
    ULONG_PTR Address,
    PULONG_PTR StartAddress,
    PULONG_PTR EndAddress
    )
{
    return GetApproximateFunctionBoundariesInline(Address,
                                                  StartAddress,
                                                  EndAddress);
}

//
// End of inline function wrappers.
//

_Use_decl_annotations_
DECLSPEC_NOINLINE
ULONG_PTR
GetInstructionPointer(
    VOID
    )
/*++

Routine Description:

    Returns the return address of a routine.  This effectively provides an
    address of the instruction pointer within the routine, which is used to
    derive an approximate size of the function when passed to the routine
    GetApproximateFunctionBoundaries().

Arguments:

    None.

Return Value:

    The address of the instruction pointer prior to entering the current call.

--*/
{
    return (ULONG_PTR)_ReturnAddress();
}

_Use_decl_annotations_
BOOL
RtlpInjectionCallbackVerifyMagicNumber(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Tests a caller's injection complete callback routine as part of injection
    packet creation.  The test is performed by calling the routine with an
    injection packet that has its Flags.CompleteCallbackTest set to TRUE.
    The callback routine is expected to return the XOR'd value of the packet's
    MagicNumber LowPart and HighPart fields, e.g.:

        ExpectedMagicNumber = (
            Packet->MagicNumber.LowPart ^
            Packet->MagicNumber.HighPart
        );

    N.B. The purpose of this routine is to help catch programmer mistakes in
         wiring up the injection code, not provide any level of additional
         security, which is why a simple XOR is chosen over a more sophisticated
         system.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Packet - Supplies a pointer to the injection context being tested.

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
    BOOL EncounteredException;
    ULONG MagicSize;
    ULONGLONG ExpectedMagicNumber;
    ULONGLONG ActualMagicNumber;
    PBYTE MagicBuffer;
    PRTL_INJECTION_PACKET Packet;
    RTL_INJECTION_ERROR Error;
    PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteCallback;

    //
    // Clear our local error variable.
    //

    Error.ErrorCode = 0;

    //
    // Wire up our local packet pointer, and extract the injection complete
    // callback function pointer.
    //

    Packet = &Context->Packet;
    InjectionCompleteCallback = Context->Callback;

    //
    // Generate 8 bytes of random data.
    //

    MagicSize = (USHORT)sizeof(Packet->MagicNumber);
    MagicBuffer = (PBYTE)&Packet->MagicNumber;
    Success = Rtl->CryptGenRandom(Rtl, MagicSize, MagicBuffer);
    if (!Success) {
        Error.InternalError = TRUE;
        goto Error;
    }

    //
    // XOR the lower and upper ULONGs to generate the ULONG we expect to get
    // back from the initial injection completion callback test.
    //

    ExpectedMagicNumber = (
        Packet->MagicNumber.LowPart ^
        Packet->MagicNumber.HighPart
    );

    //
    // Set the relevant packet flag to indicate this is a callback test.
    //

    Packet->Flags.AsULong = 0;
    Packet->Flags.IsMagicNumberTest = TRUE;

    //
    // Call the routine.
    //

    EncounteredException = FALSE;

    __try {

        ActualMagicNumber = InjectionCompleteCallback(Packet);

    } __except(
        GetExceptionCode() == STATUS_IN_PAGE_ERROR       ||
        GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
        GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {

        EncounteredException = TRUE;

        switch (GetExceptionCode()) {
            case STATUS_IN_PAGE_ERROR:
                Error.StatusInPageErrorInCallbackTest = TRUE;
                break;
            case EXCEPTION_ACCESS_VIOLATION:
                Error.AccessViolationInCallbackTest = TRUE;
                break;
            case EXCEPTION_ILLEGAL_INSTRUCTION:
                Error.IllegalInstructionInCallbackTest = TRUE;
                break;
        }
    }

    if (EncounteredException) {
        goto Error;
    }

    //
    // Verify the magic number was as we expect.
    //

    if (ExpectedMagicNumber == ActualMagicNumber) {

        //
        // Test was successful.
        //

        Success = TRUE;
        goto End;
    }

    Error.MagicNumberMismatch = TRUE;

    //
    // Intentional follow-on to Error.
    //

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the error code and return.
    //

    InjectionError->ErrorCode = Error.ErrorCode;

    return Success;
}

_Use_decl_annotations_
BOOL
RtlpInjectionCallbackExtractCodeSize(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Extracts an approximate size of the injection code.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Context - Supplies a pointer to the injection context being tested.

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
    BOOL EncounteredException;
    ULONGLONG CodeSizeFromReturnValue;
    ULONGLONG CodeSize;
    ULONG_PTR Code;
    ULONG_PTR StartAddress;
    ULONG_PTR EndAddress;
    PRTL_INJECTION_PACKET Packet;
    RTL_INJECTION_ERROR Error;
    PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteCallback;

    //
    // Clear our local error variable.
    //

    Error.ErrorCode = 0;

    //
    // Wire up our local packet pointer, and extract the injection complete
    // callback function pointer.
    //

    Packet = &Context->Packet;
    InjectionCompleteCallback = Context->Callback;

    //
    // Set the relevant packet flag to indicate this is a callback test.
    //

    Packet->Flags.AsULong = 0;
    Packet->Flags.IsCodeSizeQuery = TRUE;

    //
    // Call the routine.
    //

    EncounteredException = FALSE;

    __try {

        CodeSizeFromReturnValue = InjectionCompleteCallback(Packet);

    } __except(
        GetExceptionCode() == STATUS_IN_PAGE_ERROR       ||
        GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
        GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {

        EncounteredException = TRUE;

        switch (GetExceptionCode()) {
            case STATUS_IN_PAGE_ERROR:
                Error.StatusInPageErrorInCallbackTest = TRUE;
                break;
            case EXCEPTION_ACCESS_VIOLATION:
                Error.AccessViolationInCallbackTest = TRUE;
                break;
            case EXCEPTION_ILLEGAL_INSTRUCTION:
                Error.IllegalInstructionInCallbackTest = TRUE;
                break;
        }
    }

    if (EncounteredException) {
        goto Error;
    }

    //
    // Assume there's an error until verified otherwise.
    //

    Error.InternalError = TRUE;
    Error.ExtractCodeSizeFailedDuringCallbackTest = TRUE;

    //
    // Verify the sizes and addresses.
    //

    Code = (ULONG_PTR)InjectionCompleteCallback;
    Success = GetApproximateFunctionBoundaries(Code,
                                               &StartAddress,
                                               &EndAddress);

    if (!Success) {
        goto Error;
    }

    CodeSize = EndAddress - StartAddress;

    if (CodeSize != CodeSizeFromReturnValue) {
        goto Error;
    }

    if (CodeSize >= PAGE_SIZE) {
        Error.CallbackCodeSizeGreaterThanOrEqualToPageSize = TRUE;
        goto Error;
    }

    if (PointerToOffsetCrossesPageBoundary((PVOID)StartAddress, CodeSize)) {
        Error.CallbackCodeCrossesPageBoundary = TRUE;
        goto Error;
    }

    if (StartAddress != Context->CallerCodeStartAddress) {
        goto Error;
    }

    if (EndAddress != Context->CallerCodeEndAddress) {
        goto Error;
    }

    //
    // Code size passes tests, clear the error flags we set earlier.
    //

    Error.InternalError = FALSE;
    Error.ExtractCodeSizeFailedDuringCallbackTest = FALSE;

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the error code and return.
    //

    InjectionError->ErrorCode = Error.ErrorCode;

    return Success;
}

_Use_decl_annotations_
BOOL
RtlpInjectionVerifyContextCallback(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Verifies our internal injection thread entry adheres to the injection
    context callback protocol.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Context - Supplies a pointer to the injection context being tested.

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
    BOOL EncounteredException;
    ULONGLONG CodeSizeFromReturnValue;
    ULONGLONG CodeSize;
    ULONG_PTR Code;
    ULONG_PTR StartAddress;
    ULONG_PTR EndAddress;
    RTL_INJECTION_ERROR Error;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;

    //
    // Clear our local error variable.
    //

    Error.ErrorCode = 0;

    //
    // Wire up our local injection thunk function pointer.
    //

    InjectionThunk = Context->RemoteThread.InjectionThunk;

    //
    // Set the relevant context flag to indicate this is a callback test.
    //

    Context->Flags.IsCodeSizeQuery = TRUE;

    //
    // Call the routine.
    //

    EncounteredException = FALSE;

    __try {

        CodeSizeFromReturnValue = InjectionThunk(Context);

    } __except(
        GetExceptionCode() == STATUS_IN_PAGE_ERROR       ||
        GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
        GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {

        EncounteredException = TRUE;

        switch (GetExceptionCode()) {
            case STATUS_IN_PAGE_ERROR:
                Error.StatusInPageErrorInCallbackTest = TRUE;
                break;
            case EXCEPTION_ACCESS_VIOLATION:
                Error.AccessViolationInCallbackTest = TRUE;
                break;
            case EXCEPTION_ILLEGAL_INSTRUCTION:
                Error.IllegalInstructionInCallbackTest = TRUE;
                break;
        }
    }

    if (EncounteredException) {
        goto Error;
    }

    //
    // Assume there's an error until verified otherwise.
    //

    Error.InternalError = TRUE;
    Error.InjectionThunkExtractCodeSizeFailed = TRUE;

    //
    // Verify the sizes and addresses.
    //

    Code = (ULONG_PTR)InjectionThunk;
    Success = GetApproximateFunctionBoundaries(Code,
                                               &StartAddress,
                                               &EndAddress);

    if (!Success) {
        goto Error;
    }

    CodeSize = EndAddress - StartAddress;

    if (CodeSize != CodeSizeFromReturnValue) {
        goto Error;
    }

    if (CodeSize >= PAGE_SIZE) {
        Error.CallbackCodeSizeGreaterThanOrEqualToPageSize = TRUE;
        goto Error;
    }

    if (PointerToOffsetCrossesPageBoundary((PVOID)StartAddress, CodeSize)) {
        Error.CallbackCodeCrossesPageBoundary = TRUE;
        goto Error;
    }

    if (StartAddress != Context->InjectionThunkStartAddress) {
        goto Error;
    }

    if (EndAddress != Context->InjectionThunkEndAddress) {
        goto Error;
    }

    //
    // Code size passes tests, clear the error flags we set earlier.
    //

    Error.InternalError = FALSE;
    Error.InjectionThunkExtractCodeSizeFailed = FALSE;

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Clear the code size query flag in the context, update the error code,
    // and return.
    //

    Context->Flags.IsCodeSizeQuery = FALSE;
    InjectionError->ErrorCode = Error.ErrorCode;

    return Success;
}

_Use_decl_annotations_
BOOL
RtlpVerifyInjectionCallback(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    This routine is responsible for verifying a caller's injection complete
    callback routine prior to actually performing any injection.  It tests
    the callback's compliance with the injection protocol -- that is, that its
    code looks something like this:

        ULONGLONG
        ExampleInjectionCompleteCallback(
            _In_ PRTL_INJECTION_PACKET Packet
            )
        {
            ULONGLONG Token;

            if (Packet->IsInjectionProtocolCallback(Packet, &Token)) {
                return Token;
            }

            ...
        }

    This routine wires up the Packet->IsInjectionProtocolCallback to our
    internal routine RtlpPreInjectionProtocolCallbackImpl().  We then set
    various packet flags and invoke the caller's callback, verifying the
    return values as we go.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Context - Supplies a pointer to an RTL_INJECTION_CONTEXT structure that has
        been initialized with the caller's initial callback.

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
    PRTL_INJECTION_PACKET Packet;
    RTL_INJECTION_ERROR Error;

    //
    // Clear our local error variable.
    //

    Error.ErrorCode = 0;

    //
    // Wire our local Packet variable up to the relevant context offset.
    //

    Packet = &Context->Packet;

    //
    // Initialize the initial packet callback.
    //

    Packet->IsInjectionProtocolCallback = RtlpPreInjectionProtocolCallbackImpl;

    //
    // Perform the magic number callback test.
    //

    Success = RtlpInjectionCallbackVerifyMagicNumber(Rtl,
                                                     Context,
                                                     InjectionError);

    if (!Success) {
        goto Error;
    }

    //
    // Magic number verification was successful.  Extract the code size.
    //

    Success = RtlpInjectionCallbackExtractCodeSize(Rtl,
                                                   Context,
                                                   InjectionError);

    if (!Success) {
        goto Error;
    }

    //
    // (If we were going to ensure the injected code had no RIP-relative
    //  addresses, we'd do that here.)
    //

    //
    // Code size extraction was successful.  Verify our internal thread entry
    // thunk.
    //

    //
    // Wire up the remote thread injection thunk and set the injection
    // context protocol callback to the pre-injection implementation, then
    // perform the verification.
    //

    Context->RemoteThread.InjectionThunk = (
        SKIP_JUMPS(
            RtlpInjectionRemoteThreadEntryThunk,
            PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK
        )
    );

    Context->IsInjectionContextProtocolCallback = (
        SKIP_JUMPS(
            RtlpPreInjectionContextProtocolCallbackImpl,
            PRTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK
        )
    );

    Success = RtlpInjectionVerifyContextCallback(Rtl,
                                                 Context,
                                                 InjectionError);

    if (!Success) {
        goto Error;
    }

    //
    // Context and packet callbacks have been verified successfully, we're done.
    //

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Merge the error code and return.
    //

    InjectionError->ErrorCode |= Error.ErrorCode;

    return Success;
}


_Use_decl_annotations_
BOOL
RtlpPreInjectionProtocolCallbackImpl(
    PCRTL_INJECTION_PACKET Packet,
    PVOID Token
    )
/*++

Routine Description:

    This routine implements the injection callback protocol required by injected
    code routines.  It should be called as the first step in the injected code.
    The injected code should return the value of Token if this routine returns
    TRUE.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    Token - Supplies the address of a variable that will receive the return
        value if this is a protocol callback.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the Token parameter.

--*/
{
    if (Packet->Flags.IsMagicNumberTest) {
        ULARGE_INTEGER Magic;

        Magic.QuadPart = Packet->MagicNumber.QuadPart;
        *((PULONGLONG)Token) = Magic.LowPart ^ Magic.HighPart;

        return TRUE;

    } else if (Packet->Flags.IsCodeSizeQuery) {
        BOOL Success;
        ULONG_PTR InstructionPointer;
        ULONG_PTR StartAddress;
        ULONG_PTR EndAddress;
        ULONG_PTR CodeSize;
        PRTL_INJECTION_CONTEXT Context;

        Context = CONTAINING_RECORD(Packet,
                                    RTL_INJECTION_CONTEXT,
                                    Packet);


        InstructionPointer = (ULONG_PTR)_ReturnAddress();

        Success = GetApproximateFunctionBoundaries(InstructionPointer,
                                                   &StartAddress,
                                                   &EndAddress);

        Context->CallerCodeStartAddress = StartAddress;
        Context->CallerCodeEndAddress = EndAddress;

        if (!Success) {
            return FALSE;
        }

        CodeSize = EndAddress - StartAddress;

        if (CodeSize >= PAGE_SIZE) {
            return FALSE;
        }

        if (PointerToOffsetCrossesPageBoundary((PVOID)StartAddress, CodeSize)) {
            return FALSE;
        }

        *((PULONGLONG)Token) = CodeSize;

        Context->SizeOfCallerCodeInBytes = (ULONG)CodeSize;

        return TRUE;
    }

    return FALSE;
}

_Use_decl_annotations_
BOOL
RtlpPreInjectionContextProtocolCallbackImpl(
    PRTL_INJECTION_CONTEXT Context,
    PVOID Token
    )
/*++

Routine Description:

    This routine implements the injection context callback protocol required by
    our internal code injection routines.  It should be called as the first step
    of our injected thread's entry routine.  The code should return the value of
    Token if this routine returns TRUE.

    This implementation is responsible for obtaining the code size of the
    injection context callback.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    Token - Supplies the address of a variable that will receive the return
        value if this is a protocol callback.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the Token parameter.

--*/
{
    BOOL Success;
    ULONG_PTR InstructionPointer;
    ULONG_PTR StartAddress;
    ULONG_PTR EndAddress;
    ULONG_PTR CodeSize;

    //
    // Verify this is a code size query.
    //

    if (!Context->Flags.IsCodeSizeQuery) {
        return FALSE;
    }

    InstructionPointer = (ULONG_PTR)_ReturnAddress();

    Success = GetApproximateFunctionBoundaries(InstructionPointer,
                                               &StartAddress,
                                               &EndAddress);

    Context->InjectionThunkStartAddress = StartAddress;
    Context->InjectionThunkEndAddress = EndAddress;

    if (!Success) {
        return FALSE;
    }

    CodeSize = EndAddress - StartAddress;

    if (CodeSize >= PAGE_SIZE) {
        return FALSE;
    }

    if (PointerToOffsetCrossesPageBoundary((PVOID)StartAddress, CodeSize)) {
        return FALSE;
    }

    *((PULONGLONG)Token) = CodeSize;

    Context->SizeOfInjectionThunkInBytes = (ULONG)CodeSize;

    return TRUE;
}

//
// The post-injection implementations of the Packet->IsInjectionProtocolCallback
// and Context->IsInjectionContextProtocolCallback routines are defined below.
// Currently, they are both dummy routines that simply return FALSE, as we do no
// callback interrogation post-injection.

_Use_decl_annotations_
BOOL
NOINLINE
RtlpPostInjectionProtocolCallbackImpl(
    PCRTL_INJECTION_PACKET Packet,
    PVOID Token
    )
/*++

Routine Description:

    In an injected thread, Packet->IsInjectionProtocolCallback will be set to
    this routine.  It simply returns FALSE as we don't need to do any protocol
    checks (like magic number or code size extraction) post-injection.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    Token - Supplies the address of a variable that will receive the return
        value if this is a protocol callback.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the Token parameter.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
NOINLINE
BOOL
RtlpPostInjectionContextProtocolCallbackImpl(
    PRTL_INJECTION_CONTEXT Context,
    PVOID Token
    )
/*++

Routine Description:

    In an injected thread, Context->IsInjectionContextProtocolCallback will be
    set to this routine.  It simply returns FALSE as we don't need to do any
    protocol checks (like code size extraction) post-injection.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    Token - Supplies the address of a variable that will receive the return
        value if this is a protocol callback.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the Token parameter.

--*/
{
    return FALSE;
}

_Use_decl_annotations_
DECLSPEC_NORETURN
NOINLINE
VOID
RtlpExitThreadThunk(
    PCRTL_INJECTION_CONTEXT Context
    )
/*++

Routine Description:

    If we need to terminate the target thread, an APC will be queued using
    this routine.

Arguments:

    Context - Supplies a pointer to the remote process's injection context.

Return Value:

    None.

--*/
{
    PEXIT_THREAD ExitThread;

    ExitThread = Context->Functions.ExitThread;
    ExitThread(InjectedRemoteThreadQueuedAPCExitThread);
}

_Use_decl_annotations_
BOOL
RtlpInjectionRemoteThreadEntry(
    PRTL_INJECTION_CONTEXT Context,
    PULONGLONG ResultPointer
    )
/*++

Routine Description:

    This is the main "worker" routine of an injected remote thread.  It is
    called by our initial thread entry bootstrap code (the "injection thunk")
    once the Rtl module has been loaded and an RTL structure initialized.

    This routine differs from the initial injection thunk thread entry routine
    (RtlpInjectionRemoteThreadEntryThunk) in that it a normal unrestricted DLL
    routine -- i.e. there are no constraints regarding the use of RIP-relative
    instructions (e.g. via external symbol references, linker jump tables, etc).

    The routine is responsible for invoking the caller's callback complete
    routine, as per the details in the context's embedded packet structure.

Arguments:

    Context - Supplies a pointer to the RTL_INJECTION_CONTEXT structure that is
        being used for this injection.

    ResultPointer - Supplies a pointer to a ULONGLONG variable that will
        receive the result of the caller's injected routine.  The low 32 bits
        of this value will be used as the remote thread's exit code.

Return Value:

    TRUE on success, FALSE on error.  Context->Packet.Error will be updated
    with an appropriate RTL_INJECTION_ERROR state if an error occurs.

--*/
{
    BOOL Success;
    BOOL EncounteredException;
    PRTL_INJECTION_PACKET Packet;
    RTL_INJECTION_ERROR Error;
    PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteCallback;
    ULONGLONG Result;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Context)) {
        return InjectedRemoteThreadNullContext;
    }

    if (!ARGUMENT_PRESENT(ResultPointer)) {
        return InjectedRemoteThreadNullResultParameter;
    }

    //
    // Clear our local error variable.
    //

    Error.ErrorCode = 0;

    //
    // Initialize local aliases and complete final wiring of the packet.
    //

    Packet = &Context->Packet;
    InjectionCompleteCallback = Context->Callback;
    Packet->Rtl = Context->Rtl;
    Packet->Flags.AsULong = 0;
    Packet->Flags.IsInjected = TRUE;

    //
    // Call the routine.
    //

    __try {

        Result = InjectionCompleteCallback(Packet);

    } __except(
        GetExceptionCode() == STATUS_IN_PAGE_ERROR       ||
        GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ||
        GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ?
            EXCEPTION_EXECUTE_HANDLER :
            EXCEPTION_CONTINUE_SEARCH) {

        EncounteredException = TRUE;

        switch (GetExceptionCode()) {
            case STATUS_IN_PAGE_ERROR:
                Error.StatusInPageErrorInCallback = TRUE;
                break;
            case EXCEPTION_ACCESS_VIOLATION:
                Error.AccessViolationInCallback = TRUE;
                break;
            case EXCEPTION_ILLEGAL_INSTRUCTION:
                Error.IllegalInstructionInCallback = TRUE;
                break;
        }
    }

    if (EncounteredException) {
        goto Error;
    }

    //
    // Callback was successful.
    //

    if (Packet->InjectionFlags.InjectPayload) {
        NOTHING;
    }

    //
    // Indicate success and return.
    //

    Success = TRUE;
    Result = InjectedRemoteThreadNoError;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the packet's error variable.
    //

    Packet->Error.ErrorCode |= Error.ErrorCode;

    //
    // Update the result pointer.
    //

    *ResultPointer = Result;

    return Success;
}

_Use_decl_annotations_
BOOL
RtlpInjectContext(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PPRTL_INJECTION_CONTEXT DestContext,
    PCRTL_INJECTION_CONTEXT SourceContext,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Creates a new injection context, using a partially prepared context as a
    template, and injects it into a remote process.

    This routine is responsible for allocating and initializing a new injection
    context structure in the current process, and allocation and initialization
    of all memory required in the remote process in order to support subsequent
    injection.  This includes allocating two pages of executable memory for the
    caller's completion callback routine, as well as our initial thread entry
    point, as well as a separate memory page containing the injection structure
    and all necessary supporting buffers (e.g. for path and event names).  If a
    payload has been requested, this is allocated separately, too.

    Two global events will be created, one "signal" event and one "wait" event.
    They will be created with unique Unicode string names that will be marshaled
    into the remote process as well.  Our initial injected thread entry routine
    is responsible for calling SignalObjectAndWait() on the signal and wait
    events respectively.  This allows us to wait for the injection to complete
    before returning to the caller.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure to use for the
        injection structure in the current process.  The corresponding structure
        in the remote process will be allocated via VirtualAllocEx() calls.

    DestContext - Supplies a pointer to a variable that receives the address
        of the newly (heap) allocated RTL_INJECTION_CONTEXT structure, primed
        using relevant values from SourceContext.

    SourceContext - Supplies a pointer to the initial RTL_INJECTION_CONTEXT
        stack-allocated context that was used during initial injection request
        validation.  The relevant values will be copied from this context into
        the final DestContext.

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
    BOOL QueueAPCToTerminateThreadOnError = FALSE;
    ULONG Size;
    USHORT Offset;
    USHORT CodeSize;
    USHORT ExpectedOffset;
    ULONG RemoteThreadId;
    ULONG RemoteThreadCreationFlags;
    PSTRING String;
    NTSTATUS Status;
    PUNICODE_STRING Unicode;
    LONG_INTEGER NumberOfPages;
    HANDLE RemoteProcessHandle;
    HANDLE RemoteThreadHandle = NULL;
    SIZE_T NumberOfBytesWritten;
    HRESULT WaitResult;
    ULONG_INTEGER AlignedRtlDllPathLength;
    ULONG_INTEGER AlignedModulePathLength;
    ULONG_INTEGER AlignedCallbackFunctionNameLength;
    PRTL_INJECTION_CONTEXT Context;
    ULONG_INTEGER AllocSizeInBytes;
    ULONG_INTEGER PageAlignedAllocSizeInBytes;
    RTL_INJECTION_ERROR Error;
    PRTL_INJECTION_PACKET Packet;
    PRTL_INJECTION_PACKET RemotePacket;
    PRTL_INJECTION_PACKET ActualRemotePacket;
    PCRTL_INJECTION_PACKET SourcePacket;
    PRTL_INJECTION_CONTEXT RemoteContext;
    PRTL_INJECTION_CONTEXT ActualRemoteContext;
    PBYTE Dest;
    PBYTE Source;
    PBYTE BaseContextCode;
    PBYTE RemoteBaseContextCode;
    PBYTE ActualRemoteBaseContextCode;
    PBYTE BaseCallerCode;
    PBYTE RemoteBaseCallerCode;
    PBYTE ActualRemoteBaseCallerCode;
    PVOID RemoteContextAddress = NULL;
    LPVOID RemoteThreadParameter;
    LPTHREAD_START_ROUTINE RemoteThreadStartRoutine;
    const USHORT EventNameMaxLength=RTL_INJECTION_CONTEXT_EVENT_NAME_MAXLENGTH;
    const USHORT EventNameLength = EventNameMaxLength - 1;

    //
    // Validate parameters.
    //

    if (!ARGUMENT_PRESENT(InjectionError)) {

        return FALSE;

    } else {

        //
        // Clear our local error variable, then initialize to the "failed until
        // proven otherwise" state applicable to this routine.
        //

        Error.ErrorCode = 0;
        Error.InternalError = TRUE;
        Error.InvalidParameters = TRUE;
        Error.CreateInjectionContextFailed = TRUE;

    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(DestContext)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(SourceContext)) {
        goto Error;
    }

    //
    // Parameters are valid.
    //

    Error.InvalidParameters = FALSE;

    //
    // Initialize local aliases.
    //

    SourcePacket = &SourceContext->Packet;

    //
    // Calculate the required allocation size for the context and any strings.
    //

    //
    // Account for the context structure.
    //

    Size = sizeof(RTL_INJECTION_CONTEXT);

    //
    // Account for the four event name string buffers.
    //

    if (!AssertAligned8(EventNameMaxLength)) {
        return FALSE;
    }

    Size += (EventNameMaxLength * 4);

    //
    // Account for the fully-qualified representation of the Rtl .dll path,
    // plus a terminating NULL character.
    //

    AlignedRtlDllPathLength.LongPart = (
        ALIGN_UP_POINTER(
            Rtl->RtlDllPath.Length +
            sizeof(WCHAR)
        )
    );

    if (AlignedRtlDllPathLength.HighPart) {
        __debugbreak();
        return FALSE;
    }

    Size += AlignedRtlDllPathLength.LongPart;

    //
    // If we were injecting a module, account for the module Unicode path name
    // and function name ANSI string (with terminating NULLs for each).
    //

    if (SourcePacket->InjectionFlags.InjectModule) {

        //
        // Account for the module name (and verify it's under MAX_USHORT).
        //

        AlignedModulePathLength.LongPart = (
            ALIGN_UP_POINTER(
                SourcePacket->ModulePath.Length +
                sizeof(SourcePacket->ModulePath.Buffer[0])
            )
        );

        if (AlignedModulePathLength.HighPart) {
            __debugbreak();
            return FALSE;
        }

        Size += AlignedModulePathLength.LongPart;

        //
        // Account for the function name (and verify it's under MAX_USHORT).
        //

        AlignedCallbackFunctionNameLength.LongPart = (
            ALIGN_UP_POINTER(
                SourcePacket->CallbackFunctionName.Length +
                sizeof(SourcePacket->CallbackFunctionName.Buffer[0])
            )
        );

        if (AlignedCallbackFunctionNameLength.HighPart) {
            __debugbreak();
            return FALSE;
        }

        Size += AlignedCallbackFunctionNameLength.LongPart;
    }

    //
    // Sanity check our size is under MAX_USHORT.
    //

    AllocSizeInBytes.LongPart = Size;

    if (AllocSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Calculate the page-aligned size of the context, then count the number of
    // pages required to store it.  (It should be 1.)
    //

    PageAlignedAllocSizeInBytes.LongPart = (
        ROUND_TO_PAGES(AllocSizeInBytes.LongPart)
    );

    if (PageAlignedAllocSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    NumberOfPages.LongPart = PageAlignedAllocSizeInBytes.LongPart >> PAGE_SHIFT;

    if (NumberOfPages.LongPart != 1) {
        __debugbreak();
        return FALSE;
    }

    //
    // Allocate a page of memory in the remote process for the remote context,
    // then allocate two pages in our process; the first page is our version
    // of the context, the second is a scratch area we use to prepare what the
    // remote context will look like once it's injected.  That is, we adjust
    // all pointers and memory references here, then blit the entire page over
    // the fence via WriteProcessMemory().
    //

    RemoteProcessHandle = SourceContext->TargetProcessHandle;
    ActualRemoteContext = (PRTL_INJECTION_CONTEXT)(
        VirtualAllocEx(
            RemoteProcessHandle,
            NULL,
            PageAlignedAllocSizeInBytes.LongPart,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!ActualRemoteContext) {
        Error.VirtualAllocExFailed = TRUE;
        Error.RemoteMemoryInjectionFailed = TRUE;
        Error.CanCallGetLastErrorForMoreInfo = TRUE;
        goto Error;
    }

    //
    // Remote memory allocation was successful, proceed with local context
    // allocation of two pages.
    //

    NumberOfPages.LowPart += 1;
    PageAlignedAllocSizeInBytes.LongPart = NumberOfPages.LowPart << PAGE_SHIFT;

    Context = (PRTL_INJECTION_CONTEXT)(
        VirtualAlloc(
            NULL,
            PageAlignedAllocSizeInBytes.LongPart,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!Context) {
        Error.VirtualAllocFailed = TRUE;
        Error.InternalAllocationFailure = TRUE;
        Error.InjectionContextAllocationFailed = TRUE;
        goto Error;
    }

    //
    // Zero the entire allocation.
    //

    Rtl->FillPages((PCHAR)Context, 0, NumberOfPages.LowPart);

    //
    // Copy the source context over.
    //

    CopyMemory(Context, (const PVOID)SourceContext, sizeof(*Context));

    //
    // Carve out the temporary remote context in the trailing page.
    //

    RemoteContext = (PRTL_INJECTION_CONTEXT)(((PBYTE)Context) + PAGE_SIZE);
    Context->TemporaryRemoteContext = RemoteContext;
    Context->RemoteBaseContextAddress = ActualRemoteContext;

    //
    // Make a note of the context allocation size and number of pages required.
    //

    Context->TotalContextAllocSize.LongPart = AllocSizeInBytes.LongPart;
    Context->NumberOfPagesForContext = NumberOfPages.LowPart >> 1;

    //
    // Clear the flags.  (In particular, we want to clear IsStackAllocated.)
    //

    Context->Flags.IsStackAllocated = FALSE;

    //
    // Wire up a local packet alias.
    //

    Packet = &Context->Packet;

    //
    // Initialize the bootstrap functions; wire the packet's pointer up to them.
    //

    RtlpInitializeRtlInjectionFunctions(Rtl, &Context->Functions);
    Packet->Functions = &Context->Functions;

    //
    // Carve out pointers to string buffers via offsets past the context we
    // just allocated, then copy the underlying string buffers and update
    // lengths.
    //

    //
    // Rtl.dll.
    //

    Offset = sizeof(*Context);
    Unicode = &Context->RtlDllPath;
    Unicode->Length = Rtl->RtlDllPath.Length;
    Unicode->MaximumLength = AlignedRtlDllPathLength.LowPart;
    Unicode->Buffer = (PWSTR)RtlOffsetToPointer(Context, Offset);

    CopyMemory(Unicode->Buffer,
               Rtl->RtlDllPath.Buffer,
               Unicode->Length);

    Offset += AlignedRtlDllPathLength.LowPart;

    //
    // Signal event.
    //

    Unicode = &Context->SignalEventName;
    Unicode->Length = 0;
    Unicode->MaximumLength = EventNameMaxLength;
    Unicode->Buffer = (PWSTR)RtlOffsetToPointer(Context, Offset);

    Offset += EventNameMaxLength;

    //
    // Wait event.
    //

    Unicode = &Context->WaitEventName;
    Unicode->Length = 0;
    Unicode->MaximumLength = EventNameMaxLength;
    Unicode->Buffer = (PWSTR)RtlOffsetToPointer(Context, Offset);

    Offset += EventNameMaxLength;

    //
    // Caller's Signal event.
    //

    Unicode = &Context->CallerSignalEventName;
    Unicode->Length = 0;
    Unicode->MaximumLength = EventNameMaxLength;
    Unicode->Buffer = (PWSTR)RtlOffsetToPointer(Context, Offset);

    Offset += EventNameMaxLength;

    //
    // Caller's Wait event.
    //

    Unicode = &Context->CallerWaitEventName;
    Unicode->Length = 0;
    Unicode->MaximumLength = EventNameMaxLength;
    Unicode->Buffer = (PWSTR)RtlOffsetToPointer(Context, Offset);

    Offset += EventNameMaxLength;

    //
    // Module path and callback function name, if applicable.
    //

    if (SourcePacket->InjectionFlags.InjectModule) {

        //
        // Module path.
        //

        Unicode = &Packet->ModulePath;
        Unicode->Length = SourcePacket->ModulePath.Length;
        Unicode->MaximumLength = AlignedModulePathLength.LowPart;
        Unicode->Buffer = (PWSTR)RtlOffsetToPointer(Context, Offset);

        CopyMemory(Unicode->Buffer,
                   SourcePacket->ModulePath.Buffer,
                   Unicode->Length);

        Offset += Unicode->MaximumLength;

        //
        // Callback function name.
        //

        String = &Packet->CallbackFunctionName;
        String->Length = SourcePacket->CallbackFunctionName.Length;
        String->MaximumLength = AlignedCallbackFunctionNameLength.LowPart;
        String->Buffer = (PSTR)RtlOffsetToPointer(Context, Offset);

        CopyMemory(String->Buffer,
                   SourcePacket->CallbackFunctionName.Buffer,
                   String->Length);

        Offset += String->MaximumLength;

    }

    //
    // Sanity check our offset logic is sane.
    //

    ExpectedOffset = sizeof(RTL_INJECTION_CONTEXT);
    ExpectedOffset += (
        AllocSizeInBytes.LowPart -
        sizeof(RTL_INJECTION_CONTEXT)
    );

    if (!AssertTrue("Offset == ExpectedOffset", Offset == ExpectedOffset)) {
        return FALSE;
    }

    //
    // Create synchronization events.
    //

    Success = RtlpInjectionCreateEvents(Rtl, Context, &Error);

    if (!Success) {
        goto Error;
    }

    //
    // Copy the injection flags into the context.
    //

    Context->InjectionFlags.AsULong = SourcePacket->InjectionFlags.AsULong;

    //
    // Init the code size.  As we account for each thunk we need to inject, we
    // add a leading 16 bytes, in order to ensure the function always starts on
    // a 16 byte boundary, and we also round up to a 16 byte alignment at the
    // end, to ensure the offset is aligned properly for the next function.
    //

    CodeSize = 16;

    //
    // Wire up the exit thread thunk.
    //

    Context->ExitThreadThunk = (
        SKIP_JUMPS_INLINE(
            RtlpExitThreadThunk,
            PAPC_ROUTINE
        )
    );

    Success = (
        GetApproximateFunctionBoundaries(
            (ULONG_PTR)Context->ExitThreadThunk,
            (PULONG_PTR)&Context->ExitThreadThunkStartAddress,
            (PULONG_PTR)&Context->ExitThreadThunkEndAddress
        )
    );

    Context->SizeOfExitThreadThunkInBytes = (LONG)(
        Context->ExitThreadThunkEndAddress -
        Context->ExitThreadThunkStartAddress
    );

    if (!Success) {
        goto Error;
    }

    AssertAligned16(CodeSize);
    Context->ExitThreadThunkOffset = CodeSize;
    CodeSize += ALIGN_UP(Context->SizeOfExitThreadThunkInBytes, 16);

    //
    // Wire up the context protocol thunk.
    //

    Context->ContextProtocolThunk = (
        SKIP_JUMPS_INLINE(
            RtlpPostInjectionContextProtocolCallbackImpl,
            PAPC_ROUTINE
        )
    );

    Success = (
        GetApproximateFunctionBoundaries(
            (ULONG_PTR)Context->ContextProtocolThunk,
            (PULONG_PTR)&Context->ContextProtocolThunkStartAddress,
            (PULONG_PTR)&Context->ContextProtocolThunkEndAddress
        )
    );

    Context->SizeOfContextProtocolThunkInBytes = (LONG)(
        Context->ContextProtocolThunkEndAddress -
        Context->ContextProtocolThunkStartAddress
    );

    if (!Success) {
        goto Error;
    }

    AssertAligned16(CodeSize);
    CodeSize += 16;
    Context->ContextProtocolThunkOffset = CodeSize;
    CodeSize += ALIGN_UP(Context->SizeOfContextProtocolThunkInBytes, 16);

    //
    // Wire up the callback protocol thunk.
    //

    Packet->CallbackProtocolThunk = (
        SKIP_JUMPS_INLINE(
            RtlpPostInjectionProtocolCallbackImpl,
            PAPC_ROUTINE
        )
    );

    Success = (
        GetApproximateFunctionBoundaries(
            (ULONG_PTR)Packet->CallbackProtocolThunk,
            (PULONG_PTR)&Context->CallbackProtocolThunkStartAddress,
            (PULONG_PTR)&Context->CallbackProtocolThunkEndAddress
        )
    );

    Context->SizeOfCallerProtocolThunkInBytes = (LONG)(
        Context->CallbackProtocolThunkEndAddress -
        Context->CallbackProtocolThunkStartAddress
    );

    if (!Success) {
        goto Error;
    }

    AssertAligned16(CodeSize);
    CodeSize += 16;
    Context->CallbackProtocolThunkOffset = CodeSize;
    CodeSize += ALIGN_UP(Context->SizeOfCallerProtocolThunkInBytes, 16);

    //
    // The injection thunk will have already been wired up for us, so just
    // calculate its size and offset.
    //

    if (!Context->RemoteThread.InjectionThunk   ||
        !Context->InjectionThunkStartAddress    ||
        !Context->InjectionThunkEndAddress      ||
         Context->SizeOfInjectionThunkInBytes <= 0) {

        //
        // Invariant check.
        //

        __debugbreak();
        return FALSE;
    }

    AssertAligned16(CodeSize);
    CodeSize += 16;
    Context->InjectionThunkOffset = CodeSize;
    CodeSize += ALIGN_UP(Context->SizeOfInjectionThunkInBytes, 16);

    //
    // Ensure the final code allocation size fits into a single page.
    //

    PageAlignedAllocSizeInBytes.LongPart = ROUND_TO_PAGES(CodeSize);

    if (PageAlignedAllocSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    NumberOfPages.LongPart = PageAlignedAllocSizeInBytes.LongPart >> PAGE_SHIFT;

    if (NumberOfPages.LongPart != 1) {
        __debugbreak();
    }

    //
    // Save the final sizes.
    //

    Context->TotalContextCodeAllocSize.LongPart = (
        PageAlignedAllocSizeInBytes.LongPart
    );

    Context->NumberOfPagesForContextCode = NumberOfPages.LowPart;

    //
    // Everything checks out for our context code page.  Process the caller's
    // code page now.  Note that we reset the CodeSize.
    //

    if (!Context->CallerCode              ||
        !Context->CallerCodeStartAddress  ||
        !Context->CallerCodeEndAddress    ||
         Context->SizeOfCallerCodeInBytes <= 0) {

        //
        // Invariant check.
        //

        __debugbreak();
        return FALSE;
    }

    CodeSize = 16;
    Context->CallerCodeOffset = CodeSize;
    CodeSize += ALIGN_UP(Context->SizeOfCallerCodeInBytes, 16);

    PageAlignedAllocSizeInBytes.LongPart = ROUND_TO_PAGES(CodeSize);

    if (PageAlignedAllocSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    NumberOfPages.LongPart = PageAlignedAllocSizeInBytes.LongPart >> PAGE_SHIFT;

    if (NumberOfPages.LongPart != 1) {
        __debugbreak();
    }

    //
    // Save the final sizes.
    //

    Context->TotalCallerCodeAllocSize.LongPart = (
        PageAlignedAllocSizeInBytes.LongPart
    );

    Context->NumberOfPagesForCallerCode = NumberOfPages.LowPart;

    //
    // We now need to create the other remote memory blocks.  We use the same
    // approach as with the context and allocate two pages for every one page
    // we need such that we can prepare what the remote memory page needs to
    // look like programmatically before sending it over via WriteProcessMemory.
    //

    BaseContextCode = (PBYTE)(
        VirtualAlloc(
            NULL,
            PageAlignedAllocSizeInBytes.LongPart << 1,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!BaseContextCode) {
        Error.VirtualAllocFailed = TRUE;
        Error.InternalAllocationFailure = TRUE;
        Error.InjectionContextCodeAllocationFailed = TRUE;
        goto Error;
    }

    //
    // Two pages for the context code were allocated successfully on our side.
    //

    Context->BaseContextCodeAddress = BaseContextCode;
    RemoteBaseContextCode = BaseContextCode + PAGE_SIZE;
    Context->TemporaryRemoteBaseContextCodeAddress = RemoteBaseContextCode;

    //
    // Fill the code pages with 0xCC.
    //

    Rtl->FillPages((PCHAR)BaseContextCode, 0xCC, NumberOfPages.LowPart << 1);

    //
    // Now attempt the remote allocation.
    //

    ActualRemoteBaseContextCode = (PBYTE)(
        VirtualAllocEx(
            RemoteProcessHandle,
            NULL,
            PageAlignedAllocSizeInBytes.LongPart,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!ActualRemoteBaseContextCode) {
        Error.VirtualAllocExFailed = TRUE;
        Error.InternalAllocationFailure = TRUE;
        Error.InjectionCallerCodeAllocationFailed = TRUE;
        goto Error;
    }

    Context->RemoteBaseContextCodeAddress = ActualRemoteBaseContextCode;

    //
    // Now repeat the process for the caller code.
    //

    BaseCallerCode = (PBYTE)(
        VirtualAlloc(
            NULL,
            PageAlignedAllocSizeInBytes.LongPart << 1,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!BaseCallerCode) {
        Error.VirtualAllocFailed = TRUE;
        Error.InternalAllocationFailure = TRUE;
        Error.InjectionCallerCodeAllocationFailed = TRUE;
        goto Error;
    }

    //
    // Two pages for the caller code were allocated successfully on our side.
    //

    Context->BaseCallerCodeAddress = BaseCallerCode;
    RemoteBaseCallerCode = BaseCallerCode + PAGE_SIZE;
    Context->TemporaryRemoteBaseCallerCodeAddress = RemoteBaseCallerCode;

    //
    // Fill the code pages with 0xCC.
    //

    Rtl->FillPages((PCHAR)BaseCallerCode, 0xCC, NumberOfPages.LowPart << 1);

    //
    // Now attempt the remote allocation.
    //

    ActualRemoteBaseCallerCode = (PBYTE)(
        VirtualAllocEx(
            RemoteProcessHandle,
            NULL,
            PageAlignedAllocSizeInBytes.LongPart,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!ActualRemoteBaseCallerCode) {
        Error.VirtualAllocExFailed = TRUE;
        Error.InternalAllocationFailure = TRUE;
        Error.InjectionCallerCodeAllocationFailed = TRUE;
        goto Error;
    }

    Context->RemoteBaseCallerCodeAddress = ActualRemoteBaseCallerCode;

    //
    // All allocation was successful.  Copy the context page over to the remote
    // context staging area (which is in our process's address space).
    //

    Rtl->CopyPages((PCHAR)RemoteContext,
                   (PCHAR)Context,
                   Context->NumberOfPagesForContext >> 1);

    //
    // Update all the buffer addresses.
    //

#define UPDATE_BUFFER(Name, Target, Type) (          \
    Remote##Target##->##Name##.Buffer = (Type)(      \
        (ULONG_PTR)ActualRemote##Target## + (        \
            (ULONG_PTR)##Target##->##Name##.Buffer - \
            (ULONG_PTR)##Target##                    \
        )                                            \
    )                                                \
)

#define UPDATE_CONTEXT_BUFFER(Name) UPDATE_BUFFER(Name, Context, PWSTR)

    UPDATE_CONTEXT_BUFFER(RtlDllPath);
    UPDATE_CONTEXT_BUFFER(SignalEventName);
    UPDATE_CONTEXT_BUFFER(WaitEventName);
    UPDATE_CONTEXT_BUFFER(CallerSignalEventName);
    UPDATE_CONTEXT_BUFFER(CallerWaitEventName);

    RemotePacket = (PRTL_INJECTION_PACKET)(
        (ULONG_PTR)RemoteContext +
        FIELD_OFFSET(RTL_INJECTION_CONTEXT, Packet)
    );

    ActualRemotePacket = (PRTL_INJECTION_PACKET)(
        (ULONG_PTR)ActualRemoteContext +
        FIELD_OFFSET(RTL_INJECTION_CONTEXT, Packet)
    );

#define UPDATE_PACKET_BUFFER(Name, Type) UPDATE_BUFFER(Name, Packet, Type)
#define UPDATE_PACKET_STRING_BUFFER(Name) UPDATE_PACKET_BUFFER(Name, PSTR)
#define UPDATE_PACKET_UNICODE_BUFFER(Name) UPDATE_PACKET_BUFFER(Name, PWSTR)

    if (SourcePacket->InjectionFlags.InjectModule) {
        UPDATE_PACKET_STRING_BUFFER(CallbackFunctionName);
        UPDATE_PACKET_UNICODE_BUFFER(ModulePath);
    }

    RemotePacket->SignalEventName = (PCUNICODE_STRING)(
        (ULONG_PTR)ActualRemoteContext +
        (ULONG_PTR)FIELD_OFFSET(RTL_INJECTION_CONTEXT, CallerSignalEventName)
    );

    RemotePacket->WaitEventName = (PCUNICODE_STRING)(
        (ULONG_PTR)ActualRemoteContext +
        (ULONG_PTR)FIELD_OFFSET(RTL_INJECTION_CONTEXT, CallerWaitEventName)
    );

    RemotePacket->Functions = (PCRTL_INJECTION_FUNCTIONS)(
        (ULONG_PTR)ActualRemoteContext +
        (ULONG_PTR)FIELD_OFFSET(RTL_INJECTION_CONTEXT, Functions)
    );

    //
    // Now copy all the chunks of code over, updating references/pointers as
    // we go.  Once we're done, we'll send everything over to the remote
    // process via WriteProcessMemory().
    //

    //
    // Copy the exit thread thunk.
    //

    Dest = BaseContextCode + Context->ExitThreadThunkOffset;
    Source = (PBYTE)Context->ExitThreadThunkStartAddress;
    CodeSize = (USHORT)Context->SizeOfExitThreadThunkInBytes;

    CopyMemory(Dest, Source, CodeSize);

    RemoteContext->ExitThreadThunk = (PAPC_ROUTINE)(
        (ULONG_PTR)ActualRemoteBaseContextCode +
        (ULONG_PTR)Context->ExitThreadThunkOffset
    );

    //
    // Copy the context protocol thunk.
    //

    Dest = BaseContextCode + Context->ContextProtocolThunkOffset;
    Source = (PBYTE)Context->ContextProtocolThunkStartAddress;
    CodeSize = (USHORT)Context->SizeOfContextProtocolThunkInBytes;

    CopyMemory(Dest, Source, CodeSize);

    RemoteContext->ContextProtocolThunk = (PAPC_ROUTINE)(
        (ULONG_PTR)ActualRemoteBaseContextCode +
        (ULONG_PTR)Context->ContextProtocolThunkOffset
    );

    //
    // Copy the callback protocol thunk.
    //

    Dest = BaseContextCode + Context->CallbackProtocolThunkOffset;
    Source = (PBYTE)Context->CallbackProtocolThunkStartAddress;
    CodeSize = (USHORT)Context->SizeOfCallerProtocolThunkInBytes;

    CopyMemory(Dest, Source, CodeSize);

    RemotePacket->CallbackProtocolThunk = (PAPC_ROUTINE)(
        (ULONG_PTR)ActualRemoteBaseContextCode +
        (ULONG_PTR)Context->CallbackProtocolThunkOffset
    );

    //
    // Copy the injection thunk.
    //

    Dest = BaseContextCode + Context->InjectionThunkOffset;
    Source = (PBYTE)Context->InjectionThunkStartAddress;
    CodeSize = (USHORT)Context->SizeOfInjectionThunkInBytes;

    CopyMemory(Dest, Source, CodeSize);

    RemoteContext->RemoteThread.InjectionRoutine = (PAPC_ROUTINE)(
        (ULONG_PTR)ActualRemoteBaseContextCode +
        (ULONG_PTR)Context->InjectionThunkOffset
    );

    //
    // Copy the caller's code.
    //

    Dest = BaseCallerCode + Context->CallerCodeOffset;
    Source = (PBYTE)Context->CallerCodeStartAddress;
    CodeSize = (USHORT)Context->SizeOfCallerCodeInBytes;

    CopyMemory(Dest, Source, CodeSize);

    RemoteContext->CallerCode = (PAPC_ROUTINE)(
        (ULONG_PTR)ActualRemoteBaseCallerCode +
        (ULONG_PTR)Context->CallerCodeOffset
    );

    RemotePacket->CallbackProtocolThunk = RemoteContext->CallerCode;

    //
    // Handle payloads.
    //

    if (SourceContext->InjectionFlags.InjectPayload) {

        //
        // XXX todo.
        //

        __debugbreak();
    }

    //
    // We can now copy all of the pages into the remote process then set the
    // code pages to executable.
    //

    PageAlignedAllocSizeInBytes.LongPart = (
        Context->NumberOfPagesForContext << PAGE_SHIFT
    );

    Success = (
        WriteProcessMemory(
            RemoteProcessHandle,
            Context->RemoteBaseContextAddress,
            RemoteContext,
            PageAlignedAllocSizeInBytes.LongPart,
            &NumberOfBytesWritten
        )
    );

    if (!Success) {
        DWORD LastError = GetLastError();
        Error.InternalError = TRUE;
        Error.WriteRemoteMemoryFailed = TRUE;
        Error.WriteRemoteContextFailed = TRUE;
        goto Error;
    }

    if (NumberOfBytesWritten != PageAlignedAllocSizeInBytes.LongPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Copy the context code.
    //

    PageAlignedAllocSizeInBytes.LongPart = (
        Context->NumberOfPagesForContextCode << PAGE_SHIFT
    );

    Success = (
        WriteProcessMemory(
            RemoteProcessHandle,
            Context->RemoteBaseContextCodeAddress,
            BaseContextCode,
            PageAlignedAllocSizeInBytes.LongPart,
            &NumberOfBytesWritten
        )
    );

    if (!Success) {
        Error.InternalError = TRUE;
        Error.WriteRemoteMemoryFailed = TRUE;
        Error.WriteRemoteContextCodeFailed = TRUE;
        goto Error;
    }

    if (NumberOfBytesWritten != PageAlignedAllocSizeInBytes.LongPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Copy the caller's code.
    //

    PageAlignedAllocSizeInBytes.LongPart = (
        Context->NumberOfPagesForCallerCode << PAGE_SHIFT
    );

    Success = (
        WriteProcessMemory(
            RemoteProcessHandle,
            Context->RemoteBaseCallerCodeAddress,
            BaseCallerCode,
            PageAlignedAllocSizeInBytes.LongPart,
            &NumberOfBytesWritten
        )
    );

    if (!Success) {
        Error.InternalError = TRUE;
        Error.WriteRemoteMemoryFailed = TRUE;
        Error.WriteRemoteCallerCodeFailed = TRUE;
        goto Error;
    }

    if (NumberOfBytesWritten != PageAlignedAllocSizeInBytes.LongPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Remote memory allocation has been performed.  The start address for our
    // remote thread will be available, so we can create it now.  This is the
    // actual "injection" step.
    //

    RemoteThreadStartRoutine = RemoteContext->RemoteThread.StartRoutine;
    RemoteThreadParameter = ActualRemoteContext;
    RemoteThreadCreationFlags = 0;

    RemoteThreadHandle = (
        CreateRemoteThread(
            RemoteProcessHandle,
            NULL,
            0,
            RemoteThreadStartRoutine,
            RemoteThreadParameter,
            RemoteThreadCreationFlags,
            &RemoteThreadId
        )
    );

    if (!RemoteThreadHandle || RemoteThreadHandle == INVALID_HANDLE_VALUE) {
        Error.InternalError = TRUE;
        Error.CreateRemoteThreadFailed = TRUE;
        goto Error;
    }

    //
    // Remote thread has been created!  Signal and wait on the event pairs.
    //

    WaitResult = (
        SignalObjectAndWait(
            Context->SignalEventHandle,
            Context->WaitEventHandle,
            INFINITE,
            TRUE
        )
    );

    if (WaitResult != WAIT_OBJECT_0) {
        Error.SignalObjectAndWaitFailed = TRUE;
        Error.CanCallGetLastErrorForMoreInfo = TRUE;
        goto Error;
    }

    //
    // We're getting there...
    //

    __debugbreak();
    Error.AsULongLong = 0;
    Success = TRUE;
    goto End;

    //
    // Intentional follow-on to Error.
    //

Error:

    Success = FALSE;

    if (RemoteThreadHandle) {

        if (!QueueAPCToTerminateThreadOnError) {

            Rtl->ZwTerminateThread(RemoteThreadHandle, ERROR_SUCCESS);

        } else {
            DWORD WaitResult;
            PAPC_ROUTINE ApcRoutine;

            ApcRoutine = (PAPC_ROUTINE)RemoteContext->ExitThreadThunk;

            Status = Rtl->NtQueueApcThread(RemoteThreadHandle,
                                           RemoteContext->ExitThreadThunk,
                                           RemoteContextAddress,
                                           NULL,
                                           NULL);

            if (FAILED(Status)) {
                NOTHING;
            }

            WaitResult = WaitForSingleObject(RemoteThreadHandle, INFINITE);

            if (WaitResult != WAIT_OBJECT_0) {
                Rtl->ZwTerminateThread(RemoteThreadHandle, ERROR_SUCCESS);
            }
        }
        RemoteThreadHandle = NULL;
    }

    if (RemoteContextAddress) {

        VirtualFreeEx(
            SourceContext->TargetProcessHandle,
            RemoteContextAddress,
            0,
            MEM_RELEASE
        );

        RemoteContextAddress = NULL;
    }

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Merge error flags.
    //

    InjectionError->ErrorCode |= Error.ErrorCode;

    return Success;
}

_Use_decl_annotations_
BOOL
RtlpInjectionCreateEventName(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    RTL_INJECTION_EVENT_ID EventId,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    This routine creates a unique event name of a given type.

    The final event name will be a Unicode string of the format:

        L"Local\\RtlInjection_<our_pid>_<target_pid>_<event_type>_<timestamp>"

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Context - Supplies a pointer to an RTL_INJECTION_CONTEXT being prepared.

    EventId - Supplies the type of event for which the name is to be created.

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
    HRESULT Result;
    PCWSTR TypeName;
    PUNICODE_STRING EventName;
    LARGE_INTEGER Timestamp;
    RTL_INJECTION_ERROR Error;
    const UNICODE_STRING Prefix = RTL_CONSTANT_STRING(L"Local\\RtlInjection_");

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(InjectionError)) {

        return FALSE;

    } else {

        //
        // Clear our local error variable, then initialized to the "failed until
        // proven otherwise" state.
        //

        Error.ErrorCode = 0;
        Error.InternalError = TRUE;
        Error.CreateEventNameFailed = TRUE;

    }

    //
    // Wire up local aliases in line with the event ID.
    //

    switch (EventId) {

        case RtlInjectionSignalEventId:
            EventName = &Context->SignalEventName;
            TypeName = L"Signal_";
            break;

        case RtlInjectionWaitEventId:
            EventName = &Context->WaitEventName;
            TypeName = L"Wait_";
            break;

        case RtlInjectionCallerSignalEventId:
            EventName = &Context->CallerSignalEventName;
            TypeName = L"CallerSignal_";
            break;

        case RtlInjectionCallerWaitEventId:
            EventName = &Context->CallerWaitEventName;
            TypeName = L"CallerWait_";
            break;

        default:
            Error.InvalidEventId = TRUE;
            goto Error;
    }

    //
    // Verify the length is 0.
    //

    if (EventName->Length != 0) {
        Error.InvalidParameters = TRUE;
        goto Error;
    }

    //
    // Append the initial "Local\\RtlInjection_" prefix.
    //

    Result = Rtl->RtlAppendUnicodeStringToString(EventName, &Prefix);
    if (FAILED(Result)) {
        goto Error;
    }

    //
    // Append our process ID.
    //

    Success = AppendIntegerToUnicodeString(EventName,
                                           FastGetCurrentProcessId(),
                                           10,
                                           L'_');
    if (!Success) {
        goto Error;
    }

    //
    // Append the target process ID.
    //

    Success = AppendIntegerToUnicodeString(EventName,
                                           Context->Packet.TargetProcessId,
                                           10,
                                           L'_');
    if (!Success) {
        goto Error;
    }

    //
    // Append the type name.
    //

    Result = Rtl->RtlAppendUnicodeToString(EventName, TypeName);
    if (FAILED(Result)) {
        goto Error;
    }

    //
    // Capture a timestamp and append it as the final part of the string.
    //

    QueryPerformanceCounter(&Timestamp);

    Success = AppendLongLongIntegerToUnicodeString(EventName,
                                                   Timestamp.QuadPart,
                                                   20,
                                                   0);
    if (!Success) {
        goto Error;
    }

    //
    // Add a trailing NULL.
    //

    Success = AppendUnicodeCharToUnicodeString(EventName, L'\0');
    if (!Success) {
        goto Error;
    }

    //
    // Event name was created successfully.  Clear our error flags and finish.
    //

    Error.InternalError = FALSE;
    Error.CreateEventNameFailed = FALSE;

    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Merge error flags.
    //

    InjectionError->ErrorCode |= Error.ErrorCode;

    return Success;
}

_Use_decl_annotations_
BOOL
RtlpInjectionCreateEvents(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Creates synchronization events ("signal" and "wait") for an injection
    context.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Context - Supplies a pointer to an RTL_INJECTION_CONTEXT being prepared.

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
    BOOL ManualReset;
    BOOL InitialState;
    HANDLE EventHandle;
    RTL_INJECTION_ERROR Error;
    RTL_INJECTION_EVENT_ID EventId;
    LPSECURITY_ATTRIBUTES EventAttributes;
    PRTL_INJECTION_PACKET Packet;

    //
    // Clear our local error variable and initialize the packet alias.
    //

    Error.ErrorCode = 0;
    Packet = &Context->Packet;

    //
    // Create event names for the synchronization event objects used by the
    // injection context.
    //

    //
    // Create signal event name.
    //

    EventId = RtlInjectionSignalEventId;
    Success = RtlpInjectionCreateEventName(Rtl, Context, EventId, &Error);

    if (!Success) {
        goto Error;
    }

    //
    // Create wait event name.
    //

    EventId = RtlInjectionWaitEventId;
    Success = RtlpInjectionCreateEventName(Rtl, Context, EventId, &Error);

    if (!Success) {
        goto Error;
    }

    //
    // Create caller's signal event name.
    //

    EventId = RtlInjectionCallerSignalEventId;
    Success = RtlpInjectionCreateEventName(Rtl, Context, EventId, &Error);

    if (!Success) {
        goto Error;
    }

    //
    // Create caller's wait event name.
    //

    EventId = RtlInjectionCallerWaitEventId;
    Success = RtlpInjectionCreateEventName(Rtl, Context, EventId, &Error);

    if (!Success) {
        goto Error;
    }

    //
    // Create events from the names we just generated.  Events are created as
    // auto-reset (ManualReset = FALSE) and in a non-signaled initial state
    // (InitialState = FALSE).
    //

    EventAttributes = NULL;
    ManualReset = FALSE;
    InitialState = FALSE;

    //
    // Create signal event.
    //

    EventHandle = Rtl->CreateEventW(EventAttributes,
                                    ManualReset,
                                    InitialState,
                                    Context->SignalEventName.Buffer);

    if (!EventHandle || EventHandle == INVALID_HANDLE_VALUE) {
        Error.CreateEventWFailed = TRUE;
        goto Error;
    }

    Context->SignalEventHandle = EventHandle;

    //
    // Create wait event.
    //

    EventHandle = Rtl->CreateEventW(EventAttributes,
                                    ManualReset,
                                    InitialState,
                                    Context->WaitEventName.Buffer);

    if (!EventHandle || EventHandle == INVALID_HANDLE_VALUE) {
        Error.CreateEventWFailed = TRUE;
        goto Error;
    }

    Context->WaitEventHandle = EventHandle;

    //
    // Create caller signal event and reflect in packet.
    //

    EventHandle = Rtl->CreateEventW(EventAttributes,
                                    ManualReset,
                                    InitialState,
                                    Context->CallerSignalEventName.Buffer);

    if (!EventHandle || EventHandle == INVALID_HANDLE_VALUE) {
        Error.CreateEventWFailed = TRUE;
        goto Error;
    }

    Packet->SignalEventHandle = Context->CallerSignalEventHandle = EventHandle;
    Packet->SignalEventName = &Context->CallerSignalEventName;

    //
    // Create caller wait event and reflect in packet.
    //

    EventHandle = Rtl->CreateEventW(EventAttributes,
                                    ManualReset,
                                    InitialState,
                                    Context->CallerWaitEventName.Buffer);

    if (!EventHandle || EventHandle == INVALID_HANDLE_VALUE) {
        Error.CreateEventWFailed = TRUE;
        goto Error;
    }

    Packet->WaitEventHandle = Context->CallerWaitEventHandle = EventHandle;
    Packet->WaitEventName = &Context->CallerWaitEventName;

    //
    // Events were created successfully, indicate success and return.
    //

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Merge error flags.
    //

    InjectionError->ErrorCode |= Error.ErrorCode;

    return Success;
}

_Use_decl_annotations_
BOOL
RtlpInjectionAllocateRemoteMemory(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    This routine allocates memory required for injection in a remote process.
    Allocations are performed separately for a) the initial injection thread
    thunk, b) the caller's completion callback routine, c) the injection context
    structure, and d) the caller's payload, if one has been requested.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Context - Supplies a pointer to an RTL_INJECTION_CONTEXT structure that is
        being used for this injection.

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
    //USHORT Offset;
    //USHORT ExpectedOffset;
    //USHORT NumberOfPages;
    USHORT PageAlignedAllocSizeInBytes;
    //PSTRING String;
    //PUNICODE_STRING Unicode;
    HANDLE RemoteProcessHandle;
    RTL_INJECTION_ERROR Error;
    PRTL_INJECTION_CONTEXT RemoteContext;

    if (!ARGUMENT_PRESENT(InjectionError)) {

        return FALSE;

    } else {

        //
        // Clear our local error variable, then initialize to the "failed until
        // proven otherwise" state applicable to this routine.
        //

        Error.ErrorCode = 0;
        Error.InternalError = TRUE;
        Error.InvalidParameters = TRUE;
        Error.RemoteMemoryInjectionFailed = TRUE;

    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        goto Error;
    }

    if (!ARGUMENT_PRESENT(Context)) {
        goto Error;
    }

    //
    // Parameters are valid.
    //

    Error.InvalidParameters = FALSE;

    //
    // Initialize aliases.
    //

    RemoteProcessHandle = Context->TargetProcessHandle;

    PageAlignedAllocSizeInBytes = (
        Context->NumberOfPagesForContext << PAGE_SHIFT
    );

    //
    // Make sure it's only 1 page for now.
    //

    if (PageAlignedAllocSizeInBytes != PAGE_SIZE) {
        __debugbreak();
        return FALSE;
    }

    RemoteContext = (PRTL_INJECTION_CONTEXT)(
        VirtualAllocEx(
            RemoteProcessHandle,
            NULL,
            PageAlignedAllocSizeInBytes,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!RemoteContext) {
        Error.VirtualAllocExFailed = TRUE;
        Error.RemoteContextAllocationFailed = TRUE;
        Error.CanCallGetLastErrorForMoreInfo = TRUE;
        goto Error;
    }

    __debugbreak();

    Success = TRUE;

    if (Success) {

        //
        // Everything completed successfully, we're done.
        //

        Error.InternalError = FALSE;
        Error.RemoteMemoryInjectionFailed = FALSE;
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

    //
    // Merge error flags.
    //

    InjectionError->ErrorCode |= Error.ErrorCode;

    return Success;
}

_Use_decl_annotations_
VOID
RtlpInitializeRtlInjectionFunctions(
    PRTL Rtl,
    PRTL_INJECTION_FUNCTIONS Functions
    )
{
    PBYTE Code;
    PBYTE NewCode;
    PBYTE *Function;
    PBYTE *Pointers;
    USHORT Index;
    USHORT NumberOfFunctions;

    Functions->SetEvent = Rtl->SetEvent;
    Functions->OpenEventW = Rtl->OpenEventW;
    Functions->CloseHandle = Rtl->CloseHandle;
    Functions->GetProcAddress = Rtl->GetProcAddress;
    Functions->LoadLibraryExW = Rtl->LoadLibraryExW;
    Functions->SignalObjectAndWait = Rtl->SignalObjectAndWait;
    Functions->WaitForSingleObjectEx = Rtl->WaitForSingleObjectEx;
    Functions->OutputDebugStringA = Rtl->OutputDebugStringA;
    Functions->OutputDebugStringW = Rtl->OutputDebugStringW;
    Functions->NtQueueUserApcThread = Rtl->NtQueueApcThread;
    Functions->NtTestAlert = Rtl->NtTestAlert;
    Functions->QueueUserAPC = Rtl->QueueUserAPC;
    Functions->SleepEx = Rtl->SleepEx;
    Functions->ExitThread = Rtl->ExitThread;
    Functions->GetExitCodeThread = Rtl->GetExitCodeThread;
    Functions->CreateFileMappingW = Rtl->CreateFileMappingW;
    Functions->OpenFileMappingW = Rtl->OpenFileMappingW;
    Functions->MapViewOfFileEx = Rtl->MapViewOfFileEx;
    Functions->FlushViewOfFile = Rtl->FlushViewOfFile;
    Functions->UnmapViewOfFileEx = Rtl->UnmapViewOfFileEx;

    Pointers = (PBYTE *)Functions;
    NumberOfFunctions = sizeof(*Functions) / sizeof(ULONG_PTR);

    for (Index = 0; Index < NumberOfFunctions; Index++) {
        Function = Pointers + Index;
        Code = *Function;
        NewCode = SkipJumpsInline(Code);
        *Function = NewCode;
    }
}


_Use_decl_annotations_
BOOL
RtlpInjectionCreateRemoteThread(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR InjectionError
    )
/*++

Routine Description:

    Creates a remote thread in the process that is being targeted for injection.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Context - Supplies a pointer to the injection context being tested.

    InjectionError - Supplies a pointer to a variable that receives information
        about the error, if one occurred (as indicated by this routine returning
        FALSE).

Return Value:

    If the routine completes successfully, TRUE is returned.  If a failure
    occurs, FALSE is returned and InjectionError is set with the relevant
    error code.

--*/
{
    //
    // WIP.
    //

    __debugbreak();

    return FALSE;
}

_Use_decl_annotations_
VOID
RtlpDestroyInjectionContext(
    PPRTL_INJECTION_CONTEXT ContextPointer
    )
/*++

Routine Description:

    Destroys an injection context.

Arguments:

    ContextPointer - Supplies a pointer to a variable that contains the address
        of the context to destroy.  This pointer will be cleared upon return
        of this routine.

Return Value:

    None.

--*/
{
    PRTL_INJECTION_CONTEXT Context;

    if (!ARGUMENT_PRESENT(ContextPointer)) {
        return;
    }

    Context = *ContextPointer;

    if (!ARGUMENT_PRESENT(Context)) {
        return;
    }

    //
    // WIP.
    //

    __debugbreak();

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
