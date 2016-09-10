/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTable.h

Abstract:

    This is the main header file for the StringTable component.  It defines
    structures and functions related to the implementation of the component.

    The main structures are the STRING_ARRAY structure, which is used by
    callers of this component to indicate the set of strings they'd like to
    add to the string table, and the STRING_TABLE structure, which is the main
    data structure used by this component.

    Functions are provided for creating, destroying and searching for whether
    or not there is a prefix string for a given search string present within a
    table.

    The design is optimized for relatively short strings (less than or equal to
    16 chars), and relatively few of them (less than or equal to 16).  These
    restrictive size constraints facilitate aggressive SIMD optimizations when
    searching for the strings within the table, with the goal to minimize the
    maximum possible latency incurred by the lookup mechanism.  The trade-off
    is usability and flexibility -- two things which can be better served by
    prefix trees if the pointer-chasing behavior of said data structures can
    be tolerated.

--*/

#pragma once

#ifdef _STRING_TABLE_INTERNAL_BUILD

//
// This is an internal build of the StringTable component.
//

#define STRING_TABLE_API __declspec(dllexport)
#define STRING_TABLE_DATA extern __declspec(dllexport)

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#define STRING_TABLE_API __declspec(dllimport)
#define STRING_TABLE_DATA extern __declspec(dllimport)

#include "../Rtl/Rtl.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

typedef _Struct_size_bytes_(SizeInQuadwords >> 3) struct _STRING_ARRAY {

    //
    // Size of the structure, in quadwords.  Why quadwords?  It allows us to
    // keep this size field to a USHORT, which helps with the rest of the
    // alignment in this struct (we want the STRING Strings[] array to start
    // on an 8-byte boundary).
    //
    // N.B.: We can't express the exact field size in the SAL annotation
    //       below, because the array of buffer sizes are inexpressible;
    //       however, we know the maximum length, so we can use the implicit
    //       invariant that the total buffer size can't exceed whatever num
    //       elements * max size is.
    //

    _Field_range_(<=, (
        sizeof(struct _STRING_ARRAY) +
        ((NumberOfElements - 1) * sizeof(STRING)) +
        (MaximumLength * NumberOfElements)
    ) >> 3)
    USHORT SizeInQuadwords;

    //
    // Number of elements in the array.
    //

    USHORT NumberOfElements;

    //
    // Minimum and maximum lengths for the String->Length fields.  Optional.
    //

    USHORT MinimumLength;
    USHORT MaximumLength;

    //
    // The string array.  Number of elements in the array is governed by the
    // NumberOfElements field above.
    //

    STRING Strings[ANYSIZE_ARRAY];

} STRING_ARRAY, *PSTRING_ARRAY, **PPSTRING_ARRAY;

typedef union _STRING_SLOT {
    CHAR Char[16];
    WIDE_CHARACTER WideChar[8];
    struct {
        ULARGE_INTEGER LowChars;
        ULARGE_INTEGER HighChars;
    };
    __m128i OctChars;
} STRING_SLOT, *PSTRING_SLOT, **PPSTRING_SLOT;

typedef union _SLOT_LENGTHS {
    __m256i Slots256;
    USHORT Slots[16];
    struct {
        union {
            struct {
                USHORT Slot0;
                USHORT Slot1;
                USHORT Slot2;
                USHORT Slot3;
                USHORT Slot4;
                USHORT Slot5;
                USHORT Slot6;
                USHORT Slot7;
            };
            __m128i LowSlots;
        };
        union {
            struct {
                USHORT Slot8;
                USHORT Slot9;
                USHORT Slot10;
                USHORT Slot11;
                USHORT Slot12;
                USHORT Slot13;
                USHORT Slot14;
                USHORT Slot15;
            };
            __m128i HighSlots;
        };
    };
} SLOT_LENGTHS, *PSLOT_LENGTHS, **PPSLOT_LENGTHS;

C_ASSERT(sizeof(SLOT_LENGTHS) == 32);

