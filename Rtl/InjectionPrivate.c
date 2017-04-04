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
FollowJumpsToFirstCodeByte(
    PBYTE Code
    )
{
    return FollowJumpsToFirstCodeByteInline(Code);
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

//
// Assembly for the GetInstructionPointer() function.
//

CONST BYTE GetInstructionPointerByteCode[] = {
    0x48, 0x8B, 0x04, 0x24,     //  mov     rax, qword ptr [rsp]
    0xC3,                       //  ret
};


_Use_decl_annotations_
BOOL
RtlpInjectionCallbackVerifyMagicNumberInline(
    PRTL Rtl,
    PRTL_INJECTION_COMPLETE_CALLBACK InjectionCompleteCallback,
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
            Packet.MagicNumber.LowPart ^
            Packet.MagicNumber.HighPart
        );

    N.B. The purpose of this routine is to help catch programmer mistakes in
         wiring up the injection code, not provide any level of additional
         security, which is why a simple XOR is chosen over a more sophisticated
         system.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    InjectionCompleteCallback - Supplies a pointer to a function that will be
        invoked as per the pre-injection complete callback test.

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
    ULONG ExpectedMagicNumber;
    ULONG ActualMagicNumber;
    PBYTE MagicBuffer;
    RTL_INJECTION_PACKET Packet;
    RTL_INJECTION_ERROR Error;

    //
    // Zero the memory up-front so we don't leak any stack information.
    //

    SecureZeroMemory(&Packet, sizeof(Packet));

    //
    // Clear our local error variable.
    //

    Error.ErrorCode = 0;

    //
    // Generate 8 bytes of random data.
    //

    MagicSize = (USHORT)sizeof(Packet.MagicNumber);
    MagicBuffer = (PBYTE)&Packet.MagicNumber;
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
        Packet.MagicNumber.LowPart ^
        Packet.MagicNumber.HighPart
    );

    //
    // Set the relevant packet flag to indicate this is a callback test.
    //

    Packet.Flags.IsInjectionCompleteCallbackTest = TRUE;

    //
    // Call the routine.
    //

    EncounteredException = FALSE;

    __try {

        ActualMagicNumber = InjectionCompleteCallback(&Packet);

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
RtlpVerifyInjectionCallback(
    PRTL Rtl,
    PRTL_INJECTION_CONTEXT Context,
    PRTL_INJECTION_ERROR Error
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
    BOOL EncounteredException;
    ULONG MagicSize;
    ULONG ExpectedMagicNumber;
    ULONG ActualMagicNumber;
    PBYTE MagicBuffer;
    RTL_INJECTION_PACKET Packet;
    RTL_INJECTION_ERROR Error;

    //
    // Zero the memory up-front so we don't leak any stack information.
    //

    SecureZeroMemory(&Packet, sizeof(Packet));

    //
    // Clear our local error variable.
    //

    Error.ErrorCode = 0;

    //
    // Generate 8 bytes of random data.
    //

    MagicSize = (USHORT)sizeof(Packet.MagicNumber);
    MagicBuffer = (PBYTE)&Packet.MagicNumber;
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
        Packet.MagicNumber.LowPart ^
        Packet.MagicNumber.HighPart
    );

    //
    // Set the relevant packet flag to indicate this is a callback test.
    //

    Packet.Flags.IsInjectionCompleteCallbackTest = TRUE;

    //
    // Call the routine.
    //

    EncounteredException = FALSE;

    __try {

        ActualMagicNumber = InjectionCompleteCallback(&Packet);

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
RtlpInjectionCallbackVerifyMagicNumber(
    PRTL Rtl,
    PRTL_INJECTION_COMPLETE_CALLBACK Callback,
    PRTL_INJECTION_ERROR Error
    )
{
    return RtlpInjectionCallbackVerifyMagicNumberInline(Rtl,
                                                        Callback,
                                                        Error);
}


FORCEINLINE
BOOL
RtlIsInjectionProtocolCallbackInline(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Outptr_result_maybenull_ PVOID Token
    )
/*++

Routine Description:

    This routine implements the injection callback protocol required by injected
    code routines.  It should be called as the first step in the injected code,
    and must return the ReturnValue if it returns TRUE.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    ReturnValue - Supplies the address of a variable that will receive the
        return value if this is a protocol callback.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the ReturnValue parameter.

--*/
{
    if (RtlIsInjectionCodeSizeQueryInline(Packet, ReturnValue)) {
        return TRUE;
    }

    if (RtlIsInjectionCompleteCallbackTestInline(Packet, ReturnValue)) {
        return TRUE;
    }

    return FALSE;
}

#pragma optimize("", off)
LONG
RtlRemoteInjectionInitThunk(
    PRTL_INJECTION_CONTEXT Context
    )
{
    PRTL Rtl;
    RTL RtlLocal;
    ULONG SizeOfRtl;
    ULONG Token;
    ULONG Result;
    HMODULE Module;
    PINITIALIZE_RTL InitialzeRtl;
    PGET_PROC_ADDRESS GetProcAddress;
    PLOAD_LIBRARY_EX_W LoadLibraryExW;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Context)) {
        return InjectedRemoteThreadNullContext;
    }

    //
    // Adhere to the context callback protocol.
    //

    if (Context->IsInjectionContextProtocolCallback(Context, &Token)) {
        return Token;
    }

    //
    // Initialize local function pointers.
    //

    GetProcAddress = Context->GetProcAddress;
    LoadLibraryExW = Context->LoadLibraryExW;

    //
    // Load the Rtl module.
    //

    Module = LoadLibraryExW(Context->RtlDllPath.Buffer, NULL, 0);

    if (!Module || Module == INVALID_HANDLE_VALUE) {
        return InjectedRemoteThreadLoadLibraryRtlFailed;
    }

    //
    // Get the address of the InitializeRtl() function.
    //

    InitializeRtl = (PINITIALIZE_RTL)GetProcAddress(Module, "InitializeRtl");
    if (!InitialzeRtl) {
        return InjectedRemoteThreadResolveInitializeRtlFailed;
    }

    //
    // Initialize our local, stack-allocated RTL structure.
    //

    Rtl = &RtlLocal;
    SizeOfRtl = sizeof(*Rtl);

    if (!InitializeRtl(Rtl, &SizeOfRtl)) {
        return InjectedRemoteThreadInitializeRtlFailed;
    }

    //
    // Defer to our main dispatch routine.
    //

    Result = Rtl->InjectedRemoteThreadEntry(Context);

    return Result;
}
#pragma optimize("", on)


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
