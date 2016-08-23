#include "TracerConfig.h"
#include "TracerConfigPrivate.h"
#include "TracerConfigConstants.h"

#include "../Rtl/Rtl.h"

#include <winreg.h>

/*--

Routine Description:

    A helper routine for initializing TRACER_CONFIG Path variables.  This
    calculates the required UNICODE_STRING Buffer length to hold the full
    DLL path, which will be a concatenation of the InstallationDirectory,
    IntermediatePath (i.e. "x64\\Release") and the final DLL name, including
    all joining slashes and trailing NULL.  The TracerConfig's Allocator is
    used to allocate sufficient space, then the three strings are copied over.

Arguments:

    Index - Supplies the 0-based index into the PathOffsets[] array which
        is used to resolve a pointer to the UNICODE_STRING variable in the
        TRACER_PATHS structure, as well as a pointer to the UNICODE_STRING
        for the relevant DLL suffix for that variable.

Return Value:

    TRUE on success, FALSE on FAILURE.

--*/
_Success_(return != 0)
BOOLEAN
LoadPath(
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ USHORT Index
    )
{
    USHORT Length;
    USHORT MaximumLength;
    USHORT SizeInBytes;
    USHORT Offset;
    ULONG Bytes;
    ULONG Count;
    TRACER_FLAGS Flags;
    PWCHAR Dest;
    PWCHAR Source;
    PTRACER_PATHS Paths;
    PALLOCATOR Allocator;
    PCUNICODE_STRING DllPath;
    PUNICODE_STRING TargetPath;
    PUNICODE_STRING InstallationDir;
    PUNICODE_STRING IntermediatePath;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (Index >= NumberOfPathOffsets) {
        return FALSE;
    }

    //
    // Initialize various local variables.
    //

    Flags = TracerConfig->Flags;
    Paths = &TracerConfig->Paths;
    Offset = PathOffsets[Index].Offset;
    Allocator = TracerConfig->Allocator;

    //
    // Initialize paths.
    //

    DllPath = PathOffsets[Index].DllPath;
    TargetPath = (PUNICODE_STRING)((((ULONG_PTR)Paths) + Offset));
    InstallationDir = &Paths->InstallationDirectory;
    IntermediatePath = IntermediatePaths[Flags.LoadDebugLibraries];

    //
    // Calculate the length of the final joined path.  IntermediatePath
    // will have leading and trailing slashes.
    //

    Length = (
        InstallationDir->Length +
        IntermediatePath->Length +
        DllPath->Length
    );

    //
    // Account for the trailing NULL.
    //

    MaximumLength = Length + sizeof(WCHAR);

    //
    // This is our allocation size in bytes.
    //

    SizeInBytes = (USHORT)MaximumLength;

    //
    // Allocate space for the buffer.
    //

    __try {
        TargetPath->Buffer = (PWCHAR)(
            Allocator->Malloc(
                Allocator->Context,
                SizeInBytes
            )
        );
        if (!TargetPath->Buffer) {
            return FALSE;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    //
    // Copy the installation directory.  Note we track bytes and count
    // (i.e. number of chars) separately; the lengths are in bytes but
    // __movsw() works a WORD (WCHAR) at a time.
    //

    Dest = TargetPath->Buffer;
    Source = InstallationDir->Buffer;
    Bytes = InstallationDir->Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);

    //
    // Copy the intermediate bit of the path (e.g. "\\x64\\Debug\\").

    Dest += Count;
    Source = IntermediatePath->Buffer;
    Bytes = IntermediatePath->Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);

    //
    // Copy the final .dll path name (e.g. "Rtl.dll").
    //

    Dest += Count;
    Source = DllPath->Buffer;
    Bytes = DllPath->Length;
    Count = Bytes >> 1;
    __movsw(Dest, Source, Count);

    //
    // And set the final trailing NULL.
    //

    Dest += Count;
    *Dest = L'\0';

    //
    // Update the lengths on the string and return success.
    //

    TargetPath->Length = Length;
    TargetPath->MaximumLength = MaximumLength;

    return TRUE;
}

//
// Helper macro for reading REG_DWORD values from the registry into a local
// TRACER_FLAGS Flags structure.  Name is the name of the flag, and Default
// is the name of the default value if the registry key couldn't be read.
//

