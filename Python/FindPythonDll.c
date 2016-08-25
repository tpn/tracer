#include "stdafx.h"

_Use_decl_annotations_
BOOL
FindPythonDll(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PUNICODE_STRING Directory,
    PPUNICODE_STRING PythonDllPath
    )
/*++

Routine Description:

    Finds the first Python DLL file in the given directory.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure that will be used
        to allocate a new fully-qualified path from the base Directory and the
        Python DLL filename, if one is found.

    Directory - Supplies a pointer to a fully-qualified UNICODE_STRING
        structure that represents the directory of interest.

    PythonDllPath - Supplies a pointer that receives the address of a
        UNICODE_STRING representing the found Python DLL, if any.

Return Value:

    TRUE if no error occured, FALSE otherwise.  Note that TRUE does not
    imply that a file was found, simply that no error occurred.  You should
    test PythonDllPath for a NULL pointer to discern whether or not a DLL
    file was successfully found.

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
    PUNICODE_STRING Path;

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

    //
    // Clear the pointer immediately.
    //

    *PythonDllPath = NULL;

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

    __try {
        Path = (PUNICODE_STRING)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                AllocSize.LowPart
            )
        );
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Path = NULL;
    }

    if (!Path) {
        return FALSE;
    }

    //
    // UNICODE_STRING+Buffer was successfully allocated.  Point the
    // Buffer at the right place and initialize our Dest pointer.
    //

    Path->Buffer = (PWSTR)(
        RtlOffsetToPointer(
            Path,
            sizeof(UNICODE_STRING)
        )
    );
    Dest = Path->Buffer;

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

    Path->Length = Length;
    Path->MaximumLength = MaximumLength;

    //
    // Update the caller's pointer.
    //

    *PythonDllPath = Path;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
