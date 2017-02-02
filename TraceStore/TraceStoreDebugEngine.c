/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreDebugEngine.c

Abstract:

    This module implements functionality related to capturing C module
    (dll/exe) type and assembly information via the DebugEngine component.

--*/

#include "stdafx.h"

//
// Use an INIT_ONCE structure to ensure there's only ever one active context.
//

INIT_ONCE InitOnceTraceDebugContext = INIT_ONCE_STATIC_INIT;
PTRACE_DEBUG_CONTEXT GlobalTraceDebugContext = NULL;

#define CRITICAL_SECTION_SPIN_COUNT 1000

//
// AVL routines.
//

//
// TypeInfoTable
//

RTL_GENERIC_COMPARE_RESULTS
NTAPI
TypeInfoTableEntryCompareRoutine(
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
TypeInfoTableEntryAllocateRoutine(
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
    AllocSize = HeaderSize + sizeof(TRACE_TYPE_INFO_TABLE_ENTRY);

    return Allocate(TraceContext, TraceStore, 1, AllocSize);
}

VOID
NTAPI
TypeInfoTableEntryFreeRoutine(
    _In_ struct _RTL_AVL_TABLE *Table,
    _In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
    )
{
    __debugbreak();
}

//
// FunctionTable
//

RTL_GENERIC_COMPARE_RESULTS
NTAPI
FunctionTableEntryCompareRoutine(
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
FunctionTableEntryAllocateRoutine(
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
    AllocSize = HeaderSize + sizeof(TRACE_FUNCTION_TABLE_ENTRY);

    return Allocate(TraceContext, TraceStore, 1, AllocSize);
}

VOID
NTAPI
FunctionTableEntryFreeRoutine(
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
TypeInfoTableStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the TypeInfoTable trace store.  It calls the normal trace store bind
    complete
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
    PTRACE_TYPE_INFO_TABLE TypeInfoTable;
    PTRACE_STORES TraceStores;
    PTRACE_STORE TypeInfoTableEntryStore;

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

    TypeInfoTable = (PTRACE_TYPE_INFO_TABLE)(
        TraceStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore->TraceStore,
            1,
            sizeof(*TypeInfoTable)
        )
    );

    if (!TypeInfoTable) {
        return FALSE;
    }

    //
    // Set the structure size.
    //

    TypeInfoTable->SizeOfStruct = sizeof(*TypeInfoTable);

    //
    // Resolve the module table entry store.
    //

    TraceStores = TraceContext->TraceStores;
    TypeInfoTableEntryStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreTypeInfoTableEntryId
        )
    );

    //
    // Initialize the AVL table.
    //

    Rtl->RtlInitializeGenericTableAvl(&TypeInfoTable->TypeInfoTable,
                                      TypeInfoTableEntryCompareRoutine,
                                      TypeInfoTableEntryAllocateRoutine,
                                      TypeInfoTableEntryFreeRoutine,
                                      TypeInfoTableEntryStore);

    //
    // Initialize the prefix table.
    //

    Rtl->PfxInitialize(&TypeInfoTable->TypeInfoPrefixTable);

    //
    // Set the bind complete event and return success.
    //

    SetEvent(TraceStore->BindCompleteEvent);

    return TRUE;
}

_Use_decl_annotations_
BOOL
FunctionTableStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the FunctionTable trace store.  It calls the normal trace store bind
    complete routine.

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
    PTRACE_FUNCTION_TABLE FunctionTable;
    PTRACE_STORES TraceStores;
    PTRACE_STORE FunctionTableEntryStore;

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

    FunctionTable = (PTRACE_FUNCTION_TABLE)(
        TraceStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore->TraceStore,
            1,
            sizeof(*FunctionTable)
        )
    );

    if (!FunctionTable) {
        return FALSE;
    }

    //
    // Set the structure size.
    //

    FunctionTable->SizeOfStruct = sizeof(*FunctionTable);

    //
    // Resolve the module table entry store.
    //

    TraceStores = TraceContext->TraceStores;
    FunctionTableEntryStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreFunctionTableEntryId
        )
    );

    //
    // Initialize the AVL table.
    //

    Rtl->RtlInitializeGenericTableAvl(&FunctionTable->FunctionTable,
                                      FunctionTableEntryCompareRoutine,
                                      FunctionTableEntryAllocateRoutine,
                                      FunctionTableEntryFreeRoutine,
                                      FunctionTableEntryStore);

    //
    // Initialize the prefix table.
    //

    Rtl->PfxInitialize(&FunctionTable->FunctionPrefixTable);

    //
    // Set the bind complete event and return success.
    //

    SetEvent(TraceStore->BindCompleteEvent);

    return TRUE;
}

