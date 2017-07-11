/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionPrivate.c

Abstract:

    This module implements private routines pertaining to the injection of code
    into a remote process.  Routines are provided for getting an address of a
    currently executing function, obtaining an approximate code size of a
    function given an address within the function, skipping jumps when given
    a pointer to byte code.

--*/

#include "stdafx.h"


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
