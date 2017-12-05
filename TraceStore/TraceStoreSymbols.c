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

    Rtl->RtlInitializeGenericTableAvl(&SymbolTable->SymbolHashAvlTable,
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
SymbolTypeStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the SymbolType trace store.

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
    PTRACE_SYMBOL_CONTEXT SymbolContext;
    HANDLE Events[8];

    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        return FALSE;
    }

    SymbolContext = TraceContext->SymbolContext;

    Events[0] = SYMBOL_CONTEXT_STORE(SymbolTable)->BindCompleteEvent;
    Events[1] = SYMBOL_CONTEXT_STORE(SymbolTableEntry)->BindCompleteEvent;
    Events[2] = SYMBOL_CONTEXT_STORE(SymbolModuleInfo)->BindCompleteEvent;
    Events[3] = SYMBOL_CONTEXT_STORE(SymbolFile)->BindCompleteEvent;
    Events[4] = SYMBOL_CONTEXT_STORE(SymbolInfo)->BindCompleteEvent;
    Events[5] = SYMBOL_CONTEXT_STORE(SymbolLine)->BindCompleteEvent;
    Events[6] = SYMBOL_CONTEXT_STORE(SymbolType)->BindCompleteEvent;
    Events[7] = SYMBOL_CONTEXT_STORE(StackFrame)->BindCompleteEvent;

    NumberOfWaits = sizeof(Events) / sizeof(Events[0]);

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
    PTRACE_STORES TraceStores;
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
    InitializeGuardedListHead(&SymbolContext->WorkList);

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
    It performs initial DbgHelp symbol initialization via Rtl and then calls
    the SymbolContext->ThreadEntry function within a try/catch block that
    suppresses STATUS_IN_PAGE_ERROR exceptions.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE otherwise.


