#include "stdafx.h"

#include "TracedPythonSessionPrivate.h"
#include "../Python/PythonDllFiles.h"

_Use_decl_annotations_
BOOL
SanitizePathEnvironmentVariableForPython(
    PTRACED_PYTHON_SESSION Session
    )
/*++

Routine Description:

    This function checks every path in the current PATH environment variable
    and removes any paths that are one of the Python library paths (or prefix
    paths of said Python paths).

Arguments:

    Session - Supplies a pointer to a TRACED_PYTHON_SESSION structure that has
        has at least the following members initialized:
            Allocator
            PythonDllPath
            PythonHomePath
            NumberOfPathEntries
            PathEntries

Return Value:

    TRUE on Success, FALSE if an error occurred.

--*/
{
    BOOL Success;
    BOOL Exists;
    USHORT Bytes;
    USHORT Count;
    USHORT WhichIndex;
    USHORT ExpectedMaximumLength;
    ULONG PreviousIndex;
    ULONG Index;
    ULONG BitmapIndex;
    LONG_INTEGER ReservedUnicodeBufferSizeInBytes;
    PWCHAR Dest;
    PWCHAR ExpectedDest;
    PRTL Rtl;
    PALLOCATOR Allocator;
    PFILES_EXISTW FilesExistW;
    PRTL_FIND_SET_BITS RtlFindSetBits;
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;
    PRTL_EQUAL_UNICODE_STRING RtlEqualUnicodeString;
    PRTL_FIND_UNICODE_PREFIX RtlFindUnicodePrefix;
    PRTL_INSERT_UNICODE_PREFIX RtlInsertUnicodePrefix;
    PRTL_REMOVE_UNICODE_PREFIX RtlRemoveUnicodePrefix;
    PUNICODE_STRING WhichFilename;
    PUNICODE_STRING Directory;
    PUNICODE_STRING String;
    PUNICODE_PREFIX_TABLE KeepPrefixTable;
    PUNICODE_PREFIX_TABLE RemovePrefixTable;
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry;
    PUNICODE_PREFIX_TABLE_ENTRY RemovePrefixTableEntry;
    PPATH_ENV_VAR Path;
    PRTL_BITMAP Bitmap;
    PYTHON_HOME Home;

    Rtl = Session->Rtl;
    FilesExistW = Rtl->FilesExistW;
    RtlFindSetBits = Rtl->RtlFindSetBits;
    RtlNumberOfSetBits = Rtl->RtlNumberOfSetBits;
    RtlEqualUnicodeString = Rtl->RtlEqualUnicodeString;
    RtlFindUnicodePrefix = Rtl->RtlFindUnicodePrefix;
    RtlInsertUnicodePrefix = Rtl->RtlInsertUnicodePrefix;
    RtlRemoveUnicodePrefix = Rtl->RtlRemoveUnicodePrefix;

    Allocator = Session->Allocator;

    //
    // Calculate the length of the Python path entries we want to prefix the
    // new path entry with.
    //

    String = Session->PathEntries;
    ReservedUnicodeBufferSizeInBytes.LongPart = 0;
    for (Index = 0; Index < Session->NumberOfPathEntries; Index++) {
        ReservedUnicodeBufferSizeInBytes.LongPart += String->MaximumLength;
        String++;
    }

    //
    // Make sure we're under MAX_USHORT.
    //

    if (ReservedUnicodeBufferSizeInBytes.HighPart) {
        return FALSE;
    }

    //
    // Load the PATH_ENV_VAR struct.
    //

    Path = LoadPathEnvironmentVariable(
        Rtl,
        Allocator,
        ReservedUnicodeBufferSizeInBytes.LowPart
    );

    if (!Path) {
        return FALSE;
    }

    SecureZeroMemory(&Home, sizeof(Home));

    Bitmap = &Path->Bitmap;
    String = &Path->NewPaths;

    PreviousIndex = 0;
    BitmapIndex = 0;

    //
    // Set up some aliases.
    //

    Directory = Path->Directories;
    RemovePrefixTable = &Path->PathsToRemovePrefixTable;
    PrefixTableEntry = Path->PathsToRemovePrefixTableEntries;

    for (Index = 0; Index < Path->NumberOfElements; Index++) {

        Success = Rtl->FilesExistW(
            Rtl,
            Directory,
            NumberOfPythonDllFiles,
            (PPUNICODE_STRING)PythonDllFilesW,
            &Exists,
            &WhichIndex,
            &WhichFilename
        );

        if (!Success) {
            goto Error;
        }

        if (Exists) {

            //
            // This directory is a Python path, so add it to our prefix tree.
            //

            RtlInsertUnicodePrefix(
                RemovePrefixTable,
                Directory,
                PrefixTableEntry
            );

            //
            // Advance our PrefixTableEntry pointer to the next allocated
            // struct.
            //

            PrefixTableEntry++;
        }

        //
        // Advance our Directory pointer.
        //

        Directory++;

    }

    //
    // Enumerate over the directories again and remove any that have a prefix
    // match against the list of Python home directories we found.
    //

    Directory = Path->Directories;
    KeepPrefixTable = &Path->PathsPrefixTable;

    for (Index = 0; Index < Path->NumberOfElements; Index++) {

        //
        // Search the PathsToRemove prefix table for this directory.
        //

        RemovePrefixTableEntry = RtlFindUnicodePrefix(
            RemovePrefixTable,
            Directory,
            0
        );

        //
        // We don't care if the match wasn't exact, as long as it's a prefix,
        // let's remove it.
        //

        if (RemovePrefixTableEntry) {

            //
            // Find the corresponding prefix table entry in the "keep" table
            // and remove it.
            //

            PrefixTableEntry = RtlFindUnicodePrefix(
                KeepPrefixTable,
                Directory,
                0
            );

            if (!PrefixTableEntry) {
                __debugbreak();
                goto Error;
            }

            RtlRemoveUnicodePrefix(KeepPrefixTable, PrefixTableEntry);
        }

        //
        // Advance to the next directory.
        //

        Directory++;
    }

    //
    // Copy the Python path entries we want to the front of our final path
    // environment string.
    //

    String = &Path->NewPaths;
    Dest = String->Buffer;

    //
    // Point the directory pointer at the first of the Python home path entries
    // we want to add.
    //

    Directory = Session->PathEntries;

    for (Index = 0; Index < Session->NumberOfPathEntries; Index++) {

        Bytes = Directory->Length;
        Count = Bytes >> 1;
        __movsw(Dest, Directory->Buffer, Count);

        Dest += Count;
        *Dest++ = L';';

        Directory++;
    }

    //
    // Sanity check: make sure our Dest pointer is where we expected.
    //

    ExpectedDest = (PWCHAR)(
        RtlOffsetToPointer(
            Path->NewPaths.Buffer,
            ReservedUnicodeBufferSizeInBytes.LongPart
        )
    );

    if (ExpectedDest != Dest) {
        __debugbreak();
        goto Error;
    }

    //
    // Do a final enumeration of the directories, look for those that have an
    // exact match in the "keep" prefix table, and copy them to the final path
    // environment variable string.
    //

    Directory = Path->Directories;

    for (Index = 0; Index < Path->NumberOfElements; Index++) {

        PrefixTableEntry = RtlFindUnicodePrefix(
            KeepPrefixTable,
            Directory,
            TRUE
        );

        if (!PrefixTableEntry) {
            Directory++;
            continue;
        }

        //
        // Copy the directory.
        //

        Bytes = Directory->Length;
        Count = Bytes >> 1;
        __movsw(Dest, Directory->Buffer, Count);

        //
        // Add the path separator.
        //

        Dest += Count;
        *Dest++ = L';';

        //
        // Advance the directory pointer.
        //

        Directory++;

    }

    //
    // Change the final trailing character into a NULL and set the lengths.
    //

    --Dest;

    if (*Dest != L';') {
        __debugbreak();
        goto Error;
    }

    *Dest-- = L'\0';

    //
    // Update the string length.  (MaximumLength will already be set.)
    //

    String->Length = (USHORT)(Dest - String->Buffer);

    ExpectedMaximumLength = (
        Path->Paths.MaximumLength +
        ReservedUnicodeBufferSizeInBytes.LowPart
    );
    if (String->MaximumLength != ExpectedMaximumLength) {
        __debugbreak();
        goto Error;
    }

    //
    // Finally, set the new path environment value.
    //

    Success = SetEnvironmentVariableW(PATH_ENV_NAME, String->Buffer);

    //
    // Intentional follow-on.
    //

Error:
    DestroyPathEnvironmentVariable(&Path);

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
