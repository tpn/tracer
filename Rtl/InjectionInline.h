/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionInline.h

Abstract:

    This is the header file containing inline implementations of various
    injection helper routines.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

typedef
BOOL
(CALLBACK IS_JUMP)(
    _In_ PBYTE Code
    );
typedef IS_JUMP *PIS_JUMP;

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
IsJumpInline(
    _In_ PBYTE Code
    )
/*++

Routine Description:

    Given an address of AMD64 byte code, returns TRUE if the underlying
    instruction is a jump.

Arguments:

    Code - Supplies a pointer to the first byte of the code.

Return Value:

    TRUE if this address represents a jump, FALSE otherwise.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Code)) {
        return FALSE;
    }

    return ((
        (Code[0] == 0xFF && Code[1] == 0x25)                    |
        (Code[0] == 0x48 && Code[1] == 0xFF && Code[2] == 0x25) |
        (Code[0] == 0xE9)                                       |
        (Code[0] == 0xEB)
    ) ? TRUE : FALSE);
}

typedef
ULONG_PTR
(CALLBACK GET_INSTRUCTION_POINTER)(VOID);
typedef GET_INSTRUCTION_POINTER *PGET_INSTRUCTION_POINTER;

typedef
BOOL
(CALLBACK GET_APPROXIMATE_FUNCTION_BOUNDARIES)(
    _In_ ULONG_PTR Address,
    _Out_ PULONG_PTR StartAddress,
    _Out_ PULONG_PTR EndAddress
    );
typedef GET_APPROXIMATE_FUNCTION_BOUNDARIES
      *PGET_APPROXIMATE_FUNCTION_BOUNDARIES;

typedef
_Check_return_
_Success_(return != 0)
PBYTE
(SKIP_JUMPS)(
    _In_ PBYTE Code
    );
typedef SKIP_JUMPS *PSKIP_JUMPS;

typedef
BOOL
(CALLBACK RTL_IS_INJECTION_CODE_SIZE_QUERY)(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_opt_ PULONG CodeSize
    );
typedef RTL_IS_INJECTION_CODE_SIZE_QUERY
      *PRTL_IS_INJECTION_CODE_SIZE_QUERY;


FORCEINLINE
PBYTE
SkipJumpsInline(
    PBYTE Code
    )
/*++

Routine Description:

    Given an address of AMD64 byte code, follows any jumps until the first non-
    jump byte code is found, and returns that byte.  Alternatively, if the first
    bytes passed in do not indicate a jump, the original byte code address is
    returned.

    This is used to traverse jump tables and get to the actual underlying
    function.

Arguments:

    Code - Supplies a pointer to the first byte of the code for which any jumps
        should be followed.

Return Value:

    The address of the first non-jump byte code encountered.

--*/
{
    LONG Offset;
    CHAR ShortOffset;
    PBYTE OriginalCode = Code;
    PBYTE Target;

    if (!ARGUMENT_PRESENT(Code)) {
        return NULL;
    }

    while (TRUE) {

        Target = NULL;

        if (Code[0] == 0xFF && Code[1] == 0x25) {

            //
            // Offset is 32-bit.
            //

            Offset = *((PLONG)&Code[2]);

            Target = *((PBYTE *)(Code + 6 + Offset));

        } else if (Code[0] == 0x48 && Code[1] == 0xFF && Code[2] == 0x25) {

            //
            // REX-encoded 32-bit offset.
            //

            Offset = *((PLONG)&Code[3]);

            Target = *((PBYTE *)(Code + 7 + Offset));

        } else if (Code[0] == 0xE9) {

            //
            // Offset is 32-bit.
            //

            Offset = *((PLONG)&Code[1]);

            Target = *((PBYTE *)(Code + 7 + Offset));

        } else if (Code[0] == 0xEB) {

            //
            // Short jump; 8-bit offset.
            //

            ShortOffset = *((CHAR *)&Code[1]);

            Target = *((PBYTE *)(Code + 2 + ShortOffset));

        }

        if (!Target) {

            //
            // No more jumps, break out of the loop.
            //

            break;
        }
    }

    return Target;
}

FORCEINLINE
BOOL
GetApproximateFunctionBoundariesInline(
    ULONG_PTR Address,
    PULONG_PTR StartAddress,
    PULONG_PTR EndAddress
    )
/*++

Routine Description:

    Given an address which resides within a function, return the approximate
    start and end addresses of a function by searching forward and backward
    for repeat occurrences of the `int 3` (breakpoint) instruction, represented
    by 0xCC in byte code.  Such occurrences are usually good indicators of
    function boundaries -- in fact, the Microsoft AMD64 calling convention
    requires that function entry points should be padded with 6 bytes (to allow
    for hot-patching), and this padding is always the 0xCC byte.  The search is
    terminated in both directions once two successive 0xCC bytes are found.

    The term "approximate" is used to qualify both the start and end addresses,
    because although scanning for repeat 0xCC occurrences is quite reliable, it
    is not as reliable as a more authoritative source of function size, such as
    debug symbols.

Arguments:

    Address - Supplies an address that resides somewhere within the function
        for which the boundaries are to be obtained.

    StartAddress - Supplies a pointer to a variable that will receive the
        address of the approximate starting point of the function.

    EndAddress - Supplies a pointer to a variable that will receive the
        address of the approximate ending point of the function (this will
        typically be the `ret` (0xC3) instruction).

Return Value:

    TRUE if the method was successful, FALSE otherwise.  FALSE will only be
    returned if parameter validation fails.  StartAddress and EndAddress will
    not be updated in this case.

--*/
{
    PWORD Start;
    PWORD End;
    const WORD Int3x2 = 0xCCCC;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(Address)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StartAddress)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(EndAddress)) {
        return FALSE;
    }

    //
    // Skip through any jump instructions of the initial function address,
    // using this instruction as both the Start and End address;
    //

    Start = End = (PWORD)SkipJumpsInline((PBYTE)Address);

    //
    // Search backward through memory until we find two `int 3` instructions.
    //

    while (*Start != Int3x2) {
        --((PBYTE)Start);
    }

    //
    // Search forward through memory until we find two `int 3` instructions.
    //

    while (*End != Int3x2) {
        ++((PBYTE)End);
    }

    //
    // Update the caller's address pointers.
    //

    *StartAddress = (ULONG_PTR)(Start+1);
    *EndAddress = (ULONG_PTR)End;

    return TRUE;
}

#pragma component(browser, off)

RTL_API GET_INSTRUCTION_POINTER GetInstructionPointer;
RTL_API GET_APPROXIMATE_FUNCTION_BOUNDARIES GetApproximateFunctionBoundaries;
RTL_API SKIP_JUMPS SkipJumps;
RTL_API IS_JUMP IsJump;

#pragma component(browser, on)


#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
