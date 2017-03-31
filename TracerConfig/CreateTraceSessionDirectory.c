#include "stdafx.h"

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

    *BufferSizeInBytes = Size;

    return TRUE;
}

_Success_(return != 0)
BOOLEAN
CreateSystemTimeTraceSessionDirectoryName(
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PUNICODE_STRING DirectoryName,
    _In_ PSYSTEMTIME SystemTime
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

    SystemTime - Supplies a pointer to a SYSTEMTIME structure that is used to
        build the directory name.

Return Value:

    TRUE on success, FALSE on failure (invalid arguments or improperly sized
    DirectoryName string).

--*/
{
    USHORT BytesRemaining;
    USHORT BytesRequired;
    ULONG Value;
    PUNICODE_STRING Name;

    UNREFERENCED_PARAMETER(TracerConfig);

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(DirectoryName)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SystemTime)) {
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
    // Convert each field into the corresponding string representation.
    //

    Name = DirectoryName;

#define AppendTimeField(Field, Digits, Trailer)                        \
    Value = SystemTime->Field;                                         \
    if (!AppendIntegerToUnicodeString(Name, Value, Digits, Trailer)) { \
        goto Error;                                                    \
    }

    AppendTimeField(wYear,          4, L'-');
    AppendTimeField(wMonth,         2, L'-');
    AppendTimeField(wDay,           2, L'-');
    AppendTimeField(wHour,          2,    0);
    AppendTimeField(wMinute,        2,    0);
    AppendTimeField(wSecond,        2, L'.');
    AppendTimeField(wMilliseconds,  3,    0);

    return TRUE;

Error:

    return FALSE;
}


_Use_decl_annotations_
BOOLEAN
CreateTraceSessionDirectory(
    PTRACER_CONFIG TracerConfig,
    PPUNICODE_STRING DirectoryPointer,
    PSYSTEMTIME SystemTime
    )
{
    BOOLEAN Success;
    USHORT Attempts = 64;
    USHORT BufferSizeInBytes;
    ULONG Bytes;
    ULONG Count;
    ULONG LastError;
    PWCHAR Dest;
    PWCHAR Source;
    ULONG_INTEGER AllocationSize;
    PALLOCATOR Allocator;
    PUNICODE_STRING Directory;
    PUNICODE_STRING BaseDirectory;
    PTRACE_SESSION_DIRECTORY TraceSessionDirectory;
    PTRACE_SESSION_DIRECTORIES Directories;

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

    TraceSessionDirectory = (PTRACE_SESSION_DIRECTORY)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocationSize.LowPart
        )
    );

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
        // Get the system time.
        //

        GetSystemTime(SystemTime);

        //
        // Get the trace session directory name using the system time we just
        // obtained, appending it to the existing base trace directory name.
        //

        Success = CreateSystemTimeTraceSessionDirectoryName(
            TracerConfig,
            Directory,
            SystemTime
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
    // The trace session directory name and backing directory were created
    // successfully.  Add to the trace session directories list.
    //

    Directories = &TracerConfig->TraceSessionDirectories;

    InitializeListHead(&TraceSessionDirectory->ListEntry);

    AcquireSRWLockExclusive(&Directories->Lock);
    InterlockedIncrement(&Directories->Count);
    AppendTailList(&Directories->ListHead, &TraceSessionDirectory->ListEntry);
    ReleaseSRWLockExclusive(&Directories->Lock);

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
