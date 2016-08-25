#include "stdafx.h"

_Use_decl_annotations_
BOOL
FindPythonDllAndExe(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PUNICODE_STRING Directory,
    PPUNICODE_STRING PythonDllPath,
    PPUNICODE_STRING PythonExePath
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
    USHORT Bytes;
    USHORT Count;
    USHORT WhichIndex;
    USHORT Length;
    USHORT MaximumLength;
    LONG_INTEGER AllocSize;
    PWCHAR Dest;
    PWCHAR Source;
    PUNICODE_STRING WhichFilename = NULL;
    PUNICODE_STRING DllPath = NULL;
    PUNICODE_STRING ExePath = NULL;

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

    //
    // Clear the caller's pointers immediately.
    //

    *PythonDllPath = NULL;
    *PythonExePath = NULL;

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
    // Update the caller's pointers and return success.
    //

    *PythonDllPath = DllPath;
    *PythonExePath = ExePath;

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

    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
