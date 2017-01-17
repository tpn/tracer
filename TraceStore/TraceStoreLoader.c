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

#define FRAMES_TO_SKIP 2

RTL_GENERIC_COMPARE_RESULTS
NTAPI
ModuleTableEntryCompareRoutine(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    ULONG_PTR First = *((PULONG_PTR)FirstStruct);
    ULONG_PTR Second = *((PULONG_PTR)SecondStruct);

    if (First == Second) {
        return GenericEqual;
    } else if (First < Second) {
        return GenericLessThan;
    }
    return GenericGreaterThan;
}

PVOID
NTAPI
ModuleTableEntryAllocateRoutine(
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
    HeaderSize = ByteSize - 8;
    AllocSize = HeaderSize + sizeof(TRACE_MODULE_TABLE_ENTRY);

    return Allocate(TraceContext, TraceStore, 1, AllocSize);
}

VOID
NTAPI
ModuleTableEntryFreeRoutine(
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
    PTRACE_STORES TraceStores;
    PTRACE_STORE ModuleTableEntryStore;

    //
    // Resume allocations but do not set the bind complete event yet.
    //

    ResumeTraceStoreAllocations(TraceStore);

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
    // Resolve the module table entry store.
    //

    TraceStores = TraceContext->TraceStores;
    ModuleTableEntryStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreModuleTableEntryId
        )
    );

    //
    // Initialize the AVL table.
    //

    Rtl->RtlInitializeGenericTableAvl(&ModuleTable->BaseAddressTable,
                                      ModuleTableEntryCompareRoutine,
                                      ModuleTableEntryAllocateRoutine,
                                      ModuleTableEntryFreeRoutine,
                                      ModuleTableEntryStore);

    //
    // Initialize the Unicode prefix table.
    //

    Rtl->RtlInitializeUnicodePrefix(&ModuleTable->ModuleNamePrefixTable);

    //
    // Set the bind complete event and return success.
    //

    SetEvent(TraceStore->BindCompleteEvent);

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
    the ModuleLoadEvent trace store.  It waits for both the ModuleTable and
    ModuleTableEntry stores to be ready (i.e. have their relocation complete
    events set), then registers a callback for DLL load/unload notification
    via Rtl.

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


    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        return FALSE;
    }

    //
    // Wait on the module table and module table entry stores before we continue
    // with DLL notification registration.  We need to do this because we access
    // the module table with the SRWLock exclusively acquired; that lock lives
    // in the TRACE_STORE_SYNC metadata store and is only initialized once the
    // trace store metadata has finished its BindComplete routine.
    //

    TraceStores = TraceContext->TraceStores;

    Events[0] = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreModuleTableId
        )->BindCompleteEvent
    );

    Events[1] = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreModuleTableEntryId
        )->BindCompleteEvent
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

    Rtl = TraceContext->Rtl;
    Success = Rtl->RegisterDllNotification(Rtl,
                                           TraceStoreDllNotificationCallback,
                                           &NotificationFlags,
                                           TraceContext,
                                           &Cookie);

    if (!Success) {
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
    the duration of the call.  It is responsible for calling the Impl worker
    routine from within a try/except block that suppresses STATUS_IN_PAGE_ERROR.

Arguments:

    Reason - Supplies the reason for the callback (DLL load or DLL unload).

    Data - Supplies a pointer to notification data for this loader operation.

    Context - Supplies a pointer to the TRACE_CONTEXT structure that registered
        the loader DLL notifications.

Return Value:

    None.

--*/
{
    TRY_MAPPED_MEMORY_OP {

        TraceStoreDllNotificationCallbackImpl1(Reason, Data, Context);

    } CATCH_STATUS_IN_PAGE_ERROR {

        NOTHING;
    }
}

_Use_decl_annotations_
VOID
CALLBACK
TraceStoreDllNotificationCallbackImpl1(
    DLL_NOTIFICATION_REASON Reason,
    PDLL_NOTIFICATION_DATA Data,
    PVOID Context
    )
/*++

Routine Description:

    This routine is the callback function invoked automatically by the loader
    when a DLL has been loaded or unloaded.  The loader lock will be held for
    the duration of the call.

Arguments:

    Reason - Supplies the reason for the callback (DLL load or DLL unload).

    Data - Supplies a pointer to notification data for this loader operation.

    Context - Supplies a pointer to the TRACE_CONTEXT structure that registered
        the loader DLL notifications.

Return Value:

    None.

--*/
{
    BOOL Success;
    PRTL Rtl;
    ULONG SizeOfImage;
    ULONG NumberOfFramesToCapture;
    LARGE_INTEGER Timestamp;
    PUNICODE_STRING FullDllPath;
    PTRACE_CONTEXT TraceContext;
    PLDR_DATA_TABLE_ENTRY Module;
    PLDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    PCLDR_DLL_NOTIFICATION_DATA NotificationData;
    PTRACE_STORES TraceStores;
    PTRACE_STORE ImageFileStore;
    PTRACE_STORE ModuleTableStore;
    PTRACE_STORE ModuleTableEntryStore;
    PTRACE_STORE ModuleLoadEventStore;
    PUNICODE_PREFIX_TABLE_ENTRY UnicodePrefixTableEntry;
    PRTL_INSERT_ELEMENT_GENERIC_TABLE_AVL InsertEntryAvl;
    PTRACE_MODULE_TABLE ModuleTable;
    PTRACE_MODULE_TABLE_ENTRY ModuleEntry;
    PTRACE_MODULE_TABLE_ENTRY ExistingModuleEntry;
    PTRACE_MODULE_LOAD_EVENT LoadEvent;
    PRTL_FILE FilePointer;
    PRTL_IMAGE_FILE ImageFile;
    RTL_FILE_INIT_FLAGS InitFlags;
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
        FullDllPath = &Module->FullDllName;
        EntryPoint = Module->EntryPoint;
        BaseAddress = Module->DllBase;
        SizeOfImage = Module->SizeOfImage;
    } else {
        __debugbreak();
        FullDllPath = (PUNICODE_STRING)Loaded->FullDllName;
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

    //
    // This is a new entry, so fill out the details.
    //

    //
    // Initialize the load events and duplicate entries list heads.
    //

    InitializeListHead(&ModuleEntry->LoadEventsListHead);
    InitializeListHead(&ModuleEntry->DuplicateEntriesListHead);

    //
    // Prime our RTL_FILE structure for the given DLL.
    //

    InitFlags.AsLong = 0;
    InitFlags.IsImageFile = TRUE;
    InitFlags.CopyContents = FALSE;
    InitFlags.CopyViaMovsq = FALSE;
    FilePointer = &ModuleEntry->File;

    Success = (
        Rtl->InitializeRtlFile(
            Rtl,
            FullDllPath,
            NULL,
            &TraceContext->BitmapAllocator,
            &TraceContext->UnicodeStringBufferAllocator,
            &TraceContext->ImageFileAllocator,
            NULL,
            NULL,
            InitFlags,
            &FilePointer,
            &Timestamp
        )
    );

    if (!Success) {
        __debugbreak();
        return;
    }

    //
    // Initialize relevant RTL_IMAGE_FILE-specific details.
    //

    ImageFile = &ModuleEntry->File.ImageFile;

    ImageFile->DllBase = BaseAddress;
    ImageFile->EntryPoint = EntryPoint;
    ImageFile->SizeOfImage = SizeOfImage;

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

    LoadEvent->Flags = Reason.AsLong;
    if (Reason.Loaded) {
        LoadEvent->Timestamp.Loaded.QuadPart = Timestamp.QuadPart;
    } else {
        LoadEvent->Timestamp.Unloaded.QuadPart = Timestamp.QuadPart;
    }

    LoadEvent->BaseAddress = BaseAddress;
    LoadEvent->ModuleTableEntry = ModuleEntry;
    LoadEvent->SizeOfImage = SizeOfImage;
    LoadEvent->EntryPoint = EntryPoint;

    //
    // Point this at the base of the image file once copied.
    //

    LoadEvent->Content = ModuleEntry->File.Content;

    //
    // Fill this in if possible.
    //

    LoadEvent->PreferredBaseAddress = NULL;

    //
    // Initialize the list entry and add it to the tail of the module's list.
    //

    InitializeListHead(&LoadEvent->ListEntry);
    AppendTailList(&ModuleEntry->LoadEventsListHead, &LoadEvent->ListEntry);

    //
    // Capture a stack back trace.
    //

    NumberOfFramesToCapture = (
        sizeof(LoadEvent->BackTrace) /
        sizeof(LoadEvent->BackTrace[0])
    );

    LoadEvent->FramesCaptured = (
        Rtl->RtlCaptureStackBackTrace(
            FRAMES_TO_SKIP,
            NumberOfFramesToCapture,
            (PVOID)&LoadEvent->BackTrace,
            &LoadEvent->BackTraceHash
        )
    );

    //
    // Return now if this wasn't already a new record.
    //

    if (!NewRecord) {
        return;
    }

    //
    // Search for an entry in the Unicode prefix table for this module name.
    //

    TraceStoreAcquireLockShared(ModuleTableStore);
    UnicodePrefixTableEntry = (
        Rtl->RtlFindUnicodePrefix(
            &ModuleTable->ModuleNamePrefixTable,
            &ModuleEntry->File.Path.Full,
            0
        )
    );
    TraceStoreReleaseLockShared(ModuleTableStore);

    if (UnicodePrefixTableEntry) {

        BOOL Identical;
        PUNICODE_STRING Match = UnicodePrefixTableEntry->Prefix;

        if (Match->Length < ModuleEntry->File.Path.Full.Length) {

            //
            // The match was only a prefix match; insert the prefix.
            //

            goto InsertPrefix;
        }

        //
        // There's already a prefix table entry with this name.  Check to see
        // if it's pointing at an identical module table entry.
        //

        ExistingModuleEntry = (
            CONTAINING_RECORD(
                UnicodePrefixTableEntry,
                TRACE_MODULE_TABLE_ENTRY,
                ModuleNamePrefixTableEntry
            )
        );

        Identical = (
            ModuleEntry->File.EndOfFile.QuadPart ==
            ExistingModuleEntry->File.EndOfFile.QuadPart && (
                sizeof(ModuleEntry->File.MD5) == Rtl->RtlCompareMemory(
                    (PVOID)&ModuleEntry->File.MD5,
                    (PVOID)&ExistingModuleEntry->File.MD5,
                    sizeof(ModuleEntry->File.MD5)
                )
            )
        );

        if (Identical) {

            //
            // The two files have identical sizes and MD5 checksums.
            //

            NOTHING;

        } else {

            //
            // Two different files with the exact same name?
            //

            NOTHING;
        }

        //
        // Just append this new module table entry to the duplicate list head
        // for now.
        //

        AppendTailList(&ExistingModuleEntry->DuplicateEntriesListHead,
                       &ModuleEntry->DuplicateEntriesListEntry);

    } else {

InsertPrefix:

        TraceStoreAcquireLockExclusive(ModuleTableStore);
        Success = (
            Rtl->RtlInsertUnicodePrefix(
                &ModuleTable->ModuleNamePrefixTable,
                &ModuleEntry->File.Path.Full,
                &ModuleEntry->ModuleNamePrefixTableEntry
            )
        );
        TraceStoreReleaseLockExclusive(ModuleTableStore);

        if (!Success) {

            //
            // Unicode prefix was already in the table.  This shouldn't happen.
            //

            __debugbreak();
        }
    }

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
