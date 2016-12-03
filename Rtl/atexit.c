/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    RtlAtExit.c

Abstract:

    This module implements our custom atexit() support.  Functions are provided
    for registering new functions to call at process exit.  The actual act of
    calling registered functions at exit is done by hooking into dllmain.c's
    _DllMainCRTStartup() function; specifically, the DLL_PROCESS_DETACH message.

--*/

#include "stdafx.h"

PATEXIT atexit_impl = NULL;

VOID
SetAtExit(
    PATEXIT AtExit
    )
{
    atexit_impl = AtExit;
}

#pragma warning(push)
#pragma warning(disable: 4028 4273)

INT
atexit(
    PATEXITFUNC AtExitFunction
    )
{
    return atexit_impl(AtExitFunction);
}

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