--*/
{
    PRTL Rtl;
    ULONG Options;
    ULONG ExitCode;
    PTRACE_CONTEXT TraceContext;

    TraceContext = SymbolContext->TraceContext;
    Rtl = TraceContext->Rtl;

    Options = (
        SYMOPT_DEBUG                |
        SYMOPT_UNDNAME              |
        SYMOPT_NO_PROMPTS           |
        SYMOPT_LOAD_LINES           |
        SYMOPT_DEFERRED_LOADS       |
        SYMOPT_FAIL_CRITICAL_ERRORS
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
        ExitCode = SymbolContext->ThreadEntry(SymbolContext);
    } CATCH_STATUS_IN_PAGE_ERROR {
        ExitCode = 1;
    }

    return ExitCode;
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
                                            1000);

        AcquireTraceSymbolContextLock(SymbolContext);

        if (WaitResult == WAIT_OBJECT_0) {

            //
            // Shutdown event.
            //

            SymbolContext->State = TraceSymbolContextReceivedShutdownState;
            Result = 0;
            break;

        } else if (WaitResult == WAIT_OBJECT_0+1 ||
                   WaitResult == WAIT_TIMEOUT) {

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

    This routine processes work entries.

Arguments:

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    BOOL Success;
    PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry;

    while (TRUE) {

        Success = RemoveHeadModuleTableEntryFromSymbolContext(
            SymbolContext,
            &ModuleTableEntry
        );

        if (!Success) {
            Success = TRUE;
            break;
        }

        Success = CreateSymbolTableForModuleTableEntry(
            SymbolContext,
            ModuleTableEntry
        );

        if (1) {
            OutputDebugStringA("ProcessTraceSymbolWork: ");
            PrintUnicodeStringToDebugStream(
                &ModuleTableEntry->File.Path.Full
            );
        }

        if (!Success) {
            SymbolContext->NumberOfWorkItemsFailed++;
        } else {
            SymbolContext->NumberOfWorkItemsSucceeded++;
        }

        SymbolContext->NumberOfWorkItemsProcessed++;
    }

    return Success;
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
    PRTL Rtl;
    PDBG Dbg;
    BOOL Success;
    ULONG LastError;
    DWORD64 Address;
    HMODULE ModuleHandle;
    PRTL_PATH Path;
    PRTL_FILE File;
    PRTL_IMAGE_FILE ImageFile;
    PTRACE_STORES TraceStores;
    PTRACE_CONTEXT TraceContext;
    IMAGEHLP_MODULEW64 ModuleInfo;
    PSYM_ENUMERATESYMBOLS_CALLBACK EnumTypesCallback;
    PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback;
    PSYM_ENUMSOURCEFILES_CALLBACKW EnumSourceFilesCallback;
    PSYM_ENUMLINES_CALLBACKW EnumLinesCallback;

    //
    // Capture a timestamp for processing this module table entry.
    //

    QueryPerformanceCounter(&SymbolContext->CurrentTimestamp);

    //
    // Initialize aliases.
    //

    TraceContext = SymbolContext->TraceContext;
    TraceStores = TraceContext->TraceStores;
    Rtl = TraceContext->Rtl;
    Dbg = &Rtl->Dbg;
    File = &ModuleTableEntry->File;
    Path = &File->Path;
    ImageFile = &File->ImageFile;
    ModuleHandle = ImageFile->ModuleHandle;

    if (!AssertTrue("ModuleHandle != NULL", ModuleHandle != NULL)) {
        goto Error;
    }

    if (1) {
        OutputDebugStringA("CreateSymbolTableForModuleTableEntry: ");
        PrintUnicodeStringToDebugStream(&Path->Full);
    }

    //
    // Update the symbol context to point at this module table entry.
    //

    SymbolContext->CurrentModuleTableEntry = ModuleTableEntry;

    //
    // Attempt to load the timestamp.
    //

    ImageFile->Timestamp = Dbg->GetTimestampForLoadedLibrary(ModuleHandle);

    //
    // Load the module.
    //

    Address = Dbg->SymLoadModuleExW(SymbolContext->ThreadHandle,
                                    ModuleHandle,
                                    Path->Full.Buffer,
                                    NULL,
                                    (DWORD64)ImageFile->BaseAddress,
                                    ImageFile->SizeOfImage,
                                    NULL,
                                    0);

    if (!Address) {
        LastError = GetLastError();
        if (LastError != ERROR_SUCCESS) {
            SymbolContext->LastError = LastError;
            __debugbreak();
            goto Error;
        }
    }

    //
    // Enumerate types.
    //

    EnumTypesCallback = (PSYM_ENUMERATESYMBOLS_CALLBACK)(
        SymbolContextEnumTypesCallback
    );

    Success = Dbg->SymEnumTypes(SymbolContext->ThreadHandle,
                                (ULONG64)ImageFile->BaseAddress,
                                EnumTypesCallback,
                                SymbolContext);

    if (!Success) {
        SymbolContext->LastError = GetLastError();
        OutputDebugStringA("SymEnumTypes() failed");
        OutputDebugStringW(File->Path.Full.Buffer);
        //__debugbreak();
        //goto Error;
    }

    //
    // Enumerate symbols.
    //

    EnumSymbolsCallback = (PSYM_ENUMERATESYMBOLS_CALLBACK)(
        SymbolContextEnumSymbolsCallback
    );

    Success = Dbg->SymEnumSymbolsEx(SymbolContext->ThreadHandle,
                                    (ULONG64)ImageFile->BaseAddress,
                                    NULL,
                                    EnumSymbolsCallback,
                                    SymbolContext,
                                    SYMENUM_OPTIONS_DEFAULT);

    if (!Success) {
        SymbolContext->LastError = GetLastError();
        OutputDebugStringA("SymEnumSymbolsEx() failed");
        OutputDebugStringW(File->Path.Full.Buffer);
        //__debugbreak();
        //goto Error;
    }

    //
    // Enumerate source files.
    //

    EnumSourceFilesCallback = (PSYM_ENUMSOURCEFILES_CALLBACKW)(
        SymbolContextEnumSourceFilesCallback
    );

    Success = Dbg->SymEnumSourceFilesW(SymbolContext->ThreadHandle,
                                       (ULONG64)ImageFile->BaseAddress,
                                       NULL,
                                       EnumSourceFilesCallback,
                                       SymbolContext);

    if (!Success) {
        SymbolContext->LastError = GetLastError();
        OutputDebugStringA("SymEnumSourceFilesW() failed");
        OutputDebugStringW(File->Path.Full.Buffer);
        //__debugbreak();
        //goto Error;
    }

    //
    // Enumerate source lines.
    //

    EnumLinesCallback = (PSYM_ENUMLINES_CALLBACKW)(
        SymbolContextEnumLinesCallback
    );

    Success = Dbg->SymEnumLinesW(SymbolContext->ThreadHandle,
                                 (ULONG64)ImageFile->BaseAddress,
                                 NULL,   // Object
                                 NULL,   // File
                                 EnumLinesCallback,
                                 SymbolContext);

    if (!Success) {
        SymbolContext->LastError = GetLastError();
        OutputDebugStringA("SymEnumLinesW() failed");
        OutputDebugStringW(File->Path.Full.Buffer);
        //__debugbreak();
        //goto Error;
    }

    //
    // Get the module info.
    //

    ModuleInfo.SizeOfStruct = sizeof(ModuleInfo);
    Success = Dbg->SymGetModuleInfoW64(SymbolContext->ThreadHandle,
                                       (DWORD64)ImageFile->EntryPoint,
                                       &ModuleInfo);

    if (!Success) {
        LastError = GetLastError();
        if (LastError != ERROR_MOD_NOT_FOUND) {
            __debugbreak();
            SymbolContext->LastError = LastError;
            goto Error;
        }
    }

    //
    // Copy the module info.
    //

    Success = AllocateAndCopySymbolModuleInfo(SymbolContext, &ModuleInfo);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Intentional follow-on to End.
    //

End:

    if (ImageFile->ModuleHandle) {
        FreeLibrary(ImageFile->ModuleHandle);
        ImageFile->ModuleHandle = NULL;
    }

    if (File->MappedAddress) {
        UnmapViewOfFile(File->MappedAddress);
        File->MappedAddress = NULL;
    }

    if (File->MappingHandle) {
        CloseHandle(File->MappingHandle);
        File->MappingHandle = NULL;
    }

    if (File->FileHandle) {
        CloseHandle(File->FileHandle);
        File->FileHandle = NULL;
    }


    return Success;
}

