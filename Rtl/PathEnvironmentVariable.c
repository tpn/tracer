#include "stdafx.h"

_Use_decl_annotations_
PPATH_ENV_VAR
LoadPathEnvironmentVariable(
    PRTL Rtl,
    PALLOCATOR Allocator,
    USHORT ReservedUnicodeBufferSizeInBytes
    )
/*++

Routine Description:

    This routine creates and initializes a new PATH_ENV_VAR struct using the
    given Allocator.

Arguments:

    Allocator - Supplies a pointer to an initialized ALLOCATOR struct.

    ReservedUnicodeBufferSizeInBytes - If non-zero, indicates that the second
        UNICODE_STRING in the PATH_ENV_VAR struct, NewPaths, should have this
        many bytes reserved at the start of the string.


Return Value:

    If the PATH_ENV_VAR could be allocated and successfully initialized, a
    pointer will be returned to it.  A NULL pointer will be returned on error.

--*/
{
    WCHAR Char;
    USHORT Index;
    USHORT Count;
    USHORT BitmapBufferSizeInBytes;
    USHORT AlignedBitmapBufferSizeInBytes;
    USHORT AlignedNumberOfCharacters;
    USHORT UnicodeBufferSizeInBytes;
    USHORT UnicodePrefixTableEntriesSizeInBytes;
    USHORT DirectoryUnicodeStringsAllocSizeInBytes;
    ULONG BitmapIndex;
    ULONG PreviousBitmapIndex;
    LONG Length;
    LONG_INTEGER NumberOfChars;
    LONG_INTEGER AllocSize;
    LONG_INTEGER AlignedAllocSize;
    LONG_INTEGER SecondAllocSize;
    LONG_INTEGER SecondAlignedAllocSize;
    PPATH_ENV_VAR Path;
    PUNICODE_STRING String;
    PUNICODE_STRING NewString;
    PUNICODE_STRING Directory;
    PUNICODE_PREFIX_TABLE PrefixTable;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;
    PRTL_BITMAP Bitmap;
    PRTL_FIND_SET_BITS RtlFindSetBits;
    PRTL_INSERT_UNICODE_PREFIX RtlInsertUnicodePrefix;

    //
    // Get the number of characters in the current PATH environment variable.
    //

    Length = GetEnvironmentVariableW(PATH_ENV_NAME, NULL, 0);
    if (Length == 0) {
        return NULL;
    }

    NumberOfChars.LongPart = Length;

    //
    // Sanity check it's not longer than MAX_USHORT.
    //

    if (NumberOfChars.HighPart) {
        return NULL;
    }

    //
    // Align number of characters to a pointer boundary.
    //

    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfChars.LowPart
        )
    );


    //
    // Calculate unicode buffer size in bytes.
    //

    UnicodeBufferSizeInBytes = AlignedNumberOfCharacters << 1;

    //
    // Calculate bitmap buffer size in bytes.  One bit per character.
    //

    BitmapBufferSizeInBytes = AlignedNumberOfCharacters >> 3;

    //
    // Align it to a pointer boundary.
    //

    AlignedBitmapBufferSizeInBytes = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            BitmapBufferSizeInBytes
        )
    );

    //
    // Calculate the final allocation size.
    //

    AllocSize.LongPart = (

        //
        // Size of the containing PATH_ENV_VAR struct.
        //

        sizeof(PATH_ENV_VAR) +

        //
        // Plus the bitmap buffer size that we use to mark path boundaries.
        //

        BitmapBufferSizeInBytes +

        //
        // Plus two identically-sized unicode buffers; one for the first path,
        // one for the second one we build as part of the path removal.  The
        // second will be shorter than the first if we're removing paths, but
        // it's easier to just allocate identically sized buffers up-front and
        // have a little temporary space-wastage.
        //

        (UnicodeBufferSizeInBytes * 2)

    );

    //
    // If our reserved size is greater than zero, add that in as well.
    //

    if (ReservedUnicodeBufferSizeInBytes) {
        AllocSize.LongPart += ReservedUnicodeBufferSizeInBytes;
    }

    //
    // Align the allocation size and make sure it's still not larger than
    // MAX_USHORT.
    //

    AlignedAllocSize.LongPart = ALIGN_UP_POINTER(AllocSize.LongPart);

    if (AlignedAllocSize.HighPart) {
        return NULL;
    }

    //
    // Point the Path pointer at the start of the buffer.  Trailing elements
    // will be carved out after.
    //

    Path = (PPATH_ENV_VAR)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedAllocSize.LongPart
        )
    );

    if (!Path) {
        return NULL;
    }

    Path->Allocator = Allocator;

    //
    // Initialize the bitmap and initial path string.
    //

    Bitmap = &Path->Bitmap;
    Bitmap->SizeOfBitMap = AlignedNumberOfCharacters;
    Bitmap->Buffer = (PULONG)RtlOffsetToPointer(Path, sizeof(PATH_ENV_VAR));

    String = &Path->Paths;

    String->Length = (NumberOfChars.LowPart - 1) << 1;
    String->MaximumLength = String->Length + sizeof(WCHAR);

    String->Buffer = (PWCHAR)(
        RtlOffsetToPointer(
            Path,
            sizeof(PATH_ENV_VAR) + AlignedBitmapBufferSizeInBytes
        )
    );

    //
    // Initialize the UNICODE_STRING for the new path.
    //

    NewString = &Path->NewPaths;

    //
    // Factor in the reserved length if applicable.
    //

    if (ReservedUnicodeBufferSizeInBytes) {

        NewString->Length = ReservedUnicodeBufferSizeInBytes;

        NewString->MaximumLength = (
            String->MaximumLength +
            ReservedUnicodeBufferSizeInBytes
        );

    } else {

        //
        // Initialize the length to 0 as we haven't been requested to reserve
        // any buffer space in the second unicode string's buffer.
        //

        NewString->Length = 0;

        //
        // Use the first string's maximum length.
        //

        NewString->MaximumLength = String->MaximumLength;

    }

    //
    // Point the buffer after the struct, bitmap and first string buffer.
    //

    NewString->Buffer = (PWCHAR)(
        RtlOffsetToPointer(
            Path,
            sizeof(PATH_ENV_VAR) +
            AlignedBitmapBufferSizeInBytes +
            UnicodeBufferSizeInBytes
        )
    );

    //
    // Now save the actual environment variable into our first buffer.
    //

    Length = GetEnvironmentVariableW(
        PATH_ENV_NAME,
        String->Buffer,
        String->MaximumLength
    );

    //
    // Scan the string and construct a bitmap that indicates where each path
    // (directory) begins (by setting the next bit in the bitmap) and setting
    // the ';' character to \0.  (It's slightly nicer to debug when our strings
    // are all NULL-terminated.)
    //

    for (Index = 0, Count = 0; Index < NumberOfChars.LowPart; Index++) {
        Char = String->Buffer[Index];
        if (Char == L';') {
            Count++;
            FastSetBit(Bitmap, Index);
            String->Buffer[Index] = L'\0';
        }
    }

    Path->StructSize = sizeof(*Path);
    Path->FirstAlignedAllocSizeInBytes = AlignedAllocSize.LowPart;
    Path->NumberOfElements = Count;

    //
    // Now that we know how many directories we found, we can allocate space
    // for all of the prefix table entries we're going to need to build the
    // two prefix tables, and the array of UNICODE_STRING structs we're going
    // to use to wrap each directory.
    //

    UnicodePrefixTableEntriesSizeInBytes = (
        2 * Count * sizeof(UNICODE_PREFIX_TABLE_ENTRY)
    );

    DirectoryUnicodeStringsAllocSizeInBytes = sizeof(UNICODE_STRING) * Count;

    SecondAllocSize.LongPart = (
        UnicodePrefixTableEntriesSizeInBytes +
        DirectoryUnicodeStringsAllocSizeInBytes
    );

    //
    // Align up to a pointer boundary.
    //

    SecondAlignedAllocSize.LongPart = (
        ALIGN_UP_POINTER(SecondAllocSize.LongPart)
    );

    //
    // Sanity check we're within MAX_USHORT.
    //

    if (SecondAlignedAllocSize.HighPart) {
        goto Error;
    }

    Path->PathsPrefixTableEntries = (PUNICODE_PREFIX_TABLE_ENTRY)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            SecondAlignedAllocSize.LowPart
        )
    );

    if (!Path->PathsPrefixTableEntries) {
        goto Error;
    }

    Path->SecondAlignedAllocSizeInBytes = SecondAlignedAllocSize.LowPart;

    //
    // Point the PathsToRemovePrefixTableEntries pointer after the first array
    // of entries.
    //

    Path->PathsToRemovePrefixTableEntries = (PUNICODE_PREFIX_TABLE_ENTRY)(
        RtlOffsetToPointer(
            Path->PathsPrefixTableEntries,
            Count * sizeof(UNICODE_PREFIX_TABLE_ENTRY)
        )
    );

    //
    // Point the directories at the next buffer.
    //

    Path->Directories = (PUNICODE_STRING)(
        RtlOffsetToPointer(
            Path->PathsToRemovePrefixTableEntries,
            Count * sizeof(UNICODE_PREFIX_TABLE_ENTRY)
        )
    );

    //
    // Initialize the two prefix tables.
    //

    Rtl->RtlInitializeUnicodePrefix(&Path->PathsPrefixTable);
    Rtl->RtlInitializeUnicodePrefix(&Path->PathsToRemovePrefixTable);

    //
    // Initialize function pointer aliases.
    //

    RtlFindSetBits = Rtl->RtlFindSetBits;
    RtlInsertUnicodePrefix = Rtl->RtlInsertUnicodePrefix;

    //
    // Enumerate the directories once again and carve out the individual
    // Directory strings and add prefix table entries for them.
    //

    Directory = Path->Directories;
    PrefixTable = &Path->PathsPrefixTable;
    PrefixTableEntry = Path->PathsPrefixTableEntries;

    PreviousBitmapIndex = 0;

    for (Index = 0; Index < Count; Index++) {

        //
        // Find the offset of the next NULL character.
        //

        BitmapIndex = RtlFindSetBits(Bitmap, 1, PreviousBitmapIndex);

        if (BitmapIndex == BITS_NOT_FOUND) {

            //
            // Shouldn't ever happen.
            //

            __debugbreak();
            goto Error;
        }

        //
        // Carve out the directory pointer.
        //

        Directory->Length = (
            (USHORT)BitmapIndex -
            (USHORT)PreviousBitmapIndex
        ) << 1;

        Directory->MaximumLength = Directory->Length;
        Directory->Buffer = String->Buffer + PreviousBitmapIndex;

        //
        // Add it to our prefix tree, using the PrefixTableEntry.  We don't
        // care about the return value (that is, we don't care if the directory
        // already exists).
        //

        RtlInsertUnicodePrefix(PrefixTable, Directory, PrefixTableEntry);

        //
        // Advance the directory and prefix table entry pointers.
        //

        Directory++;
        PrefixTableEntry++;

        //
        // Update the previous bitmap index.
        //

        PreviousBitmapIndex = BitmapIndex + 1;

    }

    return Path;

