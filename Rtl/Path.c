/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Path.c

Abstract:

    This module implements routines related to path handling.

--*/
#include "stdafx.h"

#define MAX_RTL_PATH_ALLOC 4000

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
    //      unicode string buffer.
    //


    //
    // Get the number of characters, plus +1 for the trailing NULL.
    //

    NumberOfCharacters = (String->Length + 1) >> 1;

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
    // Calculate the aligned unicode string buffer size in bytes.
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
    // RTL_PATH structure and trailing bitmap buffers and unicode buffer.
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
        // Size of the unicode buffer.  Includes our trailing NULL.
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

    ReversedSlashesBitmap->SizeOfBitMap = AlignedNumberOfCharacters;
    ReversedDotsBitmap->SizeOfBitMap = AlignedNumberOfCharacters;

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
    // Set the allocator.
    //

    Path->Allocator = Allocator;

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
        Allocator
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

    Allocates and initializes a new RTL_PATH structure from Allocator, based on the
    unicode string path name returned as the filename for Module.

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
