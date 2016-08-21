
#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

//
// We use a static, constant, initialized UNICODE_STRING to represent the
// format of the trace session directory, where each character represents
// a 1:1 map to the final string, solely for getting the required string
// lengths.  (i.e. the format string is not used in any way other than
// having its Length/MaximumLength fields queried.)
//

static CONST UNICODE_STRING TraceSessionDirectoryExampleFormat = \
    RTL_CONSTANT_STRING(L"YYYY-MM-DD-d_hhmmss.SSS");

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

_Use_decl_annotations_
BOOLEAN
CreateTraceSessionDirectory(
    PTRACER_CONFIG TracerConfig,
    PUNICODE_STRING DirectoryPointer
    )
{

    PALLOCATOR Allocator;
    PUNICODE_STRING Directory;

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
    // Update the length if it hasn't been set.
    //
    
    if (!TracerConfig->TraceSessionDirectoryBufferSizeInBytes) {
        USHORT Size;

        if (!GetTraceSessionDirectoryBufferSizeInBytes((TracerConfig, &Size))) {
            return FALSE;
        }

        TracerConfig->TraceSessionDirectoryBufferSizeInBytes = Size;
        //
        // xxx todo
        //
    }

    //
    // Get the system time.
    //

    //
    // Convert into TIME_FIELDS.
    //

    //
    // Compose a directory name out of the fields.
    //





    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
