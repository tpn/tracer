/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Loader.c

Abstract:

    This module implements various loader related routines.

--*/

#include "stdafx.h"

#define RTL_LDR_NOTIFICATION_TABLE_CRITICAL_SECTION_SPIN_COUNT 4000
#define FRAMES_TO_SKIP 1
#define FRAMES_TO_CAPTURE 32

PRTL_LDR_NOTIFICATION_TABLE GlobalNotificationTable = NULL;

_Use_decl_annotations_
BOOL
InitializeRtlLdrNotificationTable(
    PRTL Rtl,
    PRTL_LDR_NOTIFICATION_TABLE NotificationTable
    )
/*++

Routine Description:

    This routine initializes an RTL_LDR_NOTIFICATION_TABLE structure.  It is
    called automatically as part of the InitializeRtl() routine.

Arguments:

    Rtl - Supplies a pointer to an RTL structure that will own the notification
        table.

    NotificationTable - Supplies a pointer to an RTL_LDR_NOTIFICATION_TABLE to
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

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(NotificationTable)) {
        return FALSE;
    }

    if (GlobalNotificationTable != NULL) {
        return FALSE;
    }

    //
    // Set the structure size.
    //

    NotificationTable->SizeOfStruct = sizeof(*NotificationTable);

    //
    // Initialize the critical section.
    //

    Success = InitializeCriticalSectionAndSpinCount(
        &NotificationTable->CriticalSection,
        RTL_LDR_NOTIFICATION_TABLE_CRITICAL_SECTION_SPIN_COUNT
    );

    if (!Success) {
        return FALSE;
    }

    EnterCriticalSection(&NotificationTable->CriticalSection);
    InitializeListHead(&NotificationTable->ListHead);
    NotificationTable->HeapHandle = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
    NotificationTable->NotificationFunction = LdrDllNotificationFunction;
    LeaveCriticalSection(&NotificationTable->CriticalSection);

    if (!NotificationTable->HeapHandle) {
        return FALSE;
    }

    NotificationTable->Rtl = Rtl;
    GlobalNotificationTable = NotificationTable;

    return TRUE;
}

_Use_decl_annotations_
VOID
DestroyRtlLdrNotificationTable(
    PRTL_LDR_NOTIFICATION_TABLE NotificationTable
    )
/*++

Routine Description:

    This routine destroys an initialized RTL_LDR_NOTIFICATION_TABLE structure.

Arguments:

    NotificationTable - Supplies a pointer to an RTL_LDR_NOTIFICATION_TABLE
        to structure to destroy.

Return Value:

    None.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(NotificationTable)) {
        return;
    }

    DeleteCriticalSection(&NotificationTable->CriticalSection);

    if (NotificationTable->HeapHandle) {
        HeapDestroy(NotificationTable->HeapHandle);
    }

    return;
}

_Use_decl_annotations_
BOOL
CreateRtlLdrNotificationEntry(
    PRTL_LDR_NOTIFICATION_TABLE NotificationTable,
    PDLL_NOTIFICATION_CALLBACK NotificationCallback,
    PDLL_NOTIFICATION_FLAGS Flags,
    PVOID Context,
    PPRTL_LDR_NOTIFICATION_ENTRY NotificationEntryPointer
    )
/*++

Routine Description:

    This routine creates an RTL_LDR_NOTIFICATION_ENTRY structure.

Arguments:

    NotificationTable - Supplies a pointer to an RTL_LDR_NOTIFICATION_TABLE
        structure.

    NotificationCallback - Supplies a pointer to a function that will be called
        on DLL load and unload.

    Flags - Supplies a pointer to RTL_LDR_NOTIFICATION_FLAGS that can be used
        to indicate specific behavior the caller wants.

    Context - Supplies a value that will be passed back to the caller upon
        subsequent invocations of their notification callback function.

    NotificationEntryPointer - Supplies the address of a variable that will
        receive the address of an opaque registration structure.  The caller
        can unregister themselves for notifications by calling the unregister
        method with this value.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    HANDLE HeapHandle;
    PRTL_LDR_NOTIFICATION_ENTRY Entry;

    //
    // Clear the caller's pointer up-front if present.
    //

    if (ARGUMENT_PRESENT(NotificationEntryPointer)) {
        *NotificationEntryPointer = NULL;
    }

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(NotificationTable)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(NotificationCallback)) {
        return FALSE;
    }

    HeapHandle = NotificationTable->HeapHandle;
    Entry = (PRTL_LDR_NOTIFICATION_ENTRY)(
        HeapAlloc(
            HeapHandle,
            HEAP_ZERO_MEMORY,
            sizeof(*Entry)
        )
    );

    if (!Entry) {
        return FALSE;
    }

    //
    // Initialize the structure.
    //

    Entry->SizeOfStruct = sizeof(*Entry);
    Entry->NotificationCallback = NotificationCallback;
    Entry->NotificationTable = NotificationTable;
    Entry->Context = Context;
    InitializeListHead(&Entry->ListEntry);

    //
    // Update the caller's pointer if applicable and return success.
    //

    *NotificationEntryPointer = Entry;

    return TRUE;
}

_Use_decl_annotations_
BOOL
RegisterDllNotification(
    PRTL Rtl,
    PDLL_NOTIFICATION_CALLBACK NotificationCallback,
    PDLL_NOTIFICATION_FLAGS Flags,
    PVOID Context,
    PPVOID Cookie
    )
/*++

Routine Description:

    This routine registers a DLL notification callback.  It creates a new
    RTL_LDR_NOTIFICATION_ENTRY structure and adds it to the notification
    table registered with the Rtl structure, acquires the loader lock, walks
    the current list of modules and invokes the callback for each one, then
    registers the thunk callback for subsequent DLL notifications (which call
    the caller's callback) and then releases the loader lock and returns.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    NotificationCallback - Supplies a pointer to the function callback to be
        invoked on DLL load/unload events.

    NotificationFlags - Optionally supplies a pointer to flags used to indicate
        specific behavior for notification callbacks.  Currently unused.

    Context - Optionally supplies a pointer that will be passed back to the
        caller when the callback is invoked.

    Cookie - Optionally supplies the address of a variable that will receive
        the address of an opaque structure that can be passed to a subsequent
        call to RtlUnregisterDllNotification() if the caller wishes to cease
        notification callbacks.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    BOOL LoaderLockAcquired = FALSE;
    PPEB Peb;
    ULONG Disposition;
    ULONG NumberOfModules = 0;
    HRESULT Result;
    PVOID LoaderCookie;
    PRTL_LDR_NOTIFICATION_TABLE Table;
    PRTL_LDR_NOTIFICATION_ENTRY Entry;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PLDR_DATA_TABLE_ENTRY Module;
    DLL_NOTIFICATION_REASON Reason;
    DLL_NOTIFICATION_DATA Data;

    UNREFERENCED_PARAMETER(Flags);

    //
    // Acquire the notification table's critical section.
    //

    Table = Rtl->LoaderNotificationTable;
    EnterCriticalSection(&Table->CriticalSection);

    //
    // Create the entry.
    //

    Success = CreateRtlLdrNotificationEntry(Table,
                                            NotificationCallback,
                                            Flags,
                                            Context,
                                            &Entry);
    if (!Success) {
        goto End;
    }

    //
    // Acquire the loader lock for the remaining duration of the call.
    //

    Result = Rtl->LdrLockLoaderLock(0, &Disposition, &LoaderCookie);
    LoaderLockAcquired = (
        SUCCEEDED(Result) &&
        Disposition == LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED
    );

    if (!LoaderLockAcquired) {
        Success = FALSE;
        goto End;
    }

    //
    // Resolve the PEB for the current process.
    //

    Peb = NtCurrentPeb();

    //
    // Clear the backtrace related members as we don't capture stack backtraces
    // during the initial registration.
    //

    Data.NumberOfStackBackTraceFrames = 0;
    Data.StackBackTraceHash = 0;
    Data.StackBackTrace = NULL;

    //
    // Initialize the reason to LOADED_INITIAL.
    //

    Reason.AsLong = LDR_DLL_NOTIFICATION_REASON_LOADED_INITIAL;

    //
    // Walk the list of modules and invoke the callback on each one.
    //

    ListHead = &Peb->Ldr->InLoadOrderModuleList;

    FOR_EACH_LIST_ENTRY(ListHead, ListEntry) {

        NumberOfModules++;

        Module = CONTAINING_RECORD(ListEntry,
                                   LDR_DATA_TABLE_ENTRY,
                                   InLoadOrderModuleList);


        //
        // Fill out our the DLL_NOTIFICATION_DATA structure.
        //

        Data.LoaderDataTableEntry = Module;
        Data.NotificationData = NULL;

        //
        // Call the caller's callback.
        //

        NotificationCallback(Reason, &Data, Context);
    }

    //
    // Insert the entry into the notification table and increment the count of
    // notification callbacks registered.
    //

    InsertTailList(&Table->ListHead, &Entry->ListEntry);
    Table->NumberOfEntries++;

    //
    // If this is the first registration, also register our thunk callback
    // for subsequent DLL load/unload operations.
    //

    if (!Table->Flags.IsRegistered) {
        ULONG LdrFlags = 0;

        Result = Rtl->LdrRegisterDllNotification(LdrFlags,
                                                 Table->NotificationFunction,
                                                 Table,
                                                 &Table->Cookie);
        if (FAILED(Result)) {
            __debugbreak();
            Success = FALSE;
            goto End;
        }

        Table->Flags.IsRegistered = TRUE;
    }

    Success = TRUE;

    //
    // Update the caller's cookie pointer if applicable.
    //

    if (ARGUMENT_PRESENT(Cookie)) {
        *Cookie = (PVOID)Entry;
    }

    //
    // Intentional follow-on to End.
    //

End:

    if (LoaderLockAcquired) {
        Result = Rtl->LdrUnlockLoaderLock(0, LoaderCookie);
        if (FAILED(Result)) {
            __debugbreak();
            Success = FALSE;
        }
    }

    LeaveCriticalSection(&Table->CriticalSection);

    return Success;
}

_Use_decl_annotations_
VOID
CALLBACK
LdrDllNotificationFunction(
    ULONG NotificationReason,
    PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    PVOID Context
    )
/*++

Routine Description:

    This routine is the callback target that is invoked by the loader on DLL
    load and unload events.  This routine is registered during the first call
    to RegisterDllNotification().

Arguments:

    NotificationReason - Supplies the reason for why the callback was invoked;
        DLL load or unload.

    NotificationData - Supplies a pointer to notification data about the load
        or unload event.

    Context - Supplies a pointer to our RTL_LDR_NOTIFICATION_TABLE structure
        we passed to LdrRegisterDllNotification().

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    USHORT FramesCaptured;
    ULONG FramesToSkip;
    ULONG FramesToCapture;
    ULONG Attempts = 0;
    PRTL Rtl;
    PPEB Peb;
    PVOID BackTrace;
    ULONG BackTraceHash;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    DLL_NOTIFICATION_REASON Reason;
    DLL_NOTIFICATION_DATA Data;
    PLDR_DATA_TABLE_ENTRY Module;
    PRTL_LDR_NOTIFICATION_TABLE IncomingTable;
    PRTL_LDR_NOTIFICATION_TABLE Table;
    PRTL_LDR_NOTIFICATION_ENTRY Entry;

    //
    // Validate arguments.
    //

    if (NotificationReason != LDR_DLL_NOTIFICATION_REASON_LOADED &&
        NotificationReason != LDR_DLL_NOTIFICATION_REASON_UNLOADED) {
        return;
    }

    if (!ARGUMENT_PRESENT(NotificationData)) {
        return;
    }

    if (!ARGUMENT_PRESENT(Context)) {
        return;
    }

    IncomingTable = (PRTL_LDR_NOTIFICATION_TABLE)Context;
    Table = GlobalNotificationTable;

    if (IncomingTable != Table) {
        __debugbreak();
    }

    Rtl = Table->Rtl;
    SecureZeroMemory(&Data, sizeof(Data));

    //
    // Acquire the notification table's critical section.
    //

    EnterCriticalSection(&Table->CriticalSection);

    //
    // Resolve the PEB for the current process.
    //

    Peb = NtCurrentPeb();

    //
    // Initialize the list head, then walk the modules in reverse looking for
    // the one that was just added.
    //

    ListHead = &Peb->Ldr->InLoadOrderModuleList;

    FOR_EACH_LIST_ENTRY_REVERSE(ListHead, ListEntry) {

        Module = CONTAINING_RECORD(ListEntry,
                                   LDR_DATA_TABLE_ENTRY,
                                   InLoadOrderModuleList);

        //
        // If the address of the module's full name UNICODE_STRING matches the
        // notification data, we've found our module.
        //

        if (NotificationData->Loaded.FullDllName == &Module->FullDllName) {
            Data.LoaderDataTableEntry = Module;
            break;
        }

        Attempts++;
    }

    //
    // Disable capturing stack back traces for now.
    //

    goto ProcessCallbacks;

    //
    // Capture a stack back trace.
    //

    FramesToSkip = FRAMES_TO_SKIP;
    FramesToCapture = FRAMES_TO_CAPTURE;

    FramesCaptured = Rtl->RtlCaptureStackBackTrace(FramesToSkip,
                                                   FramesToCapture,
                                                   &BackTrace,
                                                   &BackTraceHash);

    //
    // Finish filling out the notification data structure.
    //

    Data.NumberOfStackBackTraceFrames = FramesCaptured;
    Data.StackBackTraceHash = BackTraceHash;
    Data.StackBackTrace;

ProcessCallbacks:
    Data.NotificationData = NotificationData;

    //
    // Initialize the reason.
    //

    Reason.AsLong = NotificationReason;

    //
    // Walk the list of registered notification callbacks and invoke them.
    //

    ListHead = &Table->ListHead;

    FOR_EACH_LIST_ENTRY(ListHead, ListEntry) {

        Entry = CONTAINING_RECORD(ListEntry,
                                  RTL_LDR_NOTIFICATION_ENTRY,
                                  ListEntry);


        Entry->NotificationCallback(Reason, &Data, Entry->Context);
    }

    LeaveCriticalSection(&Table->CriticalSection);

    return;
}

_Use_decl_annotations_
BOOL
UnregisterDllNotification(
    PVOID IncomingCookie
    )
/*++

Routine Description:

    This routine unregisters a previously registered DLL notification callback.

Arguments:

    IncomingCookie - Supplies the cookie value that was provided to the caller
        when the notification function was registered.  This can be cast to
        the RTL_LDR_NOTIFICATION_ENTRY.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    PRTL_LDR_NOTIFICATION_ENTRY Entry;
    PRTL_LDR_NOTIFICATION_TABLE Table;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(IncomingCookie)) {
        return FALSE;
    }

    Entry = (PRTL_LDR_NOTIFICATION_ENTRY)IncomingCookie;

    //
    // Make sure the entry is on a list.
    //

    if (IsListEmpty(&Entry->ListEntry)) {
        return FALSE;
    }

    Table = Entry->NotificationTable;

    //
    // Acquire the table's critical section.
    //

    EnterCriticalSection(&Table->CriticalSection);

    //
    // Remove the entry from the list and decrement the number of registrations.
    //

    RemoveEntryList(&Entry->ListEntry);

    //
    // Free the space allocated for it.
    //

    HeapFree(Table->HeapHandle, 0, Entry);

    //
    // If this was the last entry, unregister our loader notification callback.
    //

    if (!--Table->NumberOfEntries) {

        if (!IsListEmpty(&Table->ListHead)) {
            __debugbreak();
        }

        Table->Rtl->LdrUnregisterDllNotification(Table->Cookie);
        Table->Cookie = NULL;
        Table->Flags.IsRegistered = FALSE;

    } else {
        if (IsListEmpty(&Table->ListHead)) {
            __debugbreak();
        }
    }

    //
    // Leave the critical section and return success.
    //

    LeaveCriticalSection(&Table->CriticalSection);

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
