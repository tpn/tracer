/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreLoader.c

Abstract:

    This module implements functionality related to capturing library loading
    information whilst a process is being traced.  This is done by hooking into
    loader's DLL notification callbacks via the facilities provided by Rtl.

--*/

#include "stdafx.h"

RTL_GENERIC_COMPARE_RESULTS
NTAPI
ModuleTableCompareRoutine(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    ULONG_PTR First = (ULONG_PTR)FirstStruct;
    ULONG_PTR Second = (ULONG_PTR)SecondStruct;

    if (First == Second) {
        return GenericEqual;
    } else if (First < Second) {
        return GenericLessThan;
    }
    return GenericGreaterThan;
}

PVOID
NTAPI
ModuleTableAllocateRoutine(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ CLONG ByteSize
    )
{
    PTRACE_STORE TraceStore;
    PTRACE_CONTEXT TraceContext;
    PALLOCATE_RECORDS Allocate;
    ULONG HeaderSize;
    ULONG_PTR AllocSize;

    TraceStore = (PTRACE_STORE)Table->TableContext;
    TraceContext = TraceStore->TraceContext;
    Allocate = TraceStore->AllocateRecords;
    __debugbreak();
    HeaderSize = ByteSize - 8;
    AllocSize = HeaderSize + sizeof(TRACE_MODULE_TABLE_ENTRY);

    return Allocate(TraceContext, TraceStore, 1, AllocSize);
}

VOID
NTAPI
ModuleTableFreeRoutine(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
    )
{
    __debugbreak();
}

_Use_decl_annotations_
BOOL
ModuleTableStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the ModuleTable trace store.  It calls the normal trace store bind complete
    routine, then initializes the single static TRACE_MODULE_TABLE structure.

Arguments:

    TraceContext - Supplies a pointer to the TRACE_CONTEXT structure to which
        the trace store was bound.

    TraceStore - Supplies a pointer to the bound TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to the first TRACE_STORE_MEMORY_MAP
        used by the trace store.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    PTRACE_MODULE_TABLE ModuleTable;

    //
    // Resolve aliases.
    //

    Rtl = TraceContext->Rtl;

    //
    // Create the initial module table structure.
    //

    ModuleTable = (PTRACE_MODULE_TABLE)(
        TraceStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore->TraceStore,
            1,
            sizeof(*ModuleTable)
        )
    );

    if (!ModuleTable) {
        return FALSE;
    }

    //
    // Set the structure size.
    //

    ModuleTable->SizeOfStruct = sizeof(*ModuleTable);

    //
    // Initialize the AVL table.
    //

    Rtl->RtlInitializeGenericTableAvl(&ModuleTable->BaseAddressTable,
                                      ModuleTableCompareRoutine,
                                      ModuleTableAllocateRoutine,
                                      ModuleTableFreeRoutine,
                                      TraceStore);

    //
    // Initialize the Unicode prefix table.
    //

    Rtl->RtlInitializeUnicodePrefix(&ModuleTable->ModuleNamePrefixTable);

    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
ModuleLoadEventStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the Loader trace store.  It calls the normal trace store bind complete
    routine, then initializes loader tracing via Rtl.  This acquires the loader
    lock, walks the current list of loaded modules and invokes our callback,
    registers our callback and the subsequent DLL load/unload notification
    routine, releases the lock, and then returns.

Arguments:

    TraceContext - Supplies a pointer to the TRACE_CONTEXT structure to which
        the trace store was bound.

    TraceStore - Supplies a pointer to the bound TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to the first TRACE_STORE_MEMORY_MAP
        used by the trace store.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL WaitAll = TRUE;
    ULONG WaitResult;
    ULONG NumberOfWaits;
    PRTL Rtl;
    PVOID Cookie;
    DLL_NOTIFICATION_FLAGS NotificationFlags = { 0 };
    PTRACE_STORES TraceStores;
    HANDLE Events[2];

    Rtl = TraceContext->Rtl;

    //
    // Wait on the module table and module table entry stores before we continue
    // with DLL notification registration.  We need to do this because we access
    // the module table with the SRWLock exclusively acquired; that lock lives
    // in the TRACE_STORE_SYNC metadata store and is only initialized once the
    // trace store metadata has finished its BindComplete routine.
    //

    TraceStores = TraceContext->TraceStores;

    Events[0] = (
        TraceStoreIdToRelocationCompleteEvent(
            TraceStores,
            TraceStoreModuleTableId
        )
    );

    Events[1] = (
        TraceStoreIdToRelocationCompleteEvent(
            TraceStores,
            TraceStoreModuleTableEntryId
        )
    );

    NumberOfWaits = 2;

    WaitResult = WaitForMultipleObjects(NumberOfWaits,
                                        Events,
                                        WaitAll,
                                        INFINITE);

    if (WaitResult != WAIT_OBJECT_0) {
        return FALSE;
    }

    //
    // All waits were satisfied, continue with registration.
    //

    Success = Rtl->RegisterDllNotification(Rtl,
                                           TraceStoreDllNotificationCallback,
                                           &NotificationFlags,
                                           TraceContext,
                                           &Cookie);

    if (!Success) {
        return FALSE;
    }

    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
