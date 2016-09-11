/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTableConstants.h

Abstract:

    This module defines constants related to the StringTable component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

static CONST USHORT StringArraysPerTable = 16;


FORCEINLINE
USHORT
GetNumberOfTablesRequiredForStringArray(
    _In_ PSTRING_ARRAY StringArray
    )
{
    return ALIGN_UP(StringArray->NumberOfElements, 16) / 16;
}

#ifdef __cplusplus
}; // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