Error:

    DestroyPathEnvironmentVariable(&Path);

    return NULL;
}


_Use_decl_annotations_
VOID
DestroyPathEnvironmentVariable(
    PPPATH_ENV_VAR PathPointer
    )
/*++

Routine Description:

    This routine destroys a PATH_ENV_VAR struct.  A partially-initialized
    struct (or NULL) can be passed.  (The intent being that this method is
    safe to call on the PATH_ENV_VAR in any state.)

Arguments:

    PathPointer - Supplies a pointer to the address of a pointer to a
        PATH_ENV_VAR struct.  This value will be cleared (as long as it is
        non-NULL).

Return Value:

    None.

--*/
{
    PPATH_ENV_VAR Path;
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(PathPointer)) {
        return;
    }

    Path = *PathPointer;

    //
    // Clear the user's pointer first.
    //

    *PathPointer = NULL;

    if (!Path) {
        return;
    }

    Allocator = Path->Allocator;

    if (!Allocator) {

        //
        // Can't do much if no allocator was set.
        //

        return;
    }

    //
    // The PATH_ENV_VAR struct is constructed via two memory allocations.  The
    // first is rooted at the Path struct and holds the struct plus bitmaps
    // and unicode string buffers.  The second is allocated once we know how
    // many directories are present, and is rooted at PathsPrefixTableEntries.
    // We check that first before blowing away Path.
    //

    if (Path->PathsPrefixTableEntries) {
        Allocator->FreePointer(
            Allocator->Context,
            &Path->PathsPrefixTableEntries
        );
    }

    Allocator->FreePointer(Allocator->Context, &Path);

    return;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
