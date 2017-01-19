/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSymbols.c

Abstract:

    This module implements functionality related to capturing C module
    (dll/exe) symbol information.

--*/

#include "stdafx.h"

//
// Use an INIT_ONCE structure to ensure there's only ever one active context.
//

INIT_ONCE InitOnceTraceSymbolContext = INIT_ONCE_STATIC_INIT;
PTRACE_SYMBOL_CONTEXT GlobalTraceSymbolContext = NULL;

#define CRITICAL_SECTION_SPIN_COUNT 1000

//
// AVL routines.
//

RTL_GENERIC_COMPARE_RESULTS
NTAPI
SymbolTableEntryCompareRoutine(
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
SymbolTableEntryAllocateRoutine(
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
    AllocSize = HeaderSize + sizeof(TRACE_SYMBOL_TABLE_ENTRY);

    return Allocate(TraceContext, TraceStore, 1, AllocSize);
}

VOID
NTAPI
SymbolTableEntryFreeRoutine(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
    )
{
    __debugbreak();
}

//
// BindComplete functions.
//

_Use_decl_annotations_
BOOL
SymbolTableStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the SymbolTable trace store.  It calls the normal trace store bind complete
    routine.

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
    PTRACE_SYMBOL_TABLE SymbolTable;
    PTRACE_STORES TraceStores;
    PTRACE_STORE SymbolTableEntryStore;

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

    SymbolTable = (PTRACE_SYMBOL_TABLE)(
        TraceStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore->TraceStore,
            1,
            sizeof(*SymbolTable)
        )
    );

    if (!SymbolTable) {
        return FALSE;
    }

    //
    // Set the structure size.
    //

    SymbolTable->SizeOfStruct = sizeof(*SymbolTable);

    //
    // Resolve the module table entry store.
    //

    TraceStores = TraceContext->TraceStores;
    SymbolTableEntryStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreSymbolTableEntryId
        )
    );

    //
    // Initialize the AVL table.
    //

    Rtl->RtlInitializeGenericTableAvl(&SymbolTable->SymbolHashTable,
                                      SymbolTableEntryCompareRoutine,
                                      SymbolTableEntryAllocateRoutine,
                                      SymbolTableEntryFreeRoutine,
                                      SymbolTableEntryStore);

    //
    // Initialize the prefix table.
    //

    Rtl->PfxInitialize(&SymbolTable->SymbolPrefixTable);

    //
    // Set the bind complete event and return success.
    //

    SetEvent(TraceStore->BindCompleteEvent);

    return TRUE;
}

