/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    InitializeTracerConfig.c

Abstract:

    This module implements routines related to the initialization of a
    TRACER_CONFIG structure.

--*/

#include "stdafx.h"


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
    PRTL_BITMAP Bitmap;
    PTRACER_PATHS Paths;
    PTRACER_CONFIG TracerConfig;
    PTRACE_SESSION_DIRECTORIES TraceSessionDirectories;
    ULONG MaxTracerDllPathId = MAX_TRACER_DLL_PATH_ID;

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
    // Invariant check: NumberOfDllPathOffsets should match the enum value of
    // TracerInvalidDllPathId - 1, which is MAX_TRACER_DLL_PATH_ID.
    //

    if (!AssertTrue("NumberOfDllPathOffsets == MAX_TRACER_DLL_PATH_ID",
                     NumberOfDllPathOffsets == MaxTracerDllPathId)) {
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
    // Initialize size, number of DLL and PTX paths, and allocator.
    //

    TracerConfig->Size = sizeof(*TracerConfig);
    TracerConfig->Paths.NumberOfDllPaths = NumberOfDllPathOffsets;
    TracerConfig->Paths.NumberOfPtxPaths = NumberOfPtxPathOffsets;

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
    READ_REG_DWORD_FLAG(InjectionThunkDebugBreakOnEntry, FALSE);
    READ_REG_DWORD_FLAG(TraceStoreSqlite3ModuleDebugBreakOnEntry, FALSE);
    READ_REG_DWORD_FLAG(DisableAllocationTimestamps, FALSE);

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

    READ_REG_DWORD_RUNTIME_PARAM(
        IntervalFramesPerSecond,
        240
    );

    //
    // Prep the TRACER_PATHS structure.
    //

    Paths = &TracerConfig->Paths;
    Paths->SizeOfStruct = sizeof(*Paths);

    //
    // Load InstallationDirectory and BaseTraceDirectory.
    //

    READ_REG_SZ_PATH(InstallationDirectory, Mandatory);
    READ_REG_SZ_PATH(BaseTraceDirectory, Mandatory);

    //
    // Load the optional DebuggerSettingsXmlPath.
    //

    READ_REG_SZ_PATH(DebuggerSettingsXmlPath, Optional);

    //
    // Load fully-qualified DLL path names.
    //

    for (Index = 0; Index < NumberOfDllPathOffsets; Index++) {
        if (!LoadTracerPath(TracerConfig, TracerDllPathType, Index)) {
            goto Error;
        }
    }

    //
    // Load fully-qualified PTX path names.
    //

    for (Index = 0; Index < NumberOfPtxPathOffsets; Index++) {
        if (!LoadTracerPath(TracerConfig, TracerPtxPathType, Index)) {
            goto Error;
        }
    }


    //
    // Initialize the bitmap that indicates which DLLs support tracer injection.
    //

    Bitmap = &Paths->TracerInjectionDllsBitmap;
    Bitmap->SizeOfBitMap = MAX_TRACER_DLL_PATH_ID;
    Bitmap->Buffer = (PULONG)&Paths->TracerInjectionDlls;

    CopyTracerDllPathType(Paths->TracerInjectionDlls,
                          TracerInjectionDllsBitmap);

    //
    // Read CuDeviceOrdinal from the registry.
    //

    READ_REG_DWORD(CuDeviceOrdinal, 0);

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
    // Copy the Rtl and InjectionThunk path names over.
    //

    if (!Rtl->SetDllPath(Rtl, Allocator, &Paths->RtlDllPath)) {
        goto Error;
    }

    if (!Rtl->SetInjectionThunkDllPath(Rtl,
                                       Allocator,
                                       &Paths->InjectionThunkDllPath)) {
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

_Use_decl_annotations_
BOOL
CreateAndInitializeTracerInjectionModules(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PTRACER_INJECTION_MODULES *InjectionModulesPointer
    )
/*++

Routine Description:

    Creates and initializes a TRACER_INJECTION_MODULES structure from the
    given tracer config.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure which will
        be used to manage the structure's memory.

    TracerConfig - Supplies a pointer to an initialized TRACER_CONFIG structure.

    InjectionModulesPointer - Supplies a pointer to a variable that will receive
        the address of the newly-allocated TRACER_INJECTION_MODULES structure
        created and initialized by this routine.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    ULONG Index = 0;
    ULONG BitIndex = 0;
    FARPROC Proc;
    USHORT Offset;
    HMODULE Module;
    PHMODULE ModuleArray;
    PRTL_BITMAP Bitmap;
    PTRACER_PATHS Paths;
    PUNICODE_STRING Path;
    PPUNICODE_STRING PathArray;
    ULONG NumberOfModules;
    LONG_INTEGER AllocSizeInBytes;
    PRTL_NUMBER_OF_SET_BITS NumberOfSetBits;
    PTRACER_INJECTION_MODULES InjectionModules;
    PINITIALIZE_TRACER_INJECTION Initializer;
    PPINITIALIZE_TRACER_INJECTION InitializerArray;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(InjectionModulesPointer)) {
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    Paths = &TracerConfig->Paths;
    Bitmap = &Paths->TracerInjectionDllsBitmap;
    NumberOfSetBits = Rtl->RtlNumberOfSetBits;

    //
    // Determine how many modules will be included.
    //

    NumberOfModules = NumberOfSetBits(Bitmap);

    if (!NumberOfModules) {
        return FALSE;
    }

    //
    // Determine the allocation size.
    //

    AllocSizeInBytes.LongPart = (

        //
        // Account for the module structure.
        //

        sizeof(TRACER_INJECTION_MODULES) +

        //
        // Account for the array of PUNICODE_STRING pointers.
        //

        (sizeof(PUNICODE_STRING) * NumberOfModules) +

        //
        // Account for the array of HMODULEs.
        //

        (sizeof(HMODULE) * NumberOfModules) +

        //
        // Account for the array of tracer initialization function pointers.
        //

        (sizeof(PINITIALIZE_TRACER_INJECTION) * NumberOfModules)

    );

    //
    // Sanity check we're not above MAX_USHORT.
    //

    if (AllocSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    //
    // Allocate space for the structure plus arrays.
    //

    InjectionModules = (PTRACER_INJECTION_MODULES)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AllocSizeInBytes.LongPart
        )
    );

    if (!InjectionModules) {
        return FALSE;
    }

    //
    // N.B. Everything after this point must `goto Error` instead of returning
    //      FALSE in order to ensure InjectionModules gets cleaned up on error.
    //

    //
    // Initialize scalar fields.
    //

    InjectionModules->SizeOfStruct = sizeof(*InjectionModules);
    InjectionModules->NumberOfModules = NumberOfModules;
    InjectionModules->TotalAllocSizeInBytes = AllocSizeInBytes.LongPart;

    //
    // Wire up the pointers to arrays that live at the end of our injection
    // modules structure.
    //

    Offset = (USHORT)sizeof(TRACER_INJECTION_MODULES);
    PathArray = InjectionModules->Paths = (PPUNICODE_STRING)(
        RtlOffsetToPointer(
            InjectionModules,
            Offset
        )
    );

    Offset += (USHORT)(sizeof(PUNICODE_STRING) * NumberOfModules);
    ModuleArray = InjectionModules->Modules = (PHMODULE)(
        RtlOffsetToPointer(
            InjectionModules,
            Offset
        )
    );

    Offset += (USHORT)(sizeof(PHMODULE) * NumberOfModules);
    InitializerArray = InjectionModules->Initializers = (
        (PPINITIALIZE_TRACER_INJECTION)(
            RtlOffsetToPointer(
                InjectionModules,
                Offset
            )
        )
    );

    //
    // Enumerate over the modules supporting injection.
    //

    while (GetNextTracerInjectionDll(Rtl, Paths, &BitIndex, &Path)) {

        //
        // Load the library.
        //

        Module = LoadLibraryExW(Path->Buffer, NULL, 0);
        if (!Module) {
            __debugbreak();
            goto Error;
        }

        //
        // Resolve the exported InitializeTracerInjection function.
        //

        Proc = GetProcAddress(Module, "InitializeTracerInjection");
        if (!Proc) {
            __debugbreak();
            goto Error;
        }

        Initializer = (PINITIALIZE_TRACER_INJECTION)Proc;

        //
        // Save each element to its respective array location.
        //

        *(PathArray + Index) = Path;
        *(ModuleArray + Index) = Module;
        *(InitializerArray + Index) = Initializer;

        //
        // Increment our index and continue the enumeration.
        //

        Index++;
    }

    //
    // We're done, indicate success and return.
    //

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    if (InjectionModules) {
        Allocator->FreePointer(Allocator->Context, &InjectionModules);
    }

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the caller's pointer and return.
    //

    *InjectionModulesPointer = InjectionModules;

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