_Use_decl_annotations_
BOOL
FunctionSourceCodeStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the FunctionSourceCode trace store.

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
    PTRACE_DEBUG_CONTEXT DebugContext;
    HANDLE Events[8];

    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        return FALSE;
    }

    DebugContext = TraceContext->DebugContext;

    Events[0] = DEBUG_CONTEXT_STORE(TypeInfoTable)->BindCompleteEvent;
    Events[1] = DEBUG_CONTEXT_STORE(TypeInfoTableEntry)->BindCompleteEvent;
    Events[2] = DEBUG_CONTEXT_STORE(TypeInfoStringBuffer)->BindCompleteEvent;
    Events[3] = DEBUG_CONTEXT_STORE(FunctionTable)->BindCompleteEvent;
    Events[4] = DEBUG_CONTEXT_STORE(FunctionTableEntry)->BindCompleteEvent;
    Events[5] = DEBUG_CONTEXT_STORE(FunctionAssembly)->BindCompleteEvent;
    Events[6] = DEBUG_CONTEXT_STORE(ExamineSymbolsLine)->BindCompleteEvent;
    Events[7] = DEBUG_CONTEXT_STORE(ExamineSymbolsText)->BindCompleteEvent;

    NumberOfWaits = ARRAYSIZE(Events);

    WaitResult = WaitForMultipleObjects(NumberOfWaits,
                                        Events,
                                        WaitAll,
                                        INFINITE);

    if (WaitResult != WAIT_OBJECT_0) {
        return FALSE;
    }

    //
    // All waits were satisfied.  We can resume the debug engine thread.
    //

    if (ResumeThread(DebugContext->ThreadHandle) == -1) {
        DebugContext->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
CALLBACK
CreateTraceDebugContextCallback(
    PINIT_ONCE InitOnce,
    PTRACE_CONTEXT TraceContext,
    PPTRACE_DEBUG_CONTEXT DebugContextPointer
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

    DebugContextPointer - Supplies the address of a variable that will receive
        the address of the newly created TRACE_DEBUG_CONTEXT structure.

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
    PTRACE_STORES TraceStores;
    PTRACE_DEBUG_CONTEXT DebugContext;
    PCRITICAL_SECTION CriticalSection;
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(DebugContextPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *DebugContextPointer = NULL;

    //
    // Continue argument validation.
    //

    if (!ARGUMENT_PRESENT(InitOnce)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    TraceStores = TraceContext->TraceStores;

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

    DebugContext = (PTRACE_DEBUG_CONTEXT)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            sizeof(*DebugContext)
        )
    );

    if (!DebugContext) {
        goto Error;
    }

    //
    // Initialize the critical section.
    //

    SpinCount = CRITICAL_SECTION_SPIN_COUNT;
    CriticalSection = &DebugContext->CriticalSection;
    Success = InitializeCriticalSectionAndSpinCount(CriticalSection, SpinCount);

    if (!Success) {
        goto Error;
    }

    //
    // Acquire the critical section and initialize the remaining fields.
    //

    AcquireTraceDebugContextLock(DebugContext);
    DebugContext->SizeOfStruct = sizeof(*DebugContext);
    DebugContext->TraceContext = TraceContext;
    DebugContext->ThreadEntry = TraceDebugEngineThreadEntryImpl;
    DebugContext->ShutdownEvent = ShutdownEvent;
    DebugContext->WorkAvailableEvent = WorkAvailableEvent;
    DebugContext->State = TraceDebugContextStructureCreatedState;
    InitializeSListHead(&DebugContext->WorkListHead);

#define RESOLVE_STORE(Name)               \
    DebugContext->TraceStores.##Name = ( \
        TraceStoreIdToTraceStore(         \
            TraceStores,                  \
            TraceStore##Name##Id          \
        )                                 \
    )

    RESOLVE_STORE(TypeInfoTable);
    RESOLVE_STORE(TypeInfoTableEntry);
    RESOLVE_STORE(TypeInfoStringBuffer);
    RESOLVE_STORE(FunctionTable);
    RESOLVE_STORE(FunctionTableEntry);
    RESOLVE_STORE(FunctionAssembly);
    RESOLVE_STORE(FunctionSourceCode);
    RESOLVE_STORE(ExamineSymbolsLine);
    RESOLVE_STORE(ExamineSymbolsText);

    ReleaseTraceDebugContextLock(DebugContext);

    //
    // Update the caller's pointer.
    //

    *DebugContextPointer = DebugContext;

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

    if (DebugContext) {
        Allocator->Free(Allocator->Context, DebugContext);
        DebugContext = NULL;
    }

End:
    return Success;
}

