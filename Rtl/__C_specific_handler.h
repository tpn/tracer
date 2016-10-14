/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    __C_specific_handler.h

Abstract:

    This module provides the necessary scaffolding to use structured exception
    handling via __try/__except/__finally C language constructs without needing
    to link to a C runtime library.

    To make use of this functionality, include this file in one of your C file
    components, after Rtl.h has been included.  After loading Rtl, set the
    __C_specific_handler_impl to the value of Rtl->__C_specific_handler.

--*/

#ifdef _NO_RTL

#include <Windows.h>

typedef
EXCEPTION_DISPOSITION
(__cdecl __C_SPECIFIC_HANDLER)(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
    );
typedef __C_SPECIFIC_HANDLER *P__C_SPECIFIC_HANDLER;

typedef
VOID
(SET_C_SPECIFIC_HANDLER)(
    _In_ P__C_SPECIFIC_HANDLER Handler
    );
typedef SET_C_SPECIFIC_HANDLER *PSET_C_SPECIFIC_HANDLER;

#endif

extern P__C_SPECIFIC_HANDLER __C_specific_handler_impl;
__declspec(dllexport) SET_C_SPECIFIC_HANDLER SetCSpecificHandler;

#pragma warning(push)
#pragma warning(disable: 4028 4273)

__C_SPECIFIC_HANDLER __C_specific_handler;

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
