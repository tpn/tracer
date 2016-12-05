/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    AtExitEx.c

Abstract:

    This module implements the SetAtExitEx() and AtExitEx() stubs.

--*/

#include "stdafx.h"

PATEXITEX AtExitExImpl = NULL;

VOID
SetAtExitEx(
    PATEXITEX AtExitEx
    )
{
    AtExitExImpl = AtExitEx;
}

#pragma warning(push)
#pragma warning(disable: 4028 4273)

_Use_decl_annotations_
BOOL
AtExitEx(
    PATEXITEX_CALLBACK Callback,
    PATEXITEX_FLAGS Flags,
    PVOID Context,
    struct _RTL_ATEXIT_ENTRY **EntryPointer
    )
{
    return AtExitExImpl(Callback, Flags, Context, EntryPointer);
}

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
