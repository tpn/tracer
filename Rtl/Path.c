/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Path.c

Abstract:

    This module implements routines related to path handling.

--*/
#include "stdafx.h"

#define MAX_RTL_PATH_ALLOC 4000

FORCEINLINE
VOID
UpdatePathFlagsIfWithinWindowsDirectories(
    _In_ PRTL Rtl,
    _In_ PRTL_PATH Path
    )
{
    BOOL CaseInsensitive = TRUE;

    Path->Flags.WithinWindowsDirectory = (
        Rtl->RtlPrefixUnicodeString(
            &Rtl->WindowsDirectory,
            &Path->Full,
            CaseInsensitive
        )
    );

    Path->Flags.WithinWindowsSxSDirectory = (
        Rtl->RtlPrefixUnicodeString(
            &Rtl->WindowsSxSDirectory,
            &Path->Full,
            CaseInsensitive
        )
    );

    Path->Flags.WithinWindowsSystemDirectory = (
        Rtl->RtlPrefixUnicodeString(
            &Rtl->WindowsSystemDirectory,
            &Path->Full,
            CaseInsensitive
        )
    );
}

_Use_decl_annotations_
BOOL
UnicodeStringToRtlPath(
    PRTL Rtl,
    PUNICODE_STRING String,
    PALLOCATOR Allocator,
    PPRTL_PATH PathPointer
    )
