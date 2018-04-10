/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracer.c

Abstract:

    This module implements functionality related to tracing an instance of a
    Python interpreter using the trace store functionality.  It relies on the
    Python and TraceStore components.

--*/

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL
InitializePythonTraceSession(
    _In_ PUNICODE_STRING BaseDirectory
    )
{
    return FALSE;
}

PVOID
TraceStoreAllocationRoutine(
    _In_ PVOID AllocationContext,
    _In_ const ULONG ByteSize
    )
{
    PTRACE_STORE TraceStore = (PTRACE_STORE)AllocationContext;
    ULONG_PTR NumberOfRecords = 1;
    ULONG_PTR RecordSize = ByteSize;

    return TraceStore->AllocateRecordsWithTimestamp(
        TraceStore->TraceContext,
        TraceStore,
        NumberOfRecords,
        RecordSize,
        NULL
    );
}

PVOID
TraceStoreNullAllocationRoutine(
    _In_ PVOID AllocationContext,
    _In_ const ULONG ByteSize
    )
{
    __debugbreak();
    return NULL;
}

PVOID
TraceStoreCallocRoutine(
    _In_ PVOID AllocationContext,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize
    )
{
    PTRACE_STORE TraceStore = (PTRACE_STORE)AllocationContext;

    return TraceStore->AllocateRecordsWithTimestamp(
        TraceStore->TraceContext,
        TraceStore,
        NumberOfElements,
        ElementSize,
        NULL
    );
}

VOID
TraceStoreFreeRoutine(
    _In_opt_ PVOID FreeContext,
    _In_     PVOID Buffer
    )
{
    __debugbreak();
    return;
}

VOID
TraceStoreNullFreeRoutine(
    _In_opt_ PVOID FreeContext,
    _In_     PVOID Buffer
    )
{
    __debugbreak();
    return;
}

BOOL
IsFunctionOfInterestPrefixTree(
    _In_    PRTL                    Rtl,
    _In_    PPYTHON_TRACE_CONTEXT   Context,
    _In_    PPYTHON_FUNCTION        Function
    )
{
    PSTRING ModuleName;
    PPREFIX_TABLE Table;
    PPREFIX_TABLE_ENTRY Entry;

    if (Context->Flags.TraceEverything) {
        return TRUE;
    }

    if (Context->Flags.TraceNothing) {
        return FALSE;
    }

    if (!Context->RuntimeState.HasModuleFilter) {
        return Context->Flags.TraceEverythingWhenNoModuleFilterSet;
    }

    ModuleName = &Function->PathEntry.ModuleName;

    Table = &Context->ModuleFilterPrefixTree;

    Entry = Rtl->PfxFindPrefix(Table, ModuleName);

    return (Entry ? TRUE : FALSE);
}

#define IsFunctionOfInterest IsFunctionOfInterestStringTable

_Use_decl_annotations_
VOID
EnableMemoryTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->Flags.TraceMemory = TRUE;
}

_Use_decl_annotations_
VOID
DisableMemoryTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->Flags.TraceMemory = FALSE;
}

_Use_decl_annotations_
VOID
EnableIoCountersTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->Flags.TraceIoCounters = TRUE;
}

_Use_decl_annotations_
VOID
DisableIoCountersTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->Flags.TraceIoCounters = FALSE;
}

_Use_decl_annotations_
VOID
EnableHandleCountTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->Flags.TraceHandleCount = TRUE;
}

_Use_decl_annotations_
VOID
DisableHandleCountTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->Flags.TraceHandleCount = FALSE;
}

_Use_decl_annotations_
LONG
PyTraceCallback(
    PVOID           UserContext,
    PPYFRAMEOBJECT  FrameObject,
    LONG            EventType,
    PPYOBJECT       ArgObject
    )
{
    BOOL Success;
    BOOL InPageError;
    PPYTHON_TRACE_CONTEXT Context;

    Context = (PPYTHON_TRACE_CONTEXT)UserContext;
    InPageError = FALSE;

    TRY_MAPPED_MEMORY_OP {

        Success = Context->CallbackWorker(Context,
                                          FrameObject,
                                          EventType,
                                          ArgObject);

    } CATCH_STATUS_IN_PAGE_ERROR {
        InPageError = TRUE;
    }

    if (!Success) {
        __debugbreak();
    }

    if (InPageError) {
        OutputDebugStringA("PythonTracer: STATUS_IN_PAGE_ERROR, disabling.\n");
        Success = FALSE;
    }

    if (!Success) {
        Stop(Context);
    }

    return 0;
}

