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

#include "stdafx.h"

#else

//
// We're being included by an external component.
//

#include "../Rtl/Rtl.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STRING_TABLE_ENTRIES 16
#define STRING_TABLE_ALIGNMENT 512

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
// which represent a unique character (with respect to other strings in the
// table) for a string in a given slot index.  The STRING_SLOT structure
// provides a convenient wrapper around this construct.
//

typedef union DECLSPEC_ALIGN(16) _STRING_SLOT {
    XMMWORD CharsXmm;
    CHAR Char[16];
} STRING_SLOT, *PSTRING_SLOT, **PPSTRING_SLOT;
C_ASSERT(sizeof(STRING_SLOT) == 16);

//
// An array of 1 byte unsigned integers used to indicate the 0-based index of
// a given unique character in the corresponding string.
//

typedef union DECLSPEC_ALIGN(16) _SLOT_INDEX {
    XMMWORD IndexXmm;
    BYTE Index[16];
} SLOT_INDEX, *PSLOT_INDEX, **PPSLOT_INDEX;
C_ASSERT(sizeof(SLOT_INDEX) == 16);

//
// A 16 element array of 1 byte unsigned integers, used to capture the length
// of each string slot in a single XMM 128-bit register.
//

typedef union DECLSPEC_ALIGN(16) _SLOT_LENGTHS {
    XMMWORD SlotsXmm;
    BYTE Slots[16];
} SLOT_LENGTHS, *PSLOT_LENGTHS, **PPSLOT_LENGTHS;
C_ASSERT(sizeof(SLOT_LENGTHS) == 16);


//
// Our string table index is simply a char, with -1 indicating no match found.
//

typedef CHAR STRING_TABLE_INDEX;
#define NO_MATCH_FOUND -1

//
// Define the string table flags structure.
//

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
    // A slot where each individual element contains a uniquely-identifying
    // letter, with respect to the other strings in the table, of each string
    // in an occupied slot.
    //

    STRING_SLOT UniqueChars;

    //
    // (16 bytes consumed.)
    //

    //
    // For each unique character identified above, the following structure
    // captures the 0-based index of that character in the underlying string.
    // This is used as an input to vpshufb to rearrange the search string's
    // characters such that it can be vpcmpeqb'd against the unique characters
    // above.
    //

    SLOT_INDEX UniqueIndex;

    //
    // (32 bytes consumed.)
    //

    //
    // Length of the underlying string in each slot.
    //

    SLOT_LENGTHS Lengths;

    //
    // (48 bytes consumed, aligned at 16 bytes.)
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
    // (56 bytes consumed, aligned at 8 bytes.)
    //

    //
    // String table flags.
    //

    STRING_TABLE_FLAGS Flags;

    //
    // (60 bytes consumed, aligned at 4 bytes.)
    //

    //
    // A 16-bit bitmap indicating which slots are occupied.
    //

    USHORT OccupiedBitmap;

    //
    // A 16-bit bitmap indicating which slots have strings longer than 16 chars.
    //

    USHORT ContinuationBitmap;

    //
    // (64 bytes consumed, aligned at 64 bytes.)
    //

    //
    // The 16-element array of STRING_SLOT structs.  We want this to be aligned
    // on a 64-byte boundary, and it consumes 256-bytes of memory.
    //

    STRING_SLOT Slots[16];

    //
    // (320 bytes consumed, aligned at 64 bytes.)
    //

    //
    // We want the structure size to be a power of 2 such that an even number
    // can fit into a 4KB page (and reducing the likelihood of crossing page
    // boundaries, which complicates SIMD boundary handling), so we have an
    // extra 192-bytes to play with here.  The CopyStringArray() routine is
    // special-cased to allocate the backing STRING_ARRAY structure plus the
    // accommodating buffers in this space if it can fit.
    //
    // (You can test whether or not this occurred by checking the invariant
    //  `StringTable->pStringArray == &StringTable->StringArray`, if this
    //  is true, the array was allocated within this remaining padding space.)
    //

    union {
        STRING_ARRAY StringArray;
        CHAR Padding[192];
    };

} STRING_TABLE, *PSTRING_TABLE, **PPSTRING_TABLE;

