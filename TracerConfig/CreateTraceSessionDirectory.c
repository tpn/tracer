
#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

#include "RtlString.h"

//
// We use RtlIntegerToUnicodeString() for converting the SYSTEMTIME
// fields into string representations for the trace session directory.
// RtlInsertEntryHashTable() is used for adding trace session directories
// to the hash table maintained by tracer config.
//

__declspec(dllimport)
NTSTATUS RtlInt64ToUnicodeString(
    _In_     ULONGLONG       Value,
    _In_opt_ ULONG           Base,
    _Inout_  PUNICODE_STRING String
);

__declspec(dllimport)
int __stdcall vswprintf_s(
    wchar_t *buffer,
    size_t numberOfElements,
    const wchar_t *format,
    ...
);

NTSTATUS 
__stdcall
RtlUnicodeStringPrintf(
    _Out_ PUNICODE_STRING  DestinationString,
    _In_  PCWSTR pszFormat,
    ...
);

NTSTATUS
__stdcall
RtlStringCchVPrintfWorkerW(
    _Out_ LPWSTR  pszDest,
    _In_  size_t  cchDest,
    _In_  LPCWSTR pszFormat,
    ...
);


RTL_INSERT_ENTRY_HASH_TABLE RtlInsertEntryHashTable;

//
// We use a static, constant, initialized UNICODE_STRING to represent the
// format of the trace session directory, where each character represents
// a 1:1 map to the final string, solely for getting the required string
// lengths.  (i.e. the format string is not used in any way other than
// having its Length/MaximumLength fields queried.)
//
#define TRACE_SESSION_DIRECTORY_EXAMPLE_FORMAT L"YYYY-MM-DD_hhmmss.SSS"

static CONST UNICODE_STRING TraceSessionDirectoryExampleFormat = \
    RTL_CONSTANT_STRING(TRACE_SESSION_DIRECTORY_EXAMPLE_FORMAT);

//
// Field widths used to determine if values should be zero padded.
//

static const USHORT FieldWidths[] = {
    4,  // YYYY
    2,  // MM
    2,  // DD
    2,  // hh
    2,  // mm
    2,  // ss
    3   // SSS
};

_Success_(return != 0)
BOOLEAN
GetTraceSessionDirectoryBufferSizeInBytes(
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PUSHORT BufferSizeInBytes
    )
