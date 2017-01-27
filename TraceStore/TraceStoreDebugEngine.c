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
    HANDLE Events[6];

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

    NumberOfWaits = sizeof(Events) / sizeof(Events[0]);

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
    ULONG ExitCode;
    PTRACE_CONTEXT TraceContext;

    TraceContext = DebugContext->TraceContext;
    Rtl = TraceContext->Rtl;

    BOOL Success;
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags = { 0 };
    PTRACER_CONFIG TracerConfig;

    InitFlags.InitializeFromCurrentProcess = TRUE;
    TracerConfig = TraceContext->TracerConfig;

    Success = LoadAndInitializeDebugEngineSession(
        &TracerConfig->Paths.DebugEngineDllPath,
        Rtl,
        TraceContext->Allocator,
        InitFlags,
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
    PDEBUG_ENGINE_SESSION DebugEngineSession;
    PDEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK Callback;
    DEBUG_ENGINE_ENUM_SYMBOLS_FLAGS EnumFlags;

    //
    // Capture a timestamp for processing this module table entry.
    //

    QueryPerformanceCounter(&DebugContext->CurrentTimestamp);

    //
    // Initialize aliases.
    //

    TraceContext = DebugContext->TraceContext;
    TraceStores = TraceContext->TraceStores;
    Rtl = TraceContext->Rtl;
    File = &ModuleTableEntry->File;
    Path = &File->Path;
    Allocator = TraceContext->Allocator;
    DebugEngineSession = DebugContext->DebugEngineSession;

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

    Callback = (PDEBUG_ENGINE_ENUM_SYMBOLS_CALLBACK)(
        TraceDebugEngineSymbolCallback
    );
    EnumFlags.Verbose = 1;
    EnumFlags.TypeInformation = 1;
    Success = DebugEngineSession->EnumSymbols(DebugEngineSession,
                                              DebugContext,
                                              Allocator,
                                              Callback,
                                              EnumFlags,
                                              Path);
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
TraceDebugEngineSymbolCallback(
    PDEBUG_ENGINE_SYMBOL Symbol,
    PTRACE_DEBUG_CONTEXT DebugContext
    )
/*++

Routine Description:

    This is the callback target invoked by the debug engine when enumerating
    symbols.

Arguments:

    Symbol - Supplies a pointer to a DEBUG_ENGINE_SYMBOL structure.

    DebugContext - Supplies a pointer to our active DEBUG_ENGINE_CONTEXT.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
