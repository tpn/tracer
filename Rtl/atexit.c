/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    atexit.c

Abstract:

    This module implements the atexit endpoint.  Include this module as a
    source file in any project that requires atexit() linkage.

    N.B. This is currently done automatically for all projects when using PGO.

--*/

#include "stdafx.h"
#include "atexit.h"

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

typedef ATEXIT atexit;

_Use_decl_annotations_
INT
atexit(
    PATEXITFUNC AtExitFunction
    )
{
    return atexit_impl(AtExitFunction);
}

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
