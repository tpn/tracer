/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This module is the main entry point for the thunk executable.
    It implements mainCRTStartup().

--*/

#include "stdafx.h"

#pragma optimize("", off)
DECLSPEC_NOINLINE
LONG
Test2(
    _In_ LONG Input
    )
{
    return Input * 2;
}

DECLSPEC_NOINLINE
LONG
Test1(
    _In_ LONG Input
    )
{
    LONG Input2;

    Input2 = Test2(Input);
    return Input / 2;
}

DECLSPEC_NOINLINE
LONG
Test3(
    _In_ LONG Input
    )
{
    return Test2(Input);
}

VOID
WINAPI
mainCRTStartup()
{
    ULONG Input = 8;
    ULONG Output;
    ULONG ExitCode = 0;

    Output = Test1(Input);
    Output = Test3(Input);

    SleepEx(INFINITE, TRUE);

    ExitProcess(ExitCode);
}
#pragma optimize("", on)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