//
// Assert critical size and alignment invariants at compile time.
//

C_ASSERT(FIELD_OFFSET(STRING_TABLE, UniqueIndex) == 16);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, Lengths) == 32);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, pStringArray) == 48);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, Slots)   == 64);
C_ASSERT(FIELD_OFFSET(STRING_TABLE, Padding) == 320);
C_ASSERT(sizeof(STRING_TABLE) == 512);

//
// This structure is used to communicate matches back to the caller.
//

typedef struct _STRING_MATCH {

    //
    // Index of the match.
    //

    BYTE Index;

    //
    // Number of characters matched.
    //

    BYTE NumberOfMatchedCharacters;

    //
    // Pad out to 8-bytes.
    //

    USHORT Padding[3];

    //
    // Pointer to the string that was matched.  The underlying buffer will
    // stay valid for as long as the STRING_TABLE struct persists.
    //

    PSTRING String;

} STRING_MATCH, *PSTRING_MATCH, **PPSTRING_MATCH;
C_ASSERT(sizeof(STRING_MATCH) == 16);

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
    _In_ PRTL Rtl,
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

typedef
STRING_TABLE_INDEX
(IS_PREFIX_OF_STRING_IN_TABLE)(
    _In_ struct _STRING_TABLE *StringTable,
    _In_ PSTRING String,
    _In_opt_ struct _STRING_MATCH *StringMatch
    );
typedef IS_PREFIX_OF_STRING_IN_TABLE *PIS_PREFIX_OF_STRING_IN_TABLE;

typedef
STRING_TABLE_INDEX
(IS_PREFIX_OF_CSTR_IN_ARRAY)(
    _In_ PCSZ *StringArray,
    _In_ PCSZ String,
    _Out_opt_ PSTRING_MATCH Match
    );
typedef IS_PREFIX_OF_CSTR_IN_ARRAY *PIS_PREFIX_OF_CSTR_IN_ARRAY;

typedef
VOID
(DESTROY_STRING_TABLE)(
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_opt_ struct _STRING_TABLE *StringTable
    );
typedef DESTROY_STRING_TABLE *PDESTROY_STRING_TABLE;

typedef
_Success_(return != 0)
STRING_TABLE_INDEX
(IS_STRING_IN_TABLE)(
    _In_ struct _STRING_TABLE *StringTable,
    _In_ PSTRING String,
    _In_opt_ struct _STRING_MATCH *StringMatch
    );
typedef IS_STRING_IN_TABLE *PIS_STRING_IN_TABLE;

////////////////////////////////////////////////////////////////////////////////
// Test-related Structures and Function Pointer Typedefs.
////////////////////////////////////////////////////////////////////////////////

typedef struct _STRING_TABLE_TEST_INPUT {
    STRING_TABLE_INDEX Expected;
    PSTRING String;
} STRING_TABLE_TEST_INPUT;
typedef STRING_TABLE_TEST_INPUT *PSTRING_TABLE_TEST_INPUT;
typedef const STRING_TABLE_TEST_INPUT *PCSTRING_TABLE_TEST_INPUT;

typedef DECLSPEC_ALIGN(32) union _ALIGNED_BUFFER {
    CHAR Chars[32];
    WCHAR WideChars[16];
} ALIGNED_BUFFER;
typedef ALIGNED_BUFFER *PALIGNED_BUFFER;
C_ASSERT(sizeof(ALIGNED_BUFFER) == 32);

#define COPY_TEST_INPUT(Inputs, Ix)                                      \
    __movsq((PDWORD64)&InputBuffer,                                      \
            (PDWORD64)Inputs[Ix].String->Buffer,                         \
            sizeof(InputBuffer) >> 3);                                   \
    AlignedInput.Length = Inputs[Ix].String->Length;                     \
    AlignedInput.MaximumLength = Inputs[Ix].String->MaximumLength;       \
    AlignedInput.Buffer = (PCHAR)&InputBuffer.Chars;