/*++

Routine Description:

    Helper routine to calculate the size, in bytes, of a UNICODE_STRING's
    Buffer (i.e. the value of MaximumLength) required in order to store a
    fully-qualified trace session directory path name, based on the base
    trace directory name specified by TracerConfig.

Arguments:

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure.  The
        BaseTraceDirectory string field is used in calculating the final
        directory length.

    BufferSizeInBytes - Supplies a pointer to a USHORT that will receive
        the size of the buffer required, in bytes.  This value should be
        set to the MaximumLength field of the corresponding string.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT Size;
    PUNICODE_STRING Base;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(BufferSizeInBytes)) {
        return FALSE;
    }

    //
    // Verify the base trace directory.
    //

    Base = &TracerConfig->Paths.BaseTraceDirectory;

    if (!IsValidMinimumDirectoryNullTerminatedUnicodeString(Base)) {
        return FALSE;
    }

    //
    // Calculate the size in bytes.
    //

    Size = (

        //
        // Initial base directory length.
        //

        Base->Length +

        //
        // Joining slash.
        //

        sizeof(WCHAR) +

        //
        // Trace session directory name.  We use MaximumLength as we
        // want to include the trailing NULL.
        //

        TraceSessionDirectoryExampleFormat.MaximumLength

    );

    //
    // Update the caller's pointer and return.
    //

    __try {
        *BufferSizeInBytes = Size;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    return TRUE;
}


#define ConvertTimeFieldToString(Value, Trail) \
    String->Length = 0; \
    if (RtlIntegerToUnicodeString(Value, 0, String)) { \
        goto Error; \
    }
    

BOOLEAN
CreateSystemTimeTraceSessionDirectoryName(
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PUNICODE_STRING DirectoryName
    )
/*++

Routine Description:

    Creates the directory name used by a trace session based on the current
    system time.  The format is "YYYY-MM-DD_hhmmss.SSS", which represents the
    year, month, day, hour, minute, second and millisecond respectively.

    Values will be 0-padded as necessary to achieve a fixed width.

Arguments:

    TracerConfig - Supplies a pointer to a TRACER_CONFIG structure.

    DirectoryName - Supplies a pointer to a UNICODE_STRING that has sufficient
        buffer space to store the directory name.
    
Return Value:

    TRUE on success, FALSE on failure (invalid arguments or improperly sized
    DirectoryName string).

--*/
{
    BOOL Separator;
    LONG Result;
    USHORT BytesRemaining;
    USHORT BytesRequired;
    NTSTATUS Status;
    USHORT Bytes;
    USHORT Count;
    PWCHAR Dest;
    PWCHAR Source;
    SYSTEMTIME SystemTime;
    UNICODE_STRING Field = RTL_CONSTANT_STRING(L"\0\0\0\0");
    UNICODE_STRING Name = \
        RTL_CONSTANT_STRING(TRACE_SESSION_DIRECTORY_EXAMPLE_FORMAT);

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(DirectoryName)) {
        return FALSE;
    }

    //
    // Ensure the buffer is sized appropriately.
    //

    BytesRequired = TraceSessionDirectoryExampleFormat.MaximumLength;

    BytesRemaining = (
        DirectoryName->MaximumLength -
        DirectoryName->Length
    );

    if (BytesRemaining < BytesRequired) {
        return FALSE;
    }

    //
    // Get the system time.
    //

    GetSystemTime(&SystemTime);

    //
    // Convert each field into the corresponding string representation.
    //

    Count = Name.Length >> 1;

    Status = RtlStringCchVPrintfWorkerW(
        Name.Buffer,
        Name.MaximumLength,
        L"%04d-%02d-%02d_%02d%02d%02d%02d%02d.%03d",
        SystemTime.wYear,
        SystemTime.wMonth,
        SystemTime.wDay,
        SystemTime.wHour,
        SystemTime.wMinute,
        SystemTime.wSecond,
        SystemTime.wMilliseconds
    );

    //Status = RtlUnicodeStringP

    Result = 0;
    if (Result != Count) {
        return FALSE;
    }

    //
    // Copy the buffer over.
    //

    Dest = (PWCHAR)(
        RtlOffsetToPointer(
            DirectoryName->Buffer,
            DirectoryName->Length
        )
    );
    __movsw(Dest, Name.Buffer, Count);

    //
    // Update the length.
    //

    DirectoryName->Length += Count;

    return TRUE;


    Name.Length = 0;
    Dest = Name.Buffer;
    Source = Field.Buffer;

    //ConvertTimeFieldToString(SystemTime.wYear, DirectoryName);
    Separator = TRUE;
    Field.Length = 0;
    if (!RtlInt64ToUnicodeString(SystemTime.wYear, 0, &Field)) {
        goto Error;
    }
    Bytes = Field.Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);
    if (Separator) {
        Bytes += 2;
        Count++;
    }
    Dest += Count;
    Name.Length += Bytes;
    
    Separator = TRUE;
    Field.Length = 0;
    if (!RtlInt64ToUnicodeString(SystemTime.wMonth, 0, &Field)) {
        goto Error;
    }
    Bytes = Field.Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);
    if (Separator) {
        Bytes += 2;
        Count++;
    }
    Dest += Count;
    Name.Length += Bytes;
    
    return FALSE;

Error:

    return FALSE;
}



