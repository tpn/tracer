/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    InitializeTracerConfig.c

Abstract:

    This module implements routines related to the initialization of a
    TRACER_CONFIG structure.  Routines are provided for loading path names
    from the registry, reading REG_SZ and REG_DWORD values from the registry,
    and initializing a new TRACER_CONFIG structure based on the registry
    configuration stored under HKCU\Software\Tracer.

--*/

#include "stdafx.h"

_Success_(return != 0)
BOOLEAN
LoadPath(
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ USHORT Index
    )
/*++

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
    TRACER_BINARY_TYPE_INDEX BinaryTypeIndex;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (Index >= NumberOfDllPathOffsets) {
        return FALSE;
    }

    //
    // Initialize various local variables.
    //

    Flags = TracerConfig->Flags;
    Paths = &TracerConfig->Paths;
    Offset = DllPathOffsets[Index].Offset;
    Allocator = TracerConfig->Allocator;
    BinaryTypeIndex = ExtractTracerBinaryTypeIndexFromFlags(Flags);

    //
    // Initialize paths.
    //

    DllPath = DllPathOffsets[Index].DllPath;
    TargetPath = (PUNICODE_STRING)((((ULONG_PTR)Paths) + Offset));
    InstallationDir = &Paths->InstallationDirectory;
    IntermediatePath = IntermediatePaths[BinaryTypeIndex];

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

    TargetPath->Buffer = (PWCHAR)(
        Allocator->Malloc(
            Allocator->Context,
            SizeInBytes
        )
    );

    if (!TargetPath->Buffer) {
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
    //

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

/*++

    VOID
    READ_REG_DWORD_FLAG(
        Name,
        Default
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry
    into a local TRACER_FLAGS Flags structure.  If the registry key isn't
    present, an attempt will be made to write the default value.

Arguments:

    Name - Name of the flag to read (e.g. LoadDebugLibraries).  The macro
        resolves this to Flags.Name (e.g. Flags.LoadDebugLibraries).

    Default - Default value to assign to the flag (Flags.Name) if the
        registry key couldn't be read successfully (because it was not
        present, or was an incorrect type).

Return Value:

    None.

--*/
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
        Value = Flags.Name = Default;           \
        RegSetValueExW(                         \
            RegistryKey,                        \
            L#Name,                             \
            0,                                  \
            REG_DWORD,                          \
            (const BYTE*)&Value,                \
            ValueLength                         \
        );                                      \
    }                                           \
} while (0)

/*++

    VOID
    READ_REG_DWORD_RUNTIME_PARAM(
        Name,
        Default
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry
    into a TRACER_RUNTIME_PARAMETERS structure.  If the registry key isn't
    present, an attempt will be made to write the default value.

Arguments:

    Name - Name of the parameter to read.

    Default - Default value to assign to the parameter if the registry key
        couldn't be read successfully (because it was not present, or was an
        incorrect type).

Return Value:

    None.

--*/
#define READ_REG_DWORD_RUNTIME_PARAM(Name, Default) do {        \
    ULONG Value;                                                \
    ULONG ValueLength = sizeof(Value);                          \
    Result = RegGetValueW(                                      \
        RegistryKey,                                            \
        NULL,                                                   \
        L#Name,                                                 \
        RRF_RT_REG_DWORD,                                       \
        NULL,                                                   \
        (PVOID)&Value,                                          \
        &ValueLength                                            \
    );                                                          \
    if (Result == ERROR_SUCCESS) {                              \
        TracerConfig->RuntimeParameters.Name = Value;           \
    } else {                                                    \
        Value = TracerConfig->RuntimeParameters.Name = Default; \
        RegSetValueExW(                                         \
            RegistryKey,                                        \
            L#Name,                                             \
            0,                                                  \
            REG_DWORD,                                          \
            (const BYTE*)&Value,                                \
            ValueLength                                         \
        );                                                      \
    }                                                           \
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