FORCEINLINE
ULONG
CalculateSymbolInfoAllocSize(
    _In_ PSYMBOL_INFO SymbolInfo
    )
{
    ULONG Size;
    const ULONG Padding = sizeof(*SymbolInfo) - FIELD_OFFSET(SYMBOL_INFO, Name);

    //
    // The +1 accounts for the trailing NULL.
    //

    Size = SymbolInfo->SizeOfStruct + SymbolInfo->NameLen - Padding + 1;

    return Size;
}

PSYMBOL_INFO
AllocateAndCopySymbolInfo(
    _In_ PTRACE_SYMBOL_CONTEXT SymbolContext,
    _In_ PSYMBOL_INFO SourceSymbolInfo
    )
{
    SIZE_T NameLength;
    LONG_INTEGER SymbolSize;
    LONG_INTEGER AllocSize;
    PSYMBOL_INFO SymbolInfo;
    PTRACE_STORE TraceStore;

    if (SourceSymbolInfo->NameLen > 0) {
        NameLength = strlen(SourceSymbolInfo->Name);
        if (NameLength != SourceSymbolInfo->NameLen) {
            __debugbreak();
        }
        if (SourceSymbolInfo->Name[SourceSymbolInfo->NameLen] != '\0') {
            __debugbreak();
        }
    }

    SymbolSize.LongPart = CalculateSymbolInfoAllocSize(SourceSymbolInfo);
    AllocSize.LongPart = ALIGN_UP_POINTER(SymbolSize.LongPart);

    if (AllocSize.HighPart) {
        __debugbreak();
    }

    if (SymbolSize.LongPart != AllocSize.LongPart) {
        //__debugbreak();
        NOTHING;
    }

    TraceStore = SYMBOL_CONTEXT_STORE(SymbolInfo);

    TRY_MAPPED_MEMORY_OP {

        //
        // Allocate space.
        //

        SymbolInfo = (PSYMBOL_INFO)(
            TraceStore->AllocateRecordsWithTimestamp(
                TraceStore->TraceContext,
                TraceStore,
                1,
                AllocSize.LongPart,
                &SymbolContext->CurrentTimestamp
            )
        );

        if (!SymbolInfo) {
            return NULL;
        }

        __stosb((PBYTE)SymbolInfo, 0xcc, AllocSize.LongPart);

        //
        // Copy the contents.
        //

        CopyMemory(SymbolInfo, SourceSymbolInfo, SymbolSize.LongPart);

        if (SymbolInfo->NameLen > 0) {
            SymbolInfo->MaxNameLen = SymbolInfo->NameLen + 1;
            if (SymbolInfo->Name[SymbolInfo->NameLen] != '\0') {
                __debugbreak();
            }
        }

    } CATCH_STATUS_IN_PAGE_ERROR {
        return NULL;
    }

    return SymbolInfo;
}


