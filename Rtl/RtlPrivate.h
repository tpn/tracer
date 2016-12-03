/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    RtlPrivate.h

Abstract:

    This is the private header file for the Rtl component.  It defines function
    typedefs and function declarations for all major (i.e. not local to the
    module) functions available for use by individual modules within this
    component.

--*/

#ifndef _RTL_INTERNAL_BUILD
#error RtlPrivate.h being included but _RTL_INTERNAL_BUILD not set.
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// RtlAtExitRundown-related structures and functions.
//

typedef struct _RTL_ATEXIT_RUNDOWN_FLAGS {
    ULONG IsActive:1;
} RTL_ATEXIT_RUNDOWN_FLAGS, *PRTL_ATEXIT_RUNDOWN_FLAGS;

typedef _Struct_size_bytes_(SizeOfStruct) struct _RTL_ATEXIT_RUNDOWN {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_ATEXIT_RUNDOWN)) USHORT SizeOfStruct;

    //
    // Pad out to 4-bytes.
    //

    USHORT Padding1;

    //
    // Flags.
    //

    RTL_ATEXIT_RUNDOWN_FLAGS Flags;

    //
    // Critical section protecting the rundown list head.
    //

    CRITICAL_SECTION CriticalSection;

    //
    // Rundown list head.
    //

    _Guarded_by_(CriticalSection)
    LIST_ENTRY ListHead;


} RTL_ATEXIT_RUNDOWN, *PRTL_ATEXIT_RUNDOWN;

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _RTL_ATEXIT_ENTRY_FLAGS {

    //
    // When set, indicates that the caller's atexit() function will be wrapped
    // in a __try/__except SEH block that suppresses all exceptions.
    //

    ULONG SuppressExceptions:1;

} RTL_ATEXIT_ENTRY_FLAGS, *PRTL_ATEXIT_ENTRY_FLAGS;
C_ASSERT(sizeof(RTL_ATEXIT_ENTRY_FLAGS) == sizeof(ULONG));

//
// The following structure is used to encapsulate a caller's atexit function
// pointer within a structure that can be added to the rundown list via the
// standard doubly-linked list facilities.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_ATEXIT_ENTRY {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_ATEXIT_RUNDOWN)) USHORT SizeOfStruct;

    //
    // Pad out to 4 bytes.
    //

    USHORT Padding1;

    //
    // Flags.
    //

    RTL_ATEXIT_ENTRY_FLAGS Flags;

    //
    // Pointer to the caller's function to be called at exit.
    //

    PATEXITFUNC AtExitFunc;

    //
    // List entry to allow the structure to be registered with the ListHead
    // field of the RTL_ATEXIT_RUNDOWN structure.
    //

    LIST_ENTRY ListEntry;

    //
    // Pointer to the rundown structure we were added to.
    //

    PRTL_ATEXIT_RUNDOWN Rundown;

    //
    // Pad out to 64 bytes.  (Currently at 40 bytes after Rundown.)
    //

    ULONGLONG Padding2[3];

} RTL_ATEXIT_ENTRY, *PRTL_ATEXIT_ENTRY, **PPRTL_ATEXIT_ENTRY;
C_ASSERT(sizeof(RTL_ATEXIT_ENTRY) == 64);

//
// RtlAtExitRundown-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_RTL_ATEXIT_ENTRY)(
    _In_ PATEXITFUNC AtExitFunc,
    _Outptr_result_nullonfailure_ PPRTL_ATEXIT_ENTRY AtExitEntryPointer
    );
typedef CREATE_RTL_ATEXIT_ENTRY *PCREATE_RTL_ATEXIT_ENTRY;
CREATE_RTL_ATEXIT_ENTRY CreateRtlAtExitEntry;

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
CreateRtlAtExitEntryInline(
    _In_ PATEXITFUNC AtExitFunc,
    _Outptr_result_nullonfailure_ PPRTL_ATEXIT_ENTRY EntryPointer
    )