/*++

    VOID
    READ_REG_SZ_PATH(
        Name,
        Optional
        )

Routine Description:

    Helper macro for reading REG_SZ path values.  The size of the string
    is obtained first, then an attempt is made to allocate a sufficiently-
    sized buffer using the TracerConfig's Allocator.  If this succeeds,
    a second call to RegGetValueW() is called with the new buffer, and
    the rest of the UNICODE_STRING is initialized (i.e. MaximumLength and
    Length are set).

    If any errors occur, the Error handler is jumped to.

    This macro is intended to be called from within the body of the
    InitializeTracerConfig() routine.

Arguments:

    Name - Name of the path in the TRACER_PATHS structure.

    Optional - If TRUE, silently ignore the condition where the key can't be
        read from the registry.

Return Value:

    None.

--*/
#define READ_REG_SZ_PATH(Name, Optional) do {                \
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
        if (Optional) {                                      \
            String->Length = 0;                              \
            String->MaximumLength = 0;                       \
            String->Buffer = NULL;                           \
            break;                                           \
        } else {                                             \
            goto Error;                                      \
        }                                                    \
    }                                                        \
                                                             \
    String->Buffer = (PWCHAR)(                               \
        Allocator->Malloc(                                   \
            Allocator->Context,                              \
            SizeInBytes                                      \
        )                                                    \
    );                                                       \
    if (!String->Buffer) {                                   \
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
    const BOOL Mandatory = FALSE;
    const BOOL Optional = TRUE;
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
    // N.B. We've successfully opened a registry key, so all failures after
    //      this point should `goto Error` to ensure the key is closed before
    //      returning.
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

    TracerConfig = (PTRACER_CONFIG)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(*TracerConfig)
        )
    );

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
    READ_REG_DWORD_FLAG(LoadPGInstrumentedLibraries, FALSE);
    READ_REG_DWORD_FLAG(LoadPGOptimizedLibraries, FALSE);
    READ_REG_DWORD_FLAG(DisableTraceSessionDirectoryCompression, FALSE);
    READ_REG_DWORD_FLAG(DisablePrefaultPages, FALSE);
    READ_REG_DWORD_FLAG(DisableFileFlagOverlapped, FALSE);
    READ_REG_DWORD_FLAG(DisableFileFlagSequentialScan, FALSE);
    READ_REG_DWORD_FLAG(EnableFileFlagRandomAccess, FALSE);
    READ_REG_DWORD_FLAG(EnableFileFlagWriteThrough, FALSE);
    READ_REG_DWORD_FLAG(EnableWorkingSetTracing, TRUE);
    READ_REG_DWORD_FLAG(EnablePerformanceTracing, TRUE);
    READ_REG_DWORD_FLAG(EnableLoaderTracing, TRUE);
    READ_REG_DWORD_FLAG(EnableSymbolTracing, TRUE);
    READ_REG_DWORD_FLAG(EnableTypeInfoTracing, TRUE);
    READ_REG_DWORD_FLAG(EnableAssemblyTracing, TRUE);
    READ_REG_DWORD_FLAG(IgnoreModulesInWindowsSystemDirectory, TRUE);
    READ_REG_DWORD_FLAG(IgnoreModulesInWindowsSxSDirectory, FALSE);
    READ_REG_DWORD_FLAG(DisableAsynchronousInitialization, FALSE);

    //
    // We only need to enforce one invariant: if FILE_FLAG_RANDOM_ACCESS has
    // been requested, disable FILE_FLAG_SEQUENTIAL_SCAN.
    //

    if (Flags.EnableFileFlagRandomAccess) {
        Flags.DisableFileFlagSequentialScan = TRUE;
    }

    //
    // Copy the flags over.
    //

    TracerConfig->Flags = Flags;

    //
    // Read the runtime parameters.
    //

    READ_REG_DWORD_RUNTIME_PARAM(
        GetWorkingSetChangesIntervalInMilliseconds,
        100
    );

    READ_REG_DWORD_RUNTIME_PARAM(
        GetWorkingSetChangesWindowLengthInMilliseconds,
        200
    );

    READ_REG_DWORD_RUNTIME_PARAM(
        WsWatchInfoExInitialBufferNumberOfElements,
        8192
    );

    READ_REG_DWORD_RUNTIME_PARAM(
        CapturePerformanceMetricsIntervalInMilliseconds,
        100
    );

    READ_REG_DWORD_RUNTIME_PARAM(
        CapturePerformanceMetricsWindowLengthInMilliseconds,
        200
    );

    READ_REG_DWORD_RUNTIME_PARAM(
        ConcurrentAllocationsCriticalSectionSpinCount,
        4000
    );

    //
    // Prep the TRACER_PATHS structure.
    //

    Paths = &TracerConfig->Paths;
    Paths->Size = sizeof(*Paths);

    //
    // Load InstallationDirectory and BaseTraceDirectory.
    //

    READ_REG_SZ_PATH(InstallationDirectory, Mandatory);
    READ_REG_SZ_PATH(BaseTraceDirectory, Mandatory);

    //
    // Load the optional DefaultPythonDirectory.
    //

    READ_REG_SZ_PATH(DefaultPythonDirectory, Optional);

    //
    // Load the optional DebuggerSettingsXmlPath.
    //

    READ_REG_SZ_PATH(DebuggerSettingsXmlPath, Optional);

    //
    // Load fully-qualified DLL path names.
    //

    for (Index = 0; Index < NumberOfDllPathOffsets; Index++) {
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
    // state (or a completely NULL TracerConfig), so we're fine to call it here
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

    //
    // Close the registry key and return TracerConfig.  (If an error occurred,
    // this will be NULL, see above.)
    //

    RegCloseKey(RegistryKey);

    return TracerConfig;
}

