#pragma once

#include "TracerConfig.h"

_Success_(return != 0)
PTRACER_CONFIG
BootstrapTracerConfig(
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
    READ_REG_DWORD_FLAG(DisableTraceSessionDirectoryCompression, TRUE);
    READ_REG_DWORD_FLAG(DisablePrefaultPages, TRUE);
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