_Use_decl_annotations_
BOOL
SymbolBufferStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the SymbolBuffer trace store.  It waits for both the SymbolTable and
    SymbolTableEntry stores to be ready (i.e. have their relocation complete
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
    BOOL WaitAll = TRUE;
    ULONG WaitResult;
    ULONG NumberOfWaits;
    PTRACE_STORES TraceStores;
    PTRACE_SYMBOL_CONTEXT SymbolContext;
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
            TraceStoreSymbolTableId
        )->BindCompleteEvent
    );

    Events[1] = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreSymbolTableEntryId
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
    // All waits were satisfied.  We can set the relevant symbol table fields
    // and resume the symbol tracing thread.
    //

    SymbolContext = TraceContext->SymbolContext;

    if (ResumeThread(SymbolContext->ThreadHandle) == -1) {
        SymbolContext->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
CALLBACK
CreateTraceSymbolContextCallback(
    PINIT_ONCE InitOnce,
    PTRACE_CONTEXT TraceContext,
    PPTRACE_SYMBOL_CONTEXT SymbolContextPointer
    )
/*++

Routine Description:

    This routine is the InitOnce callback function for creating a new trace
    symbol context.

Arguments:

    InitOnce - Supplies a pointer to the INIT_ONCE structure protecting this
        context.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure to initialize
        the symbol context with.

    SymbolContextPointer - Supplies the address of a variable that will receive
        the address of the newly created TRACE_SYMBOL_CONTEXT structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    BOOL AutoReset = FALSE;
    BOOL ManualReset = TRUE;
    ULONG SpinCount;
    HANDLE ShutdownEvent = NULL;
    HANDLE WorkAvailableEvent = NULL;
    PTRACE_SYMBOL_CONTEXT SymbolContext;
    PCRITICAL_SECTION CriticalSection;
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(SymbolContextPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *SymbolContextPointer = NULL;

    //
    // Continue argument validation.
    //

    if (!ARGUMENT_PRESENT(InitOnce)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    //
    // Attempt to create the events for communicating with the symbol tracing
    // thread.
    //

    ShutdownEvent = CreateEvent(NULL, ManualReset, FALSE, NULL);
    if (!ShutdownEvent) {
        goto Error;
    }

    WorkAvailableEvent = CreateEvent(NULL, AutoReset, FALSE, NULL);
    if (!WorkAvailableEvent) {
        goto Error;
    }

    //
    // Resolve aliases.
    //

    Allocator = TraceContext->Allocator;

    //
    // Allocate space for the context.
    //

    SymbolContext = (PTRACE_SYMBOL_CONTEXT)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(*SymbolContext)
        )
    );

    if (!SymbolContext) {
        goto Error;
    }

    //
    // Initialize the critical section.
    //

    SpinCount = CRITICAL_SECTION_SPIN_COUNT;
    CriticalSection = &SymbolContext->CriticalSection;
    Success = InitializeCriticalSectionAndSpinCount(CriticalSection, SpinCount);

    if (!Success) {
        goto Error;
    }

    //
    // Acquire the critical section and initialize the remaining fields.
    //

    AcquireTraceSymbolContextLock(SymbolContext);
    SymbolContext->SizeOfStruct = sizeof(*SymbolContext);
    SymbolContext->TraceContext = TraceContext;
    SymbolContext->ThreadEntry = TraceSymbolThreadEntryImpl;
    SymbolContext->ShutdownEvent = ShutdownEvent;
    SymbolContext->WorkAvailableEvent = WorkAvailableEvent;
    SymbolContext->State = TraceSymbolContextStructureCreatedState;
    InitializeSListHead(&SymbolContext->WorkListHead);
    ReleaseTraceSymbolContextLock(SymbolContext);

    //
    // Update the caller's pointer.
    //

    *SymbolContextPointer = SymbolContext;

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

    //
    // Free any events if they exist.
    //

    if (ShutdownEvent) {
        CloseHandle(ShutdownEvent);
        ShutdownEvent = NULL;
    }

    if (WorkAvailableEvent) {
        CloseHandle(WorkAvailableEvent);
        WorkAvailableEvent = NULL;
    }

    //
    // Free the symbol context if it exists.
    //

    if (SymbolContext) {
        Allocator->Free(Allocator->Context, SymbolContext);
        SymbolContext = NULL;
    }

End:
    return Success;
}

_Use_decl_annotations_
BOOL
CreateTraceSymbolContext(
    PTRACE_CONTEXT TraceContext
    )
/*++

Routine Description:

    This routine creates a new TRACE_SYMBOL_CONTEXT.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    PINIT_ONCE_CALLBACK Callback;
    PTRACE_SYMBOL_CONTEXT SymbolContext;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    //
    // Attempt to create the symbol context from the current trace context.
    //

    Callback = (PINIT_ONCE_CALLBACK)&CreateTraceSymbolContextCallback;
    Success = InitOnceExecuteOnce(&InitOnceTraceSymbolContext,
                                  Callback,
                                  TraceContext,
                                  &SymbolContext);
    if (!Success) {
        OutputDebugStringA("TraceSymbolContext:InitOnceExecuteOnce failed.\n");
        return FALSE;
    }

    //
    // The call was successful, make sure that SymbolContext is not NULL, its
    // TraceContext is set to us, and that it's in the Created state.
    //

    if (!SymbolContext) {
        __debugbreak();
        return FALSE;
    }

    if (SymbolContext->TraceContext != TraceContext) {
        __debugbreak();
        return FALSE;
    }

    Success = FALSE;
    AcquireTraceSymbolContextLock(SymbolContext);
    if (SymbolContextIsStructureCreated(SymbolContext)) {
        TraceContext->SymbolContext = SymbolContext;
        Success = TRUE;
    }
    ReleaseTraceSymbolContextLock(SymbolContext);

    return Success;
}

_Use_decl_annotations_
BOOL
InitializeTraceSymbolContext(
    PTRACE_CONTEXT TraceContext
    )
/*++

Routine Description:

    This routine initialize a new TRACE_SYMBOL_CONTEXT for the given trace
    context.  This includes creating the structure via CreateTraceSymbolContext
    and then creating a new thread and with the TraceSymbolThreadEntry entry
    point.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    ULONG ThreadId;
    HANDLE ThreadHandle;
    PTRACE_SYMBOL_CONTEXT SymbolContext;
    LPTHREAD_START_ROUTINE StartRoutine;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    if (!CreateTraceSymbolContext(TraceContext)) {
        OutputDebugStringA("CreateTraceSymbolContext() failed.\n");
        return FALSE;
    }

    SymbolContext = TraceContext->SymbolContext;
    if (!SymbolContext) {
        __debugbreak();
        return FALSE;
    }

    StartRoutine = (LPTHREAD_START_ROUTINE)TraceSymbolThreadEntry;
    ThreadHandle = CreateThread(NULL,
                                0,
                                StartRoutine,
                                SymbolContext,
                                CREATE_SUSPENDED,
                                &ThreadId);

    if (ThreadHandle == NULL) {
        SymbolContext->LastError = GetLastError();
        OutputDebugStringA("InitializeTraceSymbolContext:CreateThread failed");
        __debugbreak();
        return FALSE;
    }

    AcquireTraceSymbolContextLock(SymbolContext);
    if (SymbolContext->ThreadId != 0) {
        __debugbreak();
    }
    if (SymbolContext->ThreadHandle != NULL) {
        __debugbreak();
    }
    SymbolContext->ThreadId = ThreadId;
    SymbolContext->ThreadHandle = ThreadHandle;
    SymbolContext->State = TraceSymbolContextThreadCreatedState;
    ReleaseTraceSymbolContextLock(SymbolContext);

    return TRUE;
}

_Use_decl_annotations_
ULONG
WINAPI
TraceSymbolThreadEntry(
    PTRACE_SYMBOL_CONTEXT SymbolContext
    )
/*++

Routine Description:

    This routine is the standard entry point for the symbol tracing thread.
    It performs initial Dbghelp symbol initialization via Rtl and then calls
    the SymbolContext->ThreadEntry function within a try/catch block that
    suppresses STATUS_IN_PAGE_ERROR exceptions.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE otherwise.


--*/
{
    PRTL Rtl;
    ULONG Result;
    ULONG Options;

    Rtl = SymbolContext->TraceContext->Rtl;

    Options = (
        SYMOPT_DEBUG            |
        SYMOPT_UNDNAME          |
        SYMOPT_AUTO_PUBLICS     |
        SYMOPT_LOAD_LINES       |
        SYMOPT_LOAD_ANYTHING    |
        SYMOPT_CASE_INSENSITIVE
    );

    if (!Rtl->SymSetOptions(Options)) {
        OutputDebugStringA("Rtl->SymSetOptions() failed.\n");
        return 1;
    }

    if (!Rtl->SymInitialize(SymbolContext->ThreadHandle, NULL, FALSE)) {
        OutputDebugStringA("Rtl->SymInitialize() failed.\n");
        return 1;
    }

    OutputDebugStringA("Symbols successfully initialized.\n");

    TRY_MAPPED_MEMORY_OP {
        Result = SymbolContext->ThreadEntry(SymbolContext);
    } CATCH_STATUS_IN_PAGE_ERROR {
        Result = 1;
    }

    return Result;
}

_Use_decl_annotations_
ULONG
WINAPI
TraceSymbolThreadEntryImpl(
    PTRACE_SYMBOL_CONTEXT SymbolContext
    )
/*++

Routine Description:

    This routine is the implementation backend of the symbol tracing worker
    thread.

Arguments:

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

Return Value:

    0 on success, 1 on error.

--*/
{
    BOOL WaitAny = FALSE;
    ULONG Result = 1;
    ULONG WaitResult;
    ULONG NumberOfWaits = 2;
    HANDLE Events[2];
    HANDLE ShutdownEvent;
    HANDLE WorkAvailableEvent;

    //
    // Initialize event aliases.
    //

    ShutdownEvent = SymbolContext->ShutdownEvent;
    WorkAvailableEvent = SymbolContext->WorkAvailableEvent;
    Events[0] = ShutdownEvent;
    Events[1] = WorkAvailableEvent;

    do {

        AcquireTraceSymbolContextLock(SymbolContext);
        SymbolContext->State = TraceSymbolContextWaitingForWorkState;
        ReleaseTraceSymbolContextLock(SymbolContext);

        WaitResult = WaitForMultipleObjects(NumberOfWaits,
                                            Events,
                                            WaitAny,
                                            INFINITE);

        AcquireTraceSymbolContextLock(SymbolContext);

        if (WaitResult == WAIT_OBJECT_0) {

            //
            // Shutdown event.
            //

            SymbolContext->State = TraceSymbolContextReceivedShutdownState;
            Result = 0;
            break;

        } else if (WaitResult == WAIT_OBJECT_0+1) {

            //
            // Work available event.  Pop an item off the work queue and
            // process it.
            //

            SymbolContext->State = TraceSymbolContextProcessingWorkState;
            ReleaseTraceSymbolContextLock(SymbolContext);
            if (!ProcessTraceSymbolWork(SymbolContext)) {
                __debugbreak();
                Result = 1;
                break;
            }
        } else {
            SymbolContext->State = (
                TraceSymbolContextWaitFailureInducedShutdownState
            );
            Result = 1;
            break;
        }
    } while (1);

    ReleaseTraceSymbolContextLock(SymbolContext);

    return Result;
}

_Use_decl_annotations_
BOOL
ProcessTraceSymbolWork(
    PTRACE_SYMBOL_CONTEXT SymbolContext
    )
/*++

Routine Description:

    This routine processes work entries pushed to the WorkListHead of the
    SymbolContext.

Arguments:

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    PSLIST_ENTRY ListEntry;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;

    ListEntry = InterlockedFlushSList(&SymbolContext->WorkListHead);

    for (; ListEntry != NULL; ListEntry = ListEntry->Next) {

        ModuleTableEntry = CONTAINING_RECORD(ListEntry,
                                             TRACE_MODULE_TABLE_ENTRY,
                                             ListEntry);

        Success = CreateSymbolTableForModuleTableEntry(SymbolContext,
                                                       ModuleTableEntry);

        if (!Success) {
            SymbolContext->NumberOfWorkItemsFailed++;
        } else {
            SymbolContext->NumberOfWorkItemsSucceeded++;
        }

        SymbolContext->NumberOfWorkItemsProcessed++;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
CreateSymbolTableForModuleTableEntry(
    PTRACE_SYMBOL_CONTEXT SymbolContext,
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
/*++

Routine Description:

    This routine creates a new TRACE_SYMBOL_TABLE and associated
    TRACE_SYMBOL_TABLE_ENTRY for a given TRACE_MODULE_TABLE_ENTRY.

Arguments:

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

    ModuleTableEntry - Supplies a pointer to a TRACE_MODULE_TABLE_ENTRY
        structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    OutputDebugStringW(L"CreateSymbolTableForModuleTableEntry:\n");
    OutputDebugStringW(ModuleTableEntry->File.Path.Full.Buffer);
    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
