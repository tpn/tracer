/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoresRundown.c

Abstract:

    This module implements functionality related to running down a trace stores
    structure.  Rundown refers to walking the doubly-linked list of all trace
    stores structures and calling RundownTraceStores() on each one, ensuring the
    truncation logic is applied where applicable.

--*/

#include "stdafx.h"

#define TRACE_STORES_RUNDOWN_CRITICAL_SECTION_SPIN_COUNT 4000

_Use_decl_annotations_
BOOL
InitializeTraceStoresRundown(
    PTRACE_STORES_RUNDOWN Rundown
    )
/*++

Routine Description:

    This routine initializes a TRACE_STORES_RUNDOWN structure.

    This currently involves initializing the critical section and the rundown
    list head.

Arguments:

    Rundown - Supplies a pointer to a TRACE_STORES_RUNDOWN structure to
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
        TRACE_STORES_RUNDOWN_CRITICAL_SECTION_SPIN_COUNT
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
DestroyTraceStoresRundown(
    PTRACE_STORES_RUNDOWN Rundown
    )
/*++

Routine Description:

    This routine destroys an initialized TRACE_STORES_RUNDOWN structure.

    This currently involves destroying the critical section.  This should only
    be called after RundownTraceStores() has been called.

Arguments:

    Rundown - Supplies a pointer to a TRACE_STORES_RUNDOWN structure to
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
VOID
AddTraceStoresToRundown(
    PTRACE_STORES_RUNDOWN Rundown,
    PTRACE_STORES TraceStores
    )
/*++

Routine Description:

    This routine adds the trace stores to the rundown list.  The rundown lock
    must have been acquired before this routine is called.

Arguments:

    Rundown - Supplies a pointer to a TRACE_STORES_RUNDOWN structure.

    TraceStores - Supplies a pointer to a TRACE_STORES structure.

Return Value:

    None.

--*/
{
    InsertTailList(&Rundown->ListHead, &TraceStores->RundownListEntry);
}

_Use_decl_annotations_
VOID
RemoveTraceStoresFromRundown(
    PTRACE_STORES TraceStores
    )
/*++

Routine Description:

    This routine removes the trace stores from the rundown list.  The rundown
    lock must have been acquired before this routine is called.

Arguments:

    TraceStores - Supplies a pointer to a TRACE_STORES structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    RemoveEntryList(&TraceStores->RundownListEntry);
}

_Use_decl_annotations_
BOOL
RegisterTraceStores(
    PTRACE_STORES_RUNDOWN Rundown,
    PTRACE_STORES TraceStores
    )
/*++

Routine Description:

    This routine acquires the lock for the rundown structure, adds the trace
    stores structure to the rundown list, then releases the lock.

Arguments:

    Rundown - Supplies a pointer to a TRACE_STORES_RUNDOWN structure.

    TraceStores - Supplies a pointer to a TRACE_STORES structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rundown)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStores)) {
        return FALSE;
    }

    //
    // Ensure the trace stores list entry isn't pointing at something else.
    //

    if (!IsListEmpty(&TraceStores->RundownListEntry)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Ensure the trace stores rundown pointer is NULL;
    //

    if (TraceStores->Rundown != NULL) {
        __debugbreak();
        return FALSE;
    }

    TraceStores->Rundown = Rundown;

    EnterCriticalSection(&Rundown->CriticalSection);

    //
    // Add the trace stores to the list.
    //

    __try {
        AddTraceStoresToRundown(Rundown, TraceStores);
    } __finally {
        LeaveCriticalSection(&Rundown->CriticalSection);
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
UnregisterTraceStores(
    PTRACE_STORES TraceStores
    )
/*++

Routine Description:

    This routine acquires the lock for the rundown structure, removes the trace
    stores structure from the rundown list, then releases the lock.  The rundown
    structure is derived from the TraceStores->Rundown field.

Arguments:

    TraceStores - Supplies a pointer to a TRACE_STORES structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACE_STORES_RUNDOWN Rundown;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStores)) {
        return FALSE;
    }

    //
    // Ensure the trace stores list entry is linked to something.
    //

    if (IsListEmpty(&TraceStores->RundownListEntry)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Ensure there's a valid rundown pointer.
    //

    if (TraceStores->Rundown == NULL) {
        __debugbreak();
        return FALSE;
    }

    Rundown = TraceStores->Rundown;

    EnterCriticalSection(&Rundown->CriticalSection);

    //
    // Remove the trace store from the list.
    //

    __try {
        RemoveTraceStoresFromRundown(TraceStores);
        TraceStores->Rundown = NULL;
    } __finally {
        LeaveCriticalSection(&Rundown->CriticalSection);
    }

    return TRUE;
}

_Use_decl_annotations_
VOID
RundownTraceStores(
    PTRACE_STORES_RUNDOWN Rundown
    )
/*++

Routine Description:

    This routine "runs down" all trace stores associated with the rundown
    structure.  Running down a trace stores structure currently just involves
    calling CloseTraceStores() against the structure once it has been removed
    from the rundown linked list.

Arguments:

    Rundown - Supplies a pointer to a TRACE_STORES_RUNDOWN structure.

Return Value:

    None.

--*/
{
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEntry;
    PTRACE_STORES TraceStores;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rundown)) {
        return;
    }

    //
    // Acquire the rundown lock for the duration of the routine.
    //

    EnterCriticalSection(&Rundown->CriticalSection);

    Rundown->Flags.IsActive = TRUE;

    __try {

        ListHead = &Rundown->ListHead;

        while (!IsListEmpty(ListHead)) {

            ListEntry = RemoveHeadList(ListHead);

            TraceStores = CONTAINING_RECORD(
                ListEntry,
                TRACE_STORES,
                RundownListEntry
            );

            if (TraceStores->Rundown != Rundown) {
                __debugbreak();
            }

            RundownTraceStoresInline(TraceStores);

            TraceStores->Rundown = NULL;
        }

    } __finally {

        LeaveCriticalSection(&Rundown->CriticalSection);
        Rundown->Flags.IsActive = FALSE;
    }

    return;
}

_Use_decl_annotations_
BOOL
IsTraceStoresRundownActive(
    PTRACE_STORES_RUNDOWN Rundown
    )
/*++

Routine Description:

    This routine indicates if a rundown is currently active.  An active rundown
    is one that is currently in the RundownTraceStores() function.  This is
    used by the CloseTraceStore() routine to determine how much cleanup work
    needs to be done; if we're running down, it does the absolute minimum work
    (truncates then closes the file) and then exits.

Arguments:

    Rundown - Supplies a pointer to a TRACE_STORES_RUNDOWN structure.

Return Value:

    TRUE if active, FALSE otherwise.

--*/
{
    return Rundown->Flags.IsActive;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