#define COPY_STRING_MATCH(Inputs, Ix)                                    \
    StringMatch.Index = Ix;                                              \
    StringMatch.NumberOfMatchedCharacters = Inputs[Ix].String->Length;   \
    StringMatch.String = &StringTable->pStringArray->Strings[Ix];

typedef struct _STRING_TABLE_FUNCTION_OFFSET {
        STRING Name;
        USHORT Offset;
        USHORT Verify;
} STRING_TABLE_FUNCTION_OFFSET;
typedef STRING_TABLE_FUNCTION_OFFSET *PSTRING_TABLE_FUNCTION_OFFSET;
typedef const STRING_TABLE_FUNCTION_OFFSET *PCSTRING_TABLE_FUNCTION_OFFSET;

#define DEFINE_STRING_TABLE_FUNCTION_OFFSET(Name, Verify) { \
    RTL_CONSTANT_STRING(#Name),                             \
    FIELD_OFFSET(STRING_TABLE_API_EX, Name),                \
    Verify                                                  \
}

#define LOAD_FUNCTION_FROM_OFFSET(Api, Offset) \
    (PIS_PREFIX_OF_STRING_IN_TABLE)(           \
        *((PULONG_PTR)(                        \
            RtlOffsetToPointer(                \
                Api,                           \
                Offset                         \
            )                                  \
        ))                                     \
    )

typedef
_Success_(return != 0)
BOOLEAN
(NTAPI TEST_IS_PREFIX_OF_STRING_IN_TABLE_FUNCTIONS)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR StringTableAllocator,
    _In_ PALLOCATOR StringArrayAllocator,
    _In_ PSTRING_ARRAY StringArray,
    _In_reads_bytes_(SizeOfAnyApi) union _STRING_TABLE_ANY_API *AnyApi,
    _In_ ULONG SizeOfAnyApi,
    _In_reads_(NumberOfFunctions) PCSTRING_TABLE_FUNCTION_OFFSET Functions,
    _In_ ULONG NumberOfFunctions,
    _In_reads_(NumberOfTestInputs) PCSTRING_TABLE_TEST_INPUT TestInput,
    _In_ ULONG NumberOfTestInputs,
    _In_ BOOLEAN DebugBreakOnTestFailure,
    _In_ BOOLEAN AlignInputs,
    _Out_ PULONG NumberOfFailedTests,
    _Out_ PULONG NumberOfPassedTests
    );