//
// Forward declaration of functions that we include in the STRING_TABLE
// struct via function pointers.
//

typedef
_Success_(return != 0)
BOOL
(IS_PREFIX_OF_STRING_IN_TABLE)(
    _In_ struct _STRING_TABLE *StringTable,
    _In_ PSTRING String,
    _In_opt_ struct _STRING_MATCH *StringMatch
    );
typedef IS_PREFIX_OF_STRING_IN_TABLE *PIS_PREFIX_OF_STRING_IN_TABLE;
STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_C;
STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInSingleTable_C;
STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_x64_SSE42;

//
// The STRING_TABLE struct is an optimized structure for testing whether a
// prefix entry for a string is in a table, with the expectation that the
// strings being compared will be relatively short (ideally <= 16 characters),
// and the table of string prefixes to compare to will be relatively small
// (ideally <= 16 strings).
//
// The overall goal is to be able to prefix match a string with the lowest
// possible (amortized) latency.  Fixed-size, memory-aligned character arrays,
// and SIMD instructions are used to try and achieve this.
//

typedef struct _STRING_TABLE {

    //
    // A bitmap indicating whether or not the given string slot is occupied.
    // The total number of strings occupied can be ascertained by from a popcnt
    // on this field.
    //

    USHORT OccupiedBitmap;

    //
    // A bitmap indicating whether or not the given string at the index given
    // by the bit in the bitmap is continued in the next string table block.
    //
    // N.B.: this implies a table invariant that a bit cannot be set in this
    //       bitmap unless the corresponding bit is set in the OccupiedBitmap.
    //

    USHORT ContinuationBitmap;

    //
    // (4-bytes aligned.)
    //

    //
    // Horizontal depth of the table; 0 translates to the first 16 characters,
    // 1 to the second 16 characters, etc.
    //

    BYTE HorizontalDepth;

    //
    // Vertical depth of the table; 0 translates to the first array of
    // 16 characters, 1 to the second array of 16 characters, etc.
    //

    BYTE VerticalDepth;

    //
    // The number of quadwords that need be added to this structure's base
    // address in order to derive the base address of the next structure that
    // has the same vertical depth as us, but an incremented horizontal depth.
    // If this value is 0, then this is the last horizontal block for the given
    // vertical depth.  The maximum offset is thus 32KB away.
    //

    BYTE NextHorizontalOffsetInQuadwords;

    //
    // As above, but for the next vertical offset.
    //

    BYTE NextVerticalOffsetInQuadwords;

    //
    // (8-bytes aligned.)
    //

    //
    // Pointer to the STRING_ARRAY associated with this table, which we own
    // (we create it and copy the caller's contents at creation time and
    // deallocate it when we get destroyed).
    //
    // N.B.: we use pStringArray here instead of StringArray because the
    //       latter is a field name at the end of the struct.
    //
    //

    PSTRING_ARRAY pStringArray;

    //
    // (16-bytes aligned.)
    //

    //
    // A slot where each individual element contains the first letter of
    // each string in an occupied slot.
    //

    STRING_SLOT FirstChars;

    //
    // (32-bytes aligned.)
    //

    //
    // Lengths of each slot (the String->Length value) compressed into a
    // 256-bit, 32-byte struct.  This needs to be 32-byte aligned.
    //

    SLOT_LENGTHS Lengths;

    //
    // (64-bytes aligned.)
    //

    //
    // The 16-element array of STRING_SLOT structs.  We want this to be aligned
    // on a 64-byte boundary, and it consumes 256-bytes of memory.
    //

    STRING_SLOT Slots[16];

    //
    // (320-bytes consumed, aligned at 64-bytes.)
    //

    //
    // Function pointer to the function that tests whether or not a string has
    // a prefix match with a string in the table.
    //

    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    //
    // Reserve 8-bytes for flags.
    //

    ULONGLONG Flags;

    //
    // (336-bytes consumed, aligned at 16-bytes.)
    //

    //
    // We want the structure size to be a power of 2 such that an even number
    // can fit into a 4KB page (and reducing the likelihood of crossing page
    // boundaries, which complicates SIMD boundary handling), so we have an
    // extra 176-bytes to play with here.  The CopyStringArray() routine is
    // special-cased to allocate the backing STRING_ARRAY structure plus the
    // accommodating buffers in this space if it can fit.
    //
    // (You can test whether or not this occurred by checking the invariant
    //  `StringTable->pStringArray == &StringTable->StringArray`, if this
    //  is true, the array was allocated within this remaining padding space.)
    //

    union {
        STRING_ARRAY StringArray;
        CHAR Padding[172];
    };

} STRING_TABLE, *PSTRING_TABLE, **PPSTRING_TABLE;

//
// Assert critical size and alignment invariants at compile time.
//

C_ASSERT(FIELD_OFFSET(STRING_TABLE, Lengths) == 32);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, Slots)   == 64);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, Padding) == 336);
C_ASSERT(sizeof(STRING_TABLE) == 512);

//
// This structure is used to communicate matches back to the caller.  (Not yet
// currently flushed out.)
//

typedef struct _STRING_MATCH {

    //
    // Index of the match.  -1 if no match.
    //

    LONG Index;

    //
    // Number of characters matched.
    //

    USHORT NumberOfMatchedCharacters;

    //
    // Pad out to 8-bytes.
    //

    USHORT Padding1;

    //
    // Pointer to the string that was matched.  The underlying buffer will
    // stay valid for as long as the STRING_TABLE struct persists.
    //

    PSTRING String;

} STRING_MATCH, *PSTRING_MATCH, **PPSTRING_MATCH;

////////////////////////////////////////////////////////////////////////////////
// Function Type Definitions
////////////////////////////////////////////////////////////////////////////////

typedef
_Check_return_
_Success_(return != 0)
PSTRING_ARRAY
(COPY_STRING_ARRAY)(
    _In_ PALLOCATOR Allocator,
    _In_ PSTRING_ARRAY StringArray,
    _In_ USHORT StringTablePaddingOffset,
    _In_ USHORT StringTableStructSize,
    _Outptr_opt_result_maybenull_ PPSTRING_TABLE StringTablePointer
    );
typedef COPY_STRING_ARRAY *PCOPY_STRING_ARRAY;
STRING_TABLE_API COPY_STRING_ARRAY CopyStringArray;

typedef
_Check_return_
_Success_(return != 0)
PSTRING_TABLE
(CREATE_STRING_TABLE)(
    _In_ PALLOCATOR Allocator,
    _In_ PSTRING_ARRAY StringArray
    );
typedef CREATE_STRING_TABLE *PCREATE_STRING_TABLE;
STRING_TABLE_API CREATE_STRING_TABLE CreateStringTable;

typedef
VOID
(DESTROY_STRING_TABLE)(
    _In_ PALLOCATOR Allocator,
    _In_opt_ PSTRING_TABLE StringTable
    );
typedef DESTROY_STRING_TABLE *PDESTROY_STRING_TABLE;
STRING_TABLE_API DESTROY_STRING_TABLE DestroyStringTable;


////////////////////////////////////////////////////////////////////////////////
// Inline functions.
////////////////////////////////////////////////////////////////////////////////

FORCEINLINE
_Success_(return != 0)
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
        be used in conjunction with the Start parameter to limit allocation
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
    USHORT Count;
    USHORT Length;
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
    String = StringArray->Strings - 1;
    MinimumLength = (USHORT)-1;
    MaximumLength = 0;
    Count = NumberOfElements;

    do {
        ++String;

        Length = String->Length;

        //
        // Align the string's length up to our alignment value.
        //

        AlignedSize = ALIGN_UP(Length, Alignment);
        AllocSize += AlignedSize;

        //
        // Update minimum and maximum length if applicable.
        //

        if (Length < MinimumLength) {
            MinimumLength = Length;
        }

        if (Length > MaximumLength) {
            MaximumLength = Length;
        }

    } while (--Count);

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

_Check_return_
_Success_(return != 0)
FORCEINLINE
BOOL
GetStringArrayBuffersAllocationSize(
    _In_        PSTRING_ARRAY   StringArray,
    _Out_       PULONG          AllocationSizePointer,
    _Out_opt_   PUSHORT         MinimumLengthPointer,
    _Out_opt_   PUSHORT         MaximumLengthPointer
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
        AllocationSizePointer,
        MinimumLengthPointer,
        MaximumLengthPointer
    );
}

_Check_return_
_Success_(return != 0)
FORCEINLINE
BOOL
GetStringArrayAllocationInfo(
    _In_  PSTRING_ARRAY   StringArray,
    _Out_ PULONG          TotalAllocationSizePointer,
    _Out_ PULONG          StructSizePointer,
    _Out_ PULONG          StringElementsSizePointer,
    _Out_ PULONG          BufferOffsetPointer,
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
    ULONG BufferOffset;
    ULONG AlignedBufferOffset;
    ULONG AllocSize;
    ULONG PaddingSize;
    ULONG StringBufferAllocSize;
    ULONG StringElementsSize;

    //
    // Get the number of bytes required to copy all string buffers in the string
    // array, factoring in alignment up to 16-bytes.
    //

    Success = GetStringArrayBuffersAllocationSize(
        StringArray,
        &StringBufferAllocSize,
        MinimumLengthPointer,
        MaximumLengthPointer
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
    // Calculate the offset where the first string buffer will reside, and the
    // aligned value of the offset.  Make sure the final buffer offset is
    // 16-bytes aligned, adjusting padding as necessary.
    //

    BufferOffset = sizeof(STRING_ARRAY) + StringElementsSize;
    AlignedBufferOffset = ALIGN_UP(BufferOffset, 16);
    PaddingSize = (AlignedBufferOffset - BufferOffset);

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
        // Account for any alignment padding we needed to do.
        //

        PaddingSize +

        //
        // Account for the backing buffer sizes.
        //

        StringBufferAllocSize

        );

    //
    // Update the caller's pointers and return success.
    //

    *TotalAllocationSizePointer = AllocSize;
    *StructSizePointer = sizeof(STRING_ARRAY);
    *StringElementsSizePointer = StringElementsSize;
    *BufferOffsetPointer = AlignedBufferOffset;

    return TRUE;

}

FORCEINLINE
BOOL
HasEmbeddedStringArray(
    _In_ PSTRING_TABLE StringTable
    )
{
    return (StringTable->pStringArray == &StringTable->StringArray);
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
USHORT
GetNumberOfOversizedStringsInTable(
    _In_ PSTRING_TABLE StringTable
    )
{
    return (USHORT)__popcnt16(StringTable->ContinuationBitmap);
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
        Crc32 = _mm_crc32_u8(Crc32, Char);
    }

    return Crc32;
}

FORCEINLINE
USHORT
IsFirstCharacterInStringTable(
    _In_ PSTRING_TABLE StringTable,
    _In_ CHAR FirstChar
    )
{
    ULONG_INTEGER Index;
    __m128i EqualXmm;
    __m128i FirstCharXmm;
    __m128i StringTableFirstCharXmm;
    __m128i ShuffleXmm = _mm_setzero_si128();
    __m128i FirstCharShuffleXmm = { FirstChar };

    //
    // Broadcast the character into the entire XMM register.
    //

    FirstCharXmm = _mm_shuffle_epi8(FirstCharShuffleXmm, ShuffleXmm);

    //
    // Load the string table's first character array into an XMM register.
    //

    StringTableFirstCharXmm = StringTable->FirstChars.OctChars;

    EqualXmm = _mm_cmpeq_epi8(FirstCharXmm, StringTableFirstCharXmm);
    Index.LongPart = _mm_movemask_epi8(EqualXmm);

    return Index.LowPart;
}

FORCEINLINE
BOOL
MaskedCompareStringToSlots(
    _In_ PSTRING_TABLE StringTable,
    _In_ PSTRING String
    )
{
    return FALSE;
}

#define TRY_AVX __try

#define SSE42_FALLBACK __except(                          \
    GetExceptionCode() == EXCEPTION_ILLEGAL_INSTRUCTION ? \
        EXCEPTION_EXECUTE_HANDLER :                       \
        EXCEPTION_CONTINUE_SEARCH                         \
    )

FORCEINLINE
USHORT
GetBitmapForViablePrefixSlotsByLengths(
    _In_ PSTRING_TABLE StringTable,
    _In_ PSTRING String
    )
{
    union {
        USHORT_INTEGER Short;
        WIDE_CHARACTER WideChar;
    } Length;

    Length.Short.ShortPart = String->Length;

    TRY_AVX {

        int Mask = 0;
        //int Mask2;

        __m256i PrefixLengths;
        __m256i StringLength;
        __m256i IgnoreSlots16;
        //__m256i IgnoreSlots8;
        //__m256i Shuffle = _MM2
        //__m256i ViableSlots;

        __m128i Xmm1 = {
            Length.WideChar.LowPart,
            Length.WideChar.HighPart
        };

        //
        // Load the length array into a Ymm register.
        //

        PrefixLengths = _mm256_load_si256(&(StringTable->Lengths.Slots256));

        //
        // Broadcast the 16-bit String->Length to all words in a 256-byte
        // register.
        //

        StringLength = _mm256_broadcastw_epi16(Xmm1);

        //
        // Find all slots that are longer than the incoming string length.
        //

        IgnoreSlots16 = _mm256_cmpgt_epi16(PrefixLengths, StringLength);


        //
        // Mask them out and return the result.
        //

        return (USHORT)Mask;

    } SSE42_FALLBACK {
        //__m128i Xmm1;
        //__m128i Xmm2;


    }

    return 0;
}

FORCEINLINE
ULONGLONG
TrailingZeros(
    _In_ ULONGLONG Integer
    )
{
    return _tzcnt_u64(Integer);
}

FORCEINLINE
USHORT
GetAddressAlignment(_In_ PVOID Address)
{
    ULONGLONG Integer = (ULONGLONG)Address;
    ULONGLONG NumTrailingZeros = TrailingZeros(Integer);
    return (1 << NumTrailingZeros);
}

FORCEINLINE
ULONGLONG
LeadingZeros(
    _In_ ULONGLONG Integer
    )
{
    return _lzcnt_u64(Integer);
}

_Success_(return != 0)
FORCEINLINE
BOOL
AssertAligned(
    _In_ PVOID Address,
    _In_ USHORT Alignment
    )
{
    ULONGLONG CurrentAlignment = GetAddressAlignment(Address);
    ULONGLONG ExpectedAlignment = ALIGN_UP(CurrentAlignment, Alignment);
    if (CurrentAlignment < ExpectedAlignment) {
#ifdef _DEBUG
        __debugbreak();
#endif
        OutputDebugStringA("Alignment failed!\n");
        return FALSE;
    }
    return TRUE;
}

#define AssertAligned16(Address)    AssertAligned((PVOID)Address, 16)
#define AssertAligned32(Address)    AssertAligned((PVOID)Address, 32)
#define AssertAligned64(Address)    AssertAligned((PVOID)Address, 64)
#define AssertAligned512(Address)   AssertAligned((PVOID)Address, 512)

_Success_(return != 0)
FORCEINLINE
BOOL
AssertStringTableFieldAlignment(
    _In_ PSTRING_TABLE StringTable
    )
{
    BOOL Success;

    Success = AssertAligned512(StringTable);

    if (!Success) {
        return Success;
    }

    Success = AssertAligned16(&StringTable->FirstChars);

    if (!Success) {
        return Success;
    }

    Success = AssertAligned32(&StringTable->Lengths);

    return Success;

}

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