PVOID
NTAPI
CodeObjectAllocateFromStore(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ CLONG ByteSize
    )
{
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    if (!Table) {
        return NULL;
    }

    if (ByteSize <= 0) {
        return NULL;
    }

    PythonTraceContext = (PPYTHON_TRACE_CONTEXT)Table->TableContext;

    return NULL;
}

PVOID
NTAPI
CodeObjectAllocateFromHeap(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ CLONG ByteSize
    )
{
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    HANDLE HeapHandle;
    if (!Table) {
        return NULL;
    }

    if (ByteSize <= 0) {
        return NULL;
    }

    PythonTraceContext = (PPYTHON_TRACE_CONTEXT)Table->TableContext;

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        return NULL;
    }

    return HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, ByteSize);
}

VOID
NTAPI
CodeObjectFreeFromHeap(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID Buffer
    )
{
    PPYTHON_TRACE_CONTEXT PythonTraceContext;
    HANDLE HeapHandle;

    if (!Table) {
        return;
    }

    if (Buffer == NULL) {
        return;
    }

    PythonTraceContext = (PPYTHON_TRACE_CONTEXT)Table->TableContext;

    HeapHandle = GetProcessHeap();
    if (!HeapHandle) {
        return;
    }

    HeapFree(HeapHandle, 0, Buffer);
}

FORCEINLINE
RTL_GENERIC_COMPARE_RESULTS
GenericComparePointer(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    PULONG_PTR First = (PULONG_PTR)FirstStruct;
    PULONG_PTR Second = (PULONG_PTR)SecondStruct;

    if (First < Second) {
        return GenericLessThan;
    }
    else if (First > Second) {
        return GenericGreaterThan;
    }
    else {
        return GenericEqual;
    }
}