typedef TEST_IS_PREFIX_OF_STRING_IN_TABLE_FUNCTIONS
      *PTEST_IS_PREFIX_OF_STRING_IN_TABLE_FUNCTIONS;


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
    string's maximum buffer length up to the given Alignment (or 16 bytes if
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
        Defaults to 16 bytes.  (The individual String->Length field is
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
    entire StringArray table and all string buffers (aligned at 16 bytes).

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
    // array, factoring in alignment up to 16 bytes.
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
    // 16 bytes aligned, adjusting padding as necessary.
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
BYTE
GetNumberOfStringsInTable(
    _In_ PSTRING_TABLE StringTable
    )
{
    return (BYTE)__popcnt16(StringTable->OccupiedBitmap);
}

FORCEINLINE
ULONG
ComputeCrc32ForString(
    _In_ PSTRING String
    )
{
    ULONG Index;
    ULONG Length;
    ULONG StringHash;
    BYTE TrailingBytes;
    ULONG NumberOfDoubleWords;
    PULONG DoubleWords;

    Length = String->Length;
    DoubleWords = (PULONG)String->Buffer;
    TrailingBytes = Length % 4;
    StringHash = 0;
    NumberOfDoubleWords = Length >> 2;

    if (NumberOfDoubleWords) {

        //
        // Process as many 4 byte chunks as we can.
        //

        for (Index = 0; Index < NumberOfDoubleWords; Index++) {
            StringHash = _mm_crc32_u32(StringHash, DoubleWords[Index]);
        }
    }

    if (TrailingBytes) {

        //
        // There are between 1 and 3 bytes remaining at the end of the string.
        // We can't use _mm_crc32_u32() here directly on the last ULONG as we
        // will include the bytes past the end of the string, which will be
        // random and will affect our hash value.  So, we load the last ULONG
        // then zero out the high bits that we want to ignore via _bzhi_u32().
        // This ensures that only the bytes that are part of the input string
        // participate in the hash value calculation.
        //

        ULONG Last = 0;
        ULONG HighBits;

        //
        // (Sanity check we can math.)
        //

        ASSERT(TrailingBytes >= 1 && TrailingBytes <= 3);

        //
        // Initialize our HighBits to the number of bits in a ULONG (32),
        // then subtract the number of bits represented by TrailingBytes.
        //

        HighBits = sizeof(ULONG) << 3;
        HighBits -= (TrailingBytes << 3);

        //
        // Load the last ULONG, zero out the high bits, then hash.
        //

        Last = _bzhi_u32(DoubleWords[NumberOfDoubleWords], HighBits);
        StringHash = _mm_crc32_u32(StringHash, Last);
    }

    return StringHash;
}

FORCEINLINE
USHORT
IsPrefixMatchAvx2(
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

FORCEINLINE
BYTE
IsPrefixMatch(
    _In_ PCSTRING SearchString,
    _In_ PCSTRING TargetString,
    _In_ BYTE Offset
    )
{
    PBYTE Left;
    PBYTE Right;
    BYTE Matched = 0;
    BYTE Remaining = (SearchString->Length - Offset) + 1;

    Left = (PBYTE)RtlOffsetToPointer(SearchString->Buffer, Offset);
    Right = (PBYTE)RtlOffsetToPointer(TargetString->Buffer, Offset);

    while (--Remaining && *Left++ == *Right++) {
        Matched++;
    }

    Matched += Offset;
    if (Matched != TargetString->Length) {
        return NO_MATCH_FOUND;
    }

    return Matched;
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

    Success = AssertAligned16(&StringTable->UniqueChars);

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
(INITIALIZE_STRING_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP)(
    _In_ PRTL_BOOTSTRAP RtlBootstrap,
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_STRING_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP
      *PINITIALIZE_STRING_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_STRING_TABLE_ALLOCATOR)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_STRING_TABLE_ALLOCATOR *PINITIALIZE_STRING_TABLE_ALLOCATOR;

//
// Define the string table API structure.
//

typedef struct _STRING_TABLE_API {

    PSET_C_SPECIFIC_HANDLER SetCSpecificHandler;

    PCOPY_STRING_ARRAY CopyStringArray;
    PCREATE_STRING_TABLE CreateStringTable;
    PDESTROY_STRING_TABLE DestroyStringTable;

    PINITIALIZE_STRING_TABLE_ALLOCATOR
        InitializeStringTableAllocator;

    PINITIALIZE_STRING_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP
        InitializeStringTableAllocatorFromRtlBootstrap;

    PCREATE_STRING_ARRAY_FROM_DELIMITED_STRING
        CreateStringArrayFromDelimitedString;

    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE
        CreateStringTableFromDelimitedEnvironmentVariable;

    PTEST_IS_PREFIX_OF_STRING_IN_TABLE_FUNCTIONS
        TestIsPrefixOfStringInTableFunctions;

    PIS_STRING_IN_TABLE IsStringInTable;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

} STRING_TABLE_API;
typedef STRING_TABLE_API *PSTRING_TABLE_API;

typedef struct _STRING_TABLE_API_EX {

    //
    // Inline STRING_TABLE_API.
    //

    PSET_C_SPECIFIC_HANDLER SetCSpecificHandler;

    PCOPY_STRING_ARRAY CopyStringArray;
    PCREATE_STRING_TABLE CreateStringTable;
    PDESTROY_STRING_TABLE DestroyStringTable;

    PINITIALIZE_STRING_TABLE_ALLOCATOR
        InitializeStringTableAllocator;

    PINITIALIZE_STRING_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP
        InitializeStringTableAllocatorFromRtlBootstrap;

    PCREATE_STRING_ARRAY_FROM_DELIMITED_STRING
        CreateStringArrayFromDelimitedString;

    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE
        CreateStringTableFromDelimitedEnvironmentVariable;

    PTEST_IS_PREFIX_OF_STRING_IN_TABLE_FUNCTIONS
        TestIsPrefixOfStringInTableFunctions;

    PIS_STRING_IN_TABLE IsStringInTable;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    //
    // Extended API methods used for benchmarking.
    //

    PIS_PREFIX_OF_CSTR_IN_ARRAY IsPrefixOfCStrInArray;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_1;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_2;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_3;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_4;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_5;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_6;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_7;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_8;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_9;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_10;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_x64_1;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_x64_2;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable_x64_3;
    PIS_PREFIX_OF_STRING_IN_TABLE IntegerDivision_x64_1;

} STRING_TABLE_API_EX;
typedef STRING_TABLE_API_EX *PSTRING_TABLE_API_EX;

typedef union _STRING_TABLE_ANY_API {
    STRING_TABLE_API Api;
    STRING_TABLE_API_EX ApiEx;
} STRING_TABLE_ANY_API;
typedef STRING_TABLE_ANY_API *PSTRING_TABLE_ANY_API;

FORCEINLINE
BOOLEAN
LoadStringTableApi(
    _In_ PRTL Rtl,
    _Inout_ HMODULE *ModulePointer,
    _In_opt_ PUNICODE_STRING ModulePath,
    _In_ ULONG SizeOfAnyApi,
    _Out_writes_bytes_all_(SizeOfAnyApi) PSTRING_TABLE_ANY_API AnyApi
    )
/*++

Routine Description:

    Loads the string table module and resolves all API functions for either
    the STRING_TABLE_API or STRING_TABLE_API_EX structure.  The desired API
    is indicated by the SizeOfAnyApi parameter.

    Example use:

        STRING_TABLE_API_EX GlobalApi;
        PSTRING_TABLE_API_EX Api;

        Success = LoadStringTableApi(Rtl,
                                     NULL,
                                     NULL,
                                     sizeof(GlobalApi),
                                     (PSTRING_TABLE_ANY_API)&GlobalApi);
        ASSERT(Success);
        Api = &GlobalApi;

    In this example, the extended API will be provided as our sizeof(GlobalApi)
    will indicate the structure size used by STRING_TABLE_API_EX.

    See ../StringTable2BenchmarkExe/main.c for a complete example.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    ModulePointer - Optionally supplies a pointer to an existing module handle
        for which the API symbols are to be resolved.  May be NULL.  If not
        NULL, but the pointed-to value is NULL, then this parameter will
        receive the handle obtained by LoadLibrary() as part of this call.
        If the string table module is no longer needed, but the program will
        keep running, the caller should issue a FreeLibrary() against this
        module handle.

    ModulePath - Optionally supplies a pointer to a UNICODE_STRING structure
        representing a path name of the string table module to be loaded.
        If *ModulePointer is not NULL, it takes precedence over this parameter.
        If NULL, and no module has been provided via *ModulePointer, an attempt
        will be made to load the library via 'LoadLibraryA("StringTable.dll")'.

    SizeOfAnyApi - Supplies the size, in bytes, of the underlying structure
        pointed to by the AnyApi parameter.

    AnyApi - Supplies the address of a structure which will receive resolved
        API function pointers.  The API furnished will depend on the size
        indicated by the SizeOfAnyApi parameter.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    HMODULE Module = NULL;
    ULONG NumberOfSymbols;
    ULONG NumberOfResolvedSymbols;

    //
    // Define the API names.
    //
    // N.B. These names must match STRING_TABLE_API_EX exactly (including the
    //      order).
    //

    CONST PCSTR Names[] = {
        "SetCSpecificHandler",
        "CopyStringArray",
        "CreateStringTable",
        "DestroyStringTable",
        "InitializeStringTableAllocator",
        "InitializeStringTableAllocatorFromRtlBootstrap",
        "CreateStringArrayFromDelimitedString",
        "CreateStringTableFromDelimitedString",
        "CreateStringTableFromDelimitedEnvironmentVariable",
        "TestIsPrefixOfStringInTableFunctions",
        "IsStringInTable",
        "IsPrefixOfStringInTable",
        "IsPrefixOfCStrInArray",
        "IsPrefixOfStringInTable_1",
        "IsPrefixOfStringInTable_2",
        "IsPrefixOfStringInTable_3",
        "IsPrefixOfStringInTable_4",
        "IsPrefixOfStringInTable_5",
        "IsPrefixOfStringInTable_6",
        "IsPrefixOfStringInTable_7",
        "IsPrefixOfStringInTable_8",
        "IsPrefixOfStringInTable_9",
        "IsPrefixOfStringInTable_10",
        "IsPrefixOfStringInTable_x64_1",
        "IsPrefixOfStringInTable_x64_2",
        "IsPrefixOfStringInTable_x64_3",
        "IntegerDivision_x64_1",
    };

    //
    // Define an appropriately sized bitmap we can passed to Rtl->LoadSymbols().
    //

    ULONG BitmapBuffer[(ALIGN_UP(ARRAYSIZE(Names), sizeof(ULONG) << 3) >> 5)+1];
    RTL_BITMAP FailedBitmap = { ARRAYSIZE(Names)+1, (PULONG)&BitmapBuffer };

    //
    // Determine the number of symbols we want to resolve based on the size of
    // the API indicated by the caller.
    //

    if (SizeOfAnyApi == sizeof(AnyApi->Api)) {
        NumberOfSymbols = sizeof(AnyApi->Api) / sizeof(ULONG_PTR);
    } else if (SizeOfAnyApi == sizeof(AnyApi->ApiEx)) {
        NumberOfSymbols = sizeof(AnyApi->ApiEx) / sizeof(ULONG_PTR);
    } else {
        return FALSE;
    }

    //
    // Attempt to load the underlying string table module if necessary.
    //

    if (ARGUMENT_PRESENT(ModulePointer)) {
        Module = *ModulePointer;
    }

    if (!Module) {
        if (ARGUMENT_PRESENT(ModulePath)) {
            Module = LoadLibraryW(ModulePath->Buffer);
        } else {
            Module = LoadLibraryA("StringTable2.dll");
        }
    }

    if (!Module) {
        return FALSE;
    }

    //
    // We've got a handle to the string table module.  Load the symbols we want
    // dynamically via Rtl->LoadSymbols().
    //

    Success = Rtl->LoadSymbols(
        Names,
        NumberOfSymbols,
        (PULONG_PTR)AnyApi,
        NumberOfSymbols,
        Module,
        &FailedBitmap,
        TRUE,
        &NumberOfResolvedSymbols
    );

    ASSERT(Success);

    //
    // Debug helper: if the breakpoint below is hit, then the symbol names
    // have potentially become out of sync.  Look at the value of first failed
    // symbol to assist in determining the cause.
    //

    if (NumberOfSymbols != NumberOfResolvedSymbols) {
        PCSTR FirstFailedSymbolName;
        ULONG FirstFailedSymbol;
        ULONG NumberOfFailedSymbols;

        NumberOfFailedSymbols = Rtl->RtlNumberOfSetBits(&FailedBitmap);
        FirstFailedSymbol = Rtl->RtlFindSetBits(&FailedBitmap, 1, 0);
        FirstFailedSymbolName = Names[FirstFailedSymbol-1];
        __debugbreak();
    }

    //
    // Set the C specific handler for the module, such that structured
    // exception handling will work.
    //

    AnyApi->Api.SetCSpecificHandler(Rtl->__C_specific_handler);

    //
    // Update the caller's pointer and return success.
    //

    if (ARGUMENT_PRESENT(ModulePointer)) {
        *ModulePointer = Module;
    }

    return TRUE;
}


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
