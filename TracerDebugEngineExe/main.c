/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This is the main module for the TracerDebugEngineExEngineExee component.
    It implements a Main() function and provides a mainCRTStartup() entry point
    which calls Main().

--*/

#include "stdafx.h"

ULONG
Main(VOID)
{
    ULONG ExitCode = 1;

    return ExitCode;
}


VOID
WINAPI
mainCRTStartup()
{
    ExitProcess(Main());
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