_Use_decl_annotations_
BOOL
CreateTraceDebugContext(
    PTRACE_CONTEXT TraceContext
    )
/*++

Routine Description:

    This routine creates a new TRACE_DEBUG_CONTEXT.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    BOOL Success;
    PINIT_ONCE_CALLBACK Callback;
    PTRACE_DEBUG_CONTEXT DebugContext;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    //
    // Attempt to create the symbol context from the current trace context.
    //

    Callback = (PINIT_ONCE_CALLBACK)&CreateTraceDebugContextCallback;
    Success = InitOnceExecuteOnce(&InitOnceTraceDebugContext,
                                  Callback,
                                  TraceContext,
                                  &DebugContext);
    if (!Success) {
        OutputDebugStringA("TraceDebugContext:InitOnceExecuteOnce failed.\n");
        return FALSE;
    }

    //
    // The call was successful, make sure that DebugContext is not NULL, its
    // TraceContext is set to us, and that it's in the Created state.
    //

    if (!DebugContext) {
        __debugbreak();
        return FALSE;
    }

    if (DebugContext->TraceContext != TraceContext) {
        __debugbreak();
        return FALSE;
    }

    Success = FALSE;
    AcquireTraceDebugContextLock(DebugContext);
    if (DebugContextIsStructureCreated(DebugContext)) {
        TraceContext->DebugContext = DebugContext;
        Success = TRUE;
    }
    ReleaseTraceDebugContextLock(DebugContext);

    return Success;
}

_Use_decl_annotations_
BOOL
InitializeTraceDebugContext(
    PTRACE_CONTEXT TraceContext
    )
/*++

Routine Description:

    This routine initialize a new TRACE_DEBUG_CONTEXT for the given trace
    context.  This includes creating the structure via CreateTraceDebugContext
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
    PTRACE_DEBUG_CONTEXT DebugContext;
    LPTHREAD_START_ROUTINE StartRoutine;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    if (!CreateTraceDebugContext(TraceContext)) {
        OutputDebugStringA("CreateTraceDebugContext() failed.\n");
        return FALSE;
    }

    DebugContext = TraceContext->DebugContext;
    if (!DebugContext) {
        __debugbreak();
        return FALSE;
    }

    StartRoutine = (LPTHREAD_START_ROUTINE)TraceDebugEngineThreadEntry;
    ThreadHandle = CreateThread(NULL,
                                0,
                                StartRoutine,
                                DebugContext,
                                CREATE_SUSPENDED,
                                &ThreadId);

    if (ThreadHandle == NULL) {
        DebugContext->LastError = GetLastError();
        OutputDebugStringA("InitializeTraceDebugContext:CreateThread failed");
        __debugbreak();
        return FALSE;
    }

    AcquireTraceDebugContextLock(DebugContext);
    if (DebugContext->ThreadId != 0) {
        __debugbreak();
    }
    if (DebugContext->ThreadHandle != NULL) {
        __debugbreak();
    }
    DebugContext->ThreadId = ThreadId;
    DebugContext->ThreadHandle = ThreadHandle;
    DebugContext->State = TraceDebugContextThreadCreatedState;
    ReleaseTraceDebugContextLock(DebugContext);

    return TRUE;
}

_Use_decl_annotations_
ULONG
WINAPI
TraceDebugEngineThreadEntry(
    PTRACE_DEBUG_CONTEXT DebugContext
    )
/*++

Routine Description:

    This routine is the standard entry point for the debug engine thread.

Arguments:

    DebugContext - Supplies a pointer to a TRACE_DEBUG_CONTEXT structure.

Return Value:

    TRUE on success, FALSE otherwise.


--*/
{
    PRTL Rtl;
    BOOL Success;
    ULONG ExitCode;
    PTRACE_STORES TraceStores;
    PTRACE_CONTEXT TraceContext;
    PALLOCATOR StringTableAllocator;
    PALLOCATOR StringArrayAllocator;
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags = { 0 };
    PTRACER_CONFIG TracerConfig;

    TraceContext = DebugContext->TraceContext;
    TraceStores = TraceContext->TraceStores;
    Rtl = TraceContext->Rtl;

    InitFlags.InitializeFromCurrentProcess = TRUE;
    TracerConfig = TraceContext->TracerConfig;

    StringTableAllocator = &(
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreStringTableId
        )
    )->Allocator;

    StringArrayAllocator = &(
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreStringArrayId
        )
    )->Allocator;

    Success = LoadAndInitializeDebugEngineSession(
        &TracerConfig->Paths.DebugEngineDllPath,
        Rtl,
        TraceContext->Allocator,
        InitFlags,
        &TracerConfig->Paths.StringTableDllPath,
        StringArrayAllocator,
        StringTableAllocator,
        &DebugContext->DebugEngineSession,
        &DebugContext->DestroyDebugEngineSession
    );

    if (!Success) {
        OutputDebugStringA("LoadAndInitializeDebugEngineSession() failed.");
        return 1;
    }

    OutputDebugStringA("Debug Engine successfully initialized.\n");

    TRY_MAPPED_MEMORY_OP {
        ExitCode = DebugContext->ThreadEntry(DebugContext);
    } CATCH_STATUS_IN_PAGE_ERROR {
        ExitCode = 1;
    }

    return ExitCode;
}

