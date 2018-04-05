/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    StringTableAllocator.c

Abstract:

    This module wraps the Rtl bootstrap heap allocator.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeStringTableAllocator(
    PRTL_BOOTSTRAP RtlBootstrap,
    PALLOCATOR Allocator
    )
{
    return RtlBootstrap->InitializeHeapAllocator(Allocator);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
