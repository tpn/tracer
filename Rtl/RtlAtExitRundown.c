/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    RtlAtExitRundown.c

Abstract:

    This module implements functionality related to running down the structure
    RTL_ATEXIT_RUNDOWN.  In this context, rundown refers to walking the doubly-
    linked list of RTL_ATEXIT_ENTRY structures and calling the registered exit
    function (AtExitFunc) on each one.

--*/

#include "stdafx.h"

#define RTL_ATEXIT_RUNDOWN_CRITICAL_SECTION_SPIN_COUNT 4000

_Use_decl_annotations_
BOOL
InitializeRtlAtExitRundown(
    PRTL_ATEXIT_RUNDOWN Rundown
    )
/*++

Routine Description:

    This routine initializes an RTL_ATEXIT_RUNDOWN structure.

    This currently involves initializing the critical section and the rundown
    list head.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure to
        initialize.

Return Value:

    TRUE on success, FALSE on failure.  Failure will be because the critical
    section could not be initialized.

--*/
{
    BOOL Success;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rundown)) {
        return FALSE;
    }

    Rundown->SizeOfStruct = sizeof(*Rundown);

    Success = InitializeCriticalSectionAndSpinCount(
        &Rundown->CriticalSection,
        RTL_ATEXIT_RUNDOWN_CRITICAL_SECTION_SPIN_COUNT
    );

    if (!Success) {
        return FALSE;
    }

    EnterCriticalSection(&Rundown->CriticalSection);
    InitializeListHead(&Rundown->ListHead);
    LeaveCriticalSection(&Rundown->CriticalSection);

    return TRUE;
}

_Use_decl_annotations_
VOID
DestroyRtlAtExitRundown(
    PRTL_ATEXIT_RUNDOWN Rundown
    )
/*++

Routine Description:

    This routine destroys an initialized RTL_ATEXIT_RUNDOWN structure.

    This currently involves destroying the critical section.  This should only
    be called after RtlRundownAtExitFunctions() has been called.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure to
        initialize.

Return Value:

    None.

--*/
{
    if (!ARGUMENT_PRESENT(Rundown)) {
        return;
    }

    if (!IsListEmpty(&Rundown->ListHead)) {
        __debugbreak();
    }

    DeleteCriticalSection(&Rundown->CriticalSection);

    return;
}

_Use_decl_annotations_
BOOL
CreateRtlAtExitEntry(
    PATEXITFUNC AtExitFunc,
    PPRTL_ATEXIT_ENTRY EntryPointer
    )
/*++

Routine Description:

    This routine calls CreateRtlAtExitEntryInline().  See documentation for
    that routine for more information.

--*/
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(AtExitFunc)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(EntryPointer)) {
        return FALSE;
    }

    return CreateRtlAtExitEntryInline(AtExitFunc, EntryPointer);
}

_Use_decl_annotations_
VOID
AddRtlAtExitEntryToRundown(
    PRTL_ATEXIT_RUNDOWN Rundown,
    PRTL_ATEXIT_ENTRY AtExitEntry
    )
/*++

Routine Description:

    This routine adds the atexit entry to the rundown list.  The rundown lock
    must have been acquired before this routine is called.  The entry is added
    to the head of the list.  (When the list is rundown, entries are also
    removed from the head of the list.  This results in a last-in, first-out
    (LIFO) ordering, which matches the CRT's atexit() ordering.)

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure.

    AtExitEntry - Supplies a pointer to an RTL_ATEXIT_ENTRY structure.

Return Value:

    None.

--*/
{
    InsertHeadList(&Rundown->ListHead, &AtExitEntry->ListEntry);
}

_Use_decl_annotations_
BOOL
RegisterAtExitFunc(
    PRTL_ATEXIT_RUNDOWN Rundown,
    PATEXITFUNC AtExitFunc
    )
