/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    atexit.h

Abstract:

    This module provides the necessary scaffolding to use the atexit() function
    without needing to link to a C runtime library.

    To make use of this functionality, include this file in one of your C file
    components, after Rtl.h has been included.  After loading Rtl, set the
    atexit_impl to the value of Rtl->atexit.

--*/

#ifdef _NO_RTL

typedef
VOID
(__cdecl ATEXITFUNC)(
    VOID
    );
typedef ATEXITFUNC *PATEXITFUNC;

typedef
_Success_(return == 0)
INT
(ATEXIT)(
    _In_ PATEXITFUNC AtExitFunction
    );
typedef ATEXIT *PATEXIT;

#endif

extern PATEXIT atexit_impl;
__declspec(dllexport) SET_ATEXIT SetAtExit;

#pragma warning(push)
#pragma warning(disable: 4028 4273)

ATEXIT atexit;

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
