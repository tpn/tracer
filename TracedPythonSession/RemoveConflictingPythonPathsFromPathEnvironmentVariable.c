#include "stdafx.h"

#include "TracedPythonSessionPrivate.h"

static CONST WSTR PATH = L"Path";

typedef struct _PATH_ENV_VAR {
    USHORT Size;
    USHORT NumberOfElements;
    RTL_BITMAP Bitmap;
    UNICODE_STRING Paths;
    PULONG Hashes;
} PATH_ENV_VAR, *PPATH_ENV_VAR;

typedef struct _PYTHON_HOME {

    //
    // Size of the entire structure, in bytes.
    //

    USHORT Size;

    //
    // Python major version ('2' or '3') and minor version, as derived from the
    // DLL path name.
    //

    CHAR PythonMajorVersion;
    CHAR PythonMinorVersion;

    //
    // Number of paths pointed to by the PathEntries array below.
    //

    USHORT NumberOfPathEntries;

    //
    // Padding out to 8 bytes.
    //

    USHORT   Unused1;

    //
    // Array of PUNICODE_STRING paths to add to the DLL path.  Size is governed
    // by NumberOfPathEntries.
    //

    PUNICODE_STRING PathEntries;

    USHORT NumberOfPathEntries;

    UNICODE_STRING Directory;

    PUNICODE_STRING DllPath;
    PUNICODE_STRING ExePath;

} PYTHON_HOME, *PPYTHON_HOME;

PPATH_ENV_VAR
LoadPathEnvironmentVariable(
    PALLOCATOR Allocator
    )
{
    WCHAR Char;
    USHORT Index;
    USHORT Count;
    USHORT BitmapBufferSizeInBytes;
    USHORT AlignedBitmapBufferSizeInBytes;
    USHORT AlignedNumberOfCharacters;
    USHORT UnicodeBufferSizeInBytes;
    USHORT BitmapBufferSizeInBytes;
    LONG Length;
    LONG_INTEGER NumberOfChars;
    LONG_INTEGER AllocSize;
    LONG_INTEGER AlignedAllocSize;
    PPATH_ENV_VAR Path;
    PUNICODE_STRING String;
    PRTL_BITMAP Bitmap;

    Length = GetEnvironmentVariableW(PATH, NULL, 0);
    if (Length == 0) {
        return NULL;
    }

    NumberOfChars.LongPart = Length;

    if (NumberOfChars.HighPart) {
        return NULL;
    }

    AlignedNumberOfCharacters = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            NumberOfChars.LowPart
        )
    );

    UnicodeBufferSizeInBytes = AlignedNumberOfCharacters << 1;

    BitmapBufferSizeInBytes = AlignedNumberOfCharacters >> 3;

    AlignedBitmapBufferSizeInBytes = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            BitmapBufferSizeInBytes
        )
    );

    AllocSize.LongPart = (

        sizeof(PATH_ENV_VAR) +

        BitmapBufferSizeInBytes +

        UnicodeBufferSizeInBytes

    );

    AlignedAllocSize.LongPart = ALIGN_UP_POINTER(AllocSize.LongPart);

    if (AlignedAllocSize.HighPart) {
        return NULL;
    }

    Path = (PUNICODE_STRING)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedAllocSize.LongPart
        )
    );

    if (!Path) {
        return NULL;
    }

    Bitmap->SizeOfBitMap = AlignedNumberOfCharacters;
    Bitmap->Buffer = (PULONG)RtlOffsetToPointer(Path, sizeof(PATH_ENV_VAR));

    String = &Path->String;

    String->Length = (NumberOfChars.LowPart - 1) << 1;
    String->MaximumLength = String->Length + sizeof(WCHAR);

    String->Buffer = (PWCHAR)(
        RtlOffsetToPointer(
            Path,
            sizeof(PATH_ENV_VAR) + AlignedBitmapBufferSizeInBytes
        )
    );

    Length = GetEnvironmentVariableW(
        PATH,
        String->Buffer,
        String->MaximumLength
    );

    for (Index = 0, Count = 0; Index < NumberOfChars.LowPart; Index++) {
        Char = String->Buffer[Index];
        if (Char == L';') {
            Count++;
            FastSetBit(Bitmap, Index+1);
            String->Buffer[Index] = L'\0';
        }
    }

    Path->Size = AlignedAllocSize.LowPart;
    Path->NumberOfElements = Count;

    Path->Hashes = (PULONG)(
        Allocator->Calloc(
            Allocator->Context,
            sizeof(ULONG),
            Count
        )
    );

    if (!Path->Hashes) {
        Allocator->Free(Allocator->Context, Path);
        return NULL;
    }

    return Path;
}

_Use_decl_annotations_
BOOL
RemoveConflictingPythonPathsFromPathEnvironmentVariable(
    PTRACED_PYTHON_SESSION Session
    )
/*++

Routine Description:

    This function checks every path in the current PATH environment variable
    and removes any paths that are one of the Python library paths.  This
    covers any directory that is found to be a Python home directory (has a
    python[nn].dll file present), plus \Scripts and \Library\bin directories.

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
    ULONG PreviousIndex;
    ULONG Index;
    ULONG BitmapIndex;
    ULONG NextIndex;
    ULONG Hint;
    PRTL Rtl;
    PALLOCATOR Allocator;
    PRTL_FIND_SET_BITS RtlFindSetBits;
    PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;
    PRTL_EQUAL_UNICODE_STRING RtlEqualUnicodeString;
    PUNICODE_STRING PythonHome;
    UNICODE_STRING Directory;
    PUNICODE_STRING String;
    PPATH_ENV_VAR Path;
    PRTL_BITMAP Bitmap;
    PYTHON_HOME Home;

    Rtl = Session->Rtl;
    RtlFindSetBits = Rtl->RtlFindSetBits;
    RtlNumberOfSetBits = Rtl->RtlNumberOfSetBits;
    RtlEqualUnicodeString = Rtl->RtlEqualUnicodeString;

    Allocator = Session->Allocator;

    Path = LoadPathEnvironmentVariable(Allocator);
    if (!Path) {
        return FALSE;
    }

    SecureZeroMemory(&Home, sizeof(Home));

    Bitmap = &Path->Bitmap;
    String = &Path->String;

    PreviousIndex = 0;
    BitmapIndex = 0;

    for (Index = 0; Index < Path->NumberOfElements; Index++) {

        BitmapIndex = RtlFindSetBits(Bitmap, 1, Index);
        if (BitmapIndex == BITS_NOT_FOUND) {
            goto Error;
        }

        Directory.Length = (BitmapIndex - PreviousIndex);
        Directory.MaximumLength = Directory->Length;
        Directory.Buffer = String->Buffer + PreviousIndex;

        Success = Session->FindPythonDllAndExe(
            Rtl,
            Allocator,
            &Directory,
            &Home.PythonDllPath,
            &Home.PythonExePath,
            &Home.NumberOfPathEntries,
            &Home.PathEntries,
            &Home.PythonMajorVersion,
            &Home.PythonMinorVersion
        );

        if (!Success) {
            goto Error;
        }

        if (!Home.PythonDllPath) {

            //
            // No Python directory found here, go to next.
            //

        }

        //
        // Found Python directory.
        //

    }

    Success = TRUE;

    //
    // Intentional follow-on.
    //


Error:

    if (Path) {

        if (Path->Hashes) {
            Allocator->FreePointer(Allocator->Context, Path->Hashes);
        }

        Allocator->FreePointer(Allocator->Context, Path);
    }


End:

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
