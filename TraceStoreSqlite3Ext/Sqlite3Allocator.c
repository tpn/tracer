/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Allocator.c

Abstract:

    This module implements an ALLOCATOR interface that wraps the memory
    allocation routines exposed by sqlite3.

--*/

#include "stdafx.h"

#define ContextToSqlite3(Context) \
    ((PSQLITE3)((PALLOCATOR)(Context))->Context2)

_Use_decl_annotations_
PVOID
Sqlite3ExtMalloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return ContextToSqlite3(Context)->Malloc64(Size);
}

_Use_decl_annotations_
PVOID
Sqlite3ExtCalloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T SizeOfElements
    )
{
    SIZE_T Size = NumberOfElements * SizeOfElements;
    PBYTE Buffer = (PBYTE)ContextToSqlite3(Context)->Malloc64(Size);
    SecureZeroMemory(Buffer, Size);
    return Buffer;
}

_Use_decl_annotations_
PVOID
Sqlite3ExtRealloc(
    PVOID Context,
    PVOID Buffer,
    SIZE_T NewSize
    )
{
    return ContextToSqlite3(Context)->Realloc64(Buffer, NewSize);
}

_Use_decl_annotations_
VOID
Sqlite3ExtFree(
    PVOID Context,
    PVOID Buffer
    )
{
    ContextToSqlite3(Context)->Free(Buffer);
}

_Use_decl_annotations_
VOID
Sqlite3ExtFreePointer(
    PVOID Context,
    PPVOID BufferPointer
    )
{
    if (!ARGUMENT_PRESENT(BufferPointer)) {
        return;
    }

    if (!ARGUMENT_PRESENT(*BufferPointer)) {
        return;
    }

    Sqlite3ExtFree(Context, *BufferPointer);
    *BufferPointer = NULL;

    return;
}

_Use_decl_annotations_
VOID
Sqlite3ExtDestroyAllocator(
    PALLOCATOR Allocator
    )
{
    return;
}

_Use_decl_annotations_
BOOLEAN
Sqlite3ExtInitializeAllocator(
    PALLOCATOR Allocator
    )
{
    if (!Allocator) {
        return FALSE;
    }

    InitializeAllocator(
        Allocator,
        Allocator,
        Sqlite3ExtMalloc,
        Sqlite3ExtCalloc,
        Sqlite3ExtRealloc,
        Sqlite3ExtFree,
        Sqlite3ExtFreePointer,
        Sqlite3ExtInitializeAllocator,
        Sqlite3ExtDestroyAllocator,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    );

    return TRUE;
}

_Use_decl_annotations_
VOID
InitializeAllocatorFromSqlite3(
    PALLOCATOR Allocator,
    PCSQLITE3 Sqlite3
    )
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Allocator)) {
        return;
    }

    if (!ARGUMENT_PRESENT(Sqlite3)) {
        return;
    }

    //
    // Initialize the allocator, then set the Sqlite3 address to Context2.
    //

    Sqlite3ExtInitializeAllocator(Allocator);
    Allocator->Context2 = (PVOID)Sqlite3;

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
