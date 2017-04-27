/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonPathTableEntry.c

Abstract:

    This module implements functionality related to the PYTHON_PATH_TABLE_ENTRY
    structure, which extends the PREFIX_TABLE_ENTRY structure defined by NT.

    The main routine is GetPathEntryFromFrame, which is called by RegisterFrame
    when it encounters a code object it hasn't seen before.  This routine will
    then call, as necessary, RegisterFile, GetPathEntryForDirectory,
    RegisterDirectory, RegisterModuleDirectory and RegisterNonModuleDirectory,
    in order to build up the intermediate prefix table trees.  These routines
    are also defined in this module.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
GetPathEntryFromFrame(
    PPYTHON Python,
    PPYFRAMEOBJECT FrameObject,
    PYTHON_EVENT_TRAITS EventTraits,
    PPYOBJECT ArgObject,
    PSTRING FilenameString,
    PFILENAME_FLAGS FilenameFlags,
    PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer,
    PBOOL NewPathEntry
    )
/*++

Routine Description:

    This routine returns a PYTHON_PATH_TABLE_ENTRY structure for a given Python
    frame object's code object's filename.  It will indirectly create path table
    entries for the parent directory and all module ancestor directories, up to
    and including the first non-module (root) directory found.  (Note that due
    to the use of prefix trees, some of the paths may have already been seen,
    in which case, only the remaining directory components will be created.)

Arguments:

    Python - Supplies a pointer to a PYTHON structure that contains the runtime
        tables that will be used to build up the path table entry.

    FrameObject - Supplies a pointer to a PYFRAMEOBJECT structure, provided by
        the C Python tracing machinery.

    EventTraits - Supplies a PYTHON_EVENT_TRAITS value describing the event
        being traced.

    ArgObject - Supplies a pointer to a PYOBJECT structure that was provided as
        a parameter to the trace function.  Currently unused.

    FilenameString - Supplies a pointer to a PSTRING structure that has been
        initialized with the filename for this frame.

    FilenameFlags - Supplies a pointer to a FILENAME_FLAGS structure that
        captures attributes about the FilenameString parameter, such as
        whether or not the filename is a special name (starts with '<'),
        whether it's already been qualified, etc.

    PathEntryPointer - Supplies a pointer to a variable that will receive the
        address of a PYTHON_PATH_TABLE_ENTRY structure for the frame object's
        code object's filename.

    NewPathEntry - Supplies a pointer to a BOOL variable that will receive a
        TRUE value if a new path entry was created as part of processing this
        request, FALSE if the underlying file was already seen.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    PPYFRAMEOBJECT Frame = (PPYFRAMEOBJECT)FrameObject;
    PSTRING Match;
    PSTRING Path = FilenameString;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;
    PPREFIX_TABLE PrefixTable;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    BOOL TriedQualified = FALSE;
    BOOL DontTryQualifyPath = FALSE;
    BOOL Success;

    //
    // Clear the caller's NewPathEntry variable up-front.
    //

    *NewPathEntry = FALSE;

    PrefixTable = &Python->PathTable->PrefixTable;
    Rtl = Python->Rtl;

    if (FilenameFlags->IsFullyQualified || FilenameFlags->IsSpecialName) {
        DontTryQualifyPath = TRUE;
    }

Retry:
    PrefixTableEntry = Rtl->PfxFindPrefix(PrefixTable, Path);
    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)PrefixTableEntry;

    if (PathEntry) {

        //
        // A match was found, see if it matches our entire path.
        //

        Match = PathEntry->Prefix;

        if (Match->Length == Path->Length) {

            //
            // An exact match was found, we've seen this filename before.
            //

            Success = TRUE;
            goto End;

        } else if (Match->Length > Path->Length) {

            //
            // We should never get a match that is longer than the path we
            // searched for.
            //

            __debugbreak();

        }

        //
        // A shorter entry was found.  Fall through to the following code which
        // will handle inserting a new prefix table entry for the file.
        //

    } else if (!TriedQualified && !DontTryQualifyPath) {

        //
        // See if we need to qualify the path and potentially do another prefix
        // tree search.  We don't do this up front because non-qualified paths
        // are pretty infrequent and the initial prefix tree lookup is on the
        // hot path.
        //

        if (!IsQualifiedPath(Path)) {

            Success = QualifyPath(Python, Path, &Path);

            if (!Success) {
                goto End;
            }

            TriedQualified = TRUE;
            FilenameFlags->IsFullyQualified = TRUE;
            FilenameFlags->WeOwnPathBuffer = TRUE;

            goto Retry;
        }
    }

    //
    // Sanity check we've got a trailing NUL where we expect, and a non-NUL
    // character before that.
    //

    if (Path->Buffer[Path->Length] != '\0' ||
        Path->Buffer[Path->Length-1] == '\0') {
        __debugbreak();
    }

    Success = RegisterFile(Python,
                           FrameObject,
                           Path,
                           FilenameFlags,
                           &PathEntry,
                           NewPathEntry);

    //
    // Intentional follow-on to End (i.e. we let the success indicator from
    // RegisterFile() bubble back to the caller).
    //

End:

    //
    // Update the caller's PathEntryPointer (even if PathEntry is null).
    //

    if (ARGUMENT_PRESENT(PathEntryPointer)) {
        *PathEntryPointer = PathEntry;
    }

    return Success;
}

_Use_decl_annotations_
BOOL
RegisterFile(
    PPYTHON Python,
    PPYFRAMEOBJECT FrameObject,
    PSTRING FilenameString,
    PFILENAME_FLAGS FilenameFlags,
    PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer,
    PBOOL NewPathEntry
    )
/*++

Routine Description:

    This routine registers a new file for a frame object's code object's file
    name, returning a corresponding PYTHON_PATH_TABLE_ENTRY.

Arguments:

    Python - Supplies a pointer to a PYTHON structure that contains the runtime
        tables that will be used to build up the path table entry.

    FrameObject - Supplies a pointer to a PYFRAMEOBJECT structure, provided by
        the C Python tracing machinery.

    FilenameString - Supplies a pointer to a PSTRING structure that has been
        initialized with the filename for this frame.

    FilenameFlags - Supplies a pointer to a FILENAME_FLAGS structure that
        captures attributes about the FilenameString parameter, such as
        whether or not the filename is a special name (starts with '<'),
        whether it's already been qualified, etc.

    PathEntryPointer - Supplies a pointer to a variable that will receive the
        address of a newly-allocated PYTHON_PATH_TABLE_ENTRY structure.

    NewPathEntry - Supplies a pointer to a BOOL variable that will receive a
        TRUE value if a new path entry was created as part of processing this
        request, FALSE if the underlying file was already seen.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsInitPy;
    BOOL CaseSensitive = TRUE;
    BOOL Reversed = TRUE;
    BOOL IsSpecialName = FilenameFlags->IsSpecialName;
    BOOL WeOwnPathBuffer = FilenameFlags->WeOwnPathBuffer;
    BOOL ForceModuleDirectory = FALSE;
    BOOL IsFileSystemDirectory = TRUE;

    CHAR Char;

    USHORT Index;
    USHORT Limit = 0;
    USHORT Offset = 0;
    USHORT ReversedIndex;
    USHORT ExtensionLength;
    USHORT BitmapHintIndex;
    USHORT NumberOfChars;
    USHORT NumberOfBackslashes;
    USHORT NumberOfBackslashesRemaining;
    USHORT ModuleLength;

    ULONG FileAttributes;

    HANDLE HeapHandle = NULL;

    PRTL Rtl;
    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;
    PPYTHON_PATH_TABLE_ENTRY PathEntry;
    PPYTHON_PATH_TABLE_ENTRY DirectoryEntry;
    PSTR ModuleBuffer;
    PSTRING Path = FilenameString;
    PSTRING Name;
    PSTRING FullName;
    PSTRING ModuleName;
    PSTRING DirectoryName;
    PSTRING DirectoryModuleName;
    PRTL_BITMAP BitmapPointer;

    STRING Filename;
    STRING Directory;
    STRING TempPath;

    PSTR Dest;
    PSTR File;
    PSTR Start;
    USHORT PathAllocSize;
    USHORT FullNameLength;
    USHORT FullNameAllocSize;
#ifdef _DEBUG
    USHORT ExpectedMaximumLength;
#endif

    //
    // Reserve a 32 byte (256 >> 3), 256 bit stack-allocated bitmap buffer.
    //

    CHAR StackBitmapBuffer[256 >> 3];
    RTL_BITMAP Bitmap = { 256, (PULONG)&StackBitmapBuffer };
    BitmapPointer = &Bitmap;

    //
    // Initialize Rtl variable.
    //

    Rtl = Python->Rtl;

    //
    // If the filename was special (started with a "<" character), we fill
    // out a very simple path table entry structure and return.
    //

    if (IsSpecialName) {

        //
        // Invariant check: we shouldn't own the path buffer here as special
        // names don't go through the qualified path check.
        //

        if (WeOwnPathBuffer) {
            __debugbreak();
            return FALSE;
        }

        //
        // Allocate a new path table entry structure.
        //

        Success = AllocatePythonPathTableEntry(Python, &PathEntry);
        if (!Success) {
            goto End;
        }

        //
        // Set the path entry to special.
        //

        PathEntry->IsSpecial = TRUE;

        //
        // Initialize alias.
        //

        FullName = &PathEntry->FullName;

        //
        // Initialize the length and allocation size.  The latter includes a +1
        // to account for the trailing NUL.
        //

        FullNameLength = Path->Length;
        FullNameAllocSize = (
            ALIGN_UP_USHORT_TO_POINTER_SIZE(FullNameLength + 1)
        );

        //
        // Allocate a new string buffer.
        //

        if (!AllocateStringBuffer(Python, FullNameAllocSize, FullName)) {
            FreePythonPathTableEntry(Python, PathEntry);
            Success = FALSE;
            goto End;
        }

        //
        // Set the length.  Excludes the trailing NUL.
        //

        FullName->Length = FullNameLength;

        if (FullName->MaximumLength <= FullName->Length) {
            __debugbreak();
        }

        //
        // Copy the string over.
        //

        __movsb((PBYTE)FullName->Buffer,
                (PBYTE)Path->Buffer,
                FullName->Length);

        //
        // Sanity check: we should be NUL terminated at this point.
        //

        if (FullName->Buffer[FullName->Length] != '\0') {
            __debugbreak();
        }

        //
        // Initialize the PathEntry's Path pointer with the full name, so that
        // it can be added to the prefix tree.
        //

        Path = &PathEntry->Path;

        Path->Length = FullName->Length;
        Path->MaximumLength = FullName->MaximumLength;
        Path->Buffer = FullName->Buffer;

        goto AddPrefix;
    }

    //
    // Initialize local variables.
    //

    HeapHandle = Python->HeapHandle;

    NumberOfChars = Path->Length;

    //
    // Create a reversed bitmap for the backslashes in the path.
    //

    Success = Rtl->CreateBitmapIndexForString(Rtl,
                                              Path,
                                              '\\',
                                              &HeapHandle,
                                              &BitmapPointer,
                                              Reversed,
                                              NULL);

    if (!Success) {
        return FALSE;
    }

    //
    // Make sure there's at least one backslash in the path.
    //

    NumberOfBackslashes = (USHORT)Rtl->RtlNumberOfSetBits(BitmapPointer);
    if (NumberOfBackslashes == 0) {
        goto End;
    }

    //
    // Extract the filename from the path by finding the first backslash
    // and calculating the offset into the string buffer.
    //

    ReversedIndex = (USHORT)Rtl->RtlFindSetBits(BitmapPointer, 1, 0);
    Offset = NumberOfChars - ReversedIndex + 1;

    if (Path->Buffer[Offset - 1] != '\\') {
        __debugbreak();
    }

    Filename.Buffer = &Path->Buffer[Offset];

    if (Filename.Buffer[0] == '\\') {
        __debugbreak();
    }

    Filename.Length = ReversedIndex;
    if (Filename.Buffer[Filename.Length - 1] == '\0') {
        Filename.Length--;
    }

    Filename.MaximumLength = Filename.Length;

    if (Filename.Buffer[Filename.Length - 1] == '\0' ||
        Filename.Buffer[Filename.Length] != '\0') {
        __debugbreak();
    }

    //
    // If we don't own the incoming path buffer, we need to allocate space for
    // it and copy it now.  This is because we potentially add the Directory to
    // the prefix tree below, and we can't do that if the underlying path buffer
    // is stack-allocated or heap-allocated.
    //
    // N.B. "Own" in this context means that we've allocated the buffer from
    //      a trace store buffer.
    //

    if (WeOwnPathBuffer) {

        //
        // Clear the path allocation length.
        //

        PathAllocSize = 0;

        //
        // Capture the incoming filename's details into our temporary path.
        //

        TempPath.Length = FilenameString->Length;
        TempPath.MaximumLength = FilenameString->MaximumLength;
        TempPath.Buffer = FilenameString->Buffer;

    } else {


        //
        // Account for path length plus trailing NULL.
        //

        PathAllocSize = ALIGN_UP_USHORT_TO_POINTER_SIZE(Path->Length + 1);

        //
        // Allocate space for the underlying string buffer.
        //

        Success = AllocateStringBuffer(Python, PathAllocSize, &TempPath);

        //
        // Copy the underlying string.
        //

        __movsb((PBYTE)TempPath.Buffer,
                (PBYTE)FilenameString->Buffer,
                FilenameString->Length);

        //
        // Update the length.
        //

        TempPath.Length = FilenameString->Length;

        //
        // Ensure the buffer is NUL terminated.
        //

        if (TempPath.Buffer[TempPath.Length] != '\0') {
            __debugbreak();
        }
    }

    //
    // Extract the directory name.
    //

    Directory.Length = Offset - 1;
    Directory.MaximumLength = Directory.Length;
    Directory.Buffer = TempPath.Buffer;

    if (Directory.Buffer[Directory.Length] != '\\') {
        __debugbreak();
    }

    //
    // Prepare variables prior to a possible GetPathEntryForDirectory() call.
    //

    DirectoryEntry = NULL;
    BitmapHintIndex = ReversedIndex;
    NumberOfBackslashesRemaining = NumberOfBackslashes - 1;

    //
    // Check to see if our filename is __init__.py.  If it is, check to see if
    // directory is actually a file system directory.  If it's not, we need to
    // force the module directory.
    //

    IsInitPy = Rtl->RtlEqualString(&A__init__py, &Filename, FALSE);

    if (IsInitPy) {

        //
        // Temporarily make the directory buffer NUL-terminated in order to pass
        // it to GetFileAttributesA(), which expects a NUL-terminated string.
        //

        Directory.Buffer[Directory.Length] = '\0';

        //
        // Get the attributes for the directory.
        //

        FileAttributes = GetFileAttributesA(Directory.Buffer);

        //
        // Restore the trailing slash.
        //

        Directory.Buffer[Directory.Length] = '\\';

        IsFileSystemDirectory = (
            FileAttributes != INVALID_FILE_ATTRIBUTES &&
            FileAttributes & FILE_ATTRIBUTE_DIRECTORY
        );

        if (!IsFileSystemDirectory) {

            //
            // The directory isn't a file system directory (as we might be in
            // a .zip or .egg file), so toggle the force module directory flag.

            ForceModuleDirectory = TRUE;
        }
    }

    //
    // Get the PathEntry for the directory.  This will build up the path table
    // prefix tree with any intermediate module directories until we hit the
    // first "non-module" directory (i.e. the first directory that doesn't have
    // an __init__.py file).
    //

    Success = GetPathEntryForDirectory(Python,
                                       &Directory,
                                       BitmapPointer,
                                       &BitmapHintIndex,
                                       &NumberOfBackslashesRemaining,
                                       &DirectoryEntry,
                                       ForceModuleDirectory);

    if (!Success) {
        goto End;
    }

    if (!DirectoryEntry) {

        //
        // We weren't able to create a parent path entry for the directory.
        // Not sure when this would happen at the moment, so __debugbreak().
        //

        __debugbreak();
        return FALSE;
    }

    //
    // Now that we have a DirectoryEntry for our parent directory, we can
    // continue creating a PathEntry for this filename and filling in the
    // relevant details.
    //

    DirectoryEntry->IsFileSystemDirectory = IsFileSystemDirectory;

    //
    // Initialize convenience pointers.
    //

    DirectoryName = &DirectoryEntry->Name;
    DirectoryModuleName = &DirectoryEntry->ModuleName;
    ModuleBuffer = DirectoryModuleName->Buffer;
    ModuleLength = DirectoryModuleName->Length;

    //
    // Invariant check: if the directory's module name length is 0, verify
    // the corresponding path entry indicates it is a non-module directory.
    //

    if (ModuleLength == 0) {
        if (!DirectoryEntry->IsNonModuleDirectory) {
            __debugbreak();
            return FALSE;
        }
    }

    //
    // Determine the length of the file part (filename sans extension).
    //
    // N.B. We can re-use the Filename STRING here to truncate the length such
    //      that the extension is omitted (i.e. there's no need to allocate a
    //      new string as we're going to be copying into a new string buffer
    //      shortly).
    //

    ExtensionLength = 3;
    if (Filename.Buffer[Filename.Length - ExtensionLength] != '.') {
        if (Filename.Buffer[Filename.Length - ++ExtensionLength] != '.') {
            ExtensionLength = 0;
            for (Index = Filename.Length - 1; Index > 0; Index--) {
                Char = Filename.Buffer[Index];
                if (Char == '.') {
                    ExtensionLength = Filename.Length - Index - 1;
                    Filename.Length = Index;
                    break;
                }
            }
        } else {
            goto FastPathUpdateLength;
        }
    } else {

FastPathUpdateLength:

        Filename.Length = Filename.Length - --ExtensionLength - 1;
    }

    //
    // Calculate the length.  The first sizeof(CHAR) accounts for the joining
    // backslash if there's a module name, the second sizeof(CHAR) accounts
    // for the terminating NUL.
    //

    FullNameLength = (
        (ModuleLength ? ModuleLength + sizeof(CHAR) : 0)    +
        Filename.Length                                     +
        sizeof(CHAR)
    );

    FullNameAllocSize = FullNameLength;

    Success = AllocatePythonPathTableEntry(Python, &PathEntry);
    if (!Success) {
        goto End;
    }

    PathEntry->IsFile = TRUE;
    PathEntry->IsInitPy = IsInitPy;

    if (FilenameFlags->IsDll) {
        PathEntry->IsDll = TRUE;
    }

    FullName = &PathEntry->FullName;

    //
    // Allocate the full name string.
    //

    if (!AllocateStringBuffer(Python, FullNameAllocSize, FullName)) {
        FreePythonPathTableEntry(Python, PathEntry);
        goto End;
    }

    //
    // Initialize the Path entry.
    //

    Path = &PathEntry->Path;

    Path->Length = TempPath.Length;
    Path->MaximumLength = TempPath.MaximumLength;
    Path->Buffer = TempPath.Buffer;

#ifdef _DEBUG
    ExpectedMaximumLength = (
        PathAllocSize ? PathAllocSize :
                        FilenameString->MaximumLength
    );

    if (Path->MaximumLength < ExpectedMaximumLength) {
        __debugbreak();
    }
#endif

    //
    // Initialize shortcut pointers.
    //

    Name = &PathEntry->Name;
    ModuleName = &PathEntry->ModuleName;

    //
    // Update the lengths of our strings.
    //

    FullName->Length = FullNameLength - 1; // exclude trailing NUL

    if (FullName->MaximumLength <= FullName->Length) {
        __debugbreak();
    }

    Name->Length = Filename.Length;
    Name->MaximumLength = Filename.MaximumLength;

    ModuleName->Length = ModuleLength;
    ModuleName->MaximumLength = ModuleLength;
    ModuleName->Buffer = ModuleBuffer;

    //
    // Construct the final full name.  After each part has been copied, update
    // the corresponding Buffer pointer to the relevant point within the newly-
    // allocated buffer for the full name.
    //

    Dest = FullName->Buffer;

    //
    // Copy module name if applicable.
    //

    if (ModuleName->Length) {
        __movsb(Dest, (PBYTE)ModuleName->Buffer, ModuleName->Length);
        Dest += ModuleName->Length;
        ModuleName->Buffer = FullName->Buffer;

        //
        // Add joining slash.
        //

        *Dest++ = '\\';
    }

    //
    // Copy the file name.
    //

    Start = Dest;
    File = Filename.Buffer;
    __movsb(Dest, File, Filename.Length);
    Name->Buffer = Start;
    Dest += Filename.Length;

    //
    // Add the trailing NUL.
    //

    *Dest++ = '\0';

    //
    // Copy up to the first three bytes of the filename extension if one was
    // present.
    //

    if (ExtensionLength) {
        PCHAR Char = &File[Filename.Length+1];
        Dest = (PCHAR)&Path->Hash;
        for (Index = 0; Index < ExtensionLength && Index < 3; Index++) {
            *Dest++ = *Char++;
        }

        //
        // Add a trailing NUL.
        //

        ((PCHAR)&Path->Hash)[Index] = '\0';
    }

AddPrefix:

    //
    // Add the newly created PathEntry to our path table prefix tree.
    //

    PrefixTable = &Python->PathTable->PrefixTable;
    PrefixTableEntry = (PPREFIX_TABLE_ENTRY)PathEntry;

    Success = Rtl->PfxInsertPrefix(PrefixTable,
                                   Path,
                                   PrefixTableEntry);

    if (!Success) {

        //
        // This should never get hit; it indicates the path was already in the
        // prefix tree.
        //

        __debugbreak();
        goto End;
    }

    //
    // Indicate we created a new path entry.
    //

    *NewPathEntry = TRUE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Clear the PathEntry pointer if we've encountered an error.  Set the
    // valid flag otherwise.
    //

    if (!Success) {
        PathEntry = NULL;
    } else {
        PathEntry->IsValid = TRUE;
    }

    //
    // Update the caller's path entry pointer.
    //

    *PathEntryPointer = PathEntry;

    MAYBE_FREE_BITMAP_BUFFER(BitmapPointer, StackBitmapBuffer);

    return Success;
}

_Use_decl_annotations_
BOOL
GetPathEntryForDirectory(
    PPYTHON Python,
    PSTRING Directory,
    PRTL_BITMAP Backslashes,
    PUSHORT BitmapHintIndex,
    PUSHORT NumberOfBackslashesRemaining,
    PPPYTHON_PATH_TABLE_ENTRY PathEntryPointer,
    BOOL ForceModuleDirectory
    )
/*++

Routine Description:

    This routine builds up a PYTHON_PATH_TABLE_ENTRY for a given directory
    name, including any intermediate paths.

Arguments:

    Python - Supplies a pointer to a PYTHON structure that contains the runtime
        tables that will be used to build up the path table entry.

    Directory - Supplies a pointer to a STRING structure that represents the
        fully-qualified directory name to create a path table entry for.

    Backslashes - Supplies a pointer to an RTL_BITMAP structure that represents
        a reversed bitmap index of all of the backslashes in the Directory
        parameter.

    BitmapHintIndex - Supplies a pointer to a USHORT that indicates the bitmap
        hint index that should be passed to Rtl->RtlFindSetBits() in order to
        delineate the next path component offset within the directory string.

    NumberOfBackslashesRemaining - Supplies a pointer to a USHORT that tracks
        the number of backslashes remaining in the directory.  This will be
        decremented as paths are processed.

    PathEntryPointer - Supplies a pointer to a variable that will receive the
        address of a newly-allocated PYTHON_PATH_TABLE_ENTRY structure.

    ForceModuleDirectory - Supplies a boolean value that, when set, indicates
        that the directory should be considered a module directory.  This will
        disable the normal logic that walks ancestor directories looking for
        __init__.py files.  It is used to handle things like .egg files.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;

    PRTL_BITMAP Bitmap;

    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;

    PPYTHON_PATH_TABLE_ENTRY PathEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY RootEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY NextEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY ParentEntry = NULL;
    PPYTHON_PATH_TABLE_ENTRY AncestorEntry = NULL;

    PSTRING Match = NULL;
    BOOL CaseInsensitive = TRUE;
    BOOL Success;

    STRING NextName;
    STRING AncestorName;
    STRING PreviousName;
    STRING DirectoryName;

    STRING NextDirectory;
    STRING RootDirectory;
    STRING ParentDirectory;
    STRING AncestorDirectory;
    STRING PreviousDirectory;

    PSTRING RootPrefix;
    PSTRING ParentPrefix;
    PSTRING AncestorPrefix;
    PSTRING PreviousPrefix;

    PUSHORT ForwardHintIndex;

    USHORT Offset;
    USHORT ForwardIndex;
    USHORT ReversedIndex;
    USHORT NumberOfChars;
    USHORT LastForwardIndex = 0;
    USHORT LastReversedIndex;
    USHORT CumulativeForwardIndex;
    USHORT CumulativeReversedIndex;
    USHORT RemainingAncestors;
    USHORT Length;

    BOOL IsModule = FALSE;
    BOOL IsRoot = FALSE;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Backslashes)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(PathEntryPointer)) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    //
    // Initialize pointer to the PREFIX_TABLE.
    //

    PrefixTable = &Python->PathTable->PrefixTable;

    //
    // Search for the directory in the prefix table.
    //

    PrefixTableEntry = Rtl->PfxFindPrefix(PrefixTable, Directory);
    PathEntry = (PPYTHON_PATH_TABLE_ENTRY)PrefixTableEntry;

    if (PathEntry) {

        //
        // A match was found, see if it matches our entire directory string.
        //

        Match = PathEntry->Prefix;

        if (Match->Length == Directory->Length) {

            //
            // The match is exact.  The directory has already been added to
            // our path table.
            //

            Success = TRUE;
            goto End;

        }


        if (Match->Length > Directory->Length) {

            //
            // We should never get a longer match than the directory name
            // we searched for.
            //

            __debugbreak();
        }

        //
        // A shorter entry was found.  Fall through to the following code
        // which will handle inserting a new prefix table entry for the
        // directory.  Note that we just set the shorter directory as our
        // parent directory; later on, once we've actually calculated our
        // parent directory, we may revise this to be an ancestor entry.
        //

        ParentEntry = PathEntry;
        PathEntry = NULL;

    } else if (!ForceModuleDirectory) {

        //
        // No path entry was present.  If the directory *isn't* a module, it
        // becomes our root directory.  If it is a module, we let execution
        // fall through to the normal parent/ancestor directory processing
        // logic below.
        //

        Success = IsModuleDirectoryA(Rtl, Directory, &IsModule);

        if (!Success) {

            //
            // Treat failure equivalent to "IsModule = FALSE".  See comment
            // in while loop below for more explanation.
            //

            IsModule = FALSE;
        }

        if (!IsModule) {

            //
            // Register this directory as the root and return.
            //

            return RegisterNonModuleDirectory(Python,
                                              Directory,
                                              PathEntryPointer);
        }
    }

    //
    // If we get here, we will be in one of the following states:
    //
    //  1.  An ancestor path (i.e. a prefix match) of Directory already exists
    //      in the prefix tree.
    //
    //  2.  No common ancestor exists, and the Directory has an __init__.py
    //      file or the ForceModuleDirectory flag was set, indicating that it
    //      is a module.
    //
    // In either case, we need to ensure the relevant ancestor directories
    // have been added, up to the "root" directory (i.e. the first directory
    // we find that has no __init__.py file present).
    //

    //
    // Get the index of the next backslash if possible.
    //

    if (!*NumberOfBackslashesRemaining) {
        __debugbreak();

        //
        // This will be hit if the directory we just added was a root volume,
        // e.g. the file was c:\foo.py and we added c:\.  Set the module name
        // to be empty and return.
        //

        PathEntry->IsNonModuleDirectory = TRUE;
        ClearString(&PathEntry->ModuleName);
        return TRUE;

    }

    //
    // We have at least one parent directory to process.  Extract it.
    //

    LastReversedIndex = (*BitmapHintIndex)++;

    ReversedIndex = (USHORT)(
        Rtl->RtlFindSetBits(
            Backslashes,
            1,
            *BitmapHintIndex
        )
    );

    if (ReversedIndex == BITS_NOT_FOUND ||
        ReversedIndex < LastReversedIndex) {

        //
        // Should never happen.
        //

        __debugbreak();
    }

    NumberOfChars = Directory->Length;
    Offset = NumberOfChars - ReversedIndex + LastReversedIndex + 1;
    CumulativeReversedIndex = LastReversedIndex;

    //
    // Extract the directory name, and the parent directory's full path.
    //

    DirectoryName.Length = (
        (ReversedIndex - CumulativeReversedIndex) -
        sizeof(CHAR)
    );
    Length = DirectoryName.Length;
    DirectoryName.MaximumLength = DirectoryName.Length;
    DirectoryName.Buffer = &Directory->Buffer[Offset];

    ParentDirectory.Length = Offset - 1;
    ParentDirectory.MaximumLength = Offset - 1;
    ParentDirectory.Buffer = Directory->Buffer;

    //
    // Special-case fast-path: if ParentEntry is defined and the length matches
    // ParentDirectory, we can assume all paths have already been added.
    //

    if (ParentEntry) {

        ParentPrefix = ParentEntry->Prefix;

        if (ParentPrefix->Length == ParentDirectory.Length) {

            //
            // Parent has been added.
            //

            goto FoundParent;

        }

        //
        // An ancestor, not our immediate parent, has been added.
        //

        AncestorEntry = ParentEntry;
        AncestorPrefix = ParentPrefix;
        ParentEntry = NULL;
        ParentPrefix = NULL;

        goto FoundAncestor;
    }

    //
    // No parent entry was found at all, we need to find the first directory
    // in our hierarchy that *doesn't* have an __init__.py file, then add that
    // and all subsequent directories up-to and including our parent.
    //

    PreviousDirectory.Length = ParentDirectory.Length;
    PreviousDirectory.MaximumLength = ParentDirectory.MaximumLength;
    PreviousDirectory.Buffer = ParentDirectory.Buffer;

    AncestorDirectory.Length = ParentDirectory.Length;
    AncestorDirectory.MaximumLength = ParentDirectory.MaximumLength;
    AncestorDirectory.Buffer = ParentDirectory.Buffer;

    PreviousName.Length = DirectoryName.Length;
    PreviousName.MaximumLength = DirectoryName.MaximumLength;
    PreviousName.Buffer = DirectoryName.Buffer;

    AncestorName.Length = DirectoryName.Length;
    AncestorName.MaximumLength = DirectoryName.MaximumLength;
    AncestorName.Buffer = DirectoryName.Buffer;

    do {

        if (!*NumberOfBackslashesRemaining) {

            IsModule = FALSE;

        } else {

            Success = IsModuleDirectoryA(Rtl, &AncestorDirectory, &IsModule);

            if (!Success) {

                //
                // We treat failure of the IsModuleDirectory() call the same
                // as if it indicated that the directory wasn't a module (i.e.
                // didn't have an __init__.py file), mainly because there's
                // not really any other sensible option.  (Unwinding all of
                // the entries we may have added seems like overkill at this
                // point.)
                //

                IsModule = FALSE;

            }
        }

        if (!IsModule) {

            //
            // We've found the first non-module directory we're looking for.
            // This becomes our root directory.
            //

            RootDirectory.Length = AncestorDirectory.Length;
            RootDirectory.MaximumLength = AncestorDirectory.MaximumLength;
            RootDirectory.Buffer = AncestorDirectory.Buffer;
            RootPrefix = &RootDirectory;

            Success = RegisterNonModuleDirectory(Python,
                                                 RootPrefix,
                                                 &RootEntry);

            if (!Success) {
                return FALSE;
            }

            if (PreviousDirectory.Length > RootDirectory.Length) {

                //
                // Add the previous directory as the first "module" directory.
                //

                Success = RegisterDirectory(Python,
                                            &PreviousDirectory,
                                            &PreviousName,
                                            RootEntry,
                                            &AncestorEntry,
                                            FALSE);

                if (!Success) {
                    return FALSE;
                }

                //
                // Determine if we need to process more ancestors, or if that
                // was actually the parent path.
                //

                if (AncestorEntry->Prefix->Length == ParentDirectory.Length) {

                    ParentPrefix = AncestorEntry->Prefix;
                    ParentEntry = AncestorEntry;

                    goto FoundParent;

                } else {

                    goto FoundAncestor;

                }

            } else {

                //
                // Our parent directory is the root directory.
                //

                ParentPrefix = RootPrefix;
                ParentEntry = RootEntry;

                goto FoundParent;

            }

            break;
        }

        //
        // Parent directory is also a module.  Make sure we're not on the
        // root level.
        //

        if (!--(*NumberOfBackslashesRemaining)) {

            //
            // Force the loop to continue, which will trigger the check at the
            // top for number of remaining backslashes, which will fail, which
            // causes the path to be added as a root, which is what we want.
            //

            continue;
        }

        PreviousPrefix = &PreviousDirectory;

        PreviousDirectory.Length = AncestorDirectory.Length;
        PreviousDirectory.MaximumLength = AncestorDirectory.MaximumLength;
        PreviousDirectory.Buffer = AncestorDirectory.Buffer;

        PreviousName.Length = AncestorName.Length;
        PreviousName.MaximumLength = AncestorName.MaximumLength;
        PreviousName.Buffer = AncestorName.Buffer;

        LastReversedIndex = (*BitmapHintIndex)++;

        ReversedIndex = (USHORT)Rtl->RtlFindSetBits(Backslashes, 1,
                                                    *BitmapHintIndex);

        if (ReversedIndex == BITS_NOT_FOUND ||
            ReversedIndex < LastReversedIndex) {

            //
            // Should never happen.
            //

            __debugbreak();
        }

        CumulativeReversedIndex += LastReversedIndex;
        Offset = NumberOfChars - ReversedIndex + CumulativeReversedIndex + 1;

        //
        // Extract the ancestor name and directory full path.
        //

        AncestorName.Length = (
            (ReversedIndex - CumulativeReversedIndex) -
            sizeof(CHAR)
        );

        AncestorName.MaximumLength = DirectoryName.Length;
        AncestorName.Buffer = &Directory->Buffer[Offset];


        AncestorDirectory.Length = Offset - 1;
        AncestorDirectory.MaximumLength = Offset - 1;
        AncestorDirectory.Buffer = Directory->Buffer;

        //
        // Continue the loop.
        //

    } while (1);

FoundAncestor:

    //
    // Keep adding entries for the next directories as long as they're
    // not the parent directory.  AncestorEntry will be set here to the entry
    // for the initial matching ancestor path entry.  ParentDirectory will
    // still represent the target parent directory we need to add ancestor
    // entries up-to (but not including).
    //

    //
    // Re-use our reversed bitmap to create a forward bitmap of backslashes.
    //

    AncestorPrefix = AncestorEntry->Prefix;
    NextName.Length = ParentDirectory.Length - AncestorPrefix->Length - 1;
    NextName.MaximumLength = NextName.Length;
    NextName.Buffer = ParentDirectory.Buffer + AncestorPrefix->Length + 1;

    //
    // Truncate our existing bitmap to an aligned size matching the number of
    // remaining characters to scan through.
    //

    Bitmap = Backslashes;
    Bitmap->SizeOfBitMap = ALIGN_UP_USHORT_TO_POINTER_SIZE(NextName.Length);
    Rtl->RtlClearAllBits(Bitmap);

    InlineFindCharsInString(&NextName, '\\', Bitmap);

    //
    // Add one to account for the fact that our Remaining.Length omits the
    // last trailing backslash from the path.
    //

    RemainingAncestors = (USHORT)Rtl->RtlNumberOfSetBits(Bitmap) + 1;

    if (RemainingAncestors == 1) {

        //
        // We don't need to consult the bitmap at all if there is only one
        // ancestor directory remaining; the NextName string we prepared
        // above has all the details we need.
        //

        NextDirectory.Length = AncestorPrefix->Length + NextName.Length + 1;
        NextDirectory.MaximumLength = NextDirectory.Length;

    } else {

        if (RemainingAncestors == 0) {

            //
            // Shouldn't be possible.
            //

            __debugbreak();
        }

        //
        // As we have multiple ancestors to process, use the bitmap to find
        // out where the first one ends.
        //

        ForwardIndex = (USHORT)Rtl->RtlFindSetBits(Bitmap, 1, 0);
        ForwardHintIndex = &ForwardIndex;
        CumulativeForwardIndex = ForwardIndex;

        NextName.Length = ForwardIndex;
        NextName.MaximumLength = NextName.Length;

        NextDirectory.Length = AncestorPrefix->Length + ForwardIndex + 1;
        NextDirectory.MaximumLength = NextDirectory.Length;

    }

    NextDirectory.Buffer = ParentDirectory.Buffer;

    do {

        //
        // Use the same logic as above with regards to handling the failure
        // of IsModuleDirectory (i.e. treat it as if it were IsModule = FALSE).
        //

        Success = IsModuleDirectoryA(Rtl, &NextDirectory, &IsModule);

        if (!Success) {
            IsModule = FALSE;
        }

        if (!IsModule) {

            Success = RegisterNonModuleDirectory(Python,
                                                 &NextDirectory,
                                                 &NextEntry);

        } else {

            Success = RegisterModuleDirectory(Python,
                                              &NextDirectory,
                                              &NextName,
                                              AncestorEntry,
                                              &NextEntry);

        }

        if (!Success) {
            return FALSE;
        }

        if (!NextEntry) {
            __debugbreak();
        }

        //
        // See if that was the last ancestor directory we need to add.
        //

        if (!--RemainingAncestors) {

            //
            // Invariant check: if there are no more ancestors, the length of
            // the path prefix just added should match the length of our parent
            // directory.
            //

            if (NextEntry->Prefix->Length != ParentDirectory.Length) {
                __debugbreak();
            }

            //
            // This ancestor is our parent entry, continue to the final step.
            //

            ParentEntry = NextEntry;

            goto FoundParent;

        }

        //
        // There are still ancestor paths remaining.
        //

        if (RemainingAncestors == 1) {

            if (ParentDirectory.Length == NextDirectory.Length) {
                __debugbreak();
            }

            NextName.Buffer += NextName.Length + 1;

            NextName.Length = (
                ParentDirectory.Length -
                NextDirectory.Length   -
                1
            );
            NextDirectory.Length += (NextName.Length + 1);

            if (NextDirectory.Buffer[NextDirectory.Length - 1] == '\\') {
                __debugbreak();
            }

        } else {

            LastForwardIndex = (*ForwardHintIndex)++;
            CumulativeForwardIndex += LastForwardIndex;

            ForwardIndex = (USHORT)(
                Rtl->RtlFindSetBits(Bitmap, 1,
                                    *ForwardHintIndex)
            );

            if (ForwardIndex < LastForwardIndex ||
                ForwardIndex > Directory->Length) {

                __debugbreak();
            }

            NextName.Buffer += NextName.Length + 1;

            NextName.Length = ForwardIndex - (LastForwardIndex + 1);
            NextDirectory.Length += (NextName.Length + 1);

            if (NextDirectory.Buffer[NextDirectory.Length - 1] == '\\') {
                __debugbreak();
            }
        }

        if (ForwardIndex == BITS_NOT_FOUND) {

            //
            // Should never happen.
            //

            __debugbreak();

        }

        //
        // Isolate the name portion of the next ancestor.
        //

        NextName.MaximumLength = NextName.Length;

        //
        // Update the directory length.
        //

        NextDirectory.MaximumLength = NextDirectory.Length;

        //
        // Continue the loop.
        //

    } while (1);

FoundParent:

    //
    // Add a new entry for the directory.
    //

    if (ForceModuleDirectory) {
        IsModule = TRUE;
    } else {
        Success = IsModuleDirectoryA(Rtl, Directory, &IsModule);
        if (!Success) {
            IsModule = FALSE;
        }
    }

    IsRoot = (IsModule ? FALSE : TRUE);

    Success = RegisterDirectory(Python,
                                Directory,
                                &DirectoryName,
                                ParentEntry,
                                &PathEntry,
                                IsRoot);

    if (!Success) {
        return FALSE;
    }

End:
    *PathEntryPointer = PathEntry;

    return TRUE;
}

_Use_decl_annotations_
BOOL
RegisterDirectory(
    PPYTHON Python,
    PSTRING Directory,
    PSTRING DirectoryName,
    PPYTHON_PATH_TABLE_ENTRY AncestorEntry,
    PPPYTHON_PATH_TABLE_ENTRY EntryPointer,
    BOOL IsRoot
    )
/*++

Routine Description:

    This routine registers a new directory, returning a corresponding
    PYTHON_PATH_TABLE_ENTRY structure for the path.

Arguments:

    Python - Supplies a pointer to a PYTHON structure that contains the runtime
        tables that will be used to build up the path table entry.

    Directory - Supplies a pointer to a STRING structure that represents the
        fully-qualified directory name to create a path table entry for.

    DirectoryName - Supplies a pointer to a STRING structure that represents
        the name of the directory only (i.e. without the rest of the path).

    AncestorEntry - Optionally supplies a pointer to an ancestor path table
        entry if one is present.  This allows the routine to only create path
        table entries for missing intermediate paths instead of having to walk
        backwards looking for non-module directories.

    EntryPointer - Supplies a pointer to a variable that will receive the
        address of the PYTHON_PATH_TABLE_ENTRY structure corresponding to the
        Directory parameter.

    IsRoot - Supplies a boolean value indicating if this directory is considered
        a root (non-module) directory or not.  This has implications on how
        things like module names are constructed and enforces various internal
        naming and state invariants.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;

    PPREFIX_TABLE PrefixTable;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;
    PPREFIX_TABLE_ENTRY ExistingTableEntry;

    ULONG StringBufferSize;
    PPYTHON_PATH_TABLE_ENTRY Entry;
    PPYTHON_PATH_TABLE_ENTRY ExistingEntry;

    PSTRING AncestorModuleName;
    PSTRING Name;
    PSTRING ModuleName;
    USHORT Offset;
    USHORT NameLength;

    PSTRING DirectoryPrefix;

    PSTRING Match = NULL;
    BOOL CaseInsensitive = TRUE;
    BOOL Success = FALSE;
    BOOL IsModule;
    BOOL AncestorIsRoot;

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Directory)) {
        return FALSE;
    }

    if (ARGUMENT_PRESENT(EntryPointer)) {
        *EntryPointer = NULL;
    }

    //
    // Non-root nodes must have a name and ancestor provided.
    //

    if (IsRoot) {

        IsModule = FALSE;
        AncestorIsRoot = FALSE;

    } else {

        if (!ARGUMENT_PRESENT(DirectoryName)) {
            return FALSE;
        }

        if (!ARGUMENT_PRESENT(AncestorEntry)) {
            return FALSE;
        }

        IsModule = TRUE;

        AncestorIsRoot = (BOOL)AncestorEntry->IsNonModuleDirectory;

        AncestorModuleName = &AncestorEntry->ModuleName;

        NameLength = DirectoryName->Length;

        StringBufferSize = (
            AncestorModuleName->MaximumLength + // includes trailing NUL
            NameLength
        );

        if (!AncestorIsRoot) {

            //
            // Account for the joining slash + NUL.
            //

            StringBufferSize += 2;

        } else {

            //
            // Account for just the trailing NUL.
            //

            StringBufferSize += 1;
        }

        if (StringBufferSize > MAX_USTRING) {
            return FALSE;
        }

    }

    Rtl = Python->Rtl;

    //
    // Make sure there isn't already an entry for the directory in the path
    // prefix table.
    //

    PrefixTable = &Python->PathTable->PrefixTable;
    ExistingTableEntry = Rtl->PfxFindPrefix(PrefixTable, Directory);
    ExistingEntry = (PPYTHON_PATH_TABLE_ENTRY)ExistingTableEntry;

    if (ExistingEntry) {

        PSTRING Existing = &ExistingEntry->Path;

        if (Existing->Length == Directory->Length) {

            //
            // The directory already exists in the tree.
            //

            if (!ExistingEntry->IsValid) {
                __debugbreak();
            }

            //
            // Use this existing entry as the entry we potentially write to
            // the caller's EntryPointer pointer.
            //

            Entry = ExistingEntry;
            goto End;

        }

#ifdef _DEBUG
        if (Existing->Length > Directory->Length) {
            __debugbreak();
        }
#endif

    }

    if (!AllocatePythonPathTableEntry(Python, &Entry)) {
        return FALSE;
    }

    if (IsModule) {
        Entry->IsModuleDirectory = TRUE;
    } else {
        Entry->IsNonModuleDirectory = TRUE;
    }

    if (IsRoot) {

        ModuleName = &Entry->ModuleName;

        if (ModuleName->Length != 0 ||
            ModuleName->Buffer != NULL) {
            __debugbreak();
        }

    } else {

        PSTR Dest;
        PSTR Source;
        USHORT Count;

        //
        // We verified that StringBufferSizes was < MAX_USTRING above.
        //

        USHORT Size = (USHORT)StringBufferSize;

        ModuleName = &Entry->ModuleName;

        if (!AllocateStringBuffer(Python, Size, ModuleName)) {
            FreePythonPathTableEntry(Python, Entry);
            return FALSE;
        }

        Name = &Entry->Name;

        if (!AncestorIsRoot) {
            Offset = AncestorModuleName->Length + 1;
        } else {
            Offset = 0;
        }

        ModuleName->Length = Offset + NameLength;

        if (ModuleName->MaximumLength <= ModuleName->Length) {

            //
            // There should be space for at least the trailing NUL.
            //

            __debugbreak();
        }

        Name->Length = NameLength;
        Name->MaximumLength = NameLength + 1;

        //
        // Point the name into the relevant offset of the ModuleName.
        //

        Name->Buffer = &ModuleName->Buffer[Offset];

        if (!AncestorIsRoot) {

            if (AncestorModuleName->Length == 0 ||
                !AncestorModuleName->Buffer) {
                __debugbreak();
            }

            //
            // Copy the ModuleName over.
            //

            __movsb(ModuleName->Buffer,
                    AncestorModuleName->Buffer,
                    AncestorModuleName->Length);

            //
            // Add the slash.
            //

            ModuleName->Buffer[Offset-1] = '\\';

        }

        //
        // Copy the name over.
        //

        Count = NameLength;
        Dest = Name->Buffer;
        Source = DirectoryName->Buffer;

        __movsb(Dest, Source, Count);

        Dest += Count;

        //
        // And add the final trailing NUL.
        //

        *Dest++ = '\0';
    }

    DirectoryPrefix = &Entry->Path;

    DirectoryPrefix->Length = Directory->Length;
    DirectoryPrefix->MaximumLength = Directory->MaximumLength;
    DirectoryPrefix->Buffer = Directory->Buffer;

    PrefixTable = &Python->PathTable->PrefixTable;
    PrefixTableEntry = (PPREFIX_TABLE_ENTRY)Entry;

    Success = Rtl->PfxInsertPrefix(PrefixTable,
                                   DirectoryPrefix,
                                   PrefixTableEntry);

    if (!Success) {
        __debugbreak();
        FreeStringBuffer(Python, &Entry->ModuleName);
        FreePythonPathTableEntry(Python, Entry);
        return FALSE;
    } else {
        Entry->IsValid = TRUE;
    }

End:
    Success = TRUE;

    if (ARGUMENT_PRESENT(EntryPointer)) {
        *EntryPointer = Entry;
    }

    return Success;
}

_Use_decl_annotations_
BOOL
QualifyPath(
    PPYTHON Python,
    PSTRING SourcePath,
    PPSTRING DestinationPathPointer
    )
/*++

Routine Description:

    This routine converts a relative path into a fully-qualified one, using the
    current directory name to resolve the non-relative part of the path name.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    SourcePath - Supplies a pointer to a STRING struct containing the source
        path to qualify.

    DestinationPathPointer - Supplies a pointer to a variable that receives the
        address of a newly-allocated STRING structure and corresponding buffer
        representing the fully-qualified version of SourcePath.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PRTL Rtl;

    CONST ULARGE_INTEGER MaxSize = { MAX_USTRING - 2 };

    ULONG CurDirLength;

    PSTRING String;
    PCHAR Dest;

    ULONG Remaining;
    USHORT NewLength;
    USHORT NewMaximumLength;

    CHAR Buffer[_MAX_PATH];
    CHAR CanonPath[_MAX_PATH];

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SourcePath)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(DestinationPathPointer)) {
        return FALSE;
    }

    Rtl = Python->Rtl;

    Dest = (PCHAR)Buffer;
    CurDirLength = GetCurrentDirectoryA(_MAX_PATH, Dest);

    //
    // Initial validation of the directory length.
    //

    if (CurDirLength == 0) {
        return FALSE;
    } else if (CurDirLength > MaxSize.LowPart) {
        return FALSE;
    }

    //
    // Update the destination pointer.
    //

    Dest += CurDirLength;

    //
    // Append the joining slash and update the length if necessary.
    //

    if (*Dest != '\\') {
        *Dest++ = '\\';
        CurDirLength += 1;
    }

    //
    // Calculate remaining space (account for trailing NUL).
    //

    Remaining = _MAX_PATH - CurDirLength - 1;

    if (Remaining < SourcePath->Length) {

        //
        // Not enough space left for our source path to be appended.
        //

        return FALSE;
    }

    //
    // There's enough space, copy the source path over.
    //

    __movsb(Dest, SourcePath->Buffer, SourcePath->Length);

    Dest += SourcePath->Length;

    //
    // Add terminating NUL.
    //

    *Dest = '\0';

    //
    // Our temporary buffer now contains a concatenated current directory name
    // and source path plus trailing NUL.  Call PathCanonicalize() against it
    // and store the results in another stack-allocated _MAX_PATH-sized temp
    // buffer.
    //

    Success = Rtl->PathCanonicalizeA((PSTR)&CanonPath, (PSTR)&Buffer);

    if (!Success) {
        return FALSE;
    }

    //
    // Get the new length for the canonicalized path.
    //

    NewLength = (USHORT)strlen((PCSTR)&CanonPath);

    //
    // Add one for maximum length to account for the trailing NUL.
    //

    NewMaximumLength = NewLength + 1;

    //
    // Sanity check that there is still a trailing NUL where we expect.
    //

    if (CanonPath[NewLength] != '\0' || CanonPath[NewLength-1] == '\0') {
        __debugbreak();
    }

    //
    // Now allocate the string.
    //

    Success = AllocateStringAndBuffer(Python, NewMaximumLength, &String);
    if (!Success) {
        return FALSE;
    }

    //
    // Initialize sizes.
    //

    String->Length = NewLength;

    if (String->MaximumLength <= String->Length) {
        __debugbreak();
    }

    //
    // Copy over the canonicalized path.
    //

    __movsb((PBYTE)String->Buffer, (LPCBYTE)&CanonPath[0], NewLength);

    //
    // Ensure there's a trailing NUL.
    //

    String->Buffer[String->Length] = '\0';

    //
    // Update the caller's pointer and return.
    //

    *DestinationPathPointer = String;

    return TRUE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