_Use_decl_annotations_
ULONG
WINAPI
TraceDebugEngineThreadEntryImpl(
    PTRACE_DEBUG_CONTEXT DebugContext
    )
/*++

Routine Description:

    This routine is the implementation backend of the debug engine worker
    thread.

Arguments:

    DebugContext - Supplies a pointer to a TRACE_DEBUG_CONTEXT structure.

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

    ShutdownEvent = DebugContext->ShutdownEvent;
    WorkAvailableEvent = DebugContext->WorkAvailableEvent;
    Events[0] = ShutdownEvent;
    Events[1] = WorkAvailableEvent;

    do {

        AcquireTraceDebugContextLock(DebugContext);
        DebugContext->State = TraceDebugContextWaitingForWorkState;
        ReleaseTraceDebugContextLock(DebugContext);

        WaitResult = WaitForMultipleObjects(NumberOfWaits,
                                            Events,
                                            WaitAny,
                                            INFINITE);

        AcquireTraceDebugContextLock(DebugContext);

        if (WaitResult == WAIT_OBJECT_0) {

            //
            // Shutdown event.
            //

            DebugContext->State = TraceDebugContextReceivedShutdownState;
            Result = 0;
            break;

        } else if (WaitResult == WAIT_OBJECT_0+1) {

            //
            // Work available event.  Pop an item off the work queue and
            // process it.
            //

            DebugContext->State = TraceDebugContextProcessingWorkState;
            ReleaseTraceDebugContextLock(DebugContext);
            if (!ProcessTraceDebugEngineWork(DebugContext)) {
                __debugbreak();
                Result = 1;
                break;
            }
        } else {
            DebugContext->State = (
                TraceDebugContextWaitFailureInducedShutdownState
            );
            Result = 1;
            break;
        }
    } while (1);

    ReleaseTraceDebugContextLock(DebugContext);

    return Result;
}

_Use_decl_annotations_
BOOL
ProcessTraceDebugEngineWork(
    PTRACE_DEBUG_CONTEXT DebugContext
    )
/*++

Routine Description:

    This routine processes work entries pushed to the WorkListHead of the
    DebugContext.

Arguments:

    DebugContext - Supplies a pointer to a TRACE_DEBUG_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    PSLIST_ENTRY ListEntry;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;

    ListEntry = InterlockedFlushSList(&DebugContext->WorkListHead);

    for (; ListEntry != NULL; ListEntry = ListEntry->Next) {

        ModuleTableEntry = CONTAINING_RECORD(ListEntry,
                                             TRACE_MODULE_TABLE_ENTRY,
                                             DebugContextListEntry);

        Success = CreateTypeInfoTableForModuleTableEntry(DebugContext,
                                                         ModuleTableEntry);

        if (!Success) {
            DebugContext->NumberOfWorkItemsFailed++;
        } else {
            DebugContext->NumberOfWorkItemsSucceeded++;
        }

        DebugContext->NumberOfWorkItemsProcessed++;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
CreateTypeInfoTableForModuleTableEntry(
    PTRACE_DEBUG_CONTEXT DebugContext,
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
/*++

Routine Description:

    This routine creates a new TRACE_TYPE_INFO_TABLE and associated
    TRACE_TYPE_INFO_TABLE_ENTRY for a given TRACE_MODULE_TABLE_ENTRY.

Arguments:

    DebugContext - Supplies a pointer to a TRACE_DEBUG_CONTEXT structure.

    ModuleTableEntry - Supplies a pointer to a TRACE_MODULE_TABLE_ENTRY
        structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    PRTL Rtl;
    PRTL_FILE File;
    PRTL_PATH Path;
    PALLOCATOR Allocator;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    TRACE_FLAGS TraceFlags;
    DEBUG_ENGINE_OUTPUT Output;
    PDEBUG_ENGINE_SESSION DebugEngineSession;
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS ExamineSymbolsOptions;

    //
    // Capture a timestamp for processing this module table entry.
    //

    QueryPerformanceCounter(&DebugContext->CurrentTimestamp);

    //
    // Initialize aliases.
    //

    TraceContext = DebugContext->TraceContext;
    TraceStores = TraceContext->TraceStores;
    TraceFlags.AsULong = TraceStores->Flags.AsULong;

    File = &ModuleTableEntry->File;
    Path = &File->Path;

    //
    // Determine if we should continue processing this module based on whether
    // or not the trace flags indicate it should be ignored or not.
    //

    if (TraceFlags.IgnoreModulesInWindowsSystemDirectory) {
        if (Path->Flags.WithinWindowsSystemDirectory) {
            DebugContext->NumberOfWorkItemsIgnored++;
            return TRUE;
        }
    } else if (TraceFlags.IgnoreModulesInWindowsSxSDirectory) {
        if (Path->Flags.WithinWindowsSxSDirectory) {
            DebugContext->NumberOfWorkItemsIgnored++;
            return TRUE;
        }
    }

    //
    // Continue initializing aliases.
    //

    Rtl = TraceContext->Rtl;
    Allocator = TraceContext->Allocator;
    DebugEngineSession = DebugContext->DebugEngineSession;

    //
    // Initialize the DEBUG_ENGINE_EXAMINE_SYMBOLS_OUTPUT structure.  This is
    // a stack allocated structure that will persist for the lifetime of this
    // routine and is used to communicate partial output state across multiple
    // callbacks.
    //

    SecureZeroMemory(&Output, sizeof(DEBUG_ENGINE_OUTPUT));
    Output.SizeOfStruct = sizeof(DEBUG_ENGINE_OUTPUT);

    Success = DebugEngineSession->InitializeDebugEngineOutput(
        &Output,
        DebugEngineSession,
        Allocator,
        TraceDebugEngineExamineSymbolsLineOutputCallback,
        TraceDebugEngineExamineSymbolsPartialOutputCallback,
        TraceDebugEngineExamineSymbolsOutputCompleteCallback,
        DebugContext,
        Path
    );

    if (!Success) {
        return FALSE;
    }

    //
    // Update the debug context to point at this module table entry.
    //

    DebugContext->CurrentModuleTableEntry = ModuleTableEntry;

    //
    // Examine all symbols:
    //
    //      x /t /v <module>!*
    //
    //          - /t: include type info if known
    //          - /v: verbose
    //

    OutputFlags.EnableLineOutputCallbacks = TRUE;
    OutputFlags.EnablePartialOutputCallbacks = FALSE;

    ExamineSymbolsOptions.Verbose = 1;
    ExamineSymbolsOptions.TypeInformation = 1;

    Success = DebugEngineSession->ExamineSymbols(&Output,
                                                 OutputFlags,
                                                 ExamineSymbolsOptions);

    if (!Success) {
        return FALSE;
    }

    //
    // For each symbol:
    //
    //      If type == function:
    //
    //          Unassemble function:
    //
    //              uf /m /i <module>!<function name>
    //
    //                  - /m: relax blocking requirements to allow multiple
    //                        exits.
    //                  - /i: display number of instructions.
    //
    //      If symbol == type:
    //
    //          Display extended type information:
    //
    //              dt -a -b -r9 -v <module>!<typename>
    //

    //
    // Additional work:
    //
    // Functions:
    //
    //      - Capture call targets.
    //      - Capture # arguments.
    //      - Capture references to global data variables.
    //
    // Types:
    //
    //      - Look for pointers to other types and capture relationships.
    //
    //

    return TRUE;
}

_Use_decl_annotations_
BOOL
TraceDebugEngineExamineSymbolsLineOutputCallback(
    PDEBUG_ENGINE_OUTPUT Output
    )
{
    PRTL Rtl;
    CHAR Temp;
    PCHAR Text;
    BOOL Success;
    USHORT Length;
    ULONG Size;
    SHORT MatchIndex;
    PCHAR Char;
    PCHAR End;
    PSTRING Scope;
    STRING SymbolSize;
    STRING Address;
    STRING BasicType;
    STRING Function;
    STRING Parameters;
    PSTRING DestLine;
    PSTRING SourceLine;
    STRING_MATCH Match;
    ULARGE_INTEGER Addr;
    USHORT NumberOfParameters;
    PTRACE_STORE TextTraceStore;
    PTRACE_STORE LineTraceStore;
    PTRACE_DEBUG_CONTEXT DebugContext;
    PSTRING_TABLE StringTable;
    PDEBUG_ENGINE_SESSION Session;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_TYPE SymbolType;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_SCOPE SymbolScope;
    PRTL_CHAR_TO_INTEGER RtlCharToInteger;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    HANDLE HeapHandle = NULL;
    CHAR StackBitmapBuffer[32];
    RTL_BITMAP Bitmap = { 32 << 3, (PULONG)&StackBitmapBuffer };
    PRTL_BITMAP BitmapPointer = &Bitmap;

    DebugContext = (PTRACE_DEBUG_CONTEXT)Output->Context;
    Session = DebugContext->DebugEngineSession;

    SourceLine = &Output->Line;
    End = SourceLine->Buffer + SourceLine->Length;

    LineTraceStore = DebugContext->TraceStores.ExamineSymbolsLine;
    TextTraceStore = DebugContext->TraceStores.ExamineSymbolsText;

    DestLine = (PSTRING)(
        LineTraceStore->AllocateRecordsWithTimestamp(
            LineTraceStore->TraceContext,
            LineTraceStore,
            1,
            sizeof(*SourceLine),
            &Output->Timestamp.CommandStart
        )
    );

    if (!DestLine) {
        return FALSE;
    }

    Text = (PCHAR)(
        TextTraceStore->AllocateRecordsWithTimestamp(
            TextTraceStore->TraceContext,
            TextTraceStore,
            1,
            SourceLine->Length,
            &Output->Timestamp.CommandStart
        )
    );

    if (!Text) {
        return FALSE;
    }

    if (!CopyMemoryQuadwords(Text, SourceLine->Buffer, SourceLine->Length)) {
        return FALSE;
    }

    TRY_MAPPED_MEMORY_OP {
        DestLine->Length = SourceLine->Length;
        DestLine->MaximumLength = SourceLine->Length;
        DestLine->Buffer = Text;
    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }

    StringTable = Session->ExamineSymbolsPrefixStringTable;
    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;

    MatchIndex = IsPrefixOfStringInTable(StringTable,
                                         SourceLine,
                                         &Match);

    if (MatchIndex == NO_MATCH_FOUND) {
        return TRUE;
    }

    Rtl = DebugContext->TraceContext->Rtl;
    RtlCharToInteger = Rtl->RtlCharToInteger;

    SymbolScope = MatchIndex;
    Scope = Match.String;

    Length = Match.NumberOfMatchedCharacters + 1;
    Char = (SourceLine->Buffer + Length);

    while (*(++Char) == ' ');

    Address.Length = sizeof("00000000`00000000")-1;
    Address.MaximumLength = Address.Length;
    Address.Buffer = Char;

    if (Address.Buffer[8] != '`') {
        return TRUE;
    }

    Address.Buffer[8] = '\0';
    if (FAILED(RtlCharToInteger(Address.Buffer, 16, &Addr.HighPart))) {
        NOTHING;
    }
    Address.Buffer[8] = '`';

    Address.Buffer[17] = '\0';
    if (FAILED(RtlCharToInteger(Address.Buffer+9, 16, &Addr.LowPart))) {
        NOTHING;
    }
    Address.Buffer[17] = ' ';

    Char = Address.Buffer + Address.Length;

    while (*(++Char) == ' ');

    SymbolSize.Buffer = Char;

    while (*Char++ != ' ');

    SymbolSize.Length = (USHORT)(Char - SymbolSize.Buffer) - 1;
    SymbolSize.MaximumLength = SymbolSize.Length;

    BasicType.Buffer = Char;

    Temp = SymbolSize.Buffer[SymbolSize.Length];
    SymbolSize.Buffer[SymbolSize.Length] = '\0';
    if (FAILED(Rtl->RtlCharToInteger(SymbolSize.Buffer, 0, &Size))) {
        NOTHING;
    }

    SymbolSize.Buffer[SymbolSize.Length] = Temp;

    BasicType.Length = (USHORT)(End - BasicType.Buffer) - 1;
    BasicType.MaximumLength = BasicType.Length;

    StringTable = Session->ExamineSymbolsBasicTypeStringTable;
    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;

    MatchIndex = IsPrefixOfStringInTable(StringTable,
                                         &BasicType,
                                         &Match);

    if (MatchIndex == NO_MATCH_FOUND) {
        return TRUE;
    }

    SymbolType = MatchIndex;

    if (SymbolType != FunctionType) {
        return TRUE;
    }

    //
    // We've found a function.  Advance to the function name.
    //

    while (*(Char++) != '!');

    Function.Buffer = Char;

    while (*(Char++) != ' ');

    Function.Length = (USHORT)(Char - Function.Buffer) - 1;
    Function.MaximumLength = Function.Length;

    while (*(Char++) != '(');

    Parameters.Buffer = Char;
    Parameters.Length = (USHORT)(End - Parameters.Buffer) - 2;
    Parameters.MaximumLength = Parameters.Length;

    Success = Rtl->CreateBitmapIndexForString(Rtl,
                                              &Parameters,
                                              ',',
                                              &HeapHandle,
                                              &BitmapPointer,
                                              FALSE,
                                              NULL);

    NumberOfParameters = (USHORT)Rtl->RtlNumberOfSetBits(BitmapPointer);

//End:
    if (HeapHandle) {

        if ((ULONG_PTR)Bitmap.Buffer == (ULONG_PTR)BitmapPointer->Buffer) {
            __debugbreak();
        }

        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
TraceDebugEngineExamineSymbolsPartialOutputCallback(
    PDEBUG_ENGINE_OUTPUT Output
    )
/*++

Routine Description:

    This is the callback target invoked by the debug engine when the examine
    symbols command generates output.

Arguments:

    Output - Supplies a pointer to the active DEBUG_ENGINE_OUTPUT.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    PCHAR Buffer;
    USHORT SizeInBytes;
    PSTRING Chunk;
    PTRACE_STORE TraceStore;
    PTRACE_DEBUG_CONTEXT DebugContext;

    DebugContext = (PTRACE_DEBUG_CONTEXT)Output->Context;

    Chunk = &Output->Chunk;
    SizeInBytes = Chunk->Length;

    //
    // Allocate space from the TypeInfoStringBuffer store, copy the contents
    // over.
    //

    TraceStore = DebugContext->TraceStores.TypeInfoStringBuffer;

    Buffer = (PCHAR)(
        TraceStore->AllocateRecordsWithTimestamp(
            TraceStore->TraceContext,
            TraceStore,
            1,
            Chunk->Length,
            &Output->Timestamp.CommandStart
        )
    );

    if (!Buffer) {
        return FALSE;
    }

    if (!CopyMemoryQuadwords(Buffer, Chunk->Buffer, SizeInBytes)) {
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
TraceDebugEngineExamineSymbolsOutputCompleteCallback(
    PDEBUG_ENGINE_OUTPUT Output
    )
/*++

Routine Description:

    This is the callback target invoked by the debug engine when the examine
    symbols command generates output.

Arguments:

    Output - Supplies a pointer to the active DEBUG_ENGINE_OUTPUT.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL ContiguousMapping;
    PCHAR ExpectedEnd;
    PCHAR ActualEnd;
    PTRACE_STORE TraceStore;

    PTRACE_DEBUG_CONTEXT DebugContext;

    DebugContext = (PTRACE_DEBUG_CONTEXT)Output->Context;

    TraceStore = DebugContext->TraceStores.TypeInfoStringBuffer;

    ExpectedEnd = Output->Buffer + Output->TotalBufferSizeInBytes;
    ActualEnd = (PCHAR)TraceStore->MemoryMap->NextAddress;

    ContiguousMapping = (ExpectedEnd == ActualEnd);

    if (!ContiguousMapping) {
        //__debugbreak();
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
