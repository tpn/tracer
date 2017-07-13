/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    File.c

Abstract:

    This module implements functionality related to the RTL_FILE structure.
    Routines are provided to initialize a new RTL_FILE structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeRtlFile(
    PRTL Rtl,
    PUNICODE_STRING UnicodeStringPath,
    PSTRING AnsiStringPath,
    PALLOCATOR BitmapAllocator,
    PALLOCATOR UnicodeStringBufferAllocator,
    PALLOCATOR FileContentsAllocator,
    PALLOCATOR LineAllocator,
    PALLOCATOR RtlFileAllocator,
    RTL_FILE_INIT_FLAGS InitFlags,
    PPRTL_FILE FilePointer,
    PLARGE_INTEGER Timestamp
    )
/*++

Routine Description:

    Converts a UNICODE_STRING representing a fully-qualified path into a
    RTL_FILE structure.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL struct.

    UnicodeStringPath - Supplies a pointer to a UNICODE_STRING structure that
        contains a fully-qualified path name.  If this parameter is provided,
        AnsiStringPath must be NULL.

    AnsiStringPath - Supplies a pointer to a STRING structure that contains a
        fully-qualified path name.  If this parameter is provided, the
        UnicodeStringPath parameter must be NULL.

    BitmapAllocator - Supplies a pointer to an ALLOCATOR structure that is
        used for allocating all bitmaps.

    UnicodeStringBufferAllocator - Supplies a pointer to an ALLOCATOR structure
        that is used for allocating any Unicode string buffers.

    LineAllocator - Supplies a pointer to an ALLOCATOR structure that is used
        for allocating an array of STRING structures corresponding to each line
        if the incoming file is a text/source code file.  Can be NULL if the
        InitFlags parameter indicate IsImageFile.

    FileContentsAllocator - Supplies a pointer to an ALLOCATOR structure that is
        used for allocating space for the file contents.

    RtlFileAllocator - Optionally supplies a pointer to an ALLOCATOR structure
        that will be used to allocate an RTL_FILE structure if the FilePointer
        parameter is NULL.  (If FilePointer is NULL, this parameter can not be
        NULL.)

    InitFlags - Supplies a initialization flags that customize the behavior of
        this routine.

    FilePointer - Supplies the address of a variable that either points to an
        existing RTL_FILE structure to be initialized, or alternatively, if the
        pointed to address is NULL, receives the address of the newly allocated
        RTL_FILE structure (allocated via RtlFileAllocator).

    Timestamp - Supplies a pointer to a LARGE_INTEGER timestamp to use for all
        allocations.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsSourceCode;
    BOOL IsAnsiString = FALSE;
    ULONG Index;
    PCHAR SourceContent = NULL;
    PCHAR DestContent;
    PCHAR Dest;
    PCHAR Source;
    PCHAR BitmapBuffers;
    DWORD LastError;
    ULONG AllocationPages;
    ULONG BitmapAllocationAttempt;
    ULONG NumberOfBitmaps;
    ULONG_PTR BitmapBufferSize;
    ULONG_PTR AlignedBitmapBufferSize;
    ULONG_PTR SingleBitmapAllocSize;
    ULONG_PTR TotalBitmapAllocSize;
    PRTL_FILE File;
    PRTL_PATH Path;
    HANDLE FileHandle = NULL;
    HANDLE MappingHandle = NULL;
    HCRYPTPROV CryptProv = 0;
    HCRYPTHASH CryptHashMD5 = 0;
    HCRYPTHASH CryptHashSHA1 = 0;
    DWORD SizeOfMD5InBytes;
    DWORD SizeOfSHA1InBytes;
    LARGE_INTEGER StartCopy;
    LARGE_INTEGER EndCopy;
    LARGE_INTEGER Elapsed;
    PRTL_TEXT_FILE SourceCode;
    FILE_STANDARD_INFO FileInfo;
    BY_HANDLE_FILE_INFORMATION HandleInfo;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(FilePointer)) {
        return FALSE;
    }

    //
    // If FilePointer points to a NULL address, make sure RtlFileAllocator is
    // not NULL.
    //

    File = *FilePointer;
    if (!ARGUMENT_PRESENT(File)) {
        if (!ARGUMENT_PRESENT(RtlFileAllocator)) {
            return FALSE;
        }

        File = (PRTL_FILE)(
            RtlFileAllocator->CallocWithTimestamp(
                RtlFileAllocator->Context,
                1,
                sizeof(*File),
                Timestamp
            )
        );

        if (!File) {
            return FALSE;
        }
    }

    Path = &File->Path;

    //
    // Initialize the RTL_PATH structure based on the incoming path if the
    // InitPath init flag was specified.
    //

    if (!InitFlags.InitPath) {
        if (!IsValidMinimumDirectoryUnicodeString(&Path->Full)) {
            return FALSE;
        }
    } else {
        if (!ARGUMENT_PRESENT(UnicodeStringPath)) {
            if (!ARGUMENT_PRESENT(AnsiStringPath)) {
                return FALSE;
            }
            IsAnsiString = TRUE;
        }
        if (!ARGUMENT_PRESENT(BitmapAllocator)) {
            return FALSE;
        }
        if (!ARGUMENT_PRESENT(UnicodeStringBufferAllocator)) {
            return FALSE;
        }
        if (IsAnsiString) {
            Success = (
                Rtl->StringToExistingRtlPath(
                    Rtl,
                    AnsiStringPath,
                    BitmapAllocator,
                    UnicodeStringBufferAllocator,
                    Path,
                    Timestamp
                )
            );
        } else {
            Success = (
                Rtl->UnicodeStringToExistingRtlPath(
                    Rtl,
                    UnicodeStringPath,
                    BitmapAllocator,
                    UnicodeStringBufferAllocator,
                    Path,
                    Timestamp
                )
            );
        }
    }

    IsSourceCode = InitFlags.IsSourceCode;

    //
    // Open the file.
    //

    FileHandle = CreateFileW(
        Path->Full.Buffer,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (!FileHandle || FileHandle == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        if (LastError == ERROR_FILE_NOT_FOUND || ERROR_PATH_NOT_FOUND) {
            Success = TRUE;
            goto End;
        }
        goto Error;
    }

    //
    // Get the file standard information.
    //

    Success = GetFileInformationByHandleEx(
        FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        &FileInfo,
        sizeof(FileInfo)
    );

    if (!Success) {
        LastError = GetLastError();
        goto Error;
    }

    //
    // Fill in initial file details.
    //

    File->EndOfFile.QuadPart = FileInfo.EndOfFile.QuadPart;
    File->AllocationSize.QuadPart = FileInfo.AllocationSize.QuadPart;

    //
    // Break if we encounter a file above 4GB for now.
    //

    if (File->EndOfFile.HighPart) {
        __debugbreak();
        goto Error;
    }

    //
    // If we're an empty file, return now.
    //

    if (!File->EndOfFile.LowPart) {
        Success = TRUE;
        goto End;
    }

    //
    // Calculate bitmap allocation size required.  This includes the RTL_BITMAP
    // structure and backing bitmap buffer -- the buffer size is aligned up to
    // a 32 byte boundary.
    //

    if (IsSourceCode) {
        BitmapBufferSize = File->EndOfFile.QuadPart >> 3;
        AlignedBitmapBufferSize = ALIGN_UP(BitmapBufferSize, 32);
        NumberOfBitmaps = NUMBER_OF_RTL_TEXT_FILE_BITMAPS;

        SingleBitmapAllocSize = sizeof(RTL_BITMAP) + AlignedBitmapBufferSize;
        TotalBitmapAllocSize = SingleBitmapAllocSize * NumberOfBitmaps;
    }

    BitmapBuffers = NULL;
    BitmapAllocationAttempt = 0;

#define TRY_ALLOC_BITMAP_BUFFERS() do {                          \
    if (IsSourceCode && !BitmapBuffers) {                        \
        BitmapAllocationAttempt++;                               \
        BitmapBuffers = BitmapAllocator->TryCallocWithTimestamp( \
            BitmapAllocator->Context,                            \
            NumberOfBitmaps,                                     \
            SingleBitmapAllocSize,                               \
            Timestamp                                            \
        );                                                       \
    }                                                            \
} while (0)

    TRY_ALLOC_BITMAP_BUFFERS();

    //
    // Get basic info.
    //

    Success = GetFileInformationByHandleEx(
        FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileBasicInfo,
        &File->BasicInfo,
        sizeof(File->BasicInfo)
    );

    if (!Success) {
        LastError = GetLastError();
        goto Error;
    }

    //
    // Get the 128-bit file ID info if possible.
    //

    Success = GetFileInformationByHandleEx(
        FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileIdInfo,
        &File->FileIdInfo,
        sizeof(File->FileIdInfo)
    );

    if (!Success) {
        LastError = GetLastError();

        //
        // Only works on Server 2012+ and ReFS.  Ignore errors.
        //
    }

    TRY_ALLOC_BITMAP_BUFFERS();

    //
    // Get the normal file ID.
    //

    Success = GetFileInformationByHandle(FileHandle, &HandleInfo);
    if (!Success) {
        LastError = GetLastError();
        goto Error;
    }

    File->FileId.LowPart = HandleInfo.nFileIndexLow;
    File->FileId.HighPart = HandleInfo.nFileIndexHigh;

    //
    // Create a file mapping.
    //

    MappingHandle = CreateFileMapping(
        FileHandle,
        NULL,
        PAGE_READONLY,
        File->EndOfFile.HighPart,
        File->EndOfFile.LowPart,
        NULL
    );

    if (!MappingHandle || MappingHandle == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        goto Error;
    }

    TRY_ALLOC_BITMAP_BUFFERS();

    //
    // Map the file.
    //

    SourceContent = (PCHAR)MapViewOfFile(
        MappingHandle,
        FILE_MAP_READ,
        0,
        0,
        File->EndOfFile.QuadPart
    );

    if (!SourceContent) {
        LastError = GetLastError();
        goto Error;
    }

    TRY_ALLOC_BITMAP_BUFFERS();

    //
    // Get the number of pages for the contents.
    //

    File->NumberOfPages = (
        ROUND_TO_PAGES(File->EndOfFile.QuadPart) >> PAGE_SHIFT
    );

    AllocationPages = (
        ROUND_TO_PAGES(File->AllocationSize.QuadPart) >> PAGE_SHIFT
    );

    if (File->NumberOfPages != AllocationPages) {

        //
        // I've seen this getting hit on ntdll.dll for some reason; file size
        // and size on disk don't match up and it's not compressed.  (I can't
        // remember why I was debugbreak'ing either, so, ignore for now.)
        //

        //__debugbreak();

        NOTHING;
    }

    if (InitFlags.CopyContents) {

        //
        // Allocate space for the file.
        //

        DestContent = (PCHAR)(
            FileContentsAllocator->CallocWithTimestamp(
                FileContentsAllocator->Context,
                1,
                File->EndOfFile.QuadPart,
                Timestamp
            )
        );

        if (!DestContent) {
            goto Error;
        }

        //
        // Copy a page at a time using streaming loads.
        //

        Dest = DestContent;
        Source = SourceContent;
        QueryPerformanceCounter(&StartCopy);

        if (InitFlags.CopyViaMovsq) {

            Rtl->CopyPagesMovsq(DestContent,
                                SourceContent,
                                File->NumberOfPages,
                                NULL);

        } else {

            Rtl->CopyPagesAvx2(DestContent,
                               SourceContent,
                               File->NumberOfPages,
                               NULL);
        }

        QueryPerformanceCounter(&EndCopy);

        Elapsed.QuadPart = EndCopy.QuadPart - StartCopy.QuadPart;
        File->Elapsed = Elapsed.LowPart;
        File->Content = DestContent;

        //
        // Convert to microseconds and then calculate bytes per second.
        //

        Elapsed.QuadPart *= Rtl->Multiplicand.QuadPart;
        Elapsed.QuadPart /= Rtl->Frequency.QuadPart;
        File->CopyTimeInMicroseconds.QuadPart = Elapsed.QuadPart;
        if (Elapsed.QuadPart > 0) {
            File->CopiedBytesPerSecond.QuadPart = (
                (File->AllocationSize.QuadPart /
                 Elapsed.QuadPart) * Rtl->Multiplicand.QuadPart
            );
        }
    }

    //
    // We can unmap the file and close all handles now unless init flags
    // indicate otherwise.
    //

    if (InitFlags.KeepViewMapped) {
        File->MappedAddress = SourceContent;
    } else {
        UnmapViewOfFile(SourceContent);
    }

    if (InitFlags.KeepMappingHandleOpen) {
        File->MappingHandle = MappingHandle;
    } else {
        CloseHandle(MappingHandle);
    }

    if (InitFlags.KeepFileHandleOpen) {
        File->FileHandle = FileHandle;
    } else {
        CloseHandle(FileHandle);
    }

    SourceContent = NULL;
    MappingHandle = NULL;
    FileHandle = NULL;

    if (!InitFlags.CopyContents) {
        Success = TRUE;
        goto End;
    }

    CryptProv = Rtl->CryptProv;

    //
    // Create the MD5 and SHA1 hashes.
    //

    Success = CryptCreateHash(CryptProv, CALG_MD5, 0, 0, &CryptHashMD5);
    if (!Success) {
        LastError = GetLastError();
        Rtl->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    Success = CryptCreateHash(CryptProv, CALG_SHA1, 0, 0, &CryptHashSHA1);
    if (!Success) {
        LastError = GetLastError();
        Rtl->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    //
    // Hash the data.
    //

    Success = CryptHashData(CryptHashMD5,
                            DestContent,
                            File->EndOfFile.LowPart,
                            0);

    if (!Success) {
        LastError = GetLastError();
        Rtl->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    Success = CryptHashData(CryptHashSHA1,
                            DestContent,
                            File->EndOfFile.LowPart,
                            0);

    if (!Success) {
        LastError = GetLastError();
        Rtl->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    //
    // Write the hash value into our buffer.
    //

    SizeOfMD5InBytes = sizeof(File->MD5);
    Success = CryptGetHashParam(CryptHashMD5,
                                HP_HASHVAL,
                                (PBYTE)&File->MD5,
                                &SizeOfMD5InBytes,
                                0);

    if (!Success) {
        LastError = GetLastError();
        Rtl->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    SizeOfSHA1InBytes = sizeof(File->SHA1);
    Success = CryptGetHashParam(CryptHashSHA1,
                                HP_HASHVAL,
                                (PBYTE)&File->SHA1,
                                &SizeOfSHA1InBytes,
                                0);

    if (!Success) {
        LastError = GetLastError();
        Rtl->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    if (!IsSourceCode) {
        File->Type = RtlFileImageFileType;
    } else {
        ULONG NumberOfLines;
        ULONG SizeOfBitMap;
        ULONG BitmapBytes;
        ULONG BitIndex;
        ULONG LastBitIndex;
        PSTRING Line;
        PRTL_BITMAP Bitmap;
        PRTL_BITMAP LineEndings;
        PPRTL_BITMAP BitmapPointer;
        PRTL_FIND_SET_BITS FindSetBits;

        File->Type = RtlFileTextFileType;

        SourceCode = &File->SourceCode;

        if (!BitmapBuffers) {

            //
            // One last blocking attempt.
            //

            BitmapAllocationAttempt++;
            BitmapBuffers = BitmapAllocator->CallocWithTimestamp(
                BitmapAllocator->Context,
                NumberOfBitmaps,
                SingleBitmapAllocSize,
                Timestamp
            );

            if (!BitmapBuffers) {
                goto Error;
            }
        }

        //
        // Bitmaps are available, initialize them.
        //

        BitmapBytes = (ULONG)SingleBitmapAllocSize;
        SizeOfBitMap = File->EndOfFile.LowPart;
        BitmapPointer = &SourceCode->CarriageReturnBitmap;
        for (Index = 0; Index < NumberOfBitmaps; Index++) {
            Bitmap = (PRTL_BITMAP)(BitmapBuffers + (Index * BitmapBytes));
            Bitmap->SizeOfBitMap = SizeOfBitMap;
            Bitmap->Buffer = (PULONG)(
                RtlOffsetToPointer(
                    Bitmap,
                    sizeof(RTL_BITMAP)
                )
            );
            *BitmapPointer++ = Bitmap;
        }

        NumberOfLines = 0;
        for (Index = 0; Index < File->EndOfFile.LowPart; Index++) {
            CHAR Char = DestContent[Index];
            switch (Char) {
                case L'\r':
                    FastSetBit(SourceCode->CarriageReturnBitmap, Index);
                    FastSetBit(SourceCode->LineEndingBitmap, Index);
                    break;
                case L'\n':
                    NumberOfLines++;
                    FastSetBit(SourceCode->LineFeedBitmap, Index);
                    FastSetBit(SourceCode->LineEndingBitmap, Index);
                    break;
                case L' ':
                    FastSetBit(SourceCode->WhitespaceBitmap, Index);
                    break;
                case L'\t':
                    FastSetBit(SourceCode->TabBitmap, Index);
                    break;
                default:
                    break;
            }
        }

        SourceCode->NumberOfLines = NumberOfLines;
        if (!NumberOfLines) {
            Success = TRUE;
            goto End;
        }

        //
        // Create an array of STRING structures for the lines.
        //

        SourceCode->Lines = (PSTRING)(
            LineAllocator->CallocWithTimestamp(
                LineAllocator->Context,
                NumberOfLines,
                sizeof(STRING),
                Timestamp
            )
        );

        if (!SourceCode->Lines) {
            goto Error;
        }

        BitIndex = 0;
        LastBitIndex = 0;
        FindSetBits = Rtl->RtlFindSetBits;
        Line = SourceCode->Lines;
        LineEndings = SourceCode->LineEndingBitmap;
        for (Index = 0; Index < NumberOfLines; Index++) {
            BitIndex = FindSetBits(LineEndings, 1, LastBitIndex);
            if (BitIndex == BITS_NOT_FOUND || BitIndex < LastBitIndex) {
                break;
            }
            Line->Length = (USHORT)(BitIndex - (LastBitIndex-1));
            Line->MaximumLength = Line->Length;
            Line->Buffer = (PCHAR)(DestContent + LastBitIndex);
            LastBitIndex = BitIndex + 1;
            Line++;
        }
    }

    Success = TRUE;

    goto End;

Error:
    Success = FALSE;

End:
    if (CryptHashMD5) {
        CryptDestroyHash(CryptHashMD5);
        CryptHashMD5 = 0;
    }

    if (CryptHashSHA1) {
        CryptDestroyHash(CryptHashSHA1);
        CryptHashSHA1 = 0;
    }

    if (SourceContent) {
        UnmapViewOfFile(SourceContent);
        SourceContent = NULL;
    }

    if (MappingHandle) {
        CloseHandle(MappingHandle);
        MappingHandle = NULL;
    }

    if (FileHandle) {
        CloseHandle(FileHandle);
        FileHandle = NULL;
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
