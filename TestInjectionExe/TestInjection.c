/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TestInjectionExe.c

Abstract:

    This module provides routines for testing the main public interfaces of the
    Rtl component's injection functionality.

--*/

#include "stdafx.h"

typedef STARTUPINFOW *PSTARTUPINFOW;

UNICODE_STRING ThunkExeFilename = RTL_CONSTANT_STRING(L"thunk.exe");
PUNICODE_STRING ThunkExePath = NULL;

ULONG
CALLBACK
InjectionCallback1(
    PRTL_INJECTION_PACKET Packet
    )
{
    ULONGLONG Token;

    if (Packet->IsInjectionProtocolCallback(Packet, &Token)) {
        return (ULONG)Token;
    }

    Packet->Functions->OutputDebugStringA("Entered Injection1 callback.\n");

    return 1;
}

BOOL
CreateThunkExe(
    PSTARTUPINFOW StartupInfo,
    PPROCESS_INFORMATION ProcessInfo
    )
{
    BOOL Success;
    DWORD LastError;

    __stosq((PDWORD64)StartupInfo, 0, sizeof(*StartupInfo) >> 3);
    __stosq((PDWORD64)ProcessInfo, 0, sizeof(*ProcessInfo) >> 3);

    StartupInfo->cb = sizeof(*StartupInfo);

    Success = CreateProcessW(ThunkExePath->Buffer,
                             NULL,
                             NULL,
                             NULL,
                             FALSE,
                             0,
                             NULL,
                             NULL,
                             StartupInfo,
                             ProcessInfo);

    if (!Success) {
        LastError = GetLastError();
    }

    return Success;
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

    RTL_INJECTION_FLAGS Flags;
    PRTL_INJECTION_PACKET Packet;
    PRTL_INJECTION_COMPLETE_CALLBACK Callback;
    RTL_INJECTION_ERROR Error;
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;

    if (!CreateThunkExe(&StartupInfo, &ProcessInfo)) {
        return GetLastError();
    }

    if (!Rtl->LoadDbgHelp(Rtl)) {
        return (ULONG)-1;
    }

    Flags.AsULong = 0;
    Flags.InjectCode = TRUE;

    Callback = InjectionCallback1;

    Success = Rtl->Inject(Rtl,
                          Allocator,
                          &Flags,
                          NULL,
                          NULL,
                          Callback,
                          NULL,
                          ProcessInfo.dwProcessId,
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
    if (!MakeTracerPath(TracerConfig, &ThunkExeFilename, &ThunkExePath)) {
        return ~0;
    }

    TEST(1);

    return ERROR_SUCCESS;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
