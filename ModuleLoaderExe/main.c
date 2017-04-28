/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This module is the main entry point for the tracer executable.
    It implements mainCRTStartup(), which simply calls LoadModulesExeMain().

--*/

#include "stdafx.h"

LONG ModuleLoaderExeMain(VOID);

DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    ExitProcess(ModuleLoaderExeMain());
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
