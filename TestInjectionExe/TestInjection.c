/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TestInjectionExe.c

Abstract:

    This module provides routines for testing the main public interfaces of the
    Rtl component's injection functionality.

--*/

#include "stdafx.h"

ULONGLONG
CALLBACK
InjectionCallback1(
    PRTL_INJECTION_PACKET Packet
    )
{
    ULONGLONG Token;

    if (Packet->IsInjectionProtocolCallback(Packet, &Token)) {
        return Token;
    }

    return 1;
}

ULONG
TestInjection1(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    BOOL Success;
    //LONG SizeOfCode;
    //PBYTE Code;

    RTL_CREATE_INJECTION_PACKET_FLAGS CreateFlags;
    PRTL_INJECTION_PACKET Packet;
    PRTL_INJECTION_COMPLETE_CALLBACK Callback;
    RTL_INJECTION_ERROR Error;

    CreateFlags.AsULong = 0;
    CreateFlags.InjectCode = TRUE;

    Callback = InjectionCallback1;

    Success = Rtl->CreateInjectionPacket(Rtl,
                                         Allocator,
                                         &CreateFlags,
                                         NULL,
                                         NULL,
                                         Callback,
                                         1,
                                         0,
                                         &Packet,
                                         &Error);


    return ERROR_SUCCESS;
}

#define TEST(n) TestInjection##n##(Rtl, Allocator, TracerConfig, Session)

_Use_decl_annotations_
ULONG
TestInjection(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    TEST(1);

    return ERROR_SUCCESS;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :