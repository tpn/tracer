/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    __C_specific_handler.c

Abstract:

    This module implements the __C_specific_handler endpoint.

--*/

#include "stdafx.h"
#include "__C_specific_handler.h"

P__C_SPECIFIC_HANDLER __C_specific_handler_impl = NULL;

VOID
SetCSpecificHandler(
    P__C_SPECIFIC_HANDLER Handler
    )
{
    __C_specific_handler_impl = Handler;
}

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