#define READ_REG_DWORD_FLAG(Name, Default) do { \
    ULONG Value;                                \
    ULONG ValueLength = sizeof(Value);          \
    Result = RegGetValueW(                      \
        RegistryKey,                            \
        NULL,                                   \
        L#Name,                                 \
        RRF_RT_REG_DWORD,                       \
        NULL,                                   \
        (PVOID)&Value,                          \
        &ValueLength                            \
    );                                          \
    if (Result == ERROR_SUCCESS) {              \
        Flags.Name = Value;                     \
    } else {                                    \
        Flags.Name = Default;                   \
    }                                           \
} while (0)

//
// The minimum number of bytes for a valid WCHAR path name, including
// the terminating NULL, is 10:
//      length of L"C:\a" (4 * 2 = 8) + NUL (1 * 2) = 10
//

#define MINIMUM_PATH_SIZE_IN_BYTES 10

//
// Set maximum path size at (a somewhat arbitrary) 2048 bytes.
//

#define MAXIMUM_PATH_SIZE_IN_BYTES 2048

/*--

Macro Description:

    Helper macro for reading REG_SZ path values.  The size of the string
    is obtained first, then an attempt is made to allocate a sufficiently-
    sized buffer using the TracerConfig's Allocator.  If this succeeds,
    a second call to RegGetValueW() is called with the new buffer, and
    the rest of the UNICODE_STRING is initialized (i.e. MaximumLength and
    Length are set).

    If any errors occur, the Error handler is jumped to.

Arguments:

    Name - Name of the path in the TRACER_PATHS structure.

Return Value:

    N/A.

--*/
#define READ_REG_SZ_PATH(Name) do {                          \
    BOOL IsValid;                                            \
    ULONG SizeInBytes = 0;                                   \
    PUNICODE_STRING String = &Paths->Name;                   \
                                                             \
    Result = RegGetValueW(                                   \
        RegistryKey,                                         \
        NULL,                                                \
        L#Name,                                              \
        RRF_RT_REG_SZ,                                       \
        NULL,                                                \
        NULL,                                                \
        &SizeInBytes                                         \
    );                                                       \
                                                             \
    IsValid = (                                              \
        Result == ERROR_SUCCESS &&                           \
        SizeInBytes >= MINIMUM_PATH_SIZE_IN_BYTES &&         \
        SizeInBytes <= MAXIMUM_PATH_SIZE_IN_BYTES            \
    );                                                       \
                                                             \
    if (!IsValid) {                                          \
        goto Error;                                          \
    }                                                        \
                                                             \
    __try {                                                  \
        String->Buffer = (PWCHAR)(                           \
            Allocator->Malloc(                               \
                Allocator->Context,                          \
                SizeInBytes                                  \
            )                                                \
        );                                                   \
        if (!String->Buffer) {                               \
            goto Error;                                      \
        }                                                    \
    } __except (EXCEPTION_EXECUTE_HANDLER) {                 \
        goto Error;                                          \
    }                                                        \
                                                             \
    Result = RegGetValueW(                                   \
        RegistryKey,                                         \
        NULL,                                                \
        L#Name,                                              \
        RRF_RT_REG_SZ,                                       \
        NULL,                                                \
        (PVOID)String->Buffer,                               \
        &SizeInBytes                                         \
    );                                                       \
                                                             \
    if (Result != ERROR_SUCCESS) {                           \
        goto Error;                                          \
    }                                                        \
                                                             \
    String->Length =  (USHORT)(SizeInBytes - sizeof(WCHAR)); \
    String->MaximumLength = (USHORT)SizeInBytes;             \
                                                             \
} while (0)