_Use_decl_annotations_
BOOL
SymbolContextEnumTypesCallback(
    PSYMBOL_INFO pSymbolInfo,
    ULONG SymbolSize,
    PTRACE_SYMBOL_CONTEXT SymbolContext
    )
/*++

Routine Description:

    This routine is the symbol enumeration callback.

Arguments:

    SourceSymbolInfo - Supplies a pointer to a SYMBOL_INFO structure.

    SymbolSize - Supplies the size of the symbol.

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    PSYMBOL_INFO SymbolInfo;

    //OutputDebugStringA("EnumTypesCallback");
    //OutputDebugStringA(pSymbolInfo->Name);

    if (pSymbolInfo->SizeOfStruct == 0) {
        __debugbreak();
    }

    SymbolInfo = AllocateAndCopySymbolInfo(SymbolContext, pSymbolInfo);
    if (!SymbolInfo) {
        __debugbreak();
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
SymbolContextEnumSymbolsCallback(
    PSYMBOL_INFO pSymbolInfo,
    ULONG SymbolSize,
    PTRACE_SYMBOL_CONTEXT SymbolContext
    )
/*++

Routine Description:

    This routine is the symbol enumeration callback.

Arguments:

    pSymbolInfo - Supplies a pointer to a SYMBOL_INFO structure.

    SymbolSize - Supplies the size of the symbol.

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    PSYMBOL_INFO SymbolInfo;

    //OutputDebugStringA("EnumSymbolsCallback");
    //OutputDebugStringA((PSTR)pSymbolInfo->Name);

    if (pSymbolInfo->SizeOfStruct == 0) {
        __debugbreak();
    }

    SymbolInfo = AllocateAndCopySymbolInfo(SymbolContext, pSymbolInfo);
    if (!SymbolInfo) {
        __debugbreak();
        return FALSE;
    }

    if (SymbolInfo->SizeOfStruct == 0) {
        __debugbreak();
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
SymbolContextEnumSourceFilesCallback(
    PSOURCEFILEW pSourceFileW,
    PTRACE_SYMBOL_CONTEXT SymbolContext
    )
/*++

Routine Description:

    This routine is the source files enumeration callback.

Arguments:

    pSourceFile - Supplies a pointer to a SOURCEFILEW structure.

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    //OutputDebugStringA("EnumSourceFilesCallback");
    //OutputDebugStringW((PWSTR)pSourceFileW->FileName);
    return TRUE;
}

_Use_decl_annotations_
BOOL
SymbolContextEnumLinesCallback(
    PSRCCODEINFOW SourceCodeInfoPointer,
    PTRACE_SYMBOL_CONTEXT SymbolContext
    )
/*++

Routine Description:

    This routine is the source files enumeration callback.

Arguments:

    SourceCodeInfoPointer - Supplies a pointer to a SRCCODEINFOW structure.

    SymbolContext - Supplies a pointer to a TRACE_SYMBOL_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on error.

--*/
{
    PSRCCODEINFOW SourceCodeInfo;

    SourceCodeInfo = SourceCodeInfoPointer;

    //OutputDebugStringA("EnumLinesCallback");
    //OutputDebugStringW(SourceCodeInfo->Obj);
    //OutputDebugStringW(SourceCodeInfo->FileName);

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
