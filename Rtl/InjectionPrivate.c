/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionPrivate.c

Abstract:

    This module implements private routines pertaining to the remote thread
    creation and code injection functionality of the Rtl component.

--*/

#include "stdafx.h"

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

        ActualMagicNumber = InjectionCompleteCallback(Rtl, &Packet);

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
