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

#elif _STRING_TABLE_NO_API_EXPORT_IMPORT

//
// We're being included by someone who doesn't want dllexport or dllimport.
// This is useful for creating new .exe-based projects for things like unit
// testing or performance testing/profiling.
//

#define STRING_TABLE_API
#define STRING_TABLE_DATA extern

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

#define MAX_STRING_TABLE_ENTRIES 16

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

typedef struct _Struct_size_bytes_(SizeInQuadwords>>3) _STRING_ARRAY {

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
    // A pointer to the STRING_TABLE structure that "owns" us.
    //

    struct _STRING_TABLE *StringTable;

    //
    // The string array.  Number of elements in the array is governed by the
    // NumberOfElements field above.
    //

    STRING Strings[ANYSIZE_ARRAY];

} STRING_ARRAY;
typedef STRING_ARRAY *PSTRING_ARRAY;
typedef STRING_ARRAY **PPSTRING_ARRAY;


//
// String tables are composed of a 16 element array of 16 byte string "slots",
// which represent the first 16 characters of the string in a given slot index.
// The STRING_SLOT structure provides a convenient wrapper around this
// construct.
//

typedef union DECLSPEC_ALIGN(16) _STRING_SLOT {
    CHAR Char[16];
    WIDE_CHARACTER WideChar[8];
    struct {
        ULARGE_INTEGER LowChars;
        ULARGE_INTEGER HighChars;
    };
    XMMWORD CharsXmm;
} STRING_SLOT, *PSTRING_SLOT, **PPSTRING_SLOT;

//
// A 16 element array of 16-bit USHORT elements, used to capture the length
// of each string slot in a single YMM 256-bit register.
//

typedef union _SLOT_LENGTHS {
    YMMWORD SlotsYmm;
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
            struct {
                ULONGLONG LowSlots03;
                ULONGLONG LowSlots47;
            };
            XMMWORD LowSlots;
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
            struct {
                ULONGLONG HighSlots811;
                ULONGLONG HighSlots1215;
            };
            XMMWORD HighSlots;
        };
    };
} SLOT_LENGTHS, *PSLOT_LENGTHS, **PPSLOT_LENGTHS;

C_ASSERT(sizeof(SLOT_LENGTHS) == 32);

typedef SHORT STRING_TABLE_INDEX;
#define NO_MATCH_FOUND ((USHORT)(-1))

//
// Forward declaration of functions that we include in the STRING_TABLE
// struct via function pointers.
//

typedef
_Success_(return != 0)
STRING_TABLE_INDEX
(IS_PREFIX_OF_STRING_IN_TABLE)(
    _In_ struct _STRING_TABLE *StringTable,
    _In_ PSTRING String,
    _In_opt_ struct _STRING_MATCH *StringMatch
    );
typedef IS_PREFIX_OF_STRING_IN_TABLE *PIS_PREFIX_OF_STRING_IN_TABLE;

typedef
VOID
(DESTROY_STRING_TABLE)(
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_opt_ struct _STRING_TABLE *StringTable
    );
typedef DESTROY_STRING_TABLE *PDESTROY_STRING_TABLE;

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _STRING_TABLE_FLAGS {

    //
    // When set, indicates that the CopyArray flag was set to TRUE when the
    // STRING_TABLE was created with CreateStringTable().  This implies the
    // STRING_TABLE owns the STRING_ARRAY, and is responsible for its
    // destruction.
    //
    // If the table does own the array, the destruction routine should still
    // verify a separate allocation was performed for the string array by
    // checking StringTable->pStringArray against &StringTable->StringArray;
    // If the pointers point to the same location, the string array was able
    // to fit in the trailing space of the STRING_TABLE structure, and thus,
    // doesn't need to be deallocated separately (i.e. no separate call to
    // Allocator->Free() is required).
    //

    ULONG CopiedArray:1;

    //
    // Remaining space.
    //

    ULONG Unused:31;

} STRING_TABLE_FLAGS, *PSTRING_TABLE_FLAGS, **PPSTRING_TABLE_FLAGS;
C_ASSERT(sizeof(STRING_TABLE_FLAGS) == sizeof(ULONG));

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
    // N.B. This implies a table invariant that a bit cannot be set in this
    //      bitmap unless the corresponding bit is set in the OccupiedBitmap.
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
    // (328-bytes consumed, aligned at 8 bytes.)
    //

    //
    // Function pointer to the destruction function for the table.
    //

    PDESTROY_STRING_TABLE DestroyStringTable;

    //
    // (336-bytes consumed, aligned at 16-bytes.)
    //

    //
    // String table flags.
    //

    STRING_TABLE_FLAGS Flags;

    //
    // Pad to a 16-byte boundary.
    //

    ULONG Padding1[3];

    //
    // (352-bytes consumed, aligned at 32-bytes.)
    //

    //
    // We want the structure size to be a power of 2 such that an even number
    // can fit into a 4KB page (and reducing the likelihood of crossing page
    // boundaries, which complicates SIMD boundary handling), so we have an
    // extra 160-bytes to play with here.  The CopyStringArray() routine is
    // special-cased to allocate the backing STRING_ARRAY structure plus the
    // accommodating buffers in this space if it can fit.
    //
    // (You can test whether or not this occurred by checking the invariant
    //  `StringTable->pStringArray == &StringTable->StringArray`, if this
    //  is true, the array was allocated within this remaining padding space.)
    //

    union {
        STRING_ARRAY StringArray;
        CHAR Padding[160];
    };

} STRING_TABLE, *PSTRING_TABLE, **PPSTRING_TABLE;

//
// Assert critical size and alignment invariants at compile time.
//

C_ASSERT(FIELD_OFFSET(STRING_TABLE, Lengths) == 32);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, Slots)   == 64);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, Padding) == 352);
C_ASSERT(sizeof(STRING_TABLE) == 512);

//
// This structure is used to communicate matches back to the caller.
//

typedef struct _STRING_MATCH {

    //
    // Index of the match.
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
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PSTRING_ARRAY StringArray,
    _In_ USHORT StringTablePaddingOffset,
    _In_ USHORT StringTableStructSize,
    _Outptr_opt_result_maybenull_ PPSTRING_TABLE StringTablePointer
    );
typedef COPY_STRING_ARRAY *PCOPY_STRING_ARRAY;

typedef
_Check_return_
_Success_(return != 0)
PSTRING_ARRAY
(CREATE_STRING_ARRAY_FROM_DELIMITED_STRING)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PCSTRING String,
    _In_ CHAR Delimiter,
    _In_ USHORT StringTablePaddingOffset,
    _In_ USHORT StringTableStructSize,
    _Outptr_opt_result_maybenull_ PPSTRING_TABLE StringTablePointer
    );
typedef CREATE_STRING_ARRAY_FROM_DELIMITED_STRING  \
      *PCREATE_STRING_ARRAY_FROM_DELIMITED_STRING, \
    **PPCREATE_STRING_ARRAY_FROM_DELIMITED_STRING;

typedef
_Check_return_
_Success_(return != 0)
PSTRING_TABLE
(CREATE_STRING_TABLE)(
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PSTRING_ARRAY StringArray,
    _In_ BOOL CopyArray
    );
typedef CREATE_STRING_TABLE *PCREATE_STRING_TABLE;

typedef
_Check_return_
_Success_(return != 0)
PSTRING_TABLE
(CREATE_STRING_TABLE_FROM_DELIMITED_STRING)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PCSTRING String,
    _In_ CHAR Delimiter
    );
typedef CREATE_STRING_TABLE_FROM_DELIMITED_STRING  \
      *PCREATE_STRING_TABLE_FROM_DELIMITED_STRING, \
    **PPCREATE_STRING_TABLE_FROM_DELIMITED_STRING;

typedef
_Check_return_
_Success_(return != 0)
PSTRING_TABLE
(CREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PCSTR EnvironmentVariableName,
    _In_ CHAR Delimiter
    );
typedef CREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE  \
      *PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE, \
    **PPCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE;

typedef
STRING_TABLE_INDEX
(SEARCH_STRING_TABLE_SLOTS_FOR_FIRST_PREFIX_MATCH)(
    _In_ PSTRING_TABLE StringTable,
    _In_ PCSTRING String,
    _In_ USHORT Index,
    _In_opt_ PSTRING_MATCH StringMatch
    );
typedef   SEARCH_STRING_TABLE_SLOTS_FOR_FIRST_PREFIX_MATCH \
        *PSEARCH_STRING_TABLE_SLOTS_FOR_FIRST_PREFIX_MATCH;

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
    String = StringArray->Strings;
    MinimumLength = (USHORT)-1;
    MaximumLength = 0;
    Count = NumberOfElements;

    do {
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

        //
        // Advance the string pointer.
        //

        ++String;

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
    // struct includes a single STRING struct at the end of it (ANYSIZE_ARRAY).
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
    XMMWORD EqualXmm;
    XMMWORD FirstCharXmm;
    XMMWORD StringTableFirstCharXmm;
    XMMWORD ShuffleXmm = _mm_setzero_si128();
    XMMWORD FirstCharShuffleXmm = { FirstChar };

    //
    // Broadcast the character into the entire XMM register.
    //

    FirstCharXmm = _mm_shuffle_epi8(FirstCharShuffleXmm, ShuffleXmm);

    //
    // Load the string table's first character array into an XMM register.
    //

    StringTableFirstCharXmm = StringTable->FirstChars.CharsXmm;

    EqualXmm = _mm_cmpeq_epi8(FirstCharXmm, StringTableFirstCharXmm);
    Index.LongPart = _mm_movemask_epi8(EqualXmm);

    return Index.LowPart;
}

STRING_TABLE_DATA PARALLEL_SUFFIX_MOVE_MASK32 \
    ParallelSuffix32HighBitFromEveryOtherByte;

#define LoadParallelSuffix32HighBitFromEveryOtherByte() \
    _mm256_load_si256(&ParallelSuffix32HighBitFromEveryOtherByte.Move256);

FORCEINLINE
ULONG
CompressUlongHighBitFromEveryOtherByte(
    _In_ ULONG Input
    )
{
    ULONG Bit;
    ULONG Mask;
    YMMWORD Move;

    Move = LoadParallelSuffix32HighBitFromEveryOtherByte();

    Mask = ParallelSuffix32HighBitFromEveryOtherByte.Mask;

    Input = Input & Mask;

    Bit = Input & Move.m256i_u32[1]; Input = Input ^ Bit | (Bit >> 1);
    Bit = Input & Move.m256i_u32[2]; Input = Input ^ Bit | (Bit >> 2);
    Bit = Input & Move.m256i_u32[3]; Input = Input ^ Bit | (Bit >> 4);
    Bit = Input & Move.m256i_u32[4]; Input = Input ^ Bit | (Bit >> 8);
    Bit = Input & Move.m256i_u32[5]; Input = Input ^ Bit | (Bit >> 16);

    return Input;
}


FORCEINLINE
USHORT
GetBitmapForViablePrefixSlotsByLengths(
    _In_ PSTRING_TABLE StringTable,
    _In_ PSTRING String
    )
{
    ULONG Mask;
    ULONG Compressed;
    ULONG InvertedMask;

    XMMWORD LengthXmm;
    YMMWORD PrefixLengths;
    YMMWORD StringLength;
    YMMWORD IgnoreSlots16;

    //
    // Load the length array into a Ymm register.
    //

    PrefixLengths = _mm256_load_si256(&(StringTable->Lengths.SlotsYmm));

    //
    // Broadcast the 16-bit String->Length to all words in a 256-byte
    // register.
    //

    LengthXmm = _mm_set1_epi32(0);
    LengthXmm.m128i_u16[0] = String->Length;
    StringLength = _mm256_broadcastw_epi16(LengthXmm);

    //
    // Find all slots that are longer than the incoming string length, as these
    // are the ones we're going to exclude from any prefix match.
    //
    // N.B.: because we default the length of empty slots to 0x7ffff, they will
    //       handily be included in the ignored set (i.e. their words will also
    //       be set to 0xffff), which means they'll also get filtered out when
    //       we invert the mask shortly after.
    //

    IgnoreSlots16 = _mm256_cmpgt_epi16(PrefixLengths, StringLength);

    //
    // Generate a mask.  Bits set indicate invalid slots, bits cleared
    // indicate valid slots (slot length is less than or equal to search
    // string length).  Note that this mask is at the byte level; there's
    // no _mm256_movemask_epi16(), so we have to use _mm256_movemask_epi8().
    //

    Mask = _mm256_movemask_epi8(IgnoreSlots16);

    //
    // Invert it such that bits set correspond to valid slots.
    //

    InvertedMask = ~Mask;

    //
    // Compress the mask -- we only want the high bit of each word, but the
    // _mm256_movemask_epi8() will have given us the high bit for each byte.
    //

    Compressed = CompressUlongHighBitFromEveryOtherByte(InvertedMask);

    return (USHORT)Compressed;
}

FORCEINLINE
USHORT
IsPrefixMatch(
    _In_ PCSTRING SearchString,
    _In_ PCSTRING TargetString,
    _In_ USHORT Offset
    )
{
    USHORT SearchStringRemaining;
    USHORT TargetStringRemaining;
    ULONGLONG SearchStringAlignment;
    ULONGLONG TargetStringAlignment;
    USHORT CharactersMatched = Offset;

    LONG Count;
    LONG Mask;

    PCHAR SearchBuffer;
    PCHAR TargetBuffer;

    STRING_SLOT SearchSlot;

    XMMWORD SearchXmm;
    XMMWORD TargetXmm;
    XMMWORD ResultXmm;

    YMMWORD SearchYmm;
    YMMWORD TargetYmm;
    YMMWORD ResultYmm;

    SearchStringRemaining = SearchString->Length - Offset;
    TargetStringRemaining = TargetString->Length - Offset;

    SearchBuffer = (PCHAR)RtlOffsetToPointer(SearchString->Buffer, Offset);
    TargetBuffer = (PCHAR)RtlOffsetToPointer(TargetString->Buffer, Offset);

    //
    // This routine is only called in the final stage of a prefix match when
    // we've already verified the slot's corresponding original string length
    // (referred in this routine as the target string) is less than or equal
    // to the length of the search string.
    //
    // We attempt as many 32-byte comparisons as we can, then as many 16-byte
    // comparisons as we can, then a final < 16-byte comparison if necessary.
    //
    // We use aligned loads if possible, falling back to unaligned if not.
    //

StartYmm:

    if (SearchStringRemaining >= 32 && TargetStringRemaining >= 32) {

        //
        // We have at least 32 bytes to compare for each string.  Check the
        // alignment for each buffer and do an aligned streaming load (non-
        // temporal hint) if our alignment is at a 32-byte boundary or better;
        // reverting to an unaligned load when not.
        //

        SearchStringAlignment = GetAddressAlignment(SearchBuffer);
        TargetStringAlignment = GetAddressAlignment(TargetBuffer);

        if (SearchStringAlignment < 32) {
            SearchYmm = _mm256_loadu_si256((PYMMWORD)SearchBuffer);
        } else {
            SearchYmm = _mm256_stream_load_si256((PYMMWORD)SearchBuffer);
        }

        if (TargetStringAlignment < 32) {
            TargetYmm = _mm256_loadu_si256((PYMMWORD)TargetBuffer);
        } else {
            TargetYmm = _mm256_stream_load_si256((PYMMWORD)TargetBuffer);
        }

        //
        // Compare the two vectors.
        //

        ResultYmm = _mm256_cmpeq_epi8(SearchYmm, TargetYmm);

        //
        // Generate a mask from the result of the comparison.
        //

        Mask = _mm256_movemask_epi8(ResultYmm);

        //
        // There were at least 32 characters remaining in each string buffer,
        // thus, every character needs to have matched in order for this search
        // to continue.  If there were less than 32 characters, we can terminate
        // this prefix search here.  (-1 == 0xffffffff == all bits set == all
        // characters matched.)
        //

        if (Mask != -1) {

            //
            // Not all characters were matched, terminate the prefix search.
            //

            return NO_MATCH_FOUND;
        }

        //
        // All 32 characters were matched.  Update counters and pointers
        // accordingly and jump back to the start of the 32-byte processing.
        //

        SearchStringRemaining -= 32;
        TargetStringRemaining -= 32;

        CharactersMatched += 32;

        SearchBuffer += 32;
        TargetBuffer += 32;

        goto StartYmm;
    }

    //
    // Intentional follow-on to StartXmm.
    //

StartXmm:

    //
    // Update the search string's alignment.
    //

    if (SearchStringRemaining >= 16 && TargetStringRemaining >= 16) {

        //
        // We have at least 16 bytes to compare for each string.  Check the
        // alignment for each buffer and do an aligned streaming load (non-
        // temporal hint) if our alignment is at a 16-byte boundary or better;
        // reverting to an unaligned load when not.
        //

        SearchStringAlignment = GetAddressAlignment(SearchBuffer);

        if (SearchStringAlignment < 16) {
            SearchXmm = _mm_loadu_si128((XMMWORD *)SearchBuffer);
        } else {
            SearchXmm = _mm_stream_load_si128((XMMWORD *)SearchBuffer);
        }

        TargetXmm = _mm_stream_load_si128((XMMWORD *)TargetBuffer);

        //
        // Compare the two vectors.
        //

        ResultXmm = _mm_cmpeq_epi8(SearchXmm, TargetXmm);

        //
        // Generate a mask from the result of the comparison.
        //

        Mask = _mm_movemask_epi8(ResultXmm);

        //
        // There were at least 16 characters remaining in each string buffer,
        // thus, every character needs to have matched in order for this search
        // to continue.  If there were less than 16 characters, we can terminate
        // this prefix search here.  (-1 == 0xffff -> all bits set -> all chars
        // matched.)
        //

        if ((SHORT)Mask != (SHORT)-1) {

            //
            // Not all characters were matched, terminate the prefix search.
            //

            return NO_MATCH_FOUND;
        }

        //
        // All 16 characters were matched.  Update counters and pointers
        // accordingly and jump back to the start of the 16-byte processing.
        //

        SearchStringRemaining -= 16;
        TargetStringRemaining -= 16;

        CharactersMatched += 16;

        SearchBuffer += 16;
        TargetBuffer += 16;

        goto StartXmm;
    }

    if (TargetStringRemaining == 0) {

        //
        // We'll get here if we successfully prefix matched the search string
        // and all our buffers were aligned (i.e. we don't have a trailing
        // < 16 bytes comparison to perform).
        //

        return CharactersMatched;
    }

    //
    // If we get here, we have less than 16 bytes to compare.  Our target
    // strings are guaranteed to be 16-byte aligned, so we can load them
    // using an aligned stream load as in the previous cases.
    //

    TargetXmm = _mm_stream_load_si128((PXMMWORD)TargetBuffer);

    //
    // Loading the remainder of our search string's buffer is a little more
    // complicated.  It could reside within 15 bytes of the end of the page
    // boundary, which would mean that a 128-bit load would cross a page
    // boundary.
    //
    // At best, the page will belong to our process and we'll take a performance
    // hit.  At worst, we won't own the page, and we'll end up triggering a hard
    // page fault.
    //
    // So, see if the current search buffer address plus 16 bytes crosses a page
    // boundary.  If it does, take the safe but slower approach of a ranged
    // memcpy (movsb) into a local stack-allocated STRING_SLOT structure.
    //

    if (!PointerToOffsetCrossesPageBoundary(SearchBuffer, 16)) {

        //
        // No page boundary is crossed, so just do an unaligned 128-bit move
        // into our Xmm register.  (We could do the aligned/unaligned dance
        // here, but it's the last load we'll be doing (i.e. it's not
        // potentially on a loop path), so I don't think it's worth the extra
        // branch cost, although I haven't measured this empirically.)
        //

        SearchXmm = _mm_loadu_si128((XMMWORD *)SearchBuffer);

    } else {

        //
        // We cross a page boundary, so only copy the the bytes we need via
        // __movsb(), then do an aligned stream load into the Xmm register
        // we'll use in the comparison.
        //

        __movsb((PBYTE)&SearchSlot.Char,
                (PBYTE)SearchBuffer,
                SearchStringRemaining);

        SearchXmm = _mm_stream_load_si128(&SearchSlot.CharsXmm);
    }

    //
    // Compare the final vectors.
    //

    ResultXmm = _mm_cmpeq_epi8(SearchXmm, TargetXmm);

    //
    // Generate a mask from the result of the comparison, but mask off (zero
    // out) high bits from the target string's remaining length.
    //

    Mask = _bzhi_u32(_mm_movemask_epi8(ResultXmm), TargetStringRemaining);

    //
    // Count how many characters were matched and determine if we were a
    // successful prefix match or not.
    //

    Count = __popcnt(Mask);

    if ((USHORT)Count == TargetStringRemaining) {

        //
        // If we matched the same amount of characters as remaining in the
        // target string, we've successfully prefix matched the search string.
        // Return the total number of characters we matched.
        //

        CharactersMatched += (USHORT)Count;
        return CharactersMatched;
    }

    //
    // After all that work, our string match failed at the final stage!  Return
    // to the caller indicating we were unable to make a prefix match.
    //

    return NO_MATCH_FOUND;
}

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

//
// Allocator-specific typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_STRING_TABLE_ALLOCATOR)(
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_STRING_TABLE_ALLOCATOR *PINITIALIZE_STRING_TABLE_ALLOCATOR;

//
// Public exported function defs.
//

#pragma component(browser, off)

STRING_TABLE_API INITIALIZE_STRING_TABLE_ALLOCATOR
    InitializeStringTableAllocator;

STRING_TABLE_API COPY_STRING_ARRAY CopyStringArray;
STRING_TABLE_API CREATE_STRING_TABLE CreateStringTable;
STRING_TABLE_API DESTROY_STRING_TABLE DestroyStringTable;

STRING_TABLE_API CREATE_STRING_ARRAY_FROM_DELIMITED_STRING
    CreateStringArrayFromDelimitedString;

STRING_TABLE_API CREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE
    CreateStringTableFromDelimitedEnvironmentVariable;

STRING_TABLE_API CREATE_STRING_TABLE_FROM_DELIMITED_STRING
    CreateStringTableFromDelimitedString;

STRING_TABLE_API SEARCH_STRING_TABLE_SLOTS_FOR_FIRST_PREFIX_MATCH
    SearchStringTableSlotsForFirstPrefixMatch;

STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_C;
STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInSingleTable_C;
STRING_TABLE_API IS_PREFIX_OF_STRING_IN_TABLE
    IsPrefixOfStringInSingleTableInline;

#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
