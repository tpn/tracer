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
// Assembly for the GetInstructionPointer() function.
//

CONST BYTE GetInstructionPointerByteCode[] = {
    0x48, 0x8B, 0x04, 0x24,     //  mov     rax, qword ptr [rsp]
    0xC3,                       //  ret
};


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
IsJump(
    PBYTE Code
    )
/*++

Routine Description:

    Given an address of AMD64 byte code, returns TRUE if the underlying
    instruction is a jump.

Arguments:

    Code - Supplies a pointer to the first byte of the code.

Return Value:

    TRUE if this address represents a jump, FALSE otherwise.

--*/
{
    return IsJumpInline(Code);
}


_Use_decl_annotations_
PBYTE
FollowJumpsToFirstCodeByte(
    PBYTE Code
    )
/*++

Routine Description:

    Given an address of AMD64 byte code, follows any jumps until the first non-
    jump byte code is found, and returns that byte.  Alternatively, if the first
    bytes passed in do not indicate a jump, the original byte code address is
    returned.

    This is used to traverse jump tables and get to the actual underlying
    function.

Arguments:

    Code - Supplies a pointer to the first byte of the code for which any jumps
        should be followed.

Return Value:

    The address of the first non-jump byte code encountered.

--*/
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
/*++

Routine Description:

    Given an address which resides within a function, return the approximate
    start and end addresses of a function by searching forward and backward
    for repeat occurrences of the `int 3` (breakpoint) instruction, represented
    by 0xCC in byte code.  Such occurrences are usually good indicators of
    function boundaries -- in fact, the Microsoft AMD64 calling convention
    requires that function entry points should be padded with 6 bytes (to allow
    for hot-patching), and this padding is always the 0xCC byte.  The search is
    terminated in both directions once two successive 0xCC bytes are found.

    The term "approximate" is used to qualify both the start and end addresses,
    because although scanning for repeat 0xCC occurrences is quite reliable, it
    is not as reliable as a more authoritative source of function size, such as
    debug symbols.

Arguments:

    Address - Supplies an address that resides somewhere within the function
        for which the boundaries are to be obtained.

    StartAddress - Supplies a pointer to a variable that will receive the
        address of the approximate starting point of the function.

    EndAddress - Supplies a pointer to a variable that will receive the
        address of the approximate ending point of the function (this will
        typically be the `ret` (0xC3) instruction).

Return Value:

    TRUE if the method was successful, FALSE otherwise.  FALSE will only be
    returned if parameter validation fails.  StartAddress and EndAddress will
    not be updated in this case.

--*/
{
    return GetApproximateFunctionBoundariesInline(Address,
                                                  StartAddress,
                                                  EndAddress);
}

_Use_decl_annotations_
BOOL
RtlpTestInjectionCompleteCallback(
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


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