_Use_decl_annotations_
BOOL
CreateAndInitializeTracerConfigAndRtl(
    PALLOCATOR Allocator,
    PUNICODE_STRING RegistryPath,
    PPTRACER_CONFIG TracerConfigPointer,
    PPRTL RtlPointer
    )
/*++

Routine Description:

    Initializes a TRACER_CONFIG structure using the InitializeTracerConfig()
    routine, then additionally loads the Rtl module from the path indicated
    in the registry and creates an instance of the Rtl structure.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to manage the structure's memory.  This includes initial
        allocation of bytes to hold the structure, plus space for each
        UNICODE_STRING's Buffer in each of the paths in TRACER_PATHS,
        plus space for the Rtl structure.

    RegistryPath - Supplies a pointer to a fully-qualified UNICODE_STRING
        to the primary registry path that will be used to load tracer
        configuration information.  (Note that this string must be NULL
        terminated, as the underlying Buffer is passed directly to the
        RegCreateKeyExW() function, which expects an LPWSTR.)

    TracerConfigPointer - Supplies the address of a variable that will receive
        the address of the TRACER_CONFIG structure created by this routine if
        successful, NULL otherwise.

    RtlPointer - Supplies the address of a variable that will receive the
        address of an RTL structure created by this routine if successful,
        NULL otherwise.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl = NULL;
    HMODULE Module = NULL;
    ULONG RequiredSizeInBytes;
    PTRACER_PATHS Paths;
    PTRACER_CONFIG TracerConfig = NULL;
    PINITIALIZE_RTL InitializeRtl;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(RegistryPath)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TracerConfigPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(RtlPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointers up-front.
    //

    *TracerConfigPointer = NULL;
    *RtlPointer = NULL;

    //
    // Initialize the tracer config.
    //

    TracerConfig = InitializeTracerConfig(Allocator, RegistryPath);
    if (!TracerConfig) {
        return FALSE;
    }

    //
    // Successfully initialized the config.  Attempt to load the Rtl module.
    //

    Paths = &TracerConfig->Paths;
    Module = LoadLibraryW(Paths->RtlDllPath.Buffer);
    if (!Module) {
        goto Error;
    }

    //
    // Attempt to resolve the InitializeRtl function.
    //

    InitializeRtl = (PINITIALIZE_RTL)GetProcAddress(Module, "InitializeRtl");
    if (!InitializeRtl) {
        goto Error;
    }

    //
    // Obtain the size required for the RTL structure.
    //

    RequiredSizeInBytes = 0;
    InitializeRtl(NULL, &RequiredSizeInBytes);
    if (!RequiredSizeInBytes) {
        goto Error;
    }

    //
    // Allocate a buffer of the requested size.
    //

    Rtl = (PRTL)Allocator->Calloc(Allocator->Context, 1, RequiredSizeInBytes);
    if (!Rtl) {
        goto Error;
    }

    if (!InitializeRtl(Rtl, &RequiredSizeInBytes)) {
        goto Error;
    }

    //
    // Everything succeeded, update caller's pointers and return success.
    //

    *TracerConfigPointer = TracerConfig;
    *RtlPointer = Rtl;

    return TRUE;

Error:

    if (TracerConfig) {
        DestroyTracerConfig(TracerConfig);
        TracerConfig = NULL;
    }

    if (Module) {
        FreeLibrary(Module);
        Module = NULL;
    }

    if (Rtl) {
        Allocator->FreePointer(Allocator->Context, &Rtl);
    }

    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
