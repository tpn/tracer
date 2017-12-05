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

//
// Define a 2MB initial heap size.
//

#define HEAP_INITIAL_SIZE (1 << 21)

#define CRITICAL_SECTION_SPIN_COUNT 4000

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
    HANDLE Events[17];

    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        return FALSE;
    }

    DebugContext = TraceContext->DebugContext;

#define BIND_COMPLETE_EVENT(Name)                 \
    TRACE_CONTEXT_STORE(Name)->BindCompleteEvent;

    Events[0]  = BIND_COMPLETE_EVENT(TypeInfoTable);
    Events[0]  = BIND_COMPLETE_EVENT(TypeInfoTable);
    Events[1]  = BIND_COMPLETE_EVENT(TypeInfoTableEntry);
    Events[2]  = BIND_COMPLETE_EVENT(TypeInfoStringBuffer);
    Events[3]  = BIND_COMPLETE_EVENT(FunctionTable);
    Events[4]  = BIND_COMPLETE_EVENT(FunctionTableEntry);
    Events[5]  = BIND_COMPLETE_EVENT(FunctionAssembly);
    Events[6]  = BIND_COMPLETE_EVENT(ExamineSymbolsLine);
    Events[7]  = BIND_COMPLETE_EVENT(ExamineSymbolsText);
    Events[8]  = BIND_COMPLETE_EVENT(ExaminedSymbol);
    Events[9]  = BIND_COMPLETE_EVENT(ExaminedSymbolSecondary);
    Events[10] = BIND_COMPLETE_EVENT(UnassembleFunctionLine);
    Events[11] = BIND_COMPLETE_EVENT(UnassembleFunctionText);
    Events[12] = BIND_COMPLETE_EVENT(UnassembledFunction);
    Events[13] = BIND_COMPLETE_EVENT(UnassembledFunctionSecondary);
    Events[13] = BIND_COMPLETE_EVENT(DisplayTypeLine);
    Events[14] = BIND_COMPLETE_EVENT(DisplayTypeText);
    Events[15] = BIND_COMPLETE_EVENT(DisplayedType);
    Events[16] = BIND_COMPLETE_EVENT(DisplayedTypeSecondary);

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
    PTRACE_DEBUG_CONTEXT DebugContext = NULL;
    PCRITICAL_SECTION CriticalSection;
    PALLOCATOR Allocator = NULL;

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
    // Initialize a non-serialized allocator; the debug engine allocates quite
    // a lot of temporary memory -- as we're only ever running in a single
    // threaded context, we can avoid the heap synchronization overhead.
    //

    Success = InitializeHeapAllocatorExInline(&DebugContext->Allocator,
                                              HEAP_NO_SERIALIZE,
                                              HEAP_INITIAL_SIZE,
                                              0);

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
    InitializeGuardedListHead(&DebugContext->WorkList);

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
    // Free the debug context if applicable.
    //

    if (DebugContext) {
        if (DebugContext->Allocator.HeapHandle) {
            DestroyHeapAllocatorInline(&DebugContext->Allocator);
        }
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
    PTRACE_STORE StringTableStore;
    PTRACE_STORE StringArrayStore;
    PTRACE_STORES TraceStores;
    PTRACE_CONTEXT TraceContext;
    PALLOCATOR StringTableAllocator;
    PALLOCATOR StringArrayAllocator;
    DEBUG_ENGINE_SESSION_INIT_FLAGS InitFlags = { 0 };
    PTRACER_CONFIG TracerConfig;
    PDEBUG_ENGINE_SESSION DebugEngineSession;

    TraceContext = DebugContext->TraceContext;
    TraceStores = TraceContext->TraceStores;
    Rtl = TraceContext->Rtl;

    InitFlags.InitializeFromCurrentProcess = TRUE;
    TracerConfig = TraceContext->TracerConfig;

    StringTableStore = TraceStoreIdToTraceStore(TraceStores,
                                                TraceStoreStringTableId);

    StringArrayStore = TraceStoreIdToTraceStore(TraceStores,
                                                TraceStoreStringArrayId);

    StringTableAllocator = &StringTableStore->Allocator;
    StringArrayAllocator = &StringArrayStore->Allocator;

    Success = LoadAndInitializeDebugEngineSession(
        &TracerConfig->Paths.DebugEngineDllPath,
        Rtl,
        TraceContext->Allocator,
        InitFlags,
        TracerConfig,
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

    DebugEngineSession = DebugContext->DebugEngineSession;

    Success = DebugEngineSession->ListSettings(DebugEngineSession);
    if (!Success) {
        OutputDebugStringA("DebugEngineSession->ListSettings() failed.\n");
        return FALSE;
    }

    if (0) {
        UNICODE_STRING Command = RTL_CONSTANT_STRING(
            L".reload /d /f /i /v"
        );
        OutputDebugStringW(L"***** Executing: ");
        OutputDebugStringW(Command.Buffer);
        //__debugbreak();
        Success = DebugEngineSession->ExecuteStaticCommand(
            DebugEngineSession,
            &Command,
            NULL
        );
        if (!Success) {
            __debugbreak();
        }
    }

    OutputDebugStringA("Debug Engine successfully initialized.\n");

    TRY_MAPPED_MEMORY_OP {
        ExitCode = DebugContext->ThreadEntry(DebugContext);
    } CATCH_STATUS_IN_PAGE_ERROR {
        __debugbreak();
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

    This routine is the implementation back-end of the debug engine worker
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
                                            1000);

        AcquireTraceDebugContextLock(DebugContext);

        if (WaitResult == WAIT_OBJECT_0) {

            //
            // Shutdown event.
            //

            DebugContext->State = TraceDebugContextReceivedShutdownState;
            Result = 0;
            break;

        } else if (WaitResult == WAIT_OBJECT_0+1 ||
                   WaitResult == WAIT_TIMEOUT) {

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

    This routine processes work entries.

Arguments:

    DebugContext - Supplies a pointer to a TRACE_DEBUG_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;

    while (TRUE) {

        Success = RemoveHeadModuleTableEntryFromDebugContext(
            DebugContext,
            &ModuleTableEntry
        );

        if (!Success) {
            Success = TRUE;
            break;
        }

        if (1) {
            OutputDebugStringA("ProcessTraceDebugEngineWork: ");
            PrintUnicodeStringToDebugStream(
                &ModuleTableEntry->File.Path.Full
            );
        }

        Success = CreateTypeInfoTableForModuleTableEntry(
            DebugContext,
            ModuleTableEntry
        );

        if (!Success) {
            DebugContext->NumberOfWorkItemsFailed++;
        } else {
            DebugContext->NumberOfWorkItemsSucceeded++;
        }

        DebugContext->NumberOfWorkItemsProcessed++;
    }

    return Success;
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
    BOOL Skip;
    BOOL Success;
    PRTL Rtl;
    PRTL_FILE File;
    PRTL_PATH Path;
    ULONG DisplayTypeFailed;
    ULONG UnassembleFunctionFailed;
    ULONG ConvertStringToWideFailed;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEntry;
    PALLOCATOR Allocator;
    TRACE_FLAGS TraceFlags;
    PTRACE_STORES TraceStores;
    PTRACE_CONTEXT TraceContext;
    PTRACER_CONFIG TracerConfig;
    PDEBUG_ENGINE_OUTPUT Output;
    DEBUG_ENGINE_OUTPUT DisplayTypeOutput;
    DEBUG_ENGINE_OUTPUT ExamineSymbolsOutput;
    DEBUG_ENGINE_OUTPUT UnassembleFunctionOutput;
    DEBUG_ENGINE_OUTPUT_FLAGS OutputFlags;
    PDEBUG_ENGINE_EXAMINED_SYMBOL Symbol;
    PDEBUG_ENGINE_SESSION DebugEngineSession;
    PDEBUG_ENGINE_DISPLAY_TYPE DisplayType;
    PDEBUG_ENGINE_EXAMINE_SYMBOLS ExamineSymbols;
    PDEBUG_ENGINE_UNASSEMBLE_FUNCTION UnassembleFunction;
    PINITIALIZE_DEBUG_ENGINE_OUTPUT InitializeDebugEngineOutput;
    DEBUG_ENGINE_DISPLAY_TYPE_COMMAND_OPTIONS DisplayTypeOptions;
    DEBUG_ENGINE_EXAMINE_SYMBOLS_COMMAND_OPTIONS ExamineSymbolsOptions;
    DEBUG_ENGINE_UNASSEMBLE_FUNCTION_COMMAND_OPTIONS UnassembleFunctionOptions;
    CONST STRING CSpecificHandler = RTL_CONSTANT_STRING("__C_specific_handler");

    UNICODE_STRING ArgumentWide;
    WCHAR UnicodeBuffer[_MAX_PATH];
    PUNICODE_STRING ArgumentWidePointer = NULL;

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
    Allocator = &DebugContext->Allocator;
    TracerConfig = TraceContext->TracerConfig;
    DebugEngineSession = DebugContext->DebugEngineSession;

    //
    // Initialize function pointers.
    //

    DisplayType = DebugEngineSession->DisplayType;
    ExamineSymbols = DebugEngineSession->ExamineSymbols;
    UnassembleFunction = DebugEngineSession->UnassembleFunction;
    InitializeDebugEngineOutput = (
        DebugEngineSession->InitializeDebugEngineOutput
    );

    //
    // Define a helper macro to use for initializing the DEBUG_ENGINE_OUTPUT
    // structure.
    //

#define INITIALIZE_OUTPUT(Command, Name)                    \
                                                            \
    Output = &##Command##Output;                            \
    Output->SizeOfStruct = sizeof(*Output);                 \
                                                            \
    Success = InitializeDebugEngineOutput(                  \
        Output,                                             \
        DebugEngineSession,                                 \
        Allocator,                                          \
        &TRACE_CONTEXT_STORE(##Command##Line)->Allocator,   \
        &TRACE_CONTEXT_STORE(##Command##Text)->Allocator,   \
        &TRACE_CONTEXT_STORE(##Name##)->Allocator,          \
        &TRACE_CONTEXT_STORE(##Name##Secondary)->Allocator, \
        NULL,                                               \
        NULL,                                               \
        NULL,                                               \
        DebugContext,                                       \
        Path                                                \
    );                                                      \
                                                            \
    if (!Success) {                                         \
        __debugbreak();                                     \
        return FALSE;                                       \
    }

#define INITIALIZE_EXAMINE_SYMBOLS_OUTPUT()           \
    INITIALIZE_OUTPUT(ExamineSymbols, ExaminedSymbol)

#define INITIALIZE_UNASSEMBLED_FUNCTION_OUTPUT()               \
    INITIALIZE_OUTPUT(UnassembleFunction, UnassembledFunction)

#define INITIALIZE_DISPLAY_TYPE_OUTPUT()          \
    INITIALIZE_OUTPUT(DisplayType, DisplayedType)

    //
    // Initialize the DEBUG_ENGINE_EXAMINE_SYMBOLS_OUTPUT structure.  This is
    // a stack allocated structure that will persist for the lifetime of this
    // routine and is used to communicate partial output state across multiple
    // callbacks.
    //

    INITIALIZE_EXAMINE_SYMBOLS_OUTPUT();

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

    OutputFlags.AsULong = 0;
    OutputFlags.EnableLineTextAndCustomStructureAllocators = TRUE;

    ExamineSymbolsOptions.AsULong = 0;
    ExamineSymbolsOptions.Verbose = 1;
    ExamineSymbolsOptions.TypeInformation = 1;

    Success = ExamineSymbols(Output, OutputFlags, ExamineSymbolsOptions);

    if (!Success) {
        return FALSE;
    }

    //
    // Initialize the argument name unicode buffer.
    //

    ArgumentWide.Length = 0;
    ArgumentWide.MaximumLength = sizeof(UnicodeBuffer);
    ArgumentWide.Buffer = (PWSTR)&UnicodeBuffer;

    //
    // Define a helper macro for initializing the argument name relative to the
    // DEBUG_ENGINE_EXAMINED_SYMBOL structure.
    //

#define INITIALIZE_ARGUMENT(Name)                            \
                                                             \
    Success = ConvertStringToWide(&Symbol->Strings.##Name##, \
                                  &ArgumentWide,             \
                                  Allocator,                 \
                                  &ArgumentWidePointer);     \
                                                             \
    if (!Success) {                                          \
        __debugbreak();                                      \
        ConvertStringToWideFailed++;                         \
        break;                                               \
    }

    //
    // Initialize options for unassemble function and display type commands.
    //

    UnassembleFunctionOptions.AsULong = 0;
    UnassembleFunctionOptions.DisplayInstructionCount = TRUE;
    //UnassembleFunctionOptions.RelaxBlockingRequirements = TRUE;

    DisplayTypeOptions.AsULong = 0;

    DisplayTypeOptions.Verbose = TRUE;
    //DisplayTypeOptions.ShowArrayElements = TRUE;
    //DisplayTypeOptions.RecursivelyDumpSubtypes = TRUE;
    //DisplayTypeOptions.DisplayBlocksRecursively = TRUE;

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
    // Initialize counters that track failed actions.  These are helpful during
    // debugging.
    //

    DisplayTypeFailed = 0;
    UnassembleFunctionFailed = 0;
    ConvertStringToWideFailed = 0;

    //
    // Clear the ArgumentWidePointer.  It should be NULL at the top of every
    // loop.
    //

    ArgumentWidePointer = NULL;

    //
    // Initialize the list head then walk the list of examined symbols and
    // potentially perform addition processing for each symbol depending on
    // its type.
    //

    ListHead = &Output->CustomStructureListHead;

    FOR_EACH_LIST_ENTRY(ListHead, ListEntry) {

        if (ArgumentWidePointer) {
            __debugbreak();
        }

        Symbol = CONTAINING_RECORD(ListEntry,
                                   DEBUG_ENGINE_EXAMINED_SYMBOL,
                                   ListEntry);

        switch (Symbol->Type) {

            case NoType:
            case FunctionType:

                //
                // Skip function pointers and __C_specific_handler functions.
                // The unassembler doesn't appear to grok how we use the latter,
                // in that it ends up interpreting huge chunks of zero bytes
                // that usually follow the symbol in memory as part of the
                // function; so you get many useless lines like this:
                //
                // 00007fff`5560a56c 0000            add     byte ptr [rax],al
                // 00007fff`5560a56e 0000            add     byte ptr [rax],al
                //

                Skip = (
                    Symbol->Flags.IsPointer ||
                    Rtl->RtlEqualString(&Symbol->Strings.SymbolName,
                                        &CSpecificHandler,
                                        FALSE)
                );

                if (Skip) {
                    continue;
                }

                INITIALIZE_UNASSEMBLED_FUNCTION_OUTPUT();
                INITIALIZE_ARGUMENT(Function);

                Success = UnassembleFunction(Output,
                                             OutputFlags,
                                             UnassembleFunctionOptions,
                                             ArgumentWidePointer);

                if (!Success) {

                    UnassembleFunctionFailed++;

                } else {

                    //
                    // Todo: Register disassembly.  Parse arguments, identify
                    // those that haven't been seen yet and call DisplayType().
                    //

                    NOTHING;

                }

                break;

            case InlineCallerType:

                //
                // Todo: capture all inline call sites for an inline function,
                // plus a single disassembly.
                //

                break;

            case ClassType:
            case UnionType:
            case StructType:

                //
                // Todo: don't call DisplayType() on types we've already seen.
                // (Keep a prefix tree of types?)
                //

                INITIALIZE_DISPLAY_TYPE_OUTPUT();
                INITIALIZE_ARGUMENT(TypeName);

                Success = DisplayType(Output,
                                      OutputFlags,
                                      DisplayTypeOptions,
                                      ArgumentWidePointer);

                if (!Success) {

                    DisplayTypeFailed++;

                } else {

                    //
                    // Todo: Register type name.
                    //

                    NOTHING;

                }

                break;

            default:
                continue;
        }

        //
        // If ArgumentWidePointer is non-NULL, and it has a different address
        // to the stack-allocated ArgumentWide buffer, it means that the
        // ConvertStringToWide() call above (within INITIALIZE_ARGUMENT())
        // had to allocate a new string buffer via the provided allocator.
        // So, free it now.
        //

        if (ArgumentWidePointer && ArgumentWidePointer != &ArgumentWide) {
            Allocator->Free(Allocator->Context, ArgumentWidePointer);
        }

        ArgumentWidePointer = NULL;
    }

    //
    // Invariant check: ArgumentWidePointer should be clear at this point.
    //

    if (ArgumentWidePointer) {
        __debugbreak();
    }

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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