FORCEINLINE
RTL_GENERIC_COMPARE_RESULTS
GenericComparePyObjectHash(
    _In_ PPYTHON Python,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    PPYOBJECT First = (PPYOBJECT)FirstStruct;
    PPYOBJECT Second = (PPYOBJECT)SecondStruct;

    LONG FirstHash = Python->PyObject_Hash(First);
    LONG SecondHash = Python->PyObject_Hash(Second);

    if (First < Second) {
        return GenericLessThan;
    }
    else if (First > Second) {
        return GenericGreaterThan;
    }
    else {
        return GenericEqual;
    }
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
CodeObjectCompare(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    return GenericComparePointer(Table, FirstStruct, SecondStruct);
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
FunctionCompare(
    _In_ PRTL_GENERIC_TABLE Table,
    _In_ PVOID FirstStruct,
    _In_ PVOID SecondStruct
    )
{
    PPYTHON_FUNCTION First = (PPYTHON_FUNCTION)FirstStruct;
    PPYTHON_FUNCTION Second = (PPYTHON_FUNCTION)SecondStruct;

    return GenericComparePointer(Table,
                                 First->CodeObject,
                                 Second->CodeObject);
}

_Use_decl_annotations_
BOOL
PyTraceRegisterNewPathEntry(
    PVOID IncomingContext,
    PPYTHON_PATH_TABLE_ENTRY Entry
    )
{
    PPYTHON_TRACE_CONTEXT Context = (PPYTHON_TRACE_CONTEXT)IncomingContext;

    if (!Entry->IsFile) {
        return TRUE;
    }

    SubmitNewPythonPathTableEntryWork(Context, Entry);

    return TRUE;
}

_Use_decl_annotations_
BOOL
InitializePythonTraceContext(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PPYTHON_TRACE_CONTEXT Context,
    PULONG SizeOfContext,
    PPYTHON Python,
    PTRACE_CONTEXT TraceContext,
    PSTRING_TABLE_API StringTableApi,
    PVOID UserData
    )
{
    BOOL Success;
    PTRACE_STORES TraceStores;
    PTRACE_STORE StringBufferStore;
    PTRACE_STORE PythonFunctionTableStore;
    PTRACE_STORE PythonFunctionTableEntryStore;
    PTRACE_STORE PythonPathTableStore;
    PTRACE_STORE PythonPathTableEntryStore;
    PTRACE_STORE StringArrayStore;
    PTRACE_STORE StringTableStore;
    PTRACE_STORE PythonModuleTableStore;
    PTRACE_STORE PythonModuleTableEntryStore;
    PTRACE_STORE LineTableStore;
    PTRACE_STORE LineTableEntryStore;
    PTRACE_STORE LineStringBufferStore;
    PTRACE_STORE BitmapStore;
    PTRACE_STORE UnicodeStringBufferStore;
    PPY_TRACE_EVENT TraceEvent;
    PPY_TRACE_CALLBACK CallbackWorker;
    PINITIALIZE_ALLOCATOR_FROM_TRACE_STORE InitializeAllocatorFromTraceStore;
    PPYTHON_ALLOCATORS Allocators;
    ULONG NumberOfAllocators = 0;
    TRACE_STORE_ID TraceStoreId;
    ULONG Result;
    HKEY RegistryKey;

    //
    // Validate arguments.
    //

    if (!Context) {
        if (SizeOfContext) {
            *SizeOfContext = sizeof(*Context);
        }
        return FALSE;
    }

    if (!SizeOfContext) {
        return FALSE;
    }

    if (*SizeOfContext < sizeof(*Context)) {
        *SizeOfContext = sizeof(*Context);
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    };

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StringTableApi)) {
        return FALSE;
    }

    //
    // Arguments are valid, continue with initialization.
    //

    SecureZeroMemory(Context, sizeof(*Context));

    //
    // Open the root registry key.
    //

    if (!OpenRootRegistryKey(&RegistryKey)) {
        return FALSE;
    }

    //
    // Read flags.
    //

    READ_REG_DWORD_FLAG(TraceMemory, FALSE);
    READ_REG_DWORD_FLAG(TraceIoCounters, FALSE);
    READ_REG_DWORD_FLAG(TraceHandleCount, FALSE);
    READ_REG_DWORD_FLAG(ProfileOnly, FALSE);
    READ_REG_DWORD_FLAG(TraceOnly, FALSE);
    READ_REG_DWORD_FLAG(TrackMaxRefCounts, FALSE);
    READ_REG_DWORD_FLAG(CountEvents, FALSE);
    READ_REG_DWORD_FLAG(TraceCallStack, FALSE);
    READ_REG_DWORD_FLAG(TraceEverything, FALSE);
    READ_REG_DWORD_FLAG(TraceNothing, FALSE);
    READ_REG_DWORD_FLAG(TraceEverythingWhenNoModuleFilterSet, FALSE);
    READ_REG_DWORD_FLAG(DisablePathTableEntryProcessing, FALSE);

    //
    // Read runtime parameters.
    //

    READ_REG_DWORD_RUNTIME_PARAM(TraceEventType, 2);
    READ_REG_DWORD_RUNTIME_PARAM(CallbackWorkerType, 1);

    RegCloseKey(RegistryKey);

    TraceEvent = GetFunctionPointerForTraceEventType(Context);
    if (!TraceEvent) {
        return FALSE;
    }

    CallbackWorker = GetFunctionPointerForCallbackWorkerType(Context);
    if (!CallbackWorker) {
        return FALSE;
    }

    //
    // Continue initialization.
    //

    Context->Size = *SizeOfContext;
    Context->Rtl = Rtl;
    Context->Allocator = Allocator;
    Context->Python = Python;
    Context->TraceContext = TraceContext;
    Context->CallbackWorker = CallbackWorker;
    Context->TraceEventFunction = TraceEvent;
    Context->UserData = UserData;

    Context->Depth = 0;
    Context->SkipFrames = 1;
    Context->StringTableApi = StringTableApi;

    //
    // Initialize threadpool related members if applicable.
    //

    if (Context->Flags.DisablePathTableEntryProcessing) {
        goto InitializeAllocators;
    }

    Context->ThreadpoolCallbackEnvironment = (
        TraceContext->ThreadpoolCallbackEnvironment
    );

    Context->CancellationThreadpoolCallbackEnvironment = (
        TraceContext->CancellationThreadpoolCallbackEnvironment
    );

    Context->ThreadpoolCleanupGroup = (
        TraceContext->ThreadpoolCleanupGroup
    );

    Context->NewPythonPathTableEntryWork = CreateThreadpoolWork(
        NewPythonPathTableEntryCallback,
        Context,
        Context->ThreadpoolCallbackEnvironment
    );

    if (!Context->NewPythonPathTableEntryWork) {
        return FALSE;
    }

    Success = Python->SetRegisterNewPathEntryCallback(
        Python,
        PyTraceRegisterNewPathEntry,
        Context
    );

    if (!Success) {
        return FALSE;
    }

InitializeAllocators:

    TraceStores = TraceContext->TraceStores;
    InitializeAllocatorFromTraceStore = (
        TraceContext->InitializeAllocatorFromTraceStore
    );

    //
    // N.B. This allocator initialization stuff is horrendous.
    //

    Allocators = &Python->Allocators;

    //
    // Define some helper macros.
    //

#define RESOLVE_STORE_EX(AllocatorName, StoreName)                         \
    TraceStoreId = TraceStore##StoreName##Id;                              \
    StoreName##Store = TraceStoreIdToTraceStore(TraceStores, TraceStoreId)

#define INIT_ALLOCATOR_EX(AllocatorName, StoreName, Target)             \
    RESOLVE_STORE_EX(AllocatorName, StoreName);                         \
    if (!InitializeAllocatorFromTraceStore(StoreName##Store, Target)) { \
        return FALSE;                                                   \
    }

#define INIT_STORE_ALLOCATOR_EX(AllocatorName, StoreName) \
    INIT_ALLOCATOR_EX(AllocatorName,                      \
                      StoreName,                          \
                      &Allocators->##AllocatorName);      \
    NumberOfAllocators++;

#define INIT_STORE_ALLOCATOR(Name) INIT_STORE_ALLOCATOR_EX(Name, Name)

    INIT_STORE_ALLOCATOR(StringBuffer);
    INIT_STORE_ALLOCATOR(PythonFunctionTable);
    INIT_STORE_ALLOCATOR(PythonFunctionTableEntry);
    INIT_STORE_ALLOCATOR(PythonPathTable);
    INIT_STORE_ALLOCATOR(PythonPathTableEntry);
    INIT_STORE_ALLOCATOR(StringArray);
    INIT_STORE_ALLOCATOR(StringTable);
    INIT_STORE_ALLOCATOR(LineTable);
    INIT_STORE_ALLOCATOR(LineTableEntry);
    INIT_STORE_ALLOCATOR(LineStringBuffer);

    INIT_STORE_ALLOCATOR(PythonModuleTable);
    INIT_STORE_ALLOCATOR(PythonModuleTableEntry);

    Allocators->NumberOfAllocators = NumberOfAllocators;
    Allocators->SizeInBytes = sizeof(*Allocators);

    //
    // Initialize allocators internal to the trace context.
    //

#define INIT_CONTEXT_ALLOCATOR_EX(AllocatorName, StoreName) \
    INIT_ALLOCATOR_EX(AllocatorName,                        \
                      StoreName,                            \
                      &Context->##AllocatorName##Allocator)

#define INIT_CONTEXT_ALLOCATOR(Name) INIT_CONTEXT_ALLOCATOR_EX(Name, Name)

    INIT_CONTEXT_ALLOCATOR(Bitmap);
    INIT_CONTEXT_ALLOCATOR(UnicodeStringBuffer);

    //
    // Initialize runtime tables and set the register path entry callback.
    //

    Python->InitializePythonRuntimeTables(Python);

    QueryPerformanceFrequency(&Context->Frequency);
    Context->Multiplicand.QuadPart = TIMESTAMP_TO_SECONDS;

    Rtl->PfxInitialize(&Context->ModuleFilterPrefixTree);

    Context->Start = Start;
    Context->Stop = Stop;

    Context->EnableMemoryTracing = EnableMemoryTracing;
    Context->DisableMemoryTracing = DisableMemoryTracing;

    Context->EnableIoCountersTracing = EnableIoCountersTracing;
    Context->DisableIoCountersTracing = DisableIoCountersTracing;

    Context->EnableHandleCountTracing = EnableHandleCountTracing;
    Context->DisableHandleCountTracing = DisableHandleCountTracing;

    Context->AddModuleName = AddModuleName;
    Context->SetModuleNamesStringTable = SetModuleNamesStringTable;
    Context->SetSystemTimeForRunHistory = SetSystemTimeForRunHistory;

    //
    // If we've been configured to track maximum reference counts, register an
    // extended atexit function that will reflect the maximum values we saw
    // in the registry on exit (if they exceed previously set maximums).
    //
    // (This is also a good test that our SetAtExitEx() machinery is working
    //  properly, which is why we call the global version instead of going
    //  through Rtl->RegisterAtExitEx().)
    //

    if (Context->Flags.TrackMaxRefCounts) {
        Success = AtExitEx(SaveMaxRefCountsAtExit,
                           NULL,
                           Context,
                           &Context->SaveMaxCountsAtExitEntry);
        if (!Success) {
            return FALSE;
        }
    }

    //
    // Do the same for general counters if applicable.
    //

    if (Context->Flags.CountEvents) {
        Success = AtExitEx(SaveCountsToRunHistoryAtExit,
                           NULL,
                           Context,
                           &Context->SaveCountsToRunHistoryAtExitEntry);
    } else {
        Success = TRUE;
    }

    //
    // Always save performance metrics.
    //

    Success = AtExitEx(SavePerformanceMetricsAtExit,
                       NULL,
                       Context,
                       &Context->SavePerformanceMetricsAtExitEntry);

    return Success;
}

_Use_decl_annotations_
VOID
ClosePythonTraceContext(
    PPYTHON_TRACE_CONTEXT Context
    )
{
    BOOL CancelPendingCallbacks = FALSE;
    PTP_WORK Work = Context->NewPythonPathTableEntryWork;
    if (Work) {
        WaitForThreadpoolWorkCallbacks(Work, CancelPendingCallbacks);
        CloseThreadpoolWork(Work);
        Context->NewPythonPathTableEntryWork = NULL;
    }
}

_Use_decl_annotations_
VOID
SaveMaxRefCountsAtExit(
    BOOL IsProcessTerminating,
    PPYTHON_TRACE_CONTEXT Context
    )
/*++

Routine Description:

    This routine is responsible for writing the maximum reference count values
    observed during a run to the registry on process exit.  It is called by the
    Rtl AtExitEx rundown functionality.  This routine is only called if the
    TrackMaxRefCounts flag has been set.

Arguments:

    IsProcessTerminating - Supplies a boolean value that indicates whether or
        not the process is terminating.  If FALSE, indicates that the library
        has been unloaded via FreeLibrary().

    Context - Supplies a pointer to the PYTHON_TRACE_CONTEXT structure that was
        registered when AtExitEx() was called.

Return Value:

    None.

--*/
{
    ULONG Result;
    HKEY RegistryKey;

    //
    // Validate arguments.
    //

    if (!Context) {
        return;
    }

    if (!OpenRootRegistryKey(&RegistryKey)) {
        return;
    }

    //
    // Write the max ref counts to the registry if they're greater than the
    // current version.
    //

#define UPDATE_MAX_COUNT(Name)                                  \
    UPDATE_MAX_REG_QWORD(RegistryKey,                           \
                         Name,                                  \
                         (ULONGLONG)Context->##Name##.QuadPart)

    UPDATE_MAX_COUNT(MaxNoneRefCount);
    UPDATE_MAX_COUNT(MaxTrueRefCount);
    UPDATE_MAX_COUNT(MaxZeroRefCount);
    UPDATE_MAX_COUNT(MaxFalseRefCount);

    //
    // Capture MaxDepth as well (despite it not technically being a reference
    // count; it's still useful to track).
    //

    UPDATE_MAX_COUNT(MaxDepth);

    //
    // Close the registry key.
    //

    RegCloseKey(RegistryKey);

}

_Use_decl_annotations_
BOOL
SetSystemTimeForRunHistory(
    PPYTHON_TRACE_CONTEXT Context,
    PSYSTEMTIME SystemTime
    )
{
    USHORT Offset;
    ULONG Value;
    UNICODE_STRING Date;
    UNICODE_STRING RegistryPath = \
        RTL_CONSTANT_STRING(RUN_HISTORY_REGISTRY_PATH_FORMAT);
    WCHAR RegistryPathBuffer[sizeof(RUN_HISTORY_REGISTRY_PATH_FORMAT) >> 1];

    //
    // Swap the buffers out.
    //

    RegistryPath.Buffer = (PWCHAR)RegistryPathBuffer;

    //
    // Copy the format string over.
    //

    __movsw(RegistryPathBuffer,
            RunHistoryRegistryPathFormat.Buffer,
            sizeof(RegistryPathBuffer) >> 1);

    //
    // Copy the system time over.
    //

    CopySystemTime(&Context->SystemTime, SystemTime);

    //
    // Get the byte offset into the registry string where we should start
    // writing, which corresponds to the maximum length of the run history
    // path prefix.  The joining slash is accounted for by virtue of maximum
    // length including the NUL terminator.
    //

    Offset = RunHistoryRegistryPathPrefix.MaximumLength;

    //
    // Initialize the date string to point to the relevant subset.
    //

    Date.Length = 0;
    Date.MaximumLength = RunHistoryDateFormat.MaximumLength;
    Date.Buffer = (PWCHAR)RtlOffsetToPointer(RegistryPath.Buffer, Offset);

#define AppendTimeField(Field, Digits, Trailer)                         \
    Value = SystemTime->Field;                                          \
    if (!AppendIntegerToUnicodeString(&Date, Value, Digits, Trailer)) { \
        goto Error;                                                     \
    }

    AppendTimeField(wYear,          4, L'-');
    AppendTimeField(wMonth,         2, L'-');
    AppendTimeField(wDay,           2, L'-');
    AppendTimeField(wHour,          2,    0);
    AppendTimeField(wMinute,        2,    0);
    AppendTimeField(wSecond,        2, L'.');
    AppendTimeField(wMilliseconds,  3,    0);

    return OpenRegistryKey(&RegistryPath, &Context->RunHistoryRegistryKey);

Error:

    return FALSE;
}

_Use_decl_annotations_
VOID
SaveCountsToRunHistoryAtExit(
    BOOL IsProcessTerminating,
    PPYTHON_TRACE_CONTEXT Context
    )
/*++

Routine Description:

    This routine is responsible for writing various counters pertaining to an
    active trace session to the registry on process exit.  It is called by the
    Rtl AtExitEx rundown functionality.

Arguments:

    IsProcessTerminating - Supplies a boolean value that indicates whether or
        not the process is terminating.  If FALSE, indicates that the library
        has been unloaded via FreeLibrary().

    Context - Supplies a pointer to the PYTHON_TRACE_CONTEXT structure that was
        registered when AtExitEx() was called.

Return Value:

    None.

--*/
{
    HKEY RegistryKey;

    //
    // Validate arguments.
    //

    if (!Context) {
        return;
    }

    RegistryKey = Context->RunHistoryRegistryKey;
    if (!RegistryKey) {
        return;
    }

    //
    // Define some convenience macros.
    //

#define WRITE_LARGE_INTEGER(Name)                          \
    WRITE_REG_QWORD(RegistryKey,                           \
                    Name,                                  \
                    (ULONGLONG)Context->##Name##.QuadPart)

#define WRITE_ULONGLONG(Name)          \
    WRITE_REG_QWORD(RegistryKey,       \
                    Name,              \
                    Context->##Name##)

    //
    // If we've been tracking max ref counts, write those values now.
    //
    // N.B. This differs slightly from SaveMaxRefCountsAtExit() in that we don't
    //      do the "max" check; as we're writing to LastRun, we just write the
    //      value directly without checking the existing value.
    //

    if (Context->Flags.TrackMaxRefCounts) {
        WRITE_LARGE_INTEGER(MaxNoneRefCount);
        WRITE_LARGE_INTEGER(MaxTrueRefCount);
        WRITE_LARGE_INTEGER(MaxZeroRefCount);
        WRITE_LARGE_INTEGER(MaxFalseRefCount);
    }

    WRITE_LARGE_INTEGER(MaxDepth);

    //
    // Write the counters.
    //

    WRITE_ULONGLONG(FramesTraced);
    WRITE_ULONGLONG(FramesSkipped);
    WRITE_ULONGLONG(NumberOfPythonCalls);
    WRITE_ULONGLONG(NumberOfPythonReturns);
    WRITE_ULONGLONG(NumberOfPythonExceptions);
    WRITE_ULONGLONG(NumberOfPythonLines);
    WRITE_ULONGLONG(NumberOfCCalls);
    WRITE_ULONGLONG(NumberOfCReturns);
    WRITE_ULONGLONG(NumberOfCExceptions);

    return;
}

_Use_decl_annotations_
VOID
SavePerformanceMetricsAtExit(
    BOOL IsProcessTerminating,
    PPYTHON_TRACE_CONTEXT Context
    )
/*++

Routine Description:

    This routine is responsible for writing various performance related metrics
    to the registry on process exit.  It is called by the Rtl AtExitEx rundown
    functionality.

Arguments:

    IsProcessTerminating - Supplies a boolean value that indicates whether or
        not the process is terminating.  If FALSE, indicates that the library
        has been unloaded via FreeLibrary().

    Context - Supplies a pointer to the PYTHON_TRACE_CONTEXT structure that was
        registered when AtExitEx() was called.

Return Value:

    None.

--*/
{
    PRTL Rtl;
    BOOL Success;
    HKEY RegistryKey;
    FILETIMEEX Dummy;
    FILETIMEEX Duration;
    FILETIMEEX UserTime;
    FILETIMEEX ExitTime;
    FILETIMEEX KernelTime;
    FILETIMEEX CreationTime;
    HANDLE CurrentProcess;

    //
    // We use the TRACE_PERFORMANCE structure here instead of local variables
    // for each individual struct as it's more convenient (and lends itself to
    // clean macros for writing values).
    //

    TRACE_PERFORMANCE Perf;

    //
    // Validate arguments.
    //

    if (!Context) {
        return;
    }

    //
    // Initialize aliases.
    //

    CurrentProcess = GetCurrentProcess();

    RegistryKey = Context->RunHistoryRegistryKey;
    if (!RegistryKey) {
        return;
    }

    Rtl = Context->Rtl;
    if (!Rtl) {
        return;
    }

    //
    // Define convenience macros.
    //

#define WRITE_NAMED_QWORD(Name, Value) \
    WRITE_REG_QWORD(RegistryKey,       \
                    Name,              \
                    Value)

#define WRITE_PERF_QWORD(Name)   \
    WRITE_REG_QWORD(RegistryKey, \
                    Name,        \
                    Perf.##Name)

#define WRITE_FILETIMEEX(Name)        \
    WRITE_REG_QWORD(RegistryKey,      \
                    Name,             \
                    Name.AsULongLong)

    //
    // Get memory information.
    //

    Perf.ProcessMemoryCountersExSize = sizeof(Perf.MemoryCountersEx);
    Success = Rtl->K32GetProcessMemoryInfo(CurrentProcess,
                                           &Perf.MemoryCounters,
                                           Perf.ProcessMemoryCountersExSize);
    if (Success) {
        WRITE_PERF_QWORD(PageFaultCount);
        WRITE_PERF_QWORD(WorkingSetSize);
        WRITE_PERF_QWORD(QuotaPeakPagedPoolUsage);
        WRITE_PERF_QWORD(QuotaPagedPoolUsage);
        WRITE_PERF_QWORD(QuotaPeakNonPagedPoolUsage);
        WRITE_PERF_QWORD(QuotaNonPagedPoolUsage);
        WRITE_PERF_QWORD(PagefileUsage);
        WRITE_PERF_QWORD(PeakPagefileUsage);
        WRITE_PERF_QWORD(PrivateUsage);
    }

    //
    // Get I/O counter information.
    //

    Success = Rtl->GetProcessIoCounters(CurrentProcess, &Perf.IoCounters);
    if (Success) {
        WRITE_PERF_QWORD(ReadOperationCount);
        WRITE_PERF_QWORD(WriteOperationCount);
        WRITE_PERF_QWORD(OtherOperationCount);
        WRITE_PERF_QWORD(ReadTransferCount);
        WRITE_PERF_QWORD(WriteTransferCount);
        WRITE_PERF_QWORD(OtherTransferCount);
    }

    //
    // We capture exit time manually here, as it won't be set when
    // GetProcessTimes() is called below (as we're still running).
    //

    GetSystemTimeAsFileTime(&ExitTime.AsFiletime);
    WRITE_FILETIMEEX(ExitTime);

    //
    // Get process times.
    //

    Success = GetProcessTimes(CurrentProcess,
                              &CreationTime.AsFiletime,
                              &Dummy.AsFiletime,
                              &KernelTime.AsFiletime,
                              &UserTime.AsFiletime);
    if (Success) {
        WRITE_FILETIMEEX(UserTime);
        WRITE_FILETIMEEX(KernelTime);
        WRITE_FILETIMEEX(CreationTime);
        Duration.AsULongLong = ExitTime.AsULongLong - CreationTime.AsULongLong;
        WRITE_FILETIMEEX(Duration);
    }

    //
    // Query cycle time.
    //

    Success = QueryProcessCycleTime(CurrentProcess, &Perf.ProcessCycles);
    if (Success) {
        WRITE_PERF_QWORD(ProcessCycles);
    }

    return;
}

_Use_decl_annotations_
BOOL
Start(
    PPYTHON_TRACE_CONTEXT Context
    )
{
    PPYTHON Python;

    if (!Context) {
        return FALSE;
    }

    Python = Context->Python;

    if (!Python) {
        return FALSE;
    }

    if (!Context->Flags.ProfileOnly) {
        Context->RuntimeState.IsTracing = TRUE;
        Python->PyEval_SetTrace(PyTraceCallback, (PPYOBJECT)Context);
    }

    if (!Context->Flags.TraceOnly) {
        Context->RuntimeState.IsProfiling = TRUE;
        Python->PyEval_SetProfile(PyTraceCallback, (PPYOBJECT)Context);
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
Stop(
    PPYTHON_TRACE_CONTEXT Context
    )
{
    PPYTHON Python;

    if (!Context) {
        return FALSE;
    }

    Python = Context->Python;

    if (!Python) {
        return FALSE;
    }

    Context->RuntimeState.IsTracing = FALSE;
    Context->RuntimeState.IsProfiling = FALSE;

    Python->PyEval_SetTrace(NULL, NULL);
    Python->PyEval_SetProfile(NULL, NULL);

    return TRUE;
}

_Success_(return != 0)
BOOL
AddPrefixTableEntry(
    _In_  PPYTHON_TRACE_CONTEXT Context,
    _In_  PPYOBJECT             StringObject,
    _In_  PPREFIX_TABLE         PrefixTable,
    _Outptr_opt_result_nullonfailure_ PPPREFIX_TABLE_ENTRY  EntryPointer
    )
{
    PRTL Rtl;
    PPYTHON Python;
    STRING String;
    PSTRING Name;
    PPREFIX_TABLE_ENTRY Entry;
    ULONG AllocSize;
    PVOID Buffer;
    BOOL Success;

    Rtl = Context->Rtl;
    Python = Context->Python;

    //
    // Get a STRING representation of the incoming PyObject string name.
    //

    Success = WrapPythonStringAsString(Python,
                                       StringObject,
                                       &String);

    if (!Success) {
        return FALSE;
    }

    //
    // Make sure it's within our limits.
    //

    if (String.Length >= MAX_STRING) {
        return FALSE;
    }

    //
    // Make sure the entry isn't already in the tree.
    //

    Entry = Rtl->PfxFindPrefix(PrefixTable, &String);

    if (Entry) {

        //
        // Entry already exists, nothing more to do.  We don't check the
        // lengths here as our use case for this function is currently limited
        // to runtime manipulation of the module filter table, where a prefix
        // entry is sufficient to enable tracing for a module.
        //

        return TRUE;
    }

    //
    // Entry doesn't exist in the prefix table, allocate space for a
    // PREFIX_TABLE_ENTRY, the corresponding STRING, and the underlying buffer
    // (which we copy so that we can control ownership lifetime), plus 1 for
    // the trailing NUL.
    //

    AllocSize = (
        sizeof(PREFIX_TABLE_ENTRY)  +
        sizeof(STRING)              +
        String.Length               +
        1
    );

    AllocSize = ALIGN_UP_POINTER(AllocSize);

    if (!Python->AllocateBuffer(Python, AllocSize, &Buffer)) {
        return FALSE;
    }

    if (!Buffer) {
        return FALSE;
    }

    Entry = (PPREFIX_TABLE_ENTRY)Buffer;

    //
    // Point our STRING struct to after the PREFIX_TABLE_ENTRY.
    //

    Name = (PSTRING)(
        RtlOffsetToPointer(
            Buffer,
            sizeof(PREFIX_TABLE_ENTRY)
        )
    );

    //
    // And point the STRING's buffer to after the STRING struct.
    //

    Name->Buffer = (PCHAR)(
        RtlOffsetToPointer(
            Buffer,
            sizeof(PREFIX_TABLE_ENTRY) +
            sizeof(STRING)
        )
    );

    //
    // Fill in the name length details and copy the string over.
    //

    Name->Length = String.Length;
    Name->MaximumLength = String.Length+1;
    __movsb(Name->Buffer, String.Buffer, Name->Length);

    //
    // Add trailing NUL.
    //

    Name->Buffer[Name->Length] = '\0';

    //
    // Finally, add to the table.
    //

    Success = Rtl->PfxInsertPrefix(PrefixTable, Name, Entry);

    //
    // Update caller's pointer if applicable.
    //

    if (Success) {
        *EntryPointer = Entry;
    } else {

        //
        // Shouldn't be able to get here.
        //

        __debugbreak();
        return FALSE;
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
AddModuleName(
    PPYTHON_TRACE_CONTEXT   Context,
    PPYOBJECT               ModuleNameObject
    )
{
    BOOL Success;
    PPREFIX_TABLE_ENTRY PrefixTableEntry;

    if (!ARGUMENT_PRESENT(Context)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ModuleNameObject)) {
        return FALSE;
    }

    Success = AddPrefixTableEntry(Context,
                                  ModuleNameObject,
                                  &Context->ModuleFilterPrefixTree,
                                  &PrefixTableEntry);

    if (Success) {
        Context->RuntimeState.HasModuleFilter = TRUE;
    }

    return Success;
}

_Use_decl_annotations_
BOOL
SetModuleNamesStringTable(
    PPYTHON_TRACE_CONTEXT Context,
    PSTRING_TABLE StringTable
    )
{
    PSTRING_TABLE ExistingTable;

    if (!ARGUMENT_PRESENT(Context)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StringTable)) {
        return FALSE;
    }

    ExistingTable = Context->ModuleFilterStringTable;

    if (ExistingTable) {

        //
        // XXX todo: destroy existing table.
        //

    }

    Context->ModuleFilterStringTable = StringTable;

    Context->RuntimeState.HasModuleFilter = TRUE;

    return TRUE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
