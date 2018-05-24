/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    SelfTestPerfectHashTable.c

Abstract:

    This module implements the self-test routine for the PerfectHashTable
    component.  It is responsible for end-to-end testing of the entire
    component with all known test data from a single function entry point
    (SelfTestPerfectHashTable()).

--*/

#include "stdafx.h"
#include "PerfectHashTableTestData.h"

_Use_decl_annotations_
BOOLEAN
SelfTestPerfectHashTable(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PPERFECT_HASH_TABLE_API_EX Api,
    PPERFECT_HASH_TABLE_TEST_DATA TestData,
    PCUNICODE_STRING TestDataDirectory
    )
/*++

Routine Description:

    Performs a self-test of the entire PerfectHashTable component.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure that
        will be used for all memory allocations.

    Api - Supplies a pointer to an initialized PERFECT_HASH_TABLE_API_EX
        structure.

    TestData - Supplies a pointer to an initialized PERFECT_HASH_TABLE_TEST_DATA
        structure.

    TestDataDirectory - Supplies a pointer to a UNICODE_STRING structure that
        represents a fully-qualifed path of the test data directory.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PWSTR Dest;
    PWSTR Source;
    USHORT Length;
    USHORT BaseLength;
    BOOLEAN Success;
    PWCHAR Buffer;
    PWCHAR BaseBuffer;
    PWCHAR WideOutput;
    PWCHAR WideOutputBuffer;
    HANDLE FindHandle = NULL;
    HANDLE WideOutputHandle;
    HANDLE ProcessHandle = NULL;
    ULONG Failures;
    ULONG BytesWritten;
    ULONG WideCharsWritten;
    ULONGLONG BufferSize;
    ULONGLONG WideOutputBufferSize;
    LONG_INTEGER AllocSize;
    LARGE_INTEGER BytesToWrite;
    LARGE_INTEGER WideCharsToWrite;
    WIN32_FIND_DATAW FindData;
    UNICODE_STRING SearchPath;
    UNICODE_STRING KeysPath;
    PPERFECT_HASH_TABLE PerfectHashTable;
    PPERFECT_HASH_TABLE_KEYS Keys;
    PERFECT_HASH_TABLE_KEYS_LOAD_FLAGS KeysLoadFlags;
    UNICODE_STRING Suffix = RTL_CONSTANT_STRING(L"*.keys");

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Api)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TestData)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TestDataDirectory)) {
        return FALSE;
    }

    if (!IsValidMinimumDirectoryUnicodeString(TestDataDirectory)) {
        return FALSE;
    }

    //
    // Arguments have been validated, proceed.
    //

    //
    // Create a buffer we can use for stdout.
    //

    Success = Rtl->CreateBuffer(Rtl,
                                &ProcessHandle,
                                10,
                                0,
                                &WideOutputBufferSize,
                                &WideOutputBuffer);

    if (!Success) {
        return FALSE;
    }

    WideOutput = WideOutputBuffer;

    //
    // Create a buffer we can use for temporary path construction.  We want it
    // to be MAX_USHORT in size, so (1 << 16) >> PAGE_SHIFT converts this into
    // the number of pages we need.
    //

    Success = Rtl->CreateBuffer(Rtl,
                                &ProcessHandle,
                                (1 << 16) >> PAGE_SHIFT,
                                0,
                                &BufferSize,
                                &BaseBuffer);

    if (!Success) {
        return FALSE;
    }

    Buffer = BaseBuffer;

    //
    // Get a reference to the stdout handle.
    //

    WideOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    ASSERT(WideOutputHandle);

    //
    // Calculate the size required for a new concatenated wide string buffer
    // that combines the test data directory with the "*.keys" suffix.  The
    // 2 * sizeof(*Dest) accounts for the joining slash and trailing NULL.
    //

    AllocSize.LongPart = TestDataDirectory->Length;
    AllocSize.LongPart += Suffix.Length + (2 * sizeof(*Dest));

    ASSERT(!AllocSize.HighPart);

    SearchPath.Buffer = (PWSTR)Buffer;

    if (!SearchPath.Buffer) {
        goto Error;
    }

    //
    // Copy incoming test data directory name.
    //

    Length = TestDataDirectory->Length;
    CopyMemory(SearchPath.Buffer,
               TestDataDirectory->Buffer,
               Length);

    //
    // Advance our Dest pointer to the end of the directory name, write a
    // slash, then copy the suffix over.
    //

    Dest = (PWSTR)RtlOffsetToPointer(SearchPath.Buffer, Length);
    *Dest++ = L'\\';
    CopyMemory(Dest, Suffix.Buffer, Suffix.Length);

    //
    // Wire up the search path length and maximum length variables.  The max
    // length will be our AllocSize, length will be this value minus 2 to
    // account for the trailing NULL.
    //

    SearchPath.MaximumLength = AllocSize.LowPart;
    SearchPath.Length = AllocSize.LowPart - sizeof(*Dest);
    ASSERT(SearchPath.Buffer[SearchPath.Length] == L'\0');

    //
    // Advance the buffer past this string allocation, up to the next 16-byte
    // boundary.
    //

    Buffer = (PWSTR)(
        RtlOffsetToPointer(
            Buffer,
            ALIGN_UP(SearchPath.MaximumLength, 16)
        )
    );

    WIDE_OUTPUT_RAW(WideOutput,
                    L"Starting perfect hash self-test for directory: ");
    WIDE_OUTPUT_UNICODE_STRING(WideOutput, TestDataDirectory);
    WIDE_OUTPUT_RAW(WideOutput, L".\n");
    WIDE_OUTPUT_FLUSH();

    FindHandle = FindFirstFileW(SearchPath.Buffer, &FindData);
    if (!FindHandle || FindHandle == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            WIDE_OUTPUT_RAW(WideOutput,
                            L"No files matching pattern '*.keys' found in "
                            "test data directory.\n");
            goto End;
        } else {
            goto Error;
        }
    }

    //
    // Initialize the fully-qualified keys path.
    //

    KeysPath.Buffer = Buffer;
    CopyMemory(KeysPath.Buffer, TestDataDirectory->Buffer, Length);

    //
    // Advance our Dest pointer to the end of the directory name, then write
    // a slash.
    //

    Dest = (PWSTR)RtlOffsetToPointer(KeysPath.Buffer, Length);
    *Dest++ = L'\\';

    //
    // Update the length to account for the slash we just wrote, then make a
    // copy of it in the variable BaseLength.
    //

    Length += sizeof(*Dest);
    BaseLength = Length;

    //
    // Clear the keys load flags and failure count.
    //

    KeysLoadFlags.AsULong = 0;
    Failures = 0;

    do {

        WIDE_OUTPUT_RAW(WideOutput, L"Processing key file: ");
        WIDE_OUTPUT_WCSTR(WideOutput, (PCWSZ)FindData.cFileName);
        WIDE_OUTPUT_LF(WideOutput);
        WIDE_OUTPUT_FLUSH();

        //
        // Copy the filename over to the fully-qualified keys path.
        //

        Dest = (PWSTR)RtlOffsetToPointer(KeysPath.Buffer, BaseLength);
        Source = (PWSTR)FindData.cFileName;

        while (*Source) {
            *Dest++ = *Source++;
        }
        *Dest = L'\0';

        Length = (USHORT)RtlOffsetFromPointer(Dest, KeysPath.Buffer);
        KeysPath.Length = Length;
        KeysPath.MaximumLength = Length + sizeof(*Dest);
        ASSERT(KeysPath.Buffer[KeysPath.Length >> 1] == L'\0');
        ASSERT(&KeysPath.Buffer[KeysPath.Length >> 1] == Dest);

        Success = Api->LoadPerfectHashTableKeys(Rtl,
                                                Allocator,
                                                KeysLoadFlags,
                                                &KeysPath,
                                                &Keys);

        if (!Success) {

            WIDE_OUTPUT_RAW(WideOutput, L"Failed to load keys for ");
            WIDE_OUTPUT_UNICODE_STRING(WideOutput, &KeysPath);
            WIDE_OUTPUT_RAW(WideOutput, L".\n");
            WIDE_OUTPUT_FLUSH();

            Failures++;
            continue;
        }

        Success = Api->DestroyPerfectHashTableKeys(&Keys);
        if (!Success) {

            WIDE_OUTPUT_RAW(WideOutput, L"Failed to destroy keys for ");
            WIDE_OUTPUT_UNICODE_STRING(WideOutput, &KeysPath);
            WIDE_OUTPUT_RAW(WideOutput, L".\n");
            WIDE_OUTPUT_FLUSH();

            Failures++;
            continue;
        }
    } while (FindNextFile(FindHandle, &FindData));

    PerfectHashTable = NULL;

    //
    // Self test complete!
    //

    if (!Failures) {
        Success = TRUE;
        goto End;
    }

    //
    // Intentional follow-on to Error.
    //

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    //
    // We can't do much if any of these routines error out, hence the NOTHINGs.
    //

    if (WideOutputBuffer) {
        if (!Rtl->DestroyBuffer(Rtl, ProcessHandle, &WideOutputBuffer)) {
            NOTHING;
        }
        WideOutput = NULL;
    }

    if (BaseBuffer) {
        if (!Rtl->DestroyBuffer(Rtl, ProcessHandle, &BaseBuffer)) {
            NOTHING;
        }
        Buffer = NULL;
    }

    if (FindHandle) {
        if (!FindClose(FindHandle)) {
            NOTHING;
        }
        FindHandle = NULL;
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
