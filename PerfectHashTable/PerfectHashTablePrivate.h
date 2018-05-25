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
    // Pointer to an initialized RTL structure.
    //

    PRTL Rtl;

    //
    // Pointer to an initialized ALLOCATOR structure.
    //

    PALLOCATOR Allocator;

    //
    // Pointer to the API structure in use.
    //

    PPERFECT_HASH_TABLE_ANY_API AnyApi;

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
// Algorithms are required to register a callback routine with the perfect hash
// table context that matches the following signature.  This routine will be
// called for each work item it pushes to the context's main threadpool, with
// a pointer to the SLIST_ENTRY that was popped off the list.
//

typedef
VOID
(CALLBACK PERFECT_HASH_TABLE_WORK_CALLBACK)(
    _In_ PPERFECT_HASH_TABLE_CONTEXT Context,
    _In_ PSLIST_ENTRY ListEntry
    );
typedef PERFECT_HASH_TABLE_WORK_CALLBACK *PPERFECT_HASH_TABLE_WORK_CALLBACK;

//
// Define a runtime context to encapsulate threadpool resources.  This is
// passed to CreatePerfectHashTable() and allows for algorithms to search for
// perfect hash solutions in parallel.
//

typedef union _PERFECT_HASH_TABLE_CONTEXT_FLAGS {
    struct {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_CONTEXT_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_CONTEXT_FLAGS) == sizeof(ULONG));
typedef PERFECT_HASH_TABLE_CONTEXT_FLAGS *PPERFECT_HASH_TABLE_CONTEXT_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE_CONTEXT {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _PERFECT_HASH_TABLE_CONTEXT))
        ULONG SizeOfStruct;

    //
    // Flags.
    //

    PERFECT_HASH_TABLE_CONTEXT_FLAGS Flags;

    //
    // Pointer to an initialized RTL structure.
    //

    PRTL Rtl;

    //
    // Pointer to an intialized allocator.
    //

    PALLOCATOR Allocator;

    //
    // Pointer to the API structure in use.
    //

    PPERFECT_HASH_TABLE_ANY_API AnyApi;

    //
    // Define the events used to communicate various internal state changes
    // between the CreatePerfectHashTable() function and the algorithm-specific
    // creation routine.
    //
    // N.B. All of these events are created with the manual reset flag set to
    //      TRUE, such that they stay signalled even after they have satisfied
    //      a wait.
    //

    //
    // A global "shutdown" event handle that threads can query to determine
    // whether or not they should continue processing at various internal
    // checkpoints.
    //

    union {
        HANDLE ShutdownEvent;
        PVOID FirstEvent;
    };

    //
    // This event will be set if an algorithm was successful in finding a
    // perfect hash.  Either it or the FailedEvent will be set; never both.
    //

    HANDLE SucceededEvent;

    //
    // This event will be set if an algorithm failed to find a perfect hash
    // solution.  This may be due to the algorithm exhausting all possible
    // options, hitting a time limit, or potentially as a result of being
    // forcibly terminated or some other internal error.  It will never be
    // set if SucceededEvent is also set.
    //

    HANDLE FailedEvent;

    //
    // The following event is required to be set by an algorithm's creation
    // routine upon completion (regardless of success or failure).  This event
    // is waited upon by the CreatePerfectHashTable() function, and thus, is
    // critical in synchronizing the execution of parallel perfect hash solution
    // finding.
    //

    union {
        HANDLE CompletedEvent;
        PVOID LastEvent;
    };

    //
    // N.B. All events are created as named events, using the random object
    //      name generation helper Rtl->CreateRandomObjectNames().  This will
    //      fill out an array of PUNICODE_STRING pointers.  The next field
    //      points to the first element of that array.  Subsequent fields
    //      capture various book-keeping items about the random object names
    //      allocation (provided by the Rtl routine).
    //

    PPUNICODE_STRING ObjectNames;
    PWSTR ObjectNamesWideBuffer;
    ULONG SizeOfObjectNamesWideBuffer;

    //
    // We fill in the following field for convenience.  This also aligns us
    // out to an 8 byte boundary.
    //

    ULONG NumberOfObjects;

    //
    // The main threadpool callback environment, used for processing general
    // work items (such as parallel perfect hash solution finding).
    //

    TP_CALLBACK_ENVIRON MainCallbackEnv;
    PTP_CLEANUP_GROUP MainCleanupGroup;
    PTP_POOL MainThreadpool;
    PTP_WORK MainWork;
    SLIST_HEADER MainListHead;
    ULONG MinimumConcurrency;
    ULONG MaximumConcurrency;

    //
    // The algorithm is responsible for registering an appropriate callback
    // for main thread work items in this next field.
    //

    PPERFECT_HASH_TABLE_WORK_CALLBACK MainCallback;

    //
    // If a threadpool worker thread finds a perfect hash solution, it will
    // enqueue a "Finished!"-type work item to a separate threadpool, captured
    // by the following callback environment.  This allows for a separate
    // threadpool worker to schedule the cancellation of other in-progress
    // and outstanding perfect hash solution attempts without deadlocking.
    //
    // This threadpool environment is serviced by a single thread.
    //

    TP_CALLBACK_ENVIRON FinishedCallbackEnv;
    PTP_POOL FinishedThreadpool;
    PTP_WORK FinishedWork;
    SLIST_HEADER FinishedListHead;

    //
    // If a worker thread successfully finds a perfect hash solution, it will
    // push its solution to the FinishedListHead above, then submit a finished
    // work item via SubmitThreadpoolWork(Context->FinishedWork).
    //
    // This callback will be processed by the finished group above, and provides
    // a means for that thread to set the ShutdownEvent and cancel outstanding
    // main work callbacks.
    //
    // N.B. Although we only need one solution, we don't prevent multiple
    //      successful solutions from being pushed to the FinishedListHead.
    //      Whatever the first solution is that the finished callback pops
    //      off that list is the solution that wins.
    //

    volatile ULONGLONG FinishedCount;

    //
    // Similar to the Finished group above, provide an Error group that also
    // consists of a single thread.  If a main threadpool worker thread runs
    // into a fatal error that requires termination of all in-progress and
    // outstanding threadpool work items, it can just dispatch a work item
    // to this particular pool (e.g. SubmitThreadpoolWork(Context->ErrorWork)).
    //
    // There is no ErrorListHead as no error information is captured that needs
    // communicating back to a central location.
    //

    TP_CALLBACK_ENVIRON ErrorCallbackEnv;
    PTP_POOL ErrorThreadpool;
    PTP_WORK ErrorWork;

} PERFECT_HASH_TABLE_CONTEXT;
typedef PERFECT_HASH_TABLE_CONTEXT *PPERFECT_HASH_TABLE_CONTEXT;

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
    // Pointer to the API structure in use.
    //

    PPERFECT_HASH_TABLE_ANY_API AnyApi;

    //
    // Pointer to the keys corresponding to this perfect hash table.  May be
    // NULL.
    //

    PPERFECT_HASH_TABLE_KEYS Keys;

    //
    // Pointer to the PERFECT_HASH_TABLE_CONTEXT structure in use.
    //

    PPERFECT_HASH_TABLE_CONTEXT Context;

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

