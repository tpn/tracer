/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTablePrivate.h

Abstract:

    This is the private header file for the StringTable component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
// Function typedefs and inline functions for internal modules.
////////////////////////////////////////////////////////////////////////////////

#pragma intrinsic(__popcnt16)

FORCEINLINE
USHORT
GetNumberOfStringsInTable(
    _In_ PSTRING_TABLE StringTable
    )
{
    return (USHORT)__popcnt16(StringTable->OccupiedBitmap);
}

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
