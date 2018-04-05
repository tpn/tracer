/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringLoadStoreOperations.h

Abstract:

    This module contains macros and inline routines implementing various
    approaches for loading memory into XMM/YMM registers and storing XMM/YMM
    register values into memory.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

/*++

    VOID
    LoadSearchStringIntoXmmRegister_SEH(
        _In_ STRING_SLOT Slot,
        _In_ PSTRING String,
        _In_ USHORT LengthVar
        );

Routine Description:

    Attempts an aligned 128-bit load of String->Buffer into Slot.CharXmm via
    the _mm_stream_load_si128() intrinsic.  The intrinsic is surrounded in a
    __try/__except block that catches EXCEPTION_ACCESS_VIOLATION exceptions.

    If such an exception is caught, the routine will check to see if the string
    buffer's address will cross a page boundary if 16-bytes are loaded.  If a
    page boundary would be crossed, a __movsb() intrinsic is used to copy only
    the bytes specified by String->Length, otherwise, an unaligned 128-bit load
    is attemped via the _mm_loadu_si128() intrinsic.

Arguments:

    Slot - Supplies the STRING_SLOT local variable name within the calling
        function that will receive the results of the load operation.

    String - Supplies the name of the PSTRING variable that is to be loaded
        into the slot.  This will usually be one of the function parameters.

    LengthVar - Supplies the name of a USHORT local variable that will receive
        the value of min(String->Length, 16).

Return Value:

    None.

--*/
#define LoadSearchStringIntoXmmRegister_SEH(Slot, String, LengthVar)   \
    LengthVar = min(String->Length, 16);                               \
    TRY_SSE42_ALIGNED {                                                \
        Slot.CharsXmm = _mm_load_si128((PXMMWORD)String->Buffer);      \
    } CATCH_EXCEPTION_ACCESS_VIOLATION {                               \
        if (PointerToOffsetCrossesPageBoundary(String->Buffer, 16)) {  \
            __movsb(Slot.Char, String->Buffer, LengthVar);             \
        } else {                                                       \
            Slot.CharsXmm = _mm_loadu_si128((PXMMWORD)String->Buffer); \
        }                                                              \
    }

/*++

    VOID
    LoadSearchStringIntoXmmRegister_AlignmentCheck(
        _In_ STRING_SLOT Slot,
        _In_ PSTRING String,
        _In_ USHORT LengthVar
        );

Routine Description:

    This routine checks to see if a page boundary will be crossed if 16-bytes
    are loaded from the address supplied by String->Buffer.  If a page boundary
    will be crossed, a __movsb() intrinsic is used to only copy String->Length
    bytes into the given Slot.

    If no page boundary will be crossed by a 128-bit load, the alignment of
    the address supplied by String->Buffer is checked.  If the alignment isn't
    at least on a 16-byte boundary, an unaligned load will be issued via the
    _mm_loadu_si128() intrinsic, otherwise, an _mm_stream_load_si128() will be
    used.

Arguments:

    Slot - Supplies the STRING_SLOT local variable name within the calling
        function that will receive the results of the load operation.

    String - Supplies the name of the PSTRING variable that is to be loaded
        into the slot.  This will usually be one of the function parameters.

    LengthVar - Supplies the name of a USHORT local variable that will receive
        the value of min(String->Length, 16).

Return Value:

    None.

--*/
#define LoadSearchStringIntoXmmRegister_AlignmentCheck(Slot, String,LengthVar) \
    LengthVar = min(String->Length, 16);                                       \
    if (PointerToOffsetCrossesPageBoundary(String->Buffer, 16)) {              \
        __movsb(Slot.Char, String->Buffer, LengthVar);                         \
    } else if (GetAddressAlignment(String->Buffer) < 16) {                     \
        Slot.CharsXmm = _mm_loadu_si128((PXMMWORD)String->Buffer);             \
    } else {                                                                   \
        Slot.CharsXmm = _mm_load_si128((PXMMWORD)String->Buffer);              \
    }

/*++

    VOID
    LoadSearchStringIntoXmmRegister_AlwaysUnaligned(
        _In_ STRING_SLOT Slot,
        _In_ PSTRING String,
        _In_ USHORT LengthVar
        );

Routine Description:

    This routine performs an unaligned 128-bit load of the address supplied by
    String->Buffer into the given Slot via the _mm_loadu_si128() intrinsic.
    No checks are done regarding whether or not a page boundary will be crossed.

Arguments:

    Slot - Supplies the STRING_SLOT local variable name within the calling
        function that will receive the results of the load operation.

    String - Supplies the name of the PSTRING variable that is to be loaded
        into the slot.  This will usually be one of the function parameters.

    LengthVar - Supplies the name of a USHORT local variable that will receive
        the value of min(String->Length, 16).

Return Value:

    None.

--*/
#define LoadSearchStringIntoXmmRegister_Unaligned(Slot, String, LengthVar) \
    LengthVar = min(String->Length, 16);                                   \
    if (PointerToOffsetCrossesPageBoundary(String->Buffer, 16)) {          \
        __movsb(Slot.Char, String->Buffer, LengthVar);                     \
    } else if (GetAddressAlignment(String->Buffer) < 16) {                 \
        Slot.CharsXmm = _mm_loadu_si128(String->Buffer);                   \
    } else {                                                               \
        Slot.CharsXmm = _mm_load_si128(String->Buffer);                    \
    }

/*++

    VOID
    LoadSearchStringIntoXmmRegister_AlwaysMovsb(
        _In_ STRING_SLOT Slot,
        _In_ PSTRING String,
        _In_ USHORT LengthVar
        );

Routine Description:

    This routine copies min(String->Length, 16) bytes from String->Buffer
    into the given Slot via the __movsb() intrinsic.  The memory referenced by
    the Slot is not cleared first via SecureZeroMemory().

Arguments:

    Slot - Supplies the STRING_SLOT local variable name within the calling
        function that will receive the results of the load operation.

    String - Supplies the name of the PSTRING variable that is to be loaded
        into the slot.  This will usually be one of the function parameters.

    LengthVar - Supplies the name of a USHORT local variable that will receive
        the value of min(String->Length, 16).

Return Value:

    None.

--*/
#define LoadSearchStringIntoXmmRegister_AlwaysMovsb(Slot, String, LengthVar) \
    LengthVar = min(String->Length, 16);                                     \
    __movsb(Slot.Char, String->Buffer, LengthVar);

//
// Define the appropriate target for the LoadSearchStringIntoXmmRegister macro
// based on the search string strategy defined.
//

#if LOAD_SEARCH_STRING_STRATEGY_SEH

#define LoadSearchStringIntoXmmRegister \
    LoadSearchStringIntoXmmRegister_SEH

#elif LOAD_SEARCH_STRING_STRATEGY_ALIGNMENT_CHECK

#define LoadSearchStringIntoXmmRegister \
    LoadSearchStringIntoXmmRegister_AlignmentCheck

#elif LOAD_SEARCH_STRING_STRATEGY_ALWAYS_UNALIGNED

#define LoadSearchStringIntoXmmRegister \
    LoadSearchStringIntoXmmRegister_AlwaysUnaligned

#elif LOAD_SEARCH_STRING_STRATEGY_ALWAYS_MOVSB

#define LoadSearchStringIntoXmmRegister \
    LoadSearchStringIntoXmmRegister_AlwaysMovsb

#else

//
// Default setting.
//

#define LoadSearchStringIntoXmmRegister            \
    LoadSearchStringIntoXmmRegister_AlignmentCheck

#endif

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