typedef struct _PERFECT_HASH_TABLE_TLS_CONTEXT {
    PVOID Unused;
} PERFECT_HASH_TABLE_TLS_CONTEXT;
typedef PERFECT_HASH_TABLE_TLS_CONTEXT *PPERFECT_HASH_TABLE_TLS_CONTEXT;

extern ULONG PerfectHashTableTlsIndex;

//
// Function typedefs for private functions.
//

//
// Each algorithm implements a creation routine that matches the following
// signature.  It is called by CreatePerfectHashTable() after it has done all
// the initial heavy-lifting (e.g. parameter validation, table allocation and
// initialization), and thus, has a much simpler function signature.
//

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI CREATE_PERFECT_HASH_TABLE_IMPL)(
    _Inout_ PPERFECT_HASH_TABLE Table
    );
typedef CREATE_PERFECT_HASH_TABLE_IMPL *PCREATE_PERFECT_HASH_TABLE_IMPL;

//
// For each algorithm, declare the creation impl routine.  These are gathered
// in an array named CreationRoutines[] (see PerfectHashTableConstants.[ch]).
//

CREATE_PERFECT_HASH_TABLE_IMPL CreatePerfectHashTableImplChm01;

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

//
// Inline helper functions.
//

#define MAX_RDRAND_RETRY_COUNT 10

FORCEINLINE
BOOLEAN
GetRandomSeeds(
    _Out_ PULARGE_INTEGER Output,
    _Out_opt_ PULARGE_INTEGER Cycles,
    _Out_opt_ PULONG Attempts
    )
/*++

Routine Description:

    Generates a 64-bit random seed using the rdrand64 intrinisic.

Arguments:

    Output - Supplies a pointer to a ULARGE_INTEGER structure that will receive
        the random seed value.

    Cycles - Optionally supplies a pointer to a variable that will receive the
        approximate number of CPU cycles that were required in order to fulfil
        the random seed request.

        N.B. This calls __rdtsc() before and after the __rdseed64_step() call.
             If the pointer is NULL, __rdtsc() is not called either before or
             after.

    Attempts - Optionally supplies the address of a variable that receives the
        number of attempts it took before __rdseed64_step() succeeded.  (This
        is bound by the MAX_RDRAND_RETRY_COUNT constant.)

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    ULONG Index;
    BOOLEAN Success = FALSE;
    ULARGE_INTEGER Start;
    ULARGE_INTEGER End;

    if (ARGUMENT_PRESENT(Cycles)) {
        Start.QuadPart = ReadTimeStampCounter();
    }

    for (Index = 0; Index < MAX_RDRAND_RETRY_COUNT; Index++) {
        if (_rdseed64_step(&Output->QuadPart)) {
            Success = TRUE;
            break;
        }
        YieldProcessor();
    }

    if (ARGUMENT_PRESENT(Cycles)) {
        End.QuadPart = ReadTimeStampCounter();
        Cycles->QuadPart = End.QuadPart - Start.QuadPart;
    }

    if (ARGUMENT_PRESENT(Attempts)) {
        *Attempts = Index + 1;
    }

    return Success;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
