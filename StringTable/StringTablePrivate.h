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

CREATE_STRING_TABLE CreateSingleStringTable;

FORCEINLINE
BOOL
GetSlicedStringArrayBuffersAllocationSize(
    _In_        PSTRING_ARRAY   StringArray,
    _In_opt_    USHORT          Start,
    _In_opt_    USHORT          End,
    _In_opt_    USHORT          Alignment,
    _Out_       PULONG          AllocationSizePointer,
    _Out_opt_   PUSHORT         MinimumLengthPointer,
    _Out_opt_   PUSHORT         MaximumLengthPointer
    )
/*++

Routine Description:

    Calculates the total number of bytes required to copy all of the STRING
    structs and their buffers in a given STRING_ARRAY struct, aligning each
    string's maximum buffer length up to the given Alignment (or 16-bytes if
    not provided).

    The allocation size can be restricted to a subset of an array (i.e. a
    slice) by specifying values for the 0-based Start and End parameters.

Arguments:

    StringArray - Supplies a pointer to a STRING_ARRAY structure to calculate
        allocation space for.

    Start - Optionally supplies a 0-based starting index into StringArray's
        Strings array to start calculating length from.

    End - Optionally supplies a 0-based ending index into StringArray's
        Strings array to stop calculating length from.  This would typically
        be used in conjuction with the Start parameter to limit allocation
        size to a slice of the string array.

    Alignment - Optionally supplies a MaximumLength size to align up to.
        Defaults to 16-bytes.  (The individual String->Length field is
        unaffected.)

    AllocationSizePointer - Supplies a pointer to the address of a variable
        that the allocation size in bytes will be written into.

    MinimumLengthPointer - Optionally supplies a pointer to the address of a
        variable that will receive the minimum String->Length seen whilst
        calculating the size.

    MaximumLengthPointer - Optionally supplies a pointer to the address of a
        variable that will receive the maximum String->Length seen whilst
        calculating the size.

Return Value:

    TRUE on success, FALSE on failure.  Failure will be a result of invalid
    incoming arguments, or one of the following invariants being violated:

        - Start >= StringArray->NumberOfElements
        - End >= StringArray->NumberOfElements
        - Start > End

--*/
{
    USHORT Index;
    USHORT AlignedSize;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    ULONG AllocSize;
    PSTRING String;

#ifdef _DEBUG

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(StringArray)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AllocationSizePointer)) {
        return FALSE;
    }

    NumberOfElements = StringArray->NumberOfElements;

    //
    // Check invariants.
    //

    if (NumberOfElements == 0) {
        return FALSE;
    }

    if (Start >= NumberOfElements || End >= NumberOfElements) {
        return FALSE;
    }

    if (Start > End) {
        return FALSE;
    }

#else

    NumberOfElements = StringArray->NumberOfElements;

#endif

    if (End == 0) {
        End = NumberOfElements;
    }

    if (Alignment == 0) {
        Alignment = 16;
    }

    //
    // Initialize variables before the loop.
    //

    AllocSize = 0;
    String = StringArray->Strings[0] - 1;
    MinimumLength = 0;
    MaximumLength = 0;

    for (Index = Start, ++String; Index < End; Index++) {
        AlignedSize = ALIGN_UP(String->Length, Alignment);
        AllocSize += AlignedSize;
    }

    //
    // Update the caller's pointer, minimum and maximum length pointers if
    // applicable, then return success.
    //

    *AllocationSizePointer = AllocSize;

    if (ARGUMENT_PRESENT(MinimumLengthPointer)) {
        *MinimumLengthPointer = MinimumLength;
    }

    if (ARGUMENT_PRESENT(MaximumLengthPointer)) {
        *MaximumLengthPointer = MaximumLength;
    }

    return TRUE;
}

FORCEINLINE
BOOL
GetStringArrayBuffersAllocationSize(
    _In_        PSTRING_ARRAY   StringArray,
    _Out_       PULONG          AllocationSizePointer
    )
/*++

Routine Description:

    Helper routine that calls GetSlicedStringArrayBuffersAllocationSize() with
    the Start, End and Alignment parameters set to their default.

--*/
{
    return GetSlicedStringArrayBuffersAllocationSize(
        StringArray,
        0,
        0,
        0,
        AllocationSizePointer
    );
}

FORCEINLINE
BOOL
GetStringArrayAllocationInfo(
    _In_  PSTRING_ARRAY   StringArray,
    _Out_ PULONG          TotalAllocationSizePointer,
    _Out_ PULONG          StructSizePointer,
    _Out_ PULONG          StringElementsSizePointer,
    _Out_ PUSHORT         MinimumLengthPointer,
    _Out_ PUSHORT         MaximumLengthPointer
    )
/*++

Routine Description:

    This routine calculates the total size, in bytes, required to copy the
    entire StringArray table and all string buffers (aligned at 16-bytes).

--*/
{
    BOOL Success;
    ULONG AllocSize;
    ULONG StringBufferAllocSize;
    ULONG StringElementsSize;
    PCHAR Buffer;

    //
    // Get the number of bytes required to copy all string buffers in the string
    // array, factoring in alignment up to 16-bytes.
    //

    Success = GetStringArrayBuffersAllocationSize(
        StringArray,
        &StringBufferAllocSize
    );

    if (!Success) {
        return FALSE;
    }

    //
    // Account for (StringArray->NumberOfElements - 1) times size of a
    // STRING struct.  The minus 1 is because the size of the STRING_ARRAY
    // struct includes a single STRING struct at the end of it.
    //

    StringElementsSize = (StringArray->NumberOfElements - 1) * sizeof(STRING);

    //
    // Calculate the final size.
    //

    AllocSize = (

        //
        // Account for the STRING_ARRAY structure.
        //

        sizeof(STRING_ARRAY) +

        //
        // Account for the array of STRING structures, minus 1.
        //

        StringElementsSize +

        //
        // Account for the backing buffer sizes.
        //

        StringBufferAllocSize

    );

    //
    // Update the caller's pointers and return success.
    //

    *TotalAllocationSizePointer = AllocSize;
    *StructSizePointer = StructSize;
    *StringElementsSizePointer = StringElementsSize;

    return TRUE;

}

#pragma intrinsic(__popcnt16)

FORCEINLINE
USHORT
GetNumberOfStringsInTable(
    _In_ PSTRING_TABLE StringTable
    )
{
    return (USHORT)__popcnt16(StringTable->OccupiedBitmap);
}

FORCEINLINE
ULONG
ComputeCrc32ForString(
    _In_ PSTRING String
    )
{
    CHAR Char;
    USHORT Index;
    ULONG Crc32;
    PCHAR Buffer;

    Crc32 = 0;
    Buffer = String->Buffer;

    for (Index = 0; Index < String->Length; Index++) {
        Char = Buffer[Index];
        Crc32 = __mm_crc32_u8(Crc32, Char);
    }

    return Crc32;
}

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