_Use_decl_annotations_
PTRACER_CONFIG
InitializeTracerConfig(
    PALLOCATOR Allocator,
    PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Initializes a TRACER_CONFIG structure based on the values in the registry
    path pointed to by RegistryPath.  This defaults to Software\Tracer.  The
    keys InstallationDirectory and BaseTraceDirectory (both REG_SZ) must be set
    to fully-qualified path names.  All other keys are optional, and default
    values are provided if they are not present.

    In general, the registry key names map 1:1 with the corresponding fields
    in the TRACER_CONFIG structure.

    The Allocator will be used for all memory allocations. DestroyTracerConfig()
    must be called against the returned PTRACER_CONFIG when the structure is no
    longer needed in order to ensure resources are released.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to manage the structure's memory.  This includes initial
        allocation of bytes to hold the structure, plus space for each
        UNICODE_STRING's Buffer in each of the paths in TRACER_PATHS.

    RegistryPath - Supplies a pointer to a fully-qualified UNICODE_STRING
        to the primary registry path that will be used to load tracer
        configuration information.  (Note that this string must be NULL
        terminated, as the underlying Buffer is passed directly to the
        RegCreateKeyExW() function, which expects an LPWSTR.)

Return Value:

    A pointer to a valid TRACER_CONFIG structure on success, NULL on failure.
    Call DestroyTracerConfig() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    USHORT Index;
    TRACER_FLAGS Flags;
    ULONG Result;
    ULONG Disposition;
    HKEY RegistryKey;
    PTRACER_CONFIG TracerConfig;
    PTRACER_PATHS Paths;
    PTRACE_SESSION_DIRECTORIES TraceSessionDirectories;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return NULL;
    }

    if (!ARGUMENT_PRESENT(RegistryPath)) {
        return NULL;
    }

    if (!IsValidNullTerminatedUnicodeString(RegistryPath)) {
        return NULL;
    }

    //
    // Arguments are valid, open or create the registry path.
    //

    Result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        RegistryPath->Buffer,
        0,          // Reserved
        NULL,       // Class
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &RegistryKey,
        &Disposition
    );

    if (Result != ERROR_SUCCESS) {
        return NULL;
    }

    //
    // N.B.: we've successfully opened a registry key, so all failures after
    //       this point should `goto Error` to ensure the key is closed before
    //       returning.
    //

    //
    // See if the DebugBreakOnEntry flag is set first.
    //

    READ_REG_DWORD_FLAG(DebugBreakOnEntry, FALSE);

    if (Flags.DebugBreakOnEntry) {
        __debugbreak();
    }

    //
    // Allocate space for the initial structure.
    //

    __try {
        TracerConfig = (PTRACER_CONFIG)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                sizeof(*TracerConfig)
            )
        );
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        TracerConfig = NULL;
        goto Error;
    }

    //
    // Make sure we got back a non-NULL value.
    //

    if (TracerConfig == NULL) {
        goto Error;
    }

    //
    // Initialize trace session directories.
    //

    TraceSessionDirectories = &TracerConfig->TraceSessionDirectories;
    InitializeListHead(&TraceSessionDirectories->ListHead);
    InitializeSRWLock(&TraceSessionDirectories->Lock);

    //
    // Initialize size and allocator.
    //

    TracerConfig->Size = sizeof(*TracerConfig);

    TracerConfig->Allocator = Allocator;

    //
    // Read the remaining flags.
    //

    READ_REG_DWORD_FLAG(LoadDebugLibraries, FALSE);
    READ_REG_DWORD_FLAG(EnableTraceSessionDirectoryCompression, TRUE);
    READ_REG_DWORD_FLAG(PrefaultPages, TRUE);
    READ_REG_DWORD_FLAG(EnableMemoryTracing, FALSE);
    READ_REG_DWORD_FLAG(EnableIoCounterTracing, FALSE);
    READ_REG_DWORD_FLAG(EnableHandleCountTracing, FALSE);

    //
    // Copy the flags over.
    //

    TracerConfig->Flags = Flags;

    //
    // Prep the TRACER_PATHS structure.
    //

    Paths = &TracerConfig->Paths;
    Paths->Size = sizeof(*Paths);

    //
    // Load InstallationDirectory and BaseTraceDirectory.
    //

    READ_REG_SZ_PATH(InstallationDirectory);
    READ_REG_SZ_PATH(BaseTraceDirectory);

    //
    // Load fully-qualified DLL path names.
    //

    for (Index = 0; Index < NumberOfPathOffsets; Index++) {
        if (!LoadPath(TracerConfig, Index)) {
            goto Error;
        }
    }

    //
    // That's it, we're done.
    //

    goto End;

Error:

    //
    // DestroyTracerConfig() accepts a TracerConfig in a partially initialized
    // state (or a completely NULL TracerConfig), so we're find to call it here
    // regardless of the actual error.
    //

    DestroyTracerConfig(TracerConfig);

    //
    // NULL out the pointer, before we return it.
    //

    TracerConfig = NULL;

    //
    // Intentional follow-on to End.
    //

End:

    RegCloseKey(RegistryKey);

    return TracerConfig;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
