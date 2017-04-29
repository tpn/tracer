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

INITIALIZE_RTL_ATEXIT_RUNDOWN InitializeRtlAtExitRundown;

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

DESTROY_RTL_ATEXIT_RUNDOWN DestroyRtlAtExitRundown;

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

CREATE_RTL_ATEXIT_ENTRY CreateRtlAtExitEntry;

_Use_decl_annotations_
BOOL
CreateRtlAtExitEntry(
    PRTL_ATEXIT_RUNDOWN Rundown,
    PATEXITFUNC AtExitFunc,
    PPRTL_ATEXIT_ENTRY EntryPointer
    )
/*++

Routine Description:

    This routine creates an RTL_ATEXIT_ENTRY structure for a given ATEXITFUNC
    function pointer.  Memory is allocated from the default process heap.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure.

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
    PRTL_ATEXIT_ENTRY Entry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(AtExitFunc)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(EntryPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer.
    //

    *EntryPointer = NULL;

    //
    // If we haven't created a heap yet, do it now.
    //

    if (!Rundown->HeapHandle) {
        Rundown->HeapHandle = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
        if (!Rundown->HeapHandle) {
            return FALSE;
        }
    }

    //
    // Attempt to allocate memory for the entry.
    //

    Entry = (PRTL_ATEXIT_ENTRY)(
        HeapAlloc(
            Rundown->HeapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(*Entry)
        )
    );

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


CREATE_RTL_ATEXITEX_ENTRY CreateRtlAtExitExEntry;

_Use_decl_annotations_
BOOL
CreateRtlAtExitExEntry(
    PRTL_ATEXIT_RUNDOWN Rundown,
    PATEXITEX_CALLBACK Callback,
    PATEXITEX_FLAGS AtExitExFlags,
    PVOID Context,
    PPRTL_ATEXIT_ENTRY EntryPointer
    )
/*++

Routine Description:

    This routine creates an RTL_ATEXITEX_ENTRY structure for a given callback,
    context and set of flags.  Memory is allocated from the default process
    heap.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure.

    Callback - Supplies a pointer to an ATEXITEX_CALLBACK function pointer.

    AtExitExFlags - Optionally supplies a pointer to an ATEXITEX_FLAGS structure
        that can be used to customize properties of the entry.

    Context - Optionally supplies an opaque pointer that will be passed to the
        exit function when invoked.

    EntryPointer - Supplies the address of a variable that will receive the
        address of the newly allocated RTL_ATEXITEX_ENTRY structure on success,
        or a NULL value on failure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    PRTL_ATEXIT_ENTRY Entry;
    PATEXITFUNC AtExitFunc;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Callback)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(EntryPointer)) {
        return FALSE;
    }

    //
    // Create an entry using the normal creation routine.  We can cast the
    // callback to PATEXITFUNC as they are collocated at the same address in
    // the structure via the anonymous union.
    //

    AtExitFunc = (PATEXITFUNC)Callback;
    if (!CreateRtlAtExitEntry(Rundown, AtExitFunc, &Entry)) {
        return FALSE;
    }

    //
    // Initialize flags and context if applicable.
    //

    Entry->Flags.IsExtended = TRUE;

    if (ARGUMENT_PRESENT(AtExitExFlags)) {
        Entry->Flags.SuppressExceptions = AtExitExFlags->SuppressExceptions;
    }

    if (ARGUMENT_PRESENT(Context)) {
        Entry->Flags.HasContext = TRUE;
        Entry->Context = Context;
    }

    //
    // Update the caller's entry pointer.
    //

    *EntryPointer = Entry;

    //
    // Return success.
    //

    return TRUE;
}


ADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN AddRtlAtExitEntryToRundown;

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


REMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN RemoveRtlAtExitEntryFromRundown;

_Use_decl_annotations_
VOID
RemoveRtlAtExitEntryFromRundown(
    PRTL_ATEXIT_ENTRY AtExitEntry
    )
/*++

Routine Description:

    This routine removes an entry from the rundown list.  The rundown lock must
    have been acquired before this routine is called.

Arguments:

    AtExitEntry - Supplies a pointer to an RTL_ATEXIT_ENTRY structure.

Return Value:

    None.

--*/
{
    RemoveEntryList(&AtExitEntry->ListEntry);
}


REGISTER_ATEXITFUNC RegisterAtExitFunc;

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
    BOOL Success;
    PRTL_ATEXIT_ENTRY Entry = NULL;

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
    // Acquire the rundown lock.
    //

    EnterCriticalSection(&Rundown->CriticalSection);

    //
    // Attempt to create a new entry to add to the rundown list.
    //

    Success = CreateRtlAtExitEntry(Rundown, AtExitFunc, &Entry);
    if (!Success) {
        goto End;
    }

    //
    // Point the entry at its parent.
    //

    Entry->Rundown = Rundown;

    //
    // Add the entry to the rundown list.
    //

    AddRtlAtExitEntryToRundown(Rundown, Entry);

    //
    // Indicate success.
    //

    Success = TRUE;

End:
    LeaveCriticalSection(&Rundown->CriticalSection);

    return Success;
}


REGISTER_ATEXITEX_CALLBACK RegisterAtExitExCallback;

_Use_decl_annotations_
BOOL
RegisterAtExitExCallback(
    PRTL_ATEXIT_RUNDOWN Rundown,
    PATEXITEX_CALLBACK Callback,
    PATEXITEX_FLAGS Flags,
    PVOID Context,
    PPRTL_ATEXIT_ENTRY EntryPointer
    )