/*++

Routine Description:

    This routine creates an RTL_ATEXIT_ENTRY structure for a given ATEXITFUNC
    function pointer.  Memory is allocated from the default process heap.

Arguments:

    AtExitFunc - Supplies a pointer to an ATEXITFUNC function pointer.  This
        will be the value of whatever the caller originally called atexit()
        with.

    EntryPointer - Supplies the address of a variable that will receive the
        address of the newly allocated RTL_ATEXIT_ENTRY structure on success,
        or a NULL value on failure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    DWORD Flags;
    HANDLE HeapHandle;
    PRTL_ATEXIT_ENTRY Entry;

    //
    // Clear the caller's pointer.
    //

    *EntryPointer = NULL;

    //
    // Initialize local variables;
    //

    Flags = HEAP_ZERO_MEMORY;
    HeapHandle = GetProcessHeap();

    //
    // Attempt to allocate memory for the entry.
    //

    Entry = (PRTL_ATEXIT_ENTRY)HeapAlloc(HeapHandle, Flags, sizeof(*Entry));
    if (!Entry) {
        return FALSE;
    }

    //
    // Allocation succeeded.  Complete initialization of the structure.
    //

    Entry->SizeOfStruct = sizeof(*Entry);
    Entry->AtExitFunc = AtExitFunc;
    InitializeListHead(&Entry->ListEntry);

    //
    // Update the caller's pointer.
    //

    *EntryPointer = Entry;

    return TRUE;
}

typedef
_Requires_lock_not_held_(Rundown->CriticalSection)
VOID
(RUNDOWN_ATEXIT_FUNCTIONS)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_opt_ BOOL IsProcessTerminating
    );
typedef RUNDOWN_ATEXIT_FUNCTIONS *PRUNDOWN_ATEXIT_FUNCTIONS;
RUNDOWN_ATEXIT_FUNCTIONS RundownAtExitFunctions;

typedef
BOOL
(IS_RTL_ATEXIT_RUNDOWN_ACTIVE)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown
    );
typedef IS_RTL_ATEXIT_RUNDOWN_ACTIVE *PIS_RTL_ATEXIT_RUNDOWN_ACTIVE;
IS_RTL_ATEXIT_RUNDOWN_ACTIVE IsRtlAtExitRundownActive;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_RTL_ATEXIT_RUNDOWN)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown
    );
typedef INITIALIZE_RTL_ATEXIT_RUNDOWN \
      *PINITIALIZE_RTL_ATEXIT_RUNDOWN;
INITIALIZE_RTL_ATEXIT_RUNDOWN InitializeRtlAtExitRundown;

typedef
VOID
(DESTROY_RTL_ATEXIT_RUNDOWN)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown
    );
typedef DESTROY_RTL_ATEXIT_RUNDOWN \
      *PDESTROY_RTL_ATEXIT_RUNDOWN;
DESTROY_RTL_ATEXIT_RUNDOWN DestroyRtlAtExitRundown;

typedef
_Requires_lock_held_(Rundown->CriticalSection)
VOID
(ADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_ PRTL_ATEXIT_ENTRY AtExitEntry
    );
typedef ADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN *PADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN;
ADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN AddRtlAtExitEntryToRundown;

typedef
_Requires_lock_held_(AtExitFunc->Rundown->CriticalSection)
VOID
(REMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN)(
    _In_ PATEXITFUNC AtExitFunc
    );
typedef REMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN \
      *PREMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN;
REMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN RemoveRtlAtExitEntryFromRundown;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(REGISTER_ATEXITFUNC)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_ PATEXITFUNC AtExitFunc
    );
typedef REGISTER_ATEXITFUNC *PREGISTER_ATEXITFUNC;
REGISTER_ATEXITFUNC RegisterAtExitFunc;

//
// RtlGlobalAtExitRundown-related functions.
//

typedef
PRTL_ATEXIT_RUNDOWN
(GET_GLOBAL_RTL_ATEXIT_RUNDOWN)(
    VOID
    );
typedef GET_GLOBAL_RTL_ATEXIT_RUNDOWN *PGET_GLOBAL_RTL_ATEXIT_RUNDOWN;
GET_GLOBAL_RTL_ATEXIT_RUNDOWN GetGlobalRtlAtExitRundown;

typedef
BOOL
(IS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE)(
    VOID
    );
typedef IS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE \
      *PIS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE;
IS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE IsGlobalRtlAtExitRundownActive;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(REGISTER_GLOBAL_ATEXITFUNC)(
    _In_ PATEXITFUNC AtExitFunc
    );
typedef REGISTER_GLOBAL_ATEXITFUNC *PREGISTER_GLOBAL_ATEXITFUNC;
REGISTER_GLOBAL_ATEXITFUNC RegisterGlobalAtExitFunc;

typedef
VOID
(RUNDOWN_GLOBAL_ATEXIT_FUNCTIONS)(
    _In_opt_ BOOL IsProcessTerminating
    );
typedef RUNDOWN_GLOBAL_ATEXIT_FUNCTIONS *PRUNDOWN_GLOBAL_ATEXIT_FUNCTIONS;
RUNDOWN_GLOBAL_ATEXIT_FUNCTIONS RundownGlobalAtExitFunctions;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN)(
    VOID
    );
typedef INITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN \
      *PINITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN;
INITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN InitializeGlobalRtlAtExitRundown;

typedef
VOID
(DESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN)(
    VOID
    );
typedef DESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN \
      *PDESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN;
DESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN DestroyGlobalRtlAtExitRundown;

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
