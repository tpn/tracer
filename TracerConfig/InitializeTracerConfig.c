#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

#include "../Rtl/Rtl.h"

#include <winreg.h>

//
// The size of the `WCHAR Buffer[n]` buffer used to read REG_SZ registry
// values during initialization.
//

#define TEMP_STACK_BUFFER_LENGTH 1024

//
// The size of the buffer, in bytes.
//

#define TEMP_STACK_BUFFER_SIZE_IN_BYTES (TEMP_STACK_BUFFER_LENGTH << 1)

_Success_(return != 0)
BOOLEAN
AllocateAndCopyWideString(
    _In_ PALLOCATOR Allocator,
    _In_ SIZE_T SizeInBytes,
    _In_ PWCHAR Buffer,
    _In_ PUNICODE_STRING String
    )
{
    SIZE_T AlignedSizeInBytes = ALIGN_UP(SizeInBytes, sizeof(ULONG_PTR));

    __try {
        String->Buffer = (PWCHAR)(
            Allocator->Malloc(
                Allocator->Context,
                AlignedSizeInBytes
            )
        );
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    String->Length = ((SizeInBytes - 2) >> 1);
    String->MaximumLength = (AlignedSizeInBytes >> 1);

    __movsb(
        (PBYTE)String->Buffer,
        (const PBYTE)Buffer,
        SizeInBytes
    );

    return TRUE;
}

_Use_decl_annotations_
PTRACER_CONFIG
InitializeTracerConfig(
    PALLOCATOR Allocator,
    PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Initializes a TRACER_CONFIG structure based on the values in the registry
    path pointed to by RegistryPath.  If the path doesn't exist, it is created,
    and default values are set for all keys.  If any key is missing, it is also
    created with a default value.

    The Allocator will be used for all memory allocations.  DestroyTracerConfig()
    must be called against the returned PTRACER_CONFIG when the structure is no
    longer needed in order to ensure resources are released.

Arguments:

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to manage the structure's memory.  This includes initial
        allocation of bytes to hold the structure, plus space for each
        UNICODE_STRING's Buffer in each of the paths in TRACER_PATHS.

    RegistryPath - Supplies a pointer to a fully-qualified UNICODE_STRING
        to the primary registry path that will be used to load tracer
        configuration information.  Note that this string must be NULL
        terminated.

Return Value:

    A pointer to a valid TRACER_CONFIG structure on success, NULL on failure.
    Call DestroyTracerConfig() on the returned structure when it is no longer
    needed in order to ensure resources are cleaned up appropriately.

--*/
{
    BOOL Success;
    BOOL DebugBreakOnEntry;
    BOOL LoadDebugLibraries;
    ULONG Result;
    ULONG Disposition;
    ULONG Value;
    ULONG ValueLength = sizeof(Value);
    ULONG BufferLength;
    HKEY RegistryKey;
    PTRACER_CONFIG TracerConfig;
    PTRACER_PATHS Paths;
    WCHAR Buffer[TEMP_STACK_BUFFER_LENGTH];

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
    // See if the DebugBreakOnEntry flag is set first.
    //

    Result = RegGetValueW(
        RegistryKey,
        NULL,       // lpSubKey
        L"DebugBreakOnEntry",
        RRF_RT_REG_DWORD,
        NULL,
        (PVOID)&Value,
        &ValueLength
    );

    if (Result == ERROR_SUCCESS && Value != 0) {
        DebugBreakOnEntry = TRUE;
        __debugbreak();
    } else {
        DebugBreakOnEntry = FALSE;
    }

    //
    // Allocate space for the initial structure.
    //

    __try {
        TracerConfig = (PTRACER_CONFIG)(
            Allocator->Malloc(
                Allocator->Context,
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
    // Zero the memory and begin initialization.
    //

    RtlSecureZeroMemory(TracerConfig, sizeof(*TracerConfig));

    TracerConfig->Size = sizeof(*TracerConfig);

    TracerConfig->Flags.DebugBreakOnEntry = DebugBreakOnEntry;
    TracerConfig->Flags.EnableTraceSessionDirectoryCompression = TRUE;
    TracerConfig->Flags.PrefaultPages = TRUE;

    TracerConfig->SupportedRuntimes.Python = TRUE;
    TracerConfig->SupportedRuntimes.C = TRUE;

    //
    // Prep the TRACER_PATHS structure.
    //

    Paths = &TracerConfig->Paths;
    Paths->Size = sizeof(*Paths);

    //
    // Check for LoadDebugLibraries.
    //

    Result = RegGetValueW(
        RegistryKey,
        NULL,       // lpSubKey
        L"LoadDebugLibraries",
        RRF_RT_REG_DWORD,
        NULL,
        (PVOID)&Value,
        &ValueLength
    );

    if (Result == ERROR_SUCCESS && Value != 0) {
        LoadDebugLibraries = TRUE;
    } else {
        LoadDebugLibraries = FALSE;
    }

    TracerConfig->Flags.LoadDebugLibraries = LoadDebugLibraries;

    //
    // See if InstallationDirectory has been set.
    //

    BufferLength = TEMP_STACK_BUFFER_SIZE_IN_BYTES;

    Result = RegGetValueW(
        RegistryKey,
        NULL,
        L"InstallationDirectory",
        RRF_RT_REG_SZ,
        NULL,
        (PVOID)Buffer,
        &BufferLength
    );

    if (Result != ERROR_SUCCESS) {

        //
        // XXX TODO: initialize buffer size from path of current module
        // handle.
        //

        goto Error;
    }

    //
    // Allocate and copy the string.
    //

    Success = AllocateAndCopyWideString(
        Allocator,
        BufferLength,
        Buffer,
        &Paths->InstallationDirectory
    );

    if (!Success) {
        goto Error;
    }

    //
    // Reset the buffer length and load BaseTraceDirectory.
    //

    BufferLength = TEMP_STACK_BUFFER_SIZE_IN_BYTES;

    Result = RegGetValueW(
        RegistryKey,
        NULL,
        L"BaseTraceDirectory",
        RRF_RT_REG_SZ,
        NULL,
        (PVOID)Buffer,
        &BufferLength
    );

    if (Result != ERROR_SUCCESS) {

        //
        // XXX TODO: initialize buffer size from path of current module
        // handle.
        //

        goto Error;
    }

    //
    // Allocate and copy the string.
    //

    Success = AllocateAndCopyWideString(
        Allocator,
        BufferLength,
        Buffer,
        &Paths->InstallationDirectory
    );

    if (!Success) {
        goto Error;
    }


    //
    // Allocate space for the UNICODE_STRING->Buffer.
    //

    __try {
        Paths = (PTRACER_CONFIG)(
            Allocator->Malloc(
                Allocator->Context,
                sizeof(*TracerConfig)
            )
        );
    } __except (EXCEPTION_EXECUTE_HANDLER) {

        TracerConfig = NULL;
        goto Error;
    }






    //Result = RegGetKey();

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