/*++

Routine Description:

    Converts a UNICODE_STRING representing a fully-qualified path into a
    RTL_PATH structure, allocating memory from the provided Allocator.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL struct.

    String - Supplies a pointer to a UNICODE_STRING structure that contains a
        path name.

    Allocator - Supplies a pointer to an ALLOCATOR structure that is used for
        allocating the new RTL_PATH structure (and associated buffers).

    PathPointer - Supplies a pointer to an address that receives the address
        of the newly created RTL_PATH structure if the routine was successful.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    const BOOL Reverse = TRUE;
    USHORT Offset;
    USHORT Count;
    USHORT NumberOfCharacters;
    USHORT NumberOfCharactersExcludingNull;
    USHORT AlignedNumberOfCharacters;
    USHORT UnicodeBufferSizeInBytes;
    USHORT BitmapBufferSizeInBytes;
    USHORT AlignedBitmapBufferSizeInBytes;
    USHORT BitmapAllocSizeInBytes;
    USHORT NumberOfSlashes;
    USHORT NumberOfDots;
    USHORT ReversedSlashIndex;
    USHORT ReversedDotIndex;
    USHORT LengthInBytes;
    LONG_INTEGER AllocSize;
    PRTL_PATH Path;
    PWCHAR Buf;
    PWCHAR Dest;
    PWCHAR Source;
    PRTL_BITMAP ReversedSlashesBitmap;
    PRTL_BITMAP ReversedDotsBitmap;
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;
    PRTL_FIND_SET_BITS RtlFindSetBits;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathPointer)) {
        return FALSE;
    }

    if (!IsValidMinimumDirectoryNullTerminatedUnicodeString(String)) {
        return FALSE;
    }

    //
    // N.B. The vast majority of this function's code deals with calculating
    //      the total number of bytes we need to furnish a new RTL_PATH object
    //      from the incoming string, such that we only need a single alloc
    //      call.  This requires accounting for the RTL_PATH structure itself,
    //      space for two RTL_BITMAP buffers (where there is one bit per
    //      character), and then space for a copy of the NULL-terminated
    //      Unicode string buffer.
    //


    //
    // Get the number of characters, plus +1 for the trailing NULL.
    //

    NumberOfCharacters = (String->Length >> 1) + 1;
    NumberOfCharactersExcludingNull = String->Length >> 1;

    //
    // Calculate the aligned number of characters.  We do the alignment
    // in order for the bitmap buffers to have a pointer-aligned size.
    //

    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfCharacters
        )
    );

    //
    // Calculate the aligned Unicode string buffer size in bytes.
    //

    UnicodeBufferSizeInBytes = AlignedNumberOfCharacters << 1;

    //
    // Calculate the individual bitmap buffer sizes.  As there's one bit per
    // character, we shift left 3 (divide by 8) to get the number of bytes
    // required.
    //

    BitmapBufferSizeInBytes = AlignedNumberOfCharacters >> 3;

    //
    // Align to a pointer boundary.
    //

    AlignedBitmapBufferSizeInBytes = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            BitmapBufferSizeInBytes
        )
    );

    //
    // Factor in the RTL_BITMAP struct size.
    //

    BitmapAllocSizeInBytes = (
        AlignedBitmapBufferSizeInBytes +
        sizeof(RTL_BITMAP)
    );

    //
    // Calculate the total allocation size in bytes required for the
    // RTL_PATH structure and trailing bitmap buffers and Unicode buffer.
    //

    AllocSize.LongPart = (

        //
        // Size of the RTL_PATH structure.
        //

        sizeof(RTL_PATH) +

        //
        // Size of the underlying bitmap buffers (there are two of them).
        //

        (BitmapAllocSizeInBytes * 2) +

        //
        // Size of the Unicode buffer.  Includes our trailing NULL.
        //

        UnicodeBufferSizeInBytes

    );

    //
    // Sanity check that we're not using more than MAX_USHORT bytes.
    //

    if (AllocSize.HighPart) {
        return FALSE;
    }

    //
    // Allocate the memory.
    //

    Path = (PRTL_PATH)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSize.LowPart
        )
    );

    if (!Path) {
        return FALSE;
    }

    //
    // The allocation was successful.  Fill in the size fields.
    //

    Path->StructSize = sizeof(*Path);
    Path->AllocSize = AllocSize.LowPart;

    //
    // Carve up the rest of the allocated buffer past the RTL_PATH struct
    // into the two bitmap buffers and then the final unicode string buffer.
    //

    ReversedSlashesBitmap = &Path->ReversedSlashesBitmap;
    ReversedDotsBitmap = &Path->ReversedDotsBitmap;

    Offset = sizeof(RTL_PATH);

    //
    // Carve out ReversedSlashesBitmap.
    //

    ReversedSlashesBitmap->Buffer = (PULONG)(
        RtlOffsetToPointer(
            Path,
            Offset
        )
    );

    //
    // Carve out ReversedDotsBitmap.
    //

    Offset += AlignedBitmapBufferSizeInBytes;

    ReversedDotsBitmap->Buffer = (PULONG)(
        RtlOffsetToPointer(
            Path,
            Offset
        )
    );

    //
    // And finally, carve out the final unicode string buffer.
    //

    Offset += AlignedBitmapBufferSizeInBytes;

    Path->Full.Buffer = (PWSTR)(
        RtlOffsetToPointer(
            Path,
            Offset
        )
    );

    //
    // Initialize the bitmap sizes.
    //

    ReversedSlashesBitmap->SizeOfBitMap = NumberOfCharactersExcludingNull;
    ReversedDotsBitmap->SizeOfBitMap = NumberOfCharactersExcludingNull;

    //
    // Initialize the lengths of the full path.  Note that we don't use the
    // aligned sizes; they were simply for ensuring pointer-aligned buffers.
    //

    Path->Full.Length = String->Length;
    Path->Full.MaximumLength = String->Length + sizeof(WCHAR);

    //
    // Copy the unicode string over.
    //

    Dest = Path->Full.Buffer;
    Source = String->Buffer;
    Count = String->Length >> 1;
    __movsw(Dest, Source, Count);

    //
    // Add trailing NULL.
    //

    Dest += Count;
    *Dest++ = L'\0';

    //
    // Directory points at the same buffer as Full.
    //

    Path->Directory.Buffer = Path->Full.Buffer;

    //
    // Find slashes and dots in reverse.
    //

    InlineFindTwoWideCharsInUnicodeStringReversed(
        &Path->Full,
        L'\\',
        L'.',
        ReversedSlashesBitmap,
        ReversedDotsBitmap
    );

    //
    // Initialize our bitmap function aliases.
    //

    RtlNumberOfSetBits = Rtl->RtlNumberOfSetBits;
    RtlFindSetBits = Rtl->RtlFindSetBits;

    //
    // Make sure there is at least one slash in the path.
    //

    NumberOfSlashes = (USHORT)RtlNumberOfSetBits(ReversedSlashesBitmap);
    if (NumberOfSlashes == 0) {
        goto Error;
    }
    Path->NumberOfSlashes = NumberOfSlashes;

    //
    // Extract the filename from the path by finding the last backslash
    // (which will be the first bit set in the bitmap) and calculating the
    // offset into the string buffer.
    //

    ReversedSlashIndex = (USHORT)RtlFindSetBits(ReversedSlashesBitmap, 1, 0);
    Offset = NumberOfCharactersExcludingNull - ReversedSlashIndex + 1;

    LengthInBytes = (ReversedSlashIndex - 1) << 1;
    Path->Name.Length = LengthInBytes;
    Path->Name.MaximumLength = LengthInBytes + sizeof(WCHAR);
    Path->Name.Buffer = &Path->Full.Buffer[Offset];

    //
    // The directory name length is easy to isolate now that we have the offset
    // of the first slash; it's simply the character before.
    //

    LengthInBytes = (Offset - 1) << 1;
    Path->Directory.Length = LengthInBytes;
    Path->Directory.MaximumLength = LengthInBytes;

    //
    // Get the number of dots.
    //

    NumberOfDots = (USHORT)RtlNumberOfSetBits(ReversedDotsBitmap);

    //
    // Extract the extension from the path by finding the last dot (which will
    // be the first bit set in the bitmap) and calculating the offset into the
    // string buffer.
    //

    if (NumberOfDots) {
        ReversedDotIndex = (USHORT)RtlFindSetBits(ReversedDotsBitmap, 1, 0);
        Offset = NumberOfCharactersExcludingNull - ReversedDotIndex + 1;

        LengthInBytes = (ReversedDotIndex - 1) << 1;
        Path->Extension.Length = LengthInBytes;
        Path->Extension.MaximumLength = LengthInBytes + sizeof(WCHAR);
        Path->Extension.Buffer = &Path->Full.Buffer[Offset];

        Path->NumberOfDots = NumberOfDots;
    }

    //
    // Set whether or not the path is qualified and potentially set the drive
    // letter if available.
    //

    Buf = Path->Full.Buffer;

    if (Buf[1] == L':' && Buf[2] == L'\\') {

        Path->Drive = Buf[0];
        Path->Flags.IsFullyQualified = TRUE;

    } else if (Buf[0] == L'\\' && Buf[1] == L'\\') {

        Path->Flags.IsFullyQualified = TRUE;
    }

    //
    // Set the allocator and update path flags.
    //

    Path->Allocator = Allocator;

    UpdatePathFlagsIfWithinWindowsDirectories(Rtl, Path);

    //
    // We're done, update the caller's path pointer and return success.
    //

    *PathPointer = Path;

    return TRUE;

Error:

    if (Path) {
        Allocator->Free(Allocator->Context, Path);
        Path = NULL;
    }

    return FALSE;

}

BOOL
ConvertUtf8StringToUtf16String(
    _In_ PSTRING Utf8,
    _Out_ PPUNICODE_STRING Utf16Pointer,
    _In_ PALLOCATOR Allocator,
    _In_ PLARGE_INTEGER TimestampPointer
    )
/*++

Routine Description:

    Converts a UTF-8 Unicode string to a UTF-16 string using the provided
    allocator.  The 'Slow' suffix on this function name indicates that the
    MultiByteToWideChar() function is called first in order to get the required
    buffer size prior to allocating the buffer.

    (ConvertUtf8StringToUtf16String() is an alternate version of this method
     that optimizes for the case where there are no multi-byte characters.)

Arguments:

    Utf8 - Supplies a pointer to a STRING structure to be converted.

    Utf16Pointer - Supplies a pointer that receives the address of the newly
        allocated and converted UTF-16 STRING version of the UTF-8 input string.

    Allocator - Supplies a pointer to the memory allocator that will be used
        for all allocations.

    TimestampPointer - Supplies a timestamp to associate with allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT NewLengthInChars;
    LONG CharsCopied;
    LONG BufferSizeInBytes;
    LONG BufferSizeInChars;
    LONG MaximumBufferSizeInChars;
    ULONG_INTEGER AllocSize;
    ULONG_INTEGER AlignedBufferSizeInBytes;
    PUNICODE_STRING Utf16;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Utf8)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Utf16Pointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer straight away.
    //

    *Utf16Pointer = NULL;

    //
    // Calculate the number of bytes required to hold a UTF-16 encoding of the
    // UTF-8 input string.
    //

    BufferSizeInChars = MultiByteToWideChar(
        CP_UTF8,                        // CodePage
        0,                              // dwFlags
        Utf8->Buffer,                   // lpMultiByteStr
        Utf8->Length,                   // cbMultiByte
        NULL,                           // lpWideCharStr
        0                               // cchWideChar
    );

    if (BufferSizeInChars <= 0) {
        return FALSE;
    }

    //
    // Account for the trailing NULL.
    //

    NewLengthInChars = (USHORT)BufferSizeInChars + 1;

    //
    // Convert character buffer size into bytes.
    //

    BufferSizeInBytes = NewLengthInChars << 1;

    //
    // Align the buffer.
    //

    AlignedBufferSizeInBytes.LongPart = (ULONG)(
        ALIGN_UP_POINTER(
            BufferSizeInBytes
        )
    );

    //
    // Sanity check the buffer size isn't over MAX_USHORT or under the number
    // of bytes for the input UTF-8 buffer.
    //

    if (AlignedBufferSizeInBytes.HighPart != 0) {
        return FALSE;
    }

    if (AlignedBufferSizeInBytes.LowPart < Utf8->Length) {
        return FALSE;
    }

    //
    // Calculate the total allocation size required, factoring in the overhead
    // of the UNICODE_STRING struct.
    //

    AllocSize.LongPart = (

        sizeof(UNICODE_STRING) +

        AlignedBufferSizeInBytes.LowPart

    );

    //
    // Try allocate space for the buffer.
    //

    Utf16 = (PUNICODE_STRING)(
        Allocator->CallocWithTimestamp(
            Allocator->Context,
            1,
            AllocSize.LowPart,
            TimestampPointer
        )
    );

    if (!Utf16) {
        return FALSE;
    }

    //
    // Successfully allocated space.  Point the UNICODE_STRING buffer at the
    // memory trailing the struct.
    //

    Utf16->Buffer = (PWCHAR)(
        RtlOffsetToPointer(
            Utf16,
            sizeof(UNICODE_STRING)
        )
    );

    //
    // Initialize the lengths.
    //

    Utf16->Length = (USHORT)(BufferSizeInChars << 1);
    Utf16->MaximumLength = AlignedBufferSizeInBytes.LowPart;

    MaximumBufferSizeInChars = Utf16->MaximumLength >> 1;

    //
    // Attempt the conversion.
    //

    CharsCopied = MultiByteToWideChar(
        CP_UTF8,                    // CodePage
        0,                          // dwFlags
        Utf8->Buffer,               // lpMultiByteStr
        Utf8->Length,               // cbMultiByte
        Utf16->Buffer,              // lpWideCharStr
        Utf8->Length                // cchWideChar
    );

    if (CharsCopied != NewLengthInChars-1) {
        __debugbreak();
        goto Error;
    }

    //
    // Update the caller's pointer and return success.
    //

    *Utf16Pointer = Utf16;

    return TRUE;

Error:

    if (Utf16) {

        //
        // Try free the underlying buffer.
        //

        Allocator->Free(Allocator->Context, Utf16);
        Utf16 = NULL;
    }

    return FALSE;
}

_Use_decl_annotations_
BOOL
StringToExistingRtlPath(
    PRTL Rtl,
    PSTRING AnsiString,
    PALLOCATOR BitmapAllocator,
    PALLOCATOR UnicodeStringBufferAllocator,
    PRTL_PATH Path,
    PLARGE_INTEGER TimestampPointer
    )
/*++

Routine Description:

    Converts a STRING representing a fully-qualified path into an existing
    RTL_PATH structure.  Bitmaps are allocated from the bitmap allocator,
    Unicode string buffers are allocated from the Unicode string buffer.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL struct.

    AnsiString - Supplies a pointer to a STRING structure that contains a
        path name.

    BitmapAllocator - Supplies a pointer to an ALLOCATOR structure that is
        used for allocating all bitmaps.

    UnicodeStringBufferAllocator - Supplies a pointer to an ALLOCATOR structure
        that is used for allocating any Unicode string buffers.

    Path - Supplies a pointer to an existing RTL_PATH structure.

    TimestampPointer - Supplies a pointer to a timestamp to use for allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    const BOOL Reverse = TRUE;
    USHORT Offset;
    USHORT NumberOfCharacters;
    USHORT AlignedNumberOfCharacters;
    USHORT BitmapBufferSizeInBytes;
    USHORT AlignedBitmapBufferSizeInBytes;
    USHORT NumberOfSlashes;
    USHORT NumberOfDots;
    USHORT ReversedSlashIndex;
    USHORT ReversedDotIndex;
    USHORT LengthInBytes;
    PWCHAR Buf;
    PUNICODE_STRING String;
    PULONG BitmapBuffers;
    PRTL_BITMAP ReversedSlashesBitmap;
    PRTL_BITMAP ReversedDotsBitmap;
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;
    PRTL_FIND_SET_BITS RtlFindSetBits;

#ifdef _DEBUG
    SIZE_T UnicodeLength;
#endif

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BitmapAllocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(UnicodeStringBufferAllocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (AnsiString->Length > AnsiString->MaximumLength) {
        __debugbreak();
    }

    //
    // Convert the ANSI string to a Unicode string.
    //

    Success = ConvertUtf8StringToUtf16String(AnsiString,
                                             &String,
                                             UnicodeStringBufferAllocator,
                                             TimestampPointer);
    if (!Success) {
        __debugbreak();
        return FALSE;
    }

#ifdef _DEBUG
    if (String->Length > String->MaximumLength) {
        __debugbreak();
    }

    if (AnsiString->Length != (String->Length >> 1)) {
        __debugbreak();
    }

    if (String->Buffer[String->Length >> 1] != L'\0') {
        __debugbreak();
    }

    UnicodeLength = wcslen(String->Buffer);
    if (UnicodeLength != AnsiString->Length) {
        __debugbreak();
    }
#endif

    //
    // Get the number of characters.
    //

    NumberOfCharacters = String->Length >> 1;

    //
    // Calculate the aligned number of characters.  We do the alignment
    // in order for the bitmap buffers to have a pointer-aligned size.
    //

    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfCharacters
        )
    );

    //
    // Calculate the individual bitmap buffer sizes.  As there's one bit per
    // character, we shift left 3 (divide by 8) to get the number of bytes
    // required.
    //

    BitmapBufferSizeInBytes = AlignedNumberOfCharacters >> 3;

    //
    // Align to a pointer boundary.
    //

    AlignedBitmapBufferSizeInBytes = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            BitmapBufferSizeInBytes
        )
    );

    //
    // Allocate the bitmap buffers.
    //

    BitmapBuffers = (PULONG)BitmapAllocator->CallocWithTimestamp(
        BitmapAllocator->Context,
        2,
        AlignedBitmapBufferSizeInBytes,
        TimestampPointer
    );

    if (!BitmapBuffers) {
        return FALSE;
    }

    //
    // The allocation was successful.  Fill in the size fields.
    //

    Path->StructSize = sizeof(*Path);
    Path->AllocSize = (
        String->MaximumLength +
        (AlignedBitmapBufferSizeInBytes * 2)
    );

    ReversedSlashesBitmap = &Path->ReversedSlashesBitmap;
    ReversedDotsBitmap = &Path->ReversedDotsBitmap;

    //
    // Carve out the bitmap buffers.
    //

    ReversedSlashesBitmap->Buffer = BitmapBuffers;
    ReversedDotsBitmap->Buffer = (PULONG)(
        RtlOffsetToPointer(
            BitmapBuffers,
            AlignedBitmapBufferSizeInBytes
        )
    );

    //
    // Point the full path string at the newly created string buffer.
    //

    Path->Full.Buffer = String->Buffer;

    //
    // Initialize the bitmap sizes.
    //

    ReversedSlashesBitmap->SizeOfBitMap = NumberOfCharacters;
    ReversedDotsBitmap->SizeOfBitMap = NumberOfCharacters;

    //
    // Initialize the lengths of the full path.  Note that we don't use the
    // aligned sizes; they were simply for ensuring pointer-aligned buffers.
    //

    Path->Full.Length = String->Length;
    Path->Full.MaximumLength = String->MaximumLength;

    //
    // Directory points at the same buffer as Full.
    //

    Path->Directory.Buffer = Path->Full.Buffer;

    //
    // Find slashes and dots in reverse.
    //

    if (TRUE) {
        USHORT Index;
        USHORT NumberOfCharacters = String->Length >> 1;
        WCHAR  Char;
        ULONG  Bit;

        for (Index = 0; Index < NumberOfCharacters; Index++) {
            Char = String->Buffer[Index];
            Bit = NumberOfCharacters - Index;
            if (Char == L'\\') {
                FastSetBit(ReversedSlashesBitmap, Bit);
            } else if (Char == L'.') {
                FastSetBit(ReversedDotsBitmap, Bit);
            }
        }
    } else {
        InlineFindTwoWideCharsInUnicodeStringReversed(
            &Path->Full,
            L'\\',
            L'.',
            ReversedSlashesBitmap,
            ReversedDotsBitmap
        );
    }

    //
    // Initialize our bitmap function aliases.
    //

    RtlNumberOfSetBits = Rtl->RtlNumberOfSetBits;
    RtlFindSetBits = Rtl->RtlFindSetBits;

    //
    // Make sure there is at least one slash in the path.
    //

    NumberOfSlashes = (USHORT)RtlNumberOfSetBits(ReversedSlashesBitmap);
    if (NumberOfSlashes == 0) {
        goto Error;
    }
    Path->NumberOfSlashes = NumberOfSlashes;

    //
    // Extract the filename from the path by finding the last backslash
    // (which will be the first bit set in the bitmap) and calculating the
    // offset into the string buffer.
    //

    ReversedSlashIndex = (USHORT)RtlFindSetBits(ReversedSlashesBitmap, 1, 0);
    Offset = NumberOfCharacters - ReversedSlashIndex + 1;

    LengthInBytes = (ReversedSlashIndex - 1) << 1;
    Path->Name.Length = LengthInBytes;
    Path->Name.MaximumLength = LengthInBytes + sizeof(WCHAR);
    Path->Name.Buffer = &Path->Full.Buffer[Offset];

    //
    // The directory name length is easy to isolate now that we have the offset
    // of the first slash; it's simply the character before.
    //

    LengthInBytes = (Offset - 1) << 1;
    Path->Directory.Length = LengthInBytes;
    Path->Directory.MaximumLength = LengthInBytes;

    //
    // Get the number of dots.
    //

    NumberOfDots = (USHORT)RtlNumberOfSetBits(ReversedDotsBitmap);

    //
    // Extract the extension from the path by finding the last dot (which will
    // be the first bit set in the bitmap) and calculating the offset into the
    // string buffer.
    //

    if (NumberOfDots) {
        ReversedDotIndex = (USHORT)RtlFindSetBits(ReversedDotsBitmap, 1, 0);
        Offset = NumberOfCharacters - ReversedDotIndex + 1;

        LengthInBytes = (ReversedDotIndex - 1) << 1;
        Path->Extension.Length = LengthInBytes;
        Path->Extension.MaximumLength = LengthInBytes + sizeof(WCHAR);
        Path->Extension.Buffer = &Path->Full.Buffer[Offset];

        Path->NumberOfDots = NumberOfDots;
    }

    //
    // Set whether or not the path is qualified and potentially set the drive
    // letter if available.
    //

    Buf = Path->Full.Buffer;

    if (Buf[1] == L':' && Buf[2] == L'\\') {

        Path->Drive = Buf[0];
        Path->Flags.IsFullyQualified = TRUE;

    } else if (Buf[0] == L'\\' && Buf[1] == L'\\') {

        Path->Flags.IsFullyQualified = TRUE;
    }

    //
    // We're done, update path flags then return success.
    //

    UpdatePathFlagsIfWithinWindowsDirectories(Rtl, Path);

    return TRUE;

Error:

    return FALSE;
}

_Use_decl_annotations_
BOOL
StringToRtlPath(
    PRTL Rtl,
    PSTRING String,
    PALLOCATOR Allocator,
    PPRTL_PATH PathPointer
    )
/*++

Routine Description:

    Converts a STRING representing a fully-qualified path into a UNICODE_STRING,
    then calls UnicodeStringToRtlPath().

Arguments:

    Rtl - Supplies a pointer to an initialized RTL struct.

    String - Supplies a pointer to a STRING structure that contains a path name.

    Allocator - Supplies a pointer to an ALLOCATOR structure that is used for
        allocating the new RTL_PATH structure (and associated buffers).

    PathPointer - Supplies a pointer to an address that receives the address
        of the newly created RTL_PATH structure if the routine was successful.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PUNICODE_STRING UnicodeString;

    Success = ConvertUtf8StringToUtf16StringSlow(
        String,
        &UnicodeString,
        Allocator,
        NULL
    );
    if (!Success) {
        return FALSE;
    }

    Success = UnicodeStringToRtlPath(Rtl,
                                     UnicodeString,
                                     Allocator,
                                     PathPointer);
    if (!Success) {
        Allocator->Free(Allocator->Context, UnicodeString);
    }

    return Success;
}

_Use_decl_annotations_
BOOL
UnicodeStringToExistingRtlPath(
    PRTL Rtl,
    PUNICODE_STRING String,
    PALLOCATOR BitmapAllocator,
    PALLOCATOR UnicodeStringBufferAllocator,
    PRTL_PATH Path,
    PLARGE_INTEGER TimestampPointer
    )
/*++

Routine Description:

    Converts a UNICODE_STRING representing a fully-qualified path into an
    existing RTL_PATH structure.  Bitmaps are allocated from the bitmap
    allocator, Unicode string buffers are allocated from the Unicode string
    buffer allocator.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL struct.

    UnicodeString - Supplies a pointer to a UNICODE_STRING structure that
        contains a fully-qualified path name.

    BitmapAllocator - Supplies a pointer to an ALLOCATOR structure that is
        used for allocating all bitmaps.

    UnicodeStringBufferAllocator - Supplies a pointer to an ALLOCATOR structure
        that is used for allocating any Unicode string buffers.

    Path - Supplies a pointer to an existing RTL_PATH structure.

    TimestampPointer - Supplies a pointer to a timestamp to use for allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    const BOOL Reverse = TRUE;
    USHORT Offset;
    USHORT Count;
    USHORT NumberOfCharacters;
    USHORT NumberOfCharactersExcludingNull;
    USHORT AlignedNumberOfCharacters;
    USHORT UnicodeBufferSizeInBytes;
    USHORT BitmapBufferSizeInBytes;
    USHORT AlignedBitmapBufferSizeInBytes;
    USHORT NumberOfSlashes;
    USHORT NumberOfDots;
    USHORT ReversedSlashIndex;
    USHORT ReversedDotIndex;
    USHORT LengthInBytes;
    USHORT BitmapAllocationAttempt;
    USHORT UnicodeStringBufferAttempt;
    USHORT NumberOfBitmaps = 2;
    PWCHAR Buf;
    PWCHAR Dest;
    PWCHAR Source;
    PULONG BitmapBuffers;
    PWCHAR UnicodeStringBuffer;
    PRTL_BITMAP ReversedSlashesBitmap;
    PRTL_BITMAP ReversedDotsBitmap;
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;
    PRTL_FIND_SET_BITS RtlFindSetBits;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BitmapAllocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(UnicodeStringBufferAllocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Path)) {
        return FALSE;
    }

    if (!IsValidMinimumDirectoryNullTerminatedUnicodeString(String)) {
        return FALSE;
    }

    //
    // Get the number of characters, plus +1 for the trailing NULL.
    //

    NumberOfCharacters = (String->Length >> 1) + 1;
    NumberOfCharactersExcludingNull = String->Length >> 1;

    //
    // Calculate the aligned number of characters.  We do the alignment
    // in order for the bitmap buffers to have a pointer-aligned size.
    //

    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfCharacters
        )
    );

    //
    // Calculate the aligned Unicode string buffer size in bytes.
    //

    UnicodeBufferSizeInBytes = AlignedNumberOfCharacters << 1;

    UnicodeStringBuffer = NULL;
    UnicodeStringBufferAttempt = 0;

#define TRY_ALLOC_UNICODE_BUFFER() do {                           \
    if (!UnicodeStringBuffer) {                                   \
        UnicodeStringBufferAttempt++;                             \
        UnicodeStringBuffer = (PWCHAR)(                           \
            UnicodeStringBufferAllocator->TryCallocWithTimestamp( \
                UnicodeStringBufferAllocator->Context,            \
                1,                                                \
                UnicodeBufferSizeInBytes,                         \
                TimestampPointer                                  \
            )                                                     \
        );                                                        \
    }                                                             \
} while (0)

    TRY_ALLOC_UNICODE_BUFFER();

    //
    // Calculate the individual bitmap buffer sizes.  As there's one bit per
    // character, we shift left 3 (divide by 8) to get the number of bytes
    // required.
    //

    BitmapBufferSizeInBytes = AlignedNumberOfCharacters >> 3;

    //
    // Align to a pointer boundary.
    //

    AlignedBitmapBufferSizeInBytes = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            BitmapBufferSizeInBytes
        )
    );

    //
    // Allocate memory for the bitmap buffers.
    //

    BitmapBuffers = NULL;
    BitmapAllocationAttempt = 0;

#define TRY_ALLOC_BITMAP_BUFFERS() do {              \
    if (!BitmapBuffers) {                            \
        BitmapAllocationAttempt++;                   \
        BitmapBuffers = (PULONG)(                    \
            BitmapAllocator->TryCallocWithTimestamp( \
                BitmapAllocator->Context,            \
                NumberOfBitmaps,                     \
                AlignedBitmapBufferSizeInBytes,      \
                TimestampPointer                     \
            )                                        \
        );                                           \
    }                                                \
} while (0)

    TRY_ALLOC_BITMAP_BUFFERS();

    TRY_ALLOC_UNICODE_BUFFER();

    Path->StructSize = sizeof(*Path);
    Path->AllocSize = (
        UnicodeBufferSizeInBytes +
        (AlignedBitmapBufferSizeInBytes * NumberOfBitmaps)
    );

    //
    // Carve up the rest of the allocated buffer past the RTL_PATH struct
    // into the two bitmap buffers and then the final Unicode string buffer.
    //

    ReversedSlashesBitmap = &Path->ReversedSlashesBitmap;
    ReversedDotsBitmap = &Path->ReversedDotsBitmap;

    if (!BitmapBuffers) {

        //
        // Final allocation attempt.
        //

        BitmapAllocationAttempt++;
        BitmapBuffers = (PULONG)BitmapAllocator->CallocWithTimestamp(
            BitmapAllocator->Context,
            NumberOfBitmaps,
            AlignedBitmapBufferSizeInBytes,
            TimestampPointer
        );

        if (!BitmapBuffers) {
            goto Error;
        }
    }

    TRY_ALLOC_UNICODE_BUFFER();

    //
    // Point ReversedSlashesBitmap at the newly allocated buffer.
    //

    ReversedSlashesBitmap->Buffer = BitmapBuffers;

    //
    // Carve out ReversedDotsBitmap.
    //

    ReversedDotsBitmap->Buffer = (PULONG)(
        RtlOffsetToPointer(
            BitmapBuffers,
            AlignedBitmapBufferSizeInBytes
        )
    );

    //
    // Initialize the bitmap sizes.
    //

    ReversedSlashesBitmap->SizeOfBitMap = NumberOfCharactersExcludingNull;
    ReversedDotsBitmap->SizeOfBitMap = NumberOfCharactersExcludingNull;

    //
    // Final blocking attempt of the Unicode string buffer if it hasn't already
    // been allocated.
    //

    if (!UnicodeStringBuffer) {
        UnicodeStringBufferAttempt++;
        UnicodeStringBuffer = (PWCHAR)(
            UnicodeStringBufferAllocator->CallocWithTimestamp(
                UnicodeStringBufferAllocator->Context,
                1,
                UnicodeBufferSizeInBytes,
                TimestampPointer
            )
        );

        if (!UnicodeStringBuffer) {
            goto Error;
        }
    }

    //
    // Initialize the lengths of the full path.
    //

    Path->Full.Length = String->Length;
    Path->Full.MaximumLength = UnicodeBufferSizeInBytes;

    //
    // Initialize the buffer, then copy the Unicode string over.
    //

    Dest = Path->Full.Buffer = UnicodeStringBuffer;
    Source = String->Buffer;
    Count = NumberOfCharactersExcludingNull;
    __movsw(Dest, Source, Count);

    //
    // Add trailing NULL.
    //

    Dest += Count;
    *Dest++ = L'\0';

    //
    // Directory points at the same buffer as Full.
    //

    Path->Directory.Buffer = Path->Full.Buffer;

    //
    // Find slashes and dots in reverse.
    //

    InlineFindTwoWideCharsInUnicodeStringReversed(
        &Path->Full,
        L'\\',
        L'.',
        ReversedSlashesBitmap,
        ReversedDotsBitmap
    );

    //
    // Initialize our bitmap function aliases.
    //

    RtlNumberOfSetBits = Rtl->RtlNumberOfSetBits;
    RtlFindSetBits = Rtl->RtlFindSetBits;

    //
    // Make sure there is at least one slash in the path.
    //

    NumberOfSlashes = (USHORT)RtlNumberOfSetBits(ReversedSlashesBitmap);
    if (NumberOfSlashes == 0) {
        goto Error;
    }
    Path->NumberOfSlashes = NumberOfSlashes;

    //
    // Extract the filename from the path by finding the last backslash
    // (which will be the first bit set in the bitmap) and calculating the
    // offset into the string buffer.
    //

    ReversedSlashIndex = (USHORT)RtlFindSetBits(ReversedSlashesBitmap, 1, 0);
    Offset = NumberOfCharactersExcludingNull - ReversedSlashIndex + 1;

    LengthInBytes = (ReversedSlashIndex - 1) << 1;
    Path->Name.Length = LengthInBytes;
    Path->Name.MaximumLength = LengthInBytes + sizeof(WCHAR);
    Path->Name.Buffer = &Path->Full.Buffer[Offset];

    //
    // The directory name length is easy to isolate now that we have the offset
    // of the first slash; it's simply the character before.
    //

    LengthInBytes = (Offset - 1) << 1;
    Path->Directory.Length = LengthInBytes;
    Path->Directory.MaximumLength = LengthInBytes;

    //
    // Get the number of dots.
    //

    NumberOfDots = (USHORT)RtlNumberOfSetBits(ReversedDotsBitmap);

    //
    // Extract the extension from the path by finding the last dot (which will
    // be the first bit set in the bitmap) and calculating the offset into the
    // string buffer.
    //

    if (NumberOfDots) {
        ReversedDotIndex = (USHORT)RtlFindSetBits(ReversedDotsBitmap, 1, 0);
        Offset = NumberOfCharactersExcludingNull - ReversedDotIndex + 1;

        LengthInBytes = (ReversedDotIndex - 1) << 1;
        Path->Extension.Length = LengthInBytes;
        Path->Extension.MaximumLength = LengthInBytes + sizeof(WCHAR);
        Path->Extension.Buffer = &Path->Full.Buffer[Offset];

        Path->NumberOfDots = NumberOfDots;
    }

    //
    // Set whether or not the path is qualified and potentially set the drive
    // letter if available.
    //

    Buf = Path->Full.Buffer;

    if (Buf[1] == L':' && Buf[2] == L'\\') {

        Path->Drive = Buf[0];
        Path->Flags.IsFullyQualified = TRUE;

    } else if (Buf[0] == L'\\' && Buf[1] == L'\\') {

        Path->Flags.IsFullyQualified = TRUE;
    }

    //
    // We're done, update path flags and return success.
    //

    UpdatePathFlagsIfWithinWindowsDirectories(Rtl, Path);

    return TRUE;

Error:

    if (BitmapBuffers) {
        BitmapAllocator->Free(BitmapAllocator->Context, BitmapBuffers);
    }

    if (UnicodeStringBuffer) {
        UnicodeStringBufferAllocator->Free(
            UnicodeStringBufferAllocator->Context,
            UnicodeStringBuffer
        );
    }

    return FALSE;
}


_Use_decl_annotations_
BOOL
DestroyRtlPath(
    PPRTL_PATH PathPointer
    )
{
    PRTL_PATH Path;
    PALLOCATOR Allocator;

    //
    // Validate argument.
    //

    if (!ARGUMENT_PRESENT(PathPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(*PathPointer)) {
        return FALSE;
    }

    Path = *PathPointer;

    if (!ARGUMENT_PRESENT(Path->Allocator)) {
        return FALSE;
    }

    Allocator = Path->Allocator;

    Allocator->Free(Allocator->Context, Path);

    *PathPointer = NULL;

    return TRUE;
}

_Use_decl_annotations_
BOOL
GetModuleRtlPath(
    PRTL Rtl,
    HMODULE Module,
    PALLOCATOR Allocator,
    PPRTL_PATH PathPointer
    )
/*++

Routine Description:

    Allocates and initializes a new RTL_PATH structure from Allocator, based on
    the unicode string path name returned as the filename for Module.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Module - Supplies the module handle for which we will resolve the path.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure that
        will be used for all memory allocations.

    PathPointer - Supplies a pointer to an address that receives the address
        of the newly created RTL_PATH structure if the routine was successful.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PWSTR Buffer;
    ULONG Bytes;
    ULONG Count;
    ULONG LastError;
    ULONG NumberOfCharacters;
    UNICODE_STRING String;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathPointer)) {
        return FALSE;
    }

    //
    // Attempt to allocate a buffer.
    //

    Bytes = MAX_RTL_PATH_ALLOC;
    Count = Bytes >> 1;

    Buffer = (PWSTR)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            Bytes
        )
    );

    if (!Buffer) {
        return FALSE;
    }

    //
    // Get the module file name.
    //

    Success = FALSE;
    NumberOfCharacters = GetModuleFileNameW(Module, Buffer, Count);
    LastError = GetLastError();

    if (NumberOfCharacters == 0 || LastError == ERROR_INSUFFICIENT_BUFFER) {
        goto Error;
    }

    //
    // Wrap the buffer in a UNICODE_STRING and palm off to UnicodeStringToPath.
    //

    String.Hash = 0;
    String.Length = (USHORT)(NumberOfCharacters << 1);
    String.MaximumLength = String.Length + sizeof(WCHAR);
    String.Buffer = Buffer;

    Success = UnicodeStringToRtlPath(Rtl, &String, Allocator, PathPointer);

    //
    // Intentional follow-on to Error as we need to free Buffer regardless of
    // whether or not there was actually an error.
    //

Error:

    if (Buffer) {
        Allocator->Free(Allocator->Context, Buffer);
        Buffer = NULL;
    }

    return Success;
}

_Use_decl_annotations_
PUNICODE_STRING
CurrentDirectoryToUnicodeString(
    PALLOCATOR Allocator
    )
/*++

Routine Description:

    This function gets the current working directory as a wide character string,
    then allocates a new buffer to hold a UNICODE_STRING structure and a copy
    of the string using the provided Allocator, then copies the string details,
    initializes the struct's lengths, and returns a pointer to the structure.

    The size of the UNICODE_STRING structure plus the trailing WCHAR buffer
    is calculated up-front and satisfied with a single Allocator->Calloc()
    call.  The String->Buffer is then adjusted to the point past the structure.
    Thus, the structure can be freed via a single Allocator->Free(String) call.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure that will be used
        to perform allocations.

Return Value:

    A pointer to a UNICODE_STRING structure representing the current directory,
    or NULL if an error occurred.

--*/
{
    LONG_INTEGER NumberOfChars;
    LONG_INTEGER AllocSize;
    LONG_INTEGER AlignedAllocSize;
    PUNICODE_STRING String;

    NumberOfChars.LongPart = GetCurrentDirectoryW(0, NULL);
    if (NumberOfChars.LongPart <= 0) {
        return NULL;
    }

    if (NumberOfChars.HighPart) {
        return NULL;
    }

    AllocSize.LongPart = (
        (NumberOfChars.LowPart << 1) +
        sizeof(UNICODE_STRING)
    );
    AlignedAllocSize.LongPart = ALIGN_UP_POINTER(AllocSize.LongPart);

    if (AlignedAllocSize.HighPart) {
        return NULL;
    }

    String = (PUNICODE_STRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedAllocSize.LongPart
        )
    );

    if (!String) {
        return NULL;
    }

    String->Length = (USHORT)((NumberOfChars.LowPart - 1) << 1);
    String->MaximumLength = String->Length + sizeof(WCHAR);

    String->Buffer = (PWCHAR)RtlOffsetToPointer(String, sizeof(UNICODE_STRING));

    if (GetCurrentDirectoryW(String->MaximumLength, String->Buffer) <= 0) {
        Allocator->Free(Allocator->Context, String);
        return NULL;
    }

    return String;
}

_Use_decl_annotations_
PRTL_PATH
CurrentDirectoryToRtlPath(
    PALLOCATOR Allocator
    )
{

    //
    // XXX todo.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    return NULL;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
