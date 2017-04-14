/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This module is the main entry point for the thunk executable.
    It implements mainCRTStartup().

--*/

#include "stdafx.h"

VOID
WINAPI
mainCRTStartup()
{
    ULONG ExitCode = 0;

    SleepEx(INFINITE, TRUE);

    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
