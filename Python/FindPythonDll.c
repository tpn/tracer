#include "stdafx.h"

_Use_decl_annotations_
BOOL
FindPythonDllAndExe(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PUNICODE_STRING Directory,
    PPUNICODE_STRING PythonDllPath,
    PPUNICODE_STRING PythonExePath,
    PUSHORT NumberOfPathEntriesPointer,
    PPUNICODE_STRING PathEntriesPointer,
    PCHAR MajorVersionPointer,
    PCHAR MinorVersionPointer
    )
/*++

Routine Description:

    Finds the first Python DLL file in the given directory.  If a DLL can be
    found, creates two new UNICODE_STRING string structures using the given
    Allocator, one for the DLL Path (PythonDllPath) and one for the .exe in
    that directory (PythonExePath).

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure that will be used
        to allocate a new fully-qualified path from the base Directory and the
        Python DLL filename, if one is found.

    Directory - Supplies a pointer to a fully-qualified UNICODE_STRING
        structure that represents the directory of interest.

    PythonDllPath - Supplies a pointer that receives the address of a
        UNICODE_STRING representing the found Python DLL, if any.

    PythonExePath - Supplies a pointer that receives the address of a
        UNICODE_STRING representing "python.exe" in the same directory that
        PythonDllPath was found (if at all).

    NumberOfPathEntriesPointer - Supplies a pointer to an address that
        receives the number of PUNICODE_STRING pointers pointed to by the
        PathEntriesPointer array.

    PathEntriesPointer - Supplies a pointer that receives the address of an
        array of PUNICODE_STRING structures.

    MajorVersion - Supplies a pointer to a CHAR that will receive the
        major version of the Python DLL based on the filename, e.g. 2 for
        python26.dll and python27.dll, 3 for python34.dll, python35.dll etc.

    MinorVersion - Supplies a pointer to a CHAR that will receive the
        minor version of the Python DLL based on the filename, e.g. 7 for
        python27.dll, 4 for python34.dll, etc.


Return Value:

    TRUE if no error occurred, FALSE otherwise.  Note that TRUE does not
    imply that a file was found, simply that no error occurred.  You should
    test PythonDllPath for a NULL pointer to discern whether or not a DLL
    file was successfully found.

    PythonExePath will always be set if PythonDllPath is set, and it will
    never be set if PythonDllPath is not set.

--*/
{
    BOOL Exists;
    BOOL Success;
    USHORT Index;
    USHORT Bytes;
    USHORT Count;
    USHORT WhichIndex;
    USHORT Length;
    USHORT MaximumLength;
    USHORT NumberOfPathSuffixes;
    USHORT NumberOfPathEntries;
    USHORT PathsArraySize;
    USHORT PathsUnicodeStringStructsSize;
    LONG_INTEGER AllocSize;
    PWCHAR Dest;
    PWCHAR Source;
    PWCHAR ExpectedDest;
    WIDE_CHARACTER MajorVersionChar;
    WIDE_CHARACTER MinorVersionChar;
    PCUNICODE_STRING Suffix;
    PPUNICODE_STRING Paths;
    PUNICODE_STRING WhichFilename = NULL;
    PUNICODE_STRING DllPath = NULL;
    PUNICODE_STRING ExePath = NULL;
    PUNICODE_STRING Path = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PythonDllPath)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PythonExePath)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(NumberOfPathEntriesPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathEntriesPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(MajorVersionPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(MinorVersionPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointers immediately.
    //

    *PythonDllPath = NULL;
    *PythonExePath = NULL;
    *MajorVersionPointer = '\0';
    *MinorVersionPointer = '\0';

    //
    // Verify the path.
    //

    if (!IsValidMinimumDirectoryUnicodeString(Directory)) {
        return FALSE;
    }

    Success = Rtl->FilesExistW(
        Rtl,
        Directory,
        NumberOfPythonDllFiles,
        (PPUNICODE_STRING)PythonDllFilesW,
        &Exists,
        &WhichIndex,
        &WhichFilename
    );

    //
    // No Python DLL file found, return.
    //

    if (!Exists) {
        return TRUE;
    }

    //
    // Calculate string sizes.
    //

    Length = (

        //
        // Length (size in bytes) of the directory, excluding trailing NULL.
        //

        Directory->Length +

        //
        // Account for the joining slash.
        //

        sizeof(WCHAR) +

        //
        // And the Python DLL path.
        //

        WhichFilename->Length
    );

    //
    // Account for the trailing NULL for MaximumLength;
    //

    MaximumLength = Length + sizeof(WCHAR);

    //
    // Calculate the allocation size, accounting for the size of
    // the UNICODE_STRING structure.
    //

    AllocSize.LongPart = (sizeof(UNICODE_STRING) + MaximumLength);

    //
    // Sanity check of the size; should never exceed MAX_USHORT.
    //

    if (AllocSize.HighPart != 0) {
        return FALSE;
    }

    //
    // Allocate the buffer.
    //

    DllPath = (PUNICODE_STRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSize.LowPart
        )
    );

    if (!DllPath) {
        return FALSE;
    }

    //
    // UNICODE_STRING+Buffer was successfully allocated.  Point the
    // Buffer at the right place and initialize our Dest pointer.
    //

    DllPath->Buffer = (PWSTR)(
        RtlOffsetToPointer(
            DllPath,
            sizeof(UNICODE_STRING)
        )
    );
    Dest = DllPath->Buffer;

    //
    // Copy the directory.
    //

    Bytes = Directory->Length;
    Count = Bytes >> 1;
    Source = Directory->Buffer;
    __movsw(Dest, Source, Count);

    //
    // Add the joining slash.
    //

    Dest += Count;
    *Dest++ = L'\\';

    //
    // Copy the Python DLL path name.
    //

    Bytes = WhichFilename->Length;
    Count = Bytes >> 1;
    Source = WhichFilename->Buffer;
    __movsw(Dest, Source, Count);

    //
    // Abuse the fact that Dest points to the beginning of the Python DLL
    // buffer at this point.
    //

    MajorVersionChar.WidePart = *(Dest + PYTHON_MAJOR_VERSION_CHAR_OFFSET);
    MinorVersionChar.WidePart = *(Dest + PYTHON_MAJOR_VERSION_CHAR_OFFSET+1);

    //
    // And set the final trailing NULL.
    //

    Dest += Count;
    *Dest++ = L'\0';

    //
    // Update the string lengths.
    //

    DllPath->Length = Length;
    DllPath->MaximumLength = MaximumLength;

    //
    // Now fill out the PythonExePath using the same directory.
    //

    Length = (

        //
        // Length (size in bytes) of the directory, excluding trailing NULL.
        //

        Directory->Length +

        //
        // Account for the joining slash.
        //

        sizeof(WCHAR) +

        //
        // And the Python DLL path.
        //

        PythonExeW.Length
    );

    //
    // Account for the trailing NULL for MaximumLength;
    //

    MaximumLength = Length + sizeof(WCHAR);

    //
    // Calculate the allocation size, accounting for the size of
    // the UNICODE_STRING structure.
    //

    AllocSize.LongPart = (sizeof(UNICODE_STRING) + MaximumLength);

    //
    // Sanity check of the size; should never exceed MAX_USHORT.
    //

    if (AllocSize.HighPart != 0) {
        goto Error;
    }

    //
    // Allocate the buffer.
    //

    ExePath = (PUNICODE_STRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSize.LowPart
        )
    );

    if (!ExePath) {
        goto Error;
    }

    //
    // UNICODE_STRING+Buffer was successfully allocated.  Point the
    // Buffer at the right place and initialize our Dest pointer.
    //

    ExePath->Buffer = (PWSTR)(
        RtlOffsetToPointer(
            ExePath,
            sizeof(UNICODE_STRING)
        )
    );
    Dest = ExePath->Buffer;

    //
    // Copy the directory.
    //

    Bytes = Directory->Length;
    Count = Bytes >> 1;
    Source = Directory->Buffer;
    __movsw(Dest, Source, Count);

    //
    // Add the joining slash.
    //

    Dest += Count;
    *Dest++ = L'\\';

    //
    // Copy the Python exe path name.
    //

    Bytes = PythonExeW.Length;
    Count = Bytes >> 1;
    Source = PythonExeW.Buffer;
    __movsw(Dest, Source, Count);


    //
    // And set the final trailing NULL.
    //

    Dest += Count;
    *Dest++ = L'\0';

    //
    // Update the string lengths.
    //

    ExePath->Length = Length;
    ExePath->MaximumLength = MaximumLength;

    //
    // Construct the individual path entries and the containing array.
    //

    //
    // Add 1 to the suffix count to account for the initial directory.
    //

    NumberOfPathSuffixes = NUMBER_OF_PYTHON_PATH_SUFFIXES;
    NumberOfPathEntries = NumberOfPathSuffixes + 1;

    PathsArraySize = sizeof(PUNICODE_STRING) * NumberOfPathEntries;

    PathsUnicodeStringStructsSize = (
        sizeof(UNICODE_STRING) *
        NumberOfPathEntries
    );

    AllocSize.LongPart = (

        //
        // Account for the array.
        //

        PathsArraySize +

        //
        // Account for the underlying UNICODE_STRING structures.
        //

        PathsUnicodeStringStructsSize +

        //
        // Account for the first directory's buffer length, plus trailing NULL.
        //

        Directory->Length + sizeof(WCHAR)

    );

    //
    // Add the remaining lengths in (including the directory length).
    //

    for (Index = 0; Index < NumberOfPathSuffixes; Index++) {

        //
        // Load the suffix.
        //

        Suffix = PythonPathSuffixesW[Index];

        AllocSize.LongPart += (

            //
            // Account for the directory length.
            //

            Directory->Length +

            //
            // No need to account for the joining slash as the suffixes already
            // have it in place.
            //

            Suffix->Length +

            //
            // Account for the trailing NULL.
            //

            sizeof(WCHAR)

        );
    }

    //
    // Allocate space to the array first, then carve out the remaining
    // structures.
    //

    Paths = (PPUNICODE_STRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSize.LongPart
        )
    );

    if (!Paths) {
        goto Error;
    }

    //
    // Point the array pointers at the trailing UNICODE_STRING structures and
    // initialize the Buffer pointers and lengths.
    //

    Paths[0] = Path = (PUNICODE_STRING)(
        RtlOffsetToPointer(
            Paths,
            PathsArraySize
        )
    );

    Path->Length = Directory->Length;
    Path->MaximumLength = Directory->Length + sizeof(WCHAR);

    Path->Buffer = (PWCHAR)(
        RtlOffsetToPointer(
            Paths,
            PathsArraySize + PathsUnicodeStringStructsSize
        )
    );

    //
    // Copy the directory into the first path.
    //

    Count = Path->Length >> 1;
    Dest = Path->Buffer;
    __movsw(Dest, Directory->Buffer, Path->Length >> 1);

    Dest += Count;

    //
    // Add the trailing NULL and advance the destination pointer to the next
    // buffer starting point.
    //

    *Dest++ = L'\0';

    //
    // Fill in the hash/atom value.
    //

    Path->Hash = HashUnicodeStringToAtom(Path);

    //
    // Fill out the suffixes.
    //

    for (Index = 0; Index < NumberOfPathSuffixes; Index++) {

        //
        // Load the suffix.
        //

        Suffix = PythonPathSuffixesW[Index];

        //
        // Carve out the next UNICODE_STRING structure.
        //

        Paths[Index+1] = Path = (PUNICODE_STRING)(
            RtlOffsetToPointer(
                Paths[Index],
                sizeof(UNICODE_STRING)
            )
        );

        Path->Length = (

            //
            // Account for the directory length.
            //

            Directory->Length +

            //
            // Add the length of the suffix.
            //

            Suffix->Length
        );

        //
        // Add the trailing NULL.
        //

        Path->MaximumLength = Path->Length + sizeof(WCHAR);

        //
        // Carve out the buffer, using the previous Dest pointer.
        //

        Path->Buffer = Dest;

        //
        // Copy the directory over.
        //

        Count = Directory->Length >> 1;
        __movsw(Dest, Directory->Buffer, Count);

        //
        // Update the Dest pointer and copy the suffix.
        //

        Dest += Count;
        Count = Suffix->Length >> 1;
        __movsw(Dest, Suffix->Buffer, Count);

        //
        // Add the trailing NULL and increment the Dest pointer so that it
        // points at the next buffer space.
        //

        Dest += Count;
        *Dest++ = L'\0';

        //
        // Set the hash value.
        //

        Path->Hash = HashUnicodeStringToAtom(Path);

    }

    //
    // Sanity check our buffer carving logic is correct.
    //

    ExpectedDest = ((PWCHAR)(RtlOffsetToPointer(Paths, AllocSize.LongPart)));

    if (ExpectedDest != Dest) {
        __debugbreak();
    }

    //
    // Update the hashes DllPath and ExePath.
    //

    DllPath->Hash = HashUnicodeStringToAtom(DllPath);
    ExePath->Hash = HashUnicodeStringToAtom(ExePath);


    //
    // Update the caller's pointers and return success.
    //

    *PythonDllPath = DllPath;
    *PythonExePath = ExePath;
    *NumberOfPathEntriesPointer = NumberOfPathEntries;
    *PathEntriesPointer = *Paths;
    *MajorVersionPointer = MajorVersionChar.LowPart;
    *MinorVersionPointer = MinorVersionChar.LowPart;

    return TRUE;

Error:

    if (DllPath) {
        Allocator->Free(Allocator->Context, DllPath);
        DllPath = NULL;
    }

    if (ExePath) {
        Allocator->Free(Allocator->Context, ExePath);
        ExePath = NULL;
    }

    if (Paths) {
        Allocator->Free(Allocator->Context, Paths);
        Paths = NULL;
    }

    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