/*++

Routine Description:

    This routine creates a new RTL_ATEXIT_ENTRY structure for the given function
    pointer, acquires the rundown critical section, adds the entry to the list
    and then releases the critical section.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure.

    AtExitFunc - Supplies a pointer to an ATEXITFUNC to be called at rundown.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL_ATEXIT_ENTRY Entry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rundown)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AtExitFunc)) {
        return FALSE;
    }

    //
    // Attempt to create a new entry to add to the rundown list.
    //

    if (!CreateRtlAtExitEntry(AtExitFunc, &Entry)) {
        return FALSE;
    }

    //
    // Point the entry at its parent.
    //

    Entry->Rundown = Rundown;

    //
    // Acquire the rundown lock, add the entry, release the lock.
    //

    EnterCriticalSection(&Rundown->CriticalSection);

    __try {
        AddRtlAtExitEntryToRundown(Rundown, Entry);
    } __finally {
        LeaveCriticalSection(&Rundown->CriticalSection);
    }

    //
    // Return success.
    //

    return TRUE;
}

_Use_decl_annotations_
VOID
RundownAtExitFunctions(
    PRTL_ATEXIT_RUNDOWN Rundown,
    BOOL IsProcessTerminating
    )
/*++

Routine Description:

    This routine "runs down" all atexit functions associated with the rundown
    structure.  Running down an atexit function involves removing it from the
    rundown list, then calling the function.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure.

    IsProcessTerminating - Supplies a boolean variable that indicates if the
        function is being called due to process termination.  When FALSE,
        implies the function is being called because FreeLibrary() was
        called and there were no more references left to the DLL, in which
        case, HeapFree() will also be called on all allocated entries.

Return Value:

    None.

--*/
{
    HANDLE HeapHandle;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEntry;
    PATEXITFUNC AtExitFunc;
    PRTL_ATEXIT_ENTRY AtExitEntry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rundown)) {
        return;
    }

    //
    // If we're not being called as a result of process rundown, we explicitly
    // free atexit entries as we walk the rundown list.  This ensures we don't
    // leak memory if we are being rundown due to FreeLibrary() removing the
    // last reference to us.
    //

    if (!IsProcessTerminating) {
        HeapHandle = GetProcessHeap();
    }

    //
    // Acquire the rundown lock for the duration of the routine.
    //

    EnterCriticalSection(&Rundown->CriticalSection);

    Rundown->Flags.IsActive = TRUE;

    __try {

        ListHead = &Rundown->ListHead;

        while (!IsListEmpty(ListHead)) {

            //
            // Remove entries from the head in order to maintain LIFO ordering.
            //

            ListEntry = RemoveHeadList(ListHead);

            //
            // Resolve the underlying atexit entry and corresponding function
            // pointer provided by the caller when they called atexit().
            //

            AtExitEntry = CONTAINING_RECORD(ListEntry,
                                            RTL_ATEXIT_ENTRY,
                                            ListEntry);

            AtExitFunc = AtExitEntry->AtExitFunc;

            //
            // Invariant check: entry's rundown should match the rundown we're
            // enumerating.
            //

            if (AtExitEntry->Rundown != Rundown) {
                __debugbreak();
            }

            //
            // If the entry's suppress exceptions bit is set, wrap invocation
            // of the caller's atexit function in an exception handler.
            //

            if (AtExitEntry->Flags.SuppressExceptions) {

                __try {
                    AtExitFunc();
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    NOTHING;
                }

            } else {

                //
                // Call the function normally.
                //

                AtExitFunc();
            }

            //
            // If we're not terminating, free the memory backing the entry.
            //

            if (!IsProcessTerminating) {
                HeapFree(HeapHandle, 0, AtExitEntry);
            }
        }

    } __finally {

        LeaveCriticalSection(&Rundown->CriticalSection);
        Rundown->Flags.IsActive = FALSE;
    }

    return;
}

_Use_decl_annotations_
BOOL
IsRtlAtExitRundownActive(
    PRTL_ATEXIT_RUNDOWN Rundown
    )
/*++

Routine Description:

    This routine indicates if a rundown is currently active.  An active rundown
    is one that is currently in the RtlRundownAtExitFunctions() function.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure.

Return Value:

    TRUE if active, FALSE otherwise.

--*/
{
    return Rundown->Flags.IsActive;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