VOID
CALLBACK
TraceStoreDllNotificationCallback(
    DLL_NOTIFICATION_REASON Reason,
    PDLL_NOTIFICATION_DATA Data,
    PVOID Context
    )
/*++

Routine Description:

    This routine is the callback function invoked automatically by the loader
    when a DLL has been loaded or unloaded.  The loader lock will be held for
    the duration of the call, and thus, this routine does not need to deal with
    any re-entrancy issues.

Arguments:

    Reason - Supplies the reason for the callback (DLL load or DLL unload).

    Data - Supplies a pointer to notification data for this loader operation.

    Context - Supplies a pointer to the TRACE_CONTEXT structure that registered
        the loader DLL notifications.

Return Value:

    None.

--*/
{
    PRTL Rtl;
    ULONG SizeOfImage;
    LARGE_INTEGER Timestamp;
    PTRACE_CONTEXT TraceContext;
    PLDR_DATA_TABLE_ENTRY Module;
    PLDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    PCLDR_DLL_NOTIFICATION_DATA NotificationData;
    PTRACE_STORES TraceStores;
    PTRACE_STORE ImageFileStore;
    PTRACE_STORE ModuleTableStore;
    PTRACE_STORE ModuleTableEntryStore;
    PTRACE_STORE ModuleLoadEventStore;
    PRTL_INSERT_ELEMENT_GENERIC_TABLE_AVL InsertEntryAvl;
    PTRACE_MODULE_TABLE ModuleTable;
    PTRACE_MODULE_TABLE_ENTRY ModuleEntry;
    PTRACE_MODULE_LOAD_EVENT LoadEvent;
    PVOID EntryPoint;
    PVOID BaseAddress;
    BOOLEAN NewRecord;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Context)) {
        return;
    }

    //
    // Initialize aliases/variables.
    //

    TraceContext = (PTRACE_CONTEXT)Context;
    TraceStores = TraceContext->TraceStores;
    Rtl = TraceContext->Rtl;
    InsertEntryAvl = Rtl->RtlInsertElementGenericTableAvl;

    Module = Data->LoaderDataTableEntry;
    NotificationData = Data->NotificationData;
    Loaded = (PLDR_DLL_LOADED_NOTIFICATION_DATA)&NotificationData->Loaded;

    //
    // Capture a timestamp.
    //

    QueryPerformanceCounter(&Timestamp);

    //
    // Resolve the trace stores.
    //

    ImageFileStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreImageFileId
        )
    );

    ModuleTableStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreModuleTableId
        )
    );

    ModuleTableEntryStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreModuleTableEntryId
        )
    );

    ModuleLoadEventStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreModuleLoadEventId
        )
    );

    //
    // Resolve the module table.
    //

    ModuleTable = (PTRACE_MODULE_TABLE)(
        ModuleTableStore->MemoryMap->BaseAddress
    );

    if (Module) {
        EntryPoint = Module->EntryPoint;
        BaseAddress = Module->DllBase;
        SizeOfImage = Module->SizeOfImage;
    } else {
        BaseAddress = Loaded->DllBase;
        SizeOfImage = Loaded->SizeOfImage;
        EntryPoint = NULL;
    }

    TraceStoreAcquireLockExclusive(ModuleTableStore);
    ModuleEntry = InsertEntryAvl(&ModuleTable->BaseAddressTable,
                                 &BaseAddress,
                                 sizeof(BaseAddress),
                                 &NewRecord);
    TraceStoreReleaseLockExclusive(ModuleTableStore);

    if (!NewRecord) {
        goto CreateLoadEvent;
    }

    if (Reason.Unloaded) {

        //
        // We should ever hit this.
        //

        __debugbreak();
    }

    //
    // This is a new entry, so fill out the details.
    //

    //
    // Initialize the list head.
    //

    InitializeListHead(&ModuleEntry->ListHead);

    //
    // - Fill in the RTL_FILE details.
    // - Fill in the RTL_IMAGE_FILE-specific details.
    // - Copy the file contents to the image file store.
    // - Capture a backtrace and fill that in.
    //

CreateLoadEvent:

    LoadEvent = (PTRACE_MODULE_LOAD_EVENT)(
        ModuleLoadEventStore->AllocateRecordsWithTimestamp(
            TraceContext,
            ModuleLoadEventStore,
            1,
            sizeof(*LoadEvent),
            &Timestamp
        )
    );

    if (!LoadEvent) {
        return;
    }

    //
    // LoadEvent->Flags.AsLong =  ... ?
    //

    LoadEvent->Flags = Loaded->Flags;
    LoadEvent->Timestamp.Loaded = Timestamp;
    LoadEvent->BaseAddress = BaseAddress;
    LoadEvent->ModuleTableEntry = ModuleEntry;
    LoadEvent->SizeOfImage = SizeOfImage;
    LoadEvent->EntryPoint = EntryPoint;

    //
    // Point this at the base of the image file once copied.
    //

    LoadEvent->Content = NULL;

    //
    // Fill this in if possible.
    //

    LoadEvent->PreferredBaseAddress = NULL;

    //
    // Initialize the list entry and add it to the tail of the module's list.
    //

    InitializeListHead(&LoadEvent->ListEntry);
    AppendTailList(&ModuleEntry->ListHead, &LoadEvent->ListEntry);


    //
    // Add the entry to the Unicode prefix table.
    //

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
