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

#endif

//
// As we don't link to the CRT, we don't get a __C_specific_handler entry,
// which the linker will complain about as soon as we use __try/__except.
// What we do is define a __C_specific_handler_impl pointer to the original
// function (that lives in ntdll), then implement our own function by the
// same name that calls the underlying impl pointer.  In order to do this
// we have to disable some compiler/linker warnings regarding mismatched
// stuff.
//

static P__C_SPECIFIC_HANDLER __C_specific_handler_impl = NULL;

#pragma warning(push)
#pragma warning(disable: 4028 4273)

EXCEPTION_DISPOSITION
__cdecl
__C_specific_handler(
    PEXCEPTION_RECORD ExceptionRecord,
    ULONG_PTR Frame,
    PCONTEXT Context,
    struct _DISPATCHER_CONTEXT *Dispatch
    )
{
    return __C_specific_handler_impl(ExceptionRecord,
                                     Frame,
                                     Context,
                                     Dispatch);
}

#pragma warning(pop)


#include "stdafx.h"

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
