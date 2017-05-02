/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3ExtInit.c

Abstract:

    WIP.

--*/

#include "stdafx.h"

LONG
TraceStoreSqlite3ExtInit(
    PSQLITE3_DB Database,
    PCSZ *ErrorMessagePointer,
    PCSQLITE3 Sqlite3
    )
/*++

Routine Description:

    This is the main entry point for the TraceStore sqlite3 extension module.
    It is called by sqlite3 when TraceStore.dll is loaded as an extension.

Arguments:

    Database - Supplies a pointer to the active sqlite3 database.

    ErrorMessagePointer - Supplies a variable that optionally receives the
        address of an error message if initialization fails.

    Sqlite3 - Supplies a pointer to the sqlite3 API routines.

Return Value:

    SQLITE_OK on success, an appropriate error code on error.

--*/
{
    BOOL Success;
    PRTL Rtl;
    ULONG Result;
    FARPROC Proc;
    HMODULE Module;
    PALLOCATOR Allocator;
    PUNICODE_STRING DllPath;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING RegistryPath;
    PTRACE_STORE_SQLITE3_EXT_INIT Initializer;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Database)) {
        return SQLITE_ERROR;
    }

    if (!ARGUMENT_PRESENT(ErrorMessagePointer)) {
        return SQLITE_ERROR;
    }

    if (!ARGUMENT_PRESENT(Sqlite3)) {
        return SQLITE_ERROR;
    }

    //
    // Allocate space for the allocator via Sqlite3's malloc function, then
    // initialize it using the same Sqlite3 API.
    //

    Allocator = (PALLOCATOR)Sqlite3->Malloc(sizeof(*Allocator));
    if (!Allocator) {
        return SQLITE_NOMEM;
    }

    InitializeAllocatorFromSqlite3(Allocator, Sqlite3);

    //
    // Initialize TracerConfig and Rtl.
    //

    RegistryPath = (PUNICODE_STRING)&TracerRegistryPath;

    Success = (
        CreateAndInitializeTracerConfigAndRtl(
            Allocator,
            RegistryPath,
            &TracerConfig,
            &Rtl
        )
    );

    if (!Success) {
        *ErrorMessagePointer = "CreateAndInitializeTracerConfigAndRtl() failed.";
        goto Error;
    }

    //
    // Load the actual extension DLL and resolve the initialization function.
    //

    DllPath = &TracerConfig->Paths.TraceStoreDllPath;
    Module = LoadLibraryW(DllPath->Buffer);
    if (!Module) {
        *ErrorMessagePointer = "LoadLibraryW() of extension failed.";
        goto Error;
    }

    Proc = GetProcAddress(Module, "TraceStoreSqlite3ExtInit");
    if (!Proc) {
        *ErrorMessagePointer = "Failed to resolve TraceStoreSqlite3ExtInit.";
        goto Error;
    }

    //
    // We've successfully loaded the extension and found the initializer.
    //

    Initializer = (PTRACE_STORE_SQLITE3_EXT_INIT)Proc;

    Result = Initializer(Rtl,
                         Allocator,
                         TracerConfig,
                         Database,
                         ErrorMessagePointer,
                         Sqlite3);

    goto End;

Error:

    Result = SQLITE_ERROR;

    //
    // Intentional follow-on to End.
    //

End:

    return Result;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
