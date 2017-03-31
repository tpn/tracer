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
CreateInjectionPacket(
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
DestroyInjectionPacket(
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
AddInjectionPayload(
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
AddInjectionSymbols(
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
Inject(
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
