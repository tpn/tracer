/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHashTablePrivate.h

Abstract:

    This is the private header file for the PerfectHashTable component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#ifndef _PERFECT_HASH_TABLE_INTERNAL_BUILD
#error PerfectHashTablePrivate.h being included but _PERFECT_HASH_TABLE_INTERNAL_BUILD not set.
#endif

#pragma once

#include "stdafx.h"

#ifndef ASSERT
#define ASSERT(Condition)   \
    if (!(Condition)) {     \
        __debugbreak();     \
    }
#endif

//
// Define the PERFECT_HASH_TABLE_KEYS_FLAGS structure.
//

typedef union _PERFECT_HASH_TABLE_FLAGS_KEYS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Unused bits.
        //

        ULONG Unused:32;
    };

    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_KEYS_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_KEYS_FLAGS) == sizeof(ULONG));
typedef PERFECT_HASH_TABLE_KEYS_FLAGS *PPERFECT_HASH_TABLE_KEYS_FLAGS;

//
// Define the PERFECT_HASH_TABLE_KEYS structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE_KEYS {

    //
    // Reserve a slot for a vtable.  Currently unused.
    //

    PPVOID Vtbl;

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PERFECT_HASH_TABLE_KEYS))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    PERFECT_HASH_TABLE_KEYS_FLAGS Flags;

    //
    // Number of keys in the mapping.
    //

    LARGE_INTEGER NumberOfElements;

    //
    // Handle to the underlying keys file.
    //

    HANDLE FileHandle;

    //
    // Handle to the memory mapping for the keys file.
    //

    HANDLE MappingHandle;

    //
    // Base address of the memory map.
    //

    PVOID BaseAddress;

    //
    // Fully-qualifed, NULL-terminated path of the source keys file.
    //

    UNICODE_STRING Path;

} PERFECT_HASH_TABLE_KEYS;
typedef PERFECT_HASH_TABLE_KEYS *PPERFECT_HASH_TABLE_KEYS;

//
// Define the PERFECT_HASH_TABLE_FLAGS structure.
//

typedef union _PERFECT_HASH_TABLE_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Unused bits.
        //

        ULONG Unused:32;
    };

    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_FLAGS) == sizeof(ULONG));
typedef PERFECT_HASH_TABLE_FLAGS *PPERFECT_HASH_TABLE_FLAGS;

//
// Define the PERFECT_HASH_TABLE structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE {

    //
    // Reserve a slot for a vtable.  Currently unused.
    //

    PPVOID Vtbl;

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PERFECT_HASH_TABLE)) ULONG SizeOfStruct;

    //
    // PerfectHashTable flags.
    //

    PERFECT_HASH_TABLE_FLAGS Flags;

    //
    // Pointer to an initialized RTL structure.
    //

    PRTL Rtl;

    //
    // Pointer to an initialized ALLOCATOR structure.
    //

    PALLOCATOR Allocator;

    //
    // Pointer to the keys corresponding to this perfect hash table.  May be
    // NULL.
    //

    PPERFECT_HASH_TABLE_KEYS Keys;

    //
    // Handle to the backing file.
    //

    HANDLE FileHandle;

    //
    // Handle to the memory mapping for the backing file.
    //

    HANDLE MappingHandle;

    //
    // Base address of the memory map for the backing file.
    //

    PVOID BaseAddress;

    //
    // Fully-qualified, NULL-terminated path of the backing file.  The path is
    // automatically derived from the keys file.
    //

    UNICODE_STRING Path;

} PERFECT_HASH_TABLE;
typedef PERFECT_HASH_TABLE *PPERFECT_HASH_TABLE;

//
// TLS-related structures and functions.
//

typedef struct _PERFECT_HASH_TABLE_CONTEXT {
    PPERFECT_HASH_TABLE PerfectHashTable;
} PERFECT_HASH_TABLE_CONTEXT;
typedef PERFECT_HASH_TABLE_CONTEXT *PPERFECT_HASH_TABLE_CONTEXT;

extern ULONG PerfectHashTableTlsIndex;

//
// Function typedefs for private functions.
//


//
// The PROCESS_ATTACH and PROCESS_ATTACH functions share the same signature.
//

typedef
_Check_return_
_Success_(return != 0)
(PERFECT_HASH_TABLE_TLS_FUNCTION)(
    _In_    HMODULE     Module,
    _In_    DWORD       Reason,
    _In_    LPVOID      Reserved
    );
typedef PERFECT_HASH_TABLE_TLS_FUNCTION *PPERFECT_HASH_TABLE_TLS_FUNCTION;

PERFECT_HASH_TABLE_TLS_FUNCTION PerfectHashTableTlsProcessAttach;
PERFECT_HASH_TABLE_TLS_FUNCTION PerfectHashTableTlsProcessDetach;

//
// Define TLS Get/Set context functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(PERFECT_HASH_TABLE_TLS_SET_CONTEXT)(
    _In_ struct _PERFECT_HASH_TABLE_CONTEXT *Context
    );
typedef PERFECT_HASH_TABLE_TLS_SET_CONTEXT *PPERFECT_HASH_TABLE_TLS_SET_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
struct _PERFECT_HASH_TABLE_CONTEXT *
(PERFECT_HASH_TABLE_TLS_GET_CONTEXT)(
    VOID
    );
typedef PERFECT_HASH_TABLE_TLS_GET_CONTEXT *PPERFECT_HASH_TABLE_TLS_GET_CONTEXT;

extern PERFECT_HASH_TABLE_TLS_SET_CONTEXT PerfectHashTableTlsSetContext;
extern PERFECT_HASH_TABLE_TLS_GET_CONTEXT PerfectHashTableTlsGetContext;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