/*++

Routine Description:

    This routine creates a new RTL_ATEXIT_ENTRY structure for the given function
    pointer callback, flags and optional context, acquires the rundown critical
    section, adds the entry to the list and then releases the critical section.

Arguments:

    Rundown - Supplies a pointer to an RTL_ATEXIT_RUNDOWN structure.

    Callback - Supplies a pointer to an ATEXITEX_CALLBACK function pointer to
        to be called at rundown.

    Flags - Optionally supplies a pointer to an ATEXITEX_FLAGS structure that
        can be used to customize properties of the entry.

    Context - Optionally supplies an opaque pointer that will be passed to the
        exit function when invoked.

    EntryPointer - Optionally supplies a pointer to an address that will receive
        the address of the RTL_ATEXIT_ENTRY structure created for this request.
        This allows the caller to subsequently unregister the function via the
        UnregisterRtlAtExitEntry() routine.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    PRTL_ATEXIT_ENTRY Entry = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rundown)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Callback)) {
        return FALSE;
    }

    //
    // Clear the caller's entry pointer if present.
    //

    if (ARGUMENT_PRESENT(EntryPointer)) {
        *EntryPointer = NULL;
    }

    //
    // Acquire the rundown lock.
    //

    EnterCriticalSection(&Rundown->CriticalSection);

    //
    // Attempt to create a new entry to add to the rundown list.
    //

    Success = CreateRtlAtExitExEntry(Rundown, Callback, Flags, Context, &Entry);
    if (!Success) {
        goto End;
    }

    //
    // Point the entry at its parent.
    //

    Entry->Rundown = Rundown;

    //
    // Add the entry to the rundown list.
    //

    AddRtlAtExitEntryToRundown(Rundown, Entry);

    //
    // Update the caller's entry pointer if applicable.
    //

    if (ARGUMENT_PRESENT(EntryPointer)) {
        *EntryPointer = Entry;
    }

    //
    // Indicate success.
    //

    Success = TRUE;

End:
    LeaveCriticalSection(&Rundown->CriticalSection);

    return Success;
}


UNREGISTER_RTL_ATEXIT_ENTRY UnregisterRtlAtExitEntry;

_Use_decl_annotations_
BOOL
UnregisterRtlAtExitEntry(
    PRTL_ATEXIT_ENTRY Entry
    )
/*++

Routine Description:

    This routine acquires the lock for the rundown structure, removes the entry
    from the rundown list (without calling the atexit callback function pointer)
    and frees the backing memory (thus invaliding the pointer) and then releases
    the lock.

    The rundown structure is derived from the Entry->Rundown field.

Arguments:

    Entry - Supplies a pointer to an RTL_ATEXIT_ENTRY structure to remove from
        its rundown list.  This backing memory will be freed by this routine,
        so the pointer should not be accessed after this routine returns.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL_ATEXIT_RUNDOWN Rundown;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Entry)) {
        return FALSE;
    }

    //
    // Ensure the entry is linked to something.
    //

    if (IsListEmpty(&Entry->ListEntry)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Ensure there's a valid rundown pointer.
    //

    if (Entry->Rundown == NULL) {
        __debugbreak();
        return FALSE;
    }

    Rundown = Entry->Rundown;

    //
    // Acquire the rundown lock, remove the entry, free the backing heap memory,
    // release the lock, return success.
    //

    EnterCriticalSection(&Rundown->CriticalSection);
    __try {
        RemoveRtlAtExitEntryFromRundown(Entry);
        HeapFree(Rundown->HeapHandle, 0, Entry);
    } __finally {
        LeaveCriticalSection(&Rundown->CriticalSection);
    }

    return TRUE;
}


RUNDOWN_ATEXIT_FUNCTIONS RundownAtExitFunctions;

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
    BOOL IsExtended;
    BOOL HasContext;
    BOOL SuppressExceptions;
    HANDLE HeapHandle;
    PVOID Context;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEntry;
    PATEXITFUNC AtExitFunc;
    PATEXITEX_CALLBACK AtExitExCallback;
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
        HeapHandle = Rundown->HeapHandle;
    } else {
        HeapHandle = NULL;
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

            //
            // Invariant check: entry's rundown should match the rundown we're
            // enumerating.
            //

            if (AtExitEntry->Rundown != Rundown) {
                __debugbreak();
            }

            //
            // Initialize local variables/aliases for this entry.
            //

            AtExitFunc = AtExitEntry->AtExitFunc;
            AtExitExCallback = AtExitEntry->AtExitExCallback;
            IsExtended = AtExitEntry->Flags.IsExtended;
            HasContext = AtExitEntry->Flags.HasContext;
            SuppressExceptions = AtExitEntry->Flags.SuppressExceptions;

            //
            // Initialize context only if the HasContext bit was set.
            //

            Context = (HasContext ? AtExitEntry->Context : NULL);

            if (!IsExtended) {

                //
                // This isn't an extended entry, so call the function normally.
                //

                AtExitFunc();

            } else {

                ULONG Flags;
                LPCWSTR Method;
                HMODULE Handle;

                //
                // This is an extended entry.  Check to see if the module is
                // still loaded.
                //

                Method = (LPCWSTR)AtExitExCallback;

                Flags = (
                    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
                );

                if (!GetModuleHandleEx(Flags, Method, &Handle)) {

                    //
                    // The module isn't loaded anymore, ignore this entry.
                    //

                    goto AfterCallback;
                }

                //
                // If the entry's suppress exceptions bit is set, wrap the
                // callback in a __try/__except "catch-all" handler and ignore
                // any exceptions.
                //

                if (SuppressExceptions) {

                    __try {
                        AtExitExCallback(IsProcessTerminating, Context);
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        NOTHING;
                    }

                } else {

                    //
                    // Call the function normally.
                    //

                    AtExitExCallback(IsProcessTerminating, Context);
                }
            }

AfterCallback:

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


IS_RTL_ATEXIT_RUNDOWN_ACTIVE IsRtlAtExitRundownActive;

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
