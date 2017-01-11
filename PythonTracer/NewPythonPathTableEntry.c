/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    NewPythonPathTableEntry.c

Abstract:

    This module implements routines related to processing new Python path table
    entry structures.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
NewPythonPathTableEntry(
    PPYTHON_TRACE_CONTEXT Context,
    PPYTHON_PATH_TABLE_ENTRY Entry
    )
/*++

Routine Description:

    This routine is responsible for handling new Python path table entry
    structures.  It is called via threadpool worker threads.  It is responsible
    for doing the following:

        1.  Converting the STRING path to a UNICODE_STRING.
        2.  Initializing the RTL_PATH structure.
        3.  Loading the file and creating a memory map.
        4.  Saving file details to the underlying RTL_PATH structure.
        5.  Calculating MD5 and SHA1 checksums from the file contents.
        6.  Dispatching either source code or image file specific processing.

Arguments:

    Context - Supplies a pointer to a PYTHON_TRACE_CONTEXT structure.

    Entry - Supplies a pointer to the new PYTHON_PATH_TABLE_ENTRY structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    BOOL Success;
    BOOL IsSourceCode;
    BYTE Inner;
    ULONG Index;
    PCHAR SourceContent = NULL;
    PCHAR DestContent;
    PCHAR Dest;
    PCHAR Source;
    PCHAR BitmapBuffers;
    DWORD LastError;
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
    LARGE_INTEGER Timestamp;
    LARGE_INTEGER StartCopy;
    LARGE_INTEGER EndCopy;
    LARGE_INTEGER Elapsed;
    PRTL_TEXT_FILE SourceCode;
    PTRACE_STORE LineStore;
    PTRACE_STORE ContentsStore;
    PTRACE_STORE ImageFileStore;
    PTRACE_STORE SourceCodeStore;
    PTRACE_STORES TraceStores;
    FILE_STANDARD_INFO FileInfo;
    PALLOCATOR BitmapAllocator;
    PALLOCATOR UnicodeStringBufferAllocator;
    BY_HANDLE_FILE_INFORMATION HandleInfo;

    YMMWORD Ymm1;
    YMMWORD Ymm2;
    YMMWORD Ymm3;
    YMMWORD Ymm4;

    YMMWORD Ymm5;
    YMMWORD Ymm6;
    YMMWORD Ymm7;
    YMMWORD Ymm8;

    PYMMWORD DestYmm;
    PYMMWORD SourceYmm;

    //
    // Initialize aliases.
    //

    Rtl = Context->Rtl;
    File = &Entry->File;
    Path = &File->Path;
    IsSourceCode = !Entry->IsDll;
    TraceStores = Context->TraceContext->TraceStores;
    BitmapAllocator = &Context->BitmapAllocator;
    UnicodeStringBufferAllocator = &Context->UnicodeStringBufferAllocator;

    //
    // Capture a timestamp.
    //

    QueryPerformanceCounter(&Timestamp);

    //
    // Update the RTL_PATH structure.
    //

    Success = Rtl->StringToExistingRtlPath(Rtl,
                                           &Entry->Path,
                                           BitmapAllocator,
                                           UnicodeStringBufferAllocator,
                                           Path,
                                           &Timestamp);
    if (!Success) {
        goto Error;
    }

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

    BitmapBufferSize = File->EndOfFile.QuadPart >> 3;
    AlignedBitmapBufferSize = ALIGN_UP(BitmapBufferSize, 32);
    NumberOfBitmaps = NUMBER_OF_RTL_TEXT_FILE_BITMAPS;

    SingleBitmapAllocSize = sizeof(RTL_BITMAP) + AlignedBitmapBufferSize;
    TotalBitmapAllocSize = SingleBitmapAllocSize * NumberOfBitmaps;

    BitmapBuffers = NULL;
    BitmapAllocationAttempt = 0;

#define TRY_ALLOC_BITMAP_BUFFERS() do {                          \
    if (IsSourceCode && !BitmapBuffers) {                        \
        BitmapAllocationAttempt++;                               \
        BitmapBuffers = BitmapAllocator->TryCallocWithTimestamp( \
            BitmapAllocator->Context,                            \
            NumberOfBitmaps,                                     \
            SingleBitmapAllocSize,                               \
            &Timestamp                                           \
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
    //File->VolumeSerialNumber = HandleInfo.dwVolumeSerialNumber;

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

    //
    // Resolve the trace stores we need.
    //

    ImageFileStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreImageFileId
        )
    );

    SourceCodeStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreSourceCodeId
        )
    );

    LineStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreLineId
        )
    );

    //
    // Determine which store to use based on whether or not this is a DLL.
    //

    if (Entry->IsDll) {
        ContentsStore = ImageFileStore;
    } else {
        ContentsStore = SourceCodeStore;
    }

    TRY_ALLOC_BITMAP_BUFFERS();

    //
    // Allocate space for the file.
    //

    DestContent = (PCHAR)(
        ContentsStore->AllocateRecordsWithTimestamp(
            ContentsStore->TraceContext,
            ContentsStore,
            1,
            File->EndOfFile.QuadPart,
            &Timestamp
        )
    );

    if (!DestContent) {
        goto Error;
    }

    //
    // Get the number of pages for the contents.
    //

    File->NumberOfPages = (
        ROUND_TO_PAGES(File->EndOfFile.QuadPart) >> PAGE_SHIFT
    );

    //
    // Copy a page at a time using streaming loads.
    //

    Dest = DestContent;
    Source = SourceContent;
    QueryPerformanceCounter(&StartCopy);

    goto AvxCopy;

    File->Flags.MovsqCopy = TRUE;

    __movsq((PDWORD64)DestContent,
            (PDWORD64)SourceContent,
            File->AllocationSize.QuadPart >> 3);

    goto CopyComplete;

AvxCopy:

    DestYmm = (PYMMWORD)Dest;
    SourceYmm = (PYMMWORD)Source;
    File->Flags.Avx2Copy = TRUE;

    for (Index = 0; Index < File->NumberOfPages; Index++) {

        //
        // Each loop iteration copies 256 bytes.  We repeat it 16 times to copy
        // the entire 4096 byte page.
        //

        for (Inner = 0; Inner < 16; Inner++) {

            Ymm1 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm2 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm1);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm2);

            Ymm3 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm4 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm3);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm4);

            //
            // (128 bytes copied.)
            //

            Ymm5 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm6 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm5);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm6);

            Ymm7 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);
            Ymm8 = _mm256_stream_load_si256((PYMMWORD)SourceYmm++);

            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm7);
            _mm256_store_si256((PYMMWORD)DestYmm++, Ymm8);

            //
            // (256 bytes copied.)
            //
        }
    }

CopyComplete:

    QueryPerformanceCounter(&EndCopy);

    Elapsed.QuadPart = EndCopy.QuadPart - StartCopy.QuadPart;
    File->Elapsed = Elapsed.LowPart;
    File->Content = DestContent;

    //
    // Convert to microseconds and then calculate bytes per second.
    //

    Elapsed.QuadPart *= Context->Multiplicand.QuadPart;
    Elapsed.QuadPart /= Context->Frequency.QuadPart;
    File->CopyTimeInMicroseconds.QuadPart = Elapsed.QuadPart;
    if (Elapsed.QuadPart > 0) {
        File->CopiedBytesPerSecond.QuadPart = (
            (File->AllocationSize.QuadPart /
             Elapsed.QuadPart) * SECONDS_TO_MICROSECONDS
        );
    }

    //
    // We can unmap the file and close all handles now.
    //

    UnmapViewOfFile(SourceContent);
    CloseHandle(MappingHandle);
    CloseHandle(FileHandle);

    SourceContent = NULL;
    MappingHandle = NULL;
    FileHandle = NULL;

    //
    // Initialize the crypt provider for generating MD5/SHA1 hashes.
    //

    Success = CryptAcquireContextW(&CryptProv,
                                   NULL,
                                   NULL,
                                   PROV_RSA_FULL,
                                   CRYPT_VERIFYCONTEXT);

    if (!Success) {
        LastError = GetLastError();
        Context->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    //
    // Create the MD5 and SHA1 hashes.
    //

    Success = CryptCreateHash(CryptProv, CALG_MD5, 0, 0, &CryptHashMD5);
    if (!Success) {
        LastError = GetLastError();
        Context->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    Success = CryptCreateHash(CryptProv, CALG_SHA1, 0, 0, &CryptHashSHA1);
    if (!Success) {
        LastError = GetLastError();
        Context->LastError = LastError;
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
        Context->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    Success = CryptHashData(CryptHashSHA1,
                            DestContent,
                            File->EndOfFile.LowPart,
                            0);

    if (!Success) {
        LastError = GetLastError();
        Context->LastError = LastError;
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
        Context->LastError = LastError;
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
        Context->LastError = LastError;
        __debugbreak();
        goto Error;
    }

    if (IsSourceCode) {
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
                &Timestamp
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
            LineStore->AllocateRecordsWithTimestamp(
                LineStore->TraceContext,
                LineStore,
                NumberOfLines,
                sizeof(STRING),
                &Timestamp
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
    if (CryptProv) {
        CryptReleaseContext(CryptProv, 0);
        CryptProv = 0;
    }

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
