/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    DestroyTracerConfig.c

Abstract:

    This module implements the functionality to destroy a previously created
    TRACER_CONFIG structure.

--*/

#include "stdafx.h"

//
// Forward declarations of internal helper functions used in this module.
//

VOID
DestroyTraceSessionDirectories(
    _In_ PTRACER_CONFIG TracerConfig
    );

VOID
FreeUnicodeStringBuffer(
    _In_ PALLOCATOR Allocator,
    _In_ PUNICODE_STRING String
    );


_Use_decl_annotations_
VOID
DestroyTracerConfig(
    PTRACER_CONFIG TracerConfig
    )
/*++

Routine Description:

    Destroys a TracerConfig structure.  This method is as forgiving as it can
    be with regards to the state of the TracerConfig instance passed in.  A
    partially-initialized or even NULL pointer can be passed in safely.  The
    routine will attempt to clean up as much as it can based on the fields
    that have values set.

Arguments:

    TracerConfig - Supplies a pointer to a TRACER_CONFIG struct to be
        destroyed.  Can be partially-initialized or even NULL.

Return Value:

    None.

--*/
{
    USHORT Index;
    USHORT Offset;
    PALLOCATOR Allocator;
    PTRACER_PATHS Paths;
    PUNICODE_STRING String;

    //
    // Validate arguments.
    //

    if (!TracerConfig) {
        return;
    }

    //
    // If there's no allocator, we can't free anything.
    //

    if (!TracerConfig->Allocator) {
        return;
    }

    //
    // Initialize helper aliases.
    //

    Allocator = TracerConfig->Allocator;
    Paths = &TracerConfig->Paths;

    //
    // Free the installation and base trace directory strings.
    //

    FreeUnicodeStringBuffer(Allocator, &Paths->InstallationDirectory);
    FreeUnicodeStringBuffer(Allocator, &Paths->BaseTraceDirectory);

    //
    // Enumerate over the PathOffsets[], freeing each path as we go.
    //

    for (Index = 0; Index < NumberOfDllPathOffsets; Index++) {
        Offset = DllPathOffsets[Index].Offset;
        String = (PUNICODE_STRING)((((ULONG_PTR)Paths) + Offset));
        FreeUnicodeStringBuffer(Allocator, String);
    }

    //
    // Destroy trace session directories.
    //

    DestroyTraceSessionDirectories(TracerConfig);

    //
    // Free the underlying TracerConfig structure.
    //

    Allocator->Free(
        Allocator->Context,
        TracerConfig
    );

    TracerConfig = NULL;

    return;
}

_Use_decl_annotations_
VOID
FreeUnicodeStringBuffer(
    _In_ PALLOCATOR Allocator,
    _In_ PUNICODE_STRING String
    )
{
    if (!IsValidUnicodeString(String)) {
        return;
    }

    Allocator->Free(Allocator->Context, String->Buffer);

    return;
}

_Use_decl_annotations_
VOID
DestroyTraceSessionDirectories(
    PTRACER_CONFIG TracerConfig
    )
{
    PALLOCATOR Allocator;
    PTRACE_SESSION_DIRECTORIES Directories;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return;
    }

    //
    // Set up aliases.
    //

    Allocator = TracerConfig->Allocator;
    Directories = &TracerConfig->TraceSessionDirectories;

    //
    // Enumerate over dynamically created trace session directories and free
    // them.
    //

    AcquireSRWLockExclusive(&Directories->Lock);

    while (!IsListEmpty(&Directories->ListHead)) {

        PLIST_ENTRY Entry;
        PTRACE_SESSION_DIRECTORY Directory;

        Entry = RemoveHeadList(&Directories->ListHead);

        InterlockedDecrement(&Directories->Count);

        Directory = CONTAINING_RECORD(Entry,
                                      TRACE_SESSION_DIRECTORY,
                                      ListEntry);

        SecureZeroMemory(Directory, sizeof(*Directory));

        Allocator->Free(Allocator->Context, Directory);
        Directory = NULL;
    }

    ReleaseSRWLockExclusive(&Directories->Lock);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