_Use_decl_annotations_
BOOLEAN
CreateTraceSessionDirectory(
    PTRACER_CONFIG TracerConfig,
    PPUNICODE_STRING DirectoryPointer
    )
{
    BOOL Success;
    USHORT Attempts = 64;
    USHORT BufferSizeInBytes;
    ULONG Bytes;
    ULONG Count;
    ULONG LastError;
    ULONG Signature;
    PWCHAR Dest;
    PWCHAR Source;
    ULONG_INTEGER AllocationSize;
    PALLOCATOR Allocator;
    PUNICODE_STRING Directory;
    PUNICODE_STRING BaseDirectory;
    PTRACE_SESSION_DIRECTORY TraceSessionDirectory;
    PRTL_DYNAMIC_HASH_TABLE_ENTRY HashTableEntry;
    PRTL_DYNAMIC_HASH_TABLE HashTable;
    RTL_DYNAMIC_HASH_TABLE_CONTEXT HashTableContext;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(DirectoryPointer)) {
        return FALSE;
    }

    Allocator = TracerConfig->Allocator;

    if (!Allocator) {
        return FALSE;
    }

    //
    // Make sure the BaseTraceDirectory exists.
    //

    BaseDirectory = &TracerConfig->Paths.BaseTraceDirectory;
    Success = CreateDirectoryW(BaseDirectory->Buffer, NULL);
    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            return FALSE;
        }
    }

    //
    // Get the size of the buffer we need to allocate for the directory.
    //

    Success = GetTraceSessionDirectoryBufferSizeInBytes(
        TracerConfig,
        &BufferSizeInBytes
    );

    if (!Success) {
        return FALSE;
    }

    //
    // Add the size of the TRACE_SESSION_DIRECTORY struct.
    //

    AllocationSize.LongPart = (
        BufferSizeInBytes +
        sizeof(TRACE_SESSION_DIRECTORY)
    );

    //
    // Make sure we haven't overflowed.  (The total allocation size should
    // *never* be greater than a max USHORT.)
    //

    if (AllocationSize.HighPart != 0) {
        return FALSE;
    }

    //
    // Allocate the buffer.
    //

    __try {
        TraceSessionDirectory = (PTRACE_SESSION_DIRECTORY)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                AllocationSize.LowPart
            )
        );
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        TraceSessionDirectory = NULL;
    }

    if (!TraceSessionDirectory) {
        return FALSE;
    }

    //
    // Initialize the underlying UNICODE_STRING for the Directory.
    //

    Directory = &TraceSessionDirectory->Directory;
    Directory->MaximumLength = BufferSizeInBytes;
    Directory->Length = 0;

    //
    // Point the buffer at the memory after the TRACE_SESSION_DIRECTORY
    // struct we just allocated.
    //

    Directory->Buffer = (PWSTR)(
        RtlOffsetToPointer(
            TraceSessionDirectory,
            sizeof(TRACE_SESSION_DIRECTORY)
        )
    );

    //
    // Copy the base trace directory over.  Note we track bytes and count
    // (i.e. number of chars) separately; the lengths are in bytes but
    // __movsw() works a WORD (WCHAR) at a time.
    //

    Dest = Directory->Buffer;
    Source = BaseDirectory->Buffer;
    Bytes = BaseDirectory->Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);

    //
    // Add the joining slash;
    //

    Dest += Count;
    *Dest++ = L'\\';

    //
    // Update the length field with the new length.
    //

    Directory->Length = ((USHORT)Bytes) + sizeof(WCHAR);

    //
    // As our trace session directory name has a millisecond resolution (and
    // is based off the system time, which updates every ~16 milliseconds),
    // it's plausible (but unlikely) that two separate calls to this function
    // end up trying to create the same directory name.  So, wrap the logic up
    // in a loop and make a few attempts if we get a 'directory already exists'
    // error when trying to create the directory.
    //

    do {

        //
        // Get the trace session directory name, appending it to the existing
        // base trace directory name.
        //

        Success = CreateSystemTimeTraceSessionDirectoryName(
            TracerConfig,
            Directory
        );

        if (!Success) {
            goto Error;
        }

        //
        // Attempt to create the directory.
        //

        Success = CreateDirectoryW(Directory->Buffer, NULL);

        if (Success) {
            break;
        }

        //
        // Failed to create the directory.  If the error was anything other
        // than 'directory already exists', exit the loop.
        //

        LastError = GetLastError();
        if (LastError != ERROR_ALREADY_EXISTS) {
            goto Error;
        }

        //
        // Reset the Directory string back to the base trace directory in
        // preparation for CreateSystemTimeTraceSessionDirectoryName() being
        // called again at the top of the loop.
        // 

        Directory->Length = BaseDirectory->Length;

    } while (--Attempts);

    //
    // The trace session directory name (and backing directory) were
    // successfully created.  Add the directory to the hash table of trace
    // session directories.
    //

    Signature = Directory->Hash = HashUnicodeStringToAtom(Directory);
    HashTableEntry = &TraceSessionDirectory->HashTableEntry;
    HashTableEntry->Signature = Directory->Hash;
    HashTable = &TracerConfig->TraceSessionDirectories.HashTable;

    InitializeListHead(&HashTableEntry->Linkage);
    RtlInitHashTableContext(&HashTableContext);

    Success = RtlInsertEntryHashTable(
        HashTable,
        HashTableEntry,
        (ULONG_PTR)&Signature,
        &HashTableContext
    );

    if (!Success) {
        goto Error;
    }

    //
    // We're done, goto end.
    //

    goto End;

Error:

    //
    // Free the allocated TraceSessionDirectory structure if necessary.
    //

    if (TraceSessionDirectory) {
        Allocator->Free(
            Allocator->Context,
            TraceSessionDirectory
        );
        TraceSessionDirectory = NULL;
    }

    //
    // Mark Success as FALSE and clear the Directory pointer.
    //

    Success = FALSE;
    Directory = NULL;

    //
    // Intentional follow-on to End.
    //

End:

    if (Directory) {

        //
        // Update the user's pointer.
        //

        *DirectoryPointer = Directory;

    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
