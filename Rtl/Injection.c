/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Injection.c

Abstract:

    This module implements the Rtl component's remote thread and code injection
    component.  Functions are provided for creating injection packets, adding
    injection payloads to packets, adding symbol requests to packets, destroying
    packets, creating remote threads, hijacking existing threads, and performing
    the injecting.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
RtlCreateInjectionPacket(
    PRTL Rtl,
    PRTL_INJECTION_PACKET_FLAGS Flags,
    PCUNICODE_STRING ModulePath,
    PSTRING CallbackFunctionName,
    ULONG SizeOfCodeInBytes,
    PBYTE Code,
    ULONG TargetProcessId,
    ULONG OptionalTargetThreadId,
    PRTL_INJECTION_ERROR InjectionError,
    PRTL_INJECTION_PACKET *InjectionPacketPointer
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

    Flags - Supplies a pointer to injection packet flags.  At minimum, either
        Flags.InjectModule or Flags.InjectCode must be set, indicating which
        type of injection method should be used.

    ModulePath - Supplies a pointer to a fully-qualifed UNICODE_STRING
        structure representing the module path to load into the remote process.
        If Flags.InjectModule is set, both this value and CallbackFunctionName
        must be valid.

    CallbackFunctionName - Supplies a pointer to a STRING structure representing
        the name of an exported symbol to resolve once the module indicated by
        the ModulePath parameter is loaded.  This must be non-NULL and point to
        a valid string if Flags.InjectModule is TRUE, otherwise it must be NULL.

    SizeOfCodeInBytes - Supplies the size, in bytes, of the buffer pointed to
        by the Code parameter.  This parameter is mandatory if Flags.InjectCode
        is TRUE and must by non-zero.  Otherwise, it must be zero.

    Code - Supplies a pointer to a buffer of code to be injected into the
        process and then executed by the target thread.  This must be NULL
        if Flags.InjectModule is set, otherwise, it should point to a valid
        buffer whose size is indicated by the SizeOfCodeInBytes parameter if
        Flags.InjectCode is set.

    TargetProcessId - Supplies the ID of the target process to performn the
        injection.

    TargetThreadId - Optionally supplies the ID of an existing thread in the
        target process to hijack instead of creating a new remote thread.

    InjectionError - Supplies a pointer to a variable that receives information
        about the error, if one occurred (as indicated by this routine returning
        FALSE).

    InjectionPacketPointer - Supplies a pointer to a variable that will receive
        the address of the newly-created RTL_INJECTION_PACKET if this routine
        completes successfully.  The packet can then have payload or symbols
        added to it before being injected.

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
RtlDestroyInjectionPacket(
    PRTL Rtl,
    PRTL_INJECTION_PACKET *PacketPointer
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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
