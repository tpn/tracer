/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracer.c

Abstract:

    This module implements functionality related to tracing an instance of a
    Python interpreter using the trace store functionality.  It relies on the
    Python and TraceStore components.

    N.B.: The functionality in this file should be split up into smaller,
          more self-contained modules.

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
    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { ByteSize };

    return TraceStore->AllocateRecords(
        TraceStore->TraceContext,
        TraceStore,
        &RecordSize,
        &NumberOfRecords
    );
}

PVOID
TraceStoreCallocRoutine(
    _In_ PVOID AllocationContext,
    _In_ SIZE_T NumberOfElements,
    _In_ SIZE_T ElementSize
    )
{
    PTRACE_STORE TraceStore = (PTRACE_STORE)AllocationContext;
    ULARGE_INTEGER NumberOfRecords;
    ULARGE_INTEGER RecordSize;

    NumberOfRecords.QuadPart = NumberOfElements;
    RecordSize.QuadPart = ElementSize;

    return TraceStore->AllocateRecords(
        TraceStore->TraceContext,
        TraceStore,
        &RecordSize,
        &NumberOfRecords
    );
}

PPYTHON_TRACE_EVENT
AllocatePythonTraceEvent(
    _In_ PTRACE_STORE TraceStore
    )
{
    ULARGE_INTEGER NumberOfRecords = { 1 };
    ULARGE_INTEGER RecordSize = { sizeof(PYTHON_TRACE_EVENT) };

    return (PPYTHON_TRACE_EVENT)(
        TraceStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore,
            &RecordSize,
            &NumberOfRecords
        )
    );
}

VOID
TraceStoreFreeRoutine(
    _In_opt_ PVOID FreeContext,
    _In_     PVOID Buffer
    )
{
    return;

    /*
    PTRACE_STORE TraceStore;
    TraceStore = (PTRACE_STORE)FreeContext;

    TraceStore->FreeRecords(TraceStore->TraceContext,
                            TraceStore,
                            Buffer);
    */
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

    if (!Context->HasModuleFilter) {

        //
        // Trace everything.
        //

        return TRUE;
    }

    ModuleName = &Function->PathEntry.ModuleName;

    Table = &Context->ModuleFilterPrefixTree;

    Entry = Rtl->PfxFindPrefix(Table, ModuleName);

    return (Entry ? TRUE : FALSE);
}

FORCEINLINE
BOOL
IsFunctionOfInterestStringTable(
    _In_    PRTL                    Rtl,
    _In_    PPYTHON_TRACE_CONTEXT   Context,
    _In_    PPYTHON_FUNCTION        Function
    )
{
    STRING Name;
    PSTRING ModuleName;
    PSTRING_TABLE StringTable = Context->ModuleFilterStringTable;
    STRING_TABLE_INDEX Index;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    if (!Context->HasModuleFilter) {

        //
        // Trace everything.
        //

        return TRUE;
    }

    ModuleName = &Function->PathEntry.ModuleName;

    if (!StringTable || !ModuleName || ModuleName->Length <= 1) {
        return FALSE;
    }

    if (ModuleName->Buffer[0] == '\\') {
        Name.Length = ModuleName->Length - 1;
        Name.MaximumLength = ModuleName->MaximumLength - 1;
        Name.Buffer = ModuleName->Buffer + 1;
        ModuleName = &Name;
    }

    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;
    Index = IsPrefixOfStringInTable(StringTable, ModuleName, NULL);

    return (Index != NO_MATCH_FOUND);
}

#define IsFunctionOfInterest IsFunctionOfInterestStringTable

_Use_decl_annotations_
VOID
EnableMemoryTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->TraceMemory = TRUE;
}

_Use_decl_annotations_
VOID
DisableMemoryTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->TraceMemory = FALSE;
}

_Use_decl_annotations_
VOID
EnableIoCountersTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->TraceIoCounters = TRUE;
}

_Use_decl_annotations_
VOID
DisableIoCountersTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->TraceIoCounters = FALSE;
}

_Use_decl_annotations_
VOID
EnableHandleCountTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->TraceHandleCount = TRUE;
}

_Use_decl_annotations_
VOID
DisableHandleCountTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PythonTraceContext->TraceHandleCount = FALSE;
}

LONG
PyTraceCallback(
    _In_        PPYTHON_TRACE_CONTEXT   Context,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    )
{
    BOOL Success;
    BOOL IsCall;
    BOOL IsReturn;
    BOOL IsLine;
    BOOL IsException;
    BOOL IsC;

    PYTHON_TRACE_CONTEXT_FLAGS Flags = Context->Flags;

    PRTL Rtl;
    PPYTHON Python;
    HRESULT Result;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    PTRACE_STORE EventStore;
    PYTHON_TRACE_EVENT Event;
    PYTHON_TRACE_EVENT LastEvent;
    PPYTHON_TRACE_EVENT LastEventPointer;
    PPYTHON_TRACE_EVENT ThisEvent;
    PPYTHON_FUNCTION Function = NULL;
    LARGE_INTEGER Elapsed;
    PROCESS_MEMORY_COUNTERS_EX MemoryCounters;
    IO_COUNTERS IoCounters;
    DWORD HandleCount;
    HANDLE CurrentProcess = (HANDLE)-1;

    IsCall = (
        EventType == TraceEventType_PyTrace_CALL        ||
        EventType == TraceEventType_PyTrace_C_CALL
    );

    IsReturn = (
        EventType == TraceEventType_PyTrace_RETURN      ||
        EventType == TraceEventType_PyTrace_C_RETURN
    );

    IsLine = (
        EventType == TraceEventType_PyTrace_LINE
    );

    IsException = (
        EventType == TraceEventType_PyTrace_EXCEPTION   ||
        EventType == TraceEventType_PyTrace_C_EXCEPTION
    );

    IsC = (
        EventType == TraceEventType_PyTrace_C_CALL      ||
        EventType == TraceEventType_PyTrace_C_RETURN    ||
        EventType == TraceEventType_PyTrace_C_EXCEPTION
    );


    if (!Flags.HasStarted) {

        //
        // We haven't started tracing/profiling yet.
        //

        if (Flags.IsProfile) {

            //
            // We are in profile mode.
            //

            if (!IsCall) {

                //
                // If we haven't started profiling yet, we can ignore any
                // event that isn't a call event.  (In practice, there will
                // usually be one return event/frame before we get a call
                // event we're interested in.)
                //

                return 0;

            }

        } else {

            //
            // We are in tracing mode.
            //

            if (!IsLine) {

                //
                // If we haven't started tracing yet, we can ignore any event
                // that isn't a line event.  (In practice, there will usually
                // be one return event/frame before we get a line event we're
                // interested in.)
                //

                return 0;
            }

        }

        //
        // We've received our first profile/trace event of interest.  Toggle
        // our 'HasStarted' flag and set our context depth to 1.
        //

        Flags.HasStarted = Context->Flags.HasStarted = TRUE;

        Context->Depth = 1;

    } else {

        //
        // We're already tracing/profiling, so just update our depth counter
        // accordingly if we're a call/return.
        //

        if (IsCall) {

            Context->Depth++;

        } else if (IsReturn) {

            Context->Depth--;

        }

    }

    //
    // Attempt to register the frame and get the underlying function object.
    //

    Rtl = Context->Rtl;
    Python = Context->Python;


    Success = Python->RegisterFrame(Python,
                                    FrameObject,
                                    EventType,
                                    ArgObject,
                                    &Function);

    if (!Success) {

        //
        // We can't do anything more if we weren't able to resolve the
        // function for this frame.
        //

        return 0;
    }

    if (!Function->PathEntry.IsValid) {
        __debugbreak();
        return 0;
    }

    //
    // We obtained the PYTHON_FUNCTION for this frame, check to see if it's
    // of interest to this tracing session.
    //

    if (!IsFunctionOfInterest(Rtl, Context, Function)) {

        //
        // Function isn't of interest (i.e. doesn't reside in a module we're
        // tracing), so return.
        //

        return 0;
    }

    //
    // The function resides in a module (or submodule) we're tracing, continue.
    //


    //
    // Load the events trace store and previous event record, if any.
    //

    TraceContext = Context->TraceContext;
    TraceStores = TraceContext->TraceStores;
    EventStore = &TraceStores->Stores[TraceStoreEventIndex];

    LastEventPointer = (PPYTHON_TRACE_EVENT)EventStore->PrevAddress;

    if (Flags.TraceMemory) {

        Success = Rtl->K32GetProcessMemoryInfo(
            CurrentProcess,
            (PPROCESS_MEMORY_COUNTERS)&MemoryCounters,
            sizeof(MemoryCounters)
        );

        if (!Success) {
            Flags.TraceMemory = FALSE;
        }

    }

    if (Flags.TraceIoCounters) {

        Success = Rtl->GetProcessIoCounters(CurrentProcess,
                                            &IoCounters);

        if (!Success) {
            Flags.TraceIoCounters = FALSE;
        }
    }

    if (Flags.TraceHandleCount) {

        Success = Rtl->GetProcessHandleCount(CurrentProcess,
                                             &HandleCount);

        if (!Success) {
            Flags.TraceHandleCount = TRUE;
        }
    }

    SecureZeroMemory(&Event, sizeof(Event));

    //
    // Save the timestamp for this event.
    //

    TraceContextQueryPerformanceCounter(TraceContext, &Elapsed);

    //
    // Fill out the function.
    //

    Event.Timestamp.QuadPart = Elapsed.QuadPart;
    Event.Function = Function;
    Event.IsC = IsC;
    Event.CodeObjectHash = Function->CodeObjectHash;
    Event.FunctionHash = Function->FunctionHash;
    Event.FirstLineNumber = Function->FirstLineNumber;
    Event.NumberOfLines = Function->NumberOfLines;
    Event.NumberOfCodeLines = Function->NumberOfCodeLines;
    Event.PathHash = Function->PathEntry.PathHash;
    Event.FullNameHash = Function->PathEntry.FullNameHash;
    Event.ModuleNameHash = Function->PathEntry.ModuleNameHash;
    Event.ClassNameHash = Function->PathEntry.ClassNameHash;
    Event.NameHash = Function->PathEntry.NameHash;
    Event.ThreadId = FastGetCurrentThreadId();

    if (IsException) {

        //
        // We don't do anything for exceptions at the moment.
        //

        Event.IsException = TRUE;

    } else if (IsCall) {

        Event.IsCall = TRUE;

    } else if (IsReturn) {

        Event.IsReturn = TRUE;

    } else if (IsLine) {

        Event.IsLine = TRUE;

        //
        // Update the line number for this event.
        //

        Event.LineNumber = (USHORT)Python->PyFrame_GetLineNumber(FrameObject);

    }

    //
    // Save memory, I/O and handle counts if applicable.
    //

    if (Flags.TraceMemory) {
        Event.WorkingSetSize = MemoryCounters.WorkingSetSize;
        Event.PageFaultCount = MemoryCounters.PageFaultCount;
        Event.CommittedSize  = MemoryCounters.PrivateUsage;
    }

    if (Flags.TraceIoCounters) {
        Event.ReadTransferCount = IoCounters.ReadTransferCount;
        Event.WriteTransferCount = IoCounters.WriteTransferCount;
    }

    if (Flags.TraceHandleCount) {
        Event.HandleCount = HandleCount;
    }

    if (!LastEventPointer) {
        goto Finalize;
    }

    //
    // Take a local copy of the last event.
    //

    Result = Rtl->RtlCopyMappedMemory(&LastEvent,
                                      LastEventPointer,
                                      sizeof(LastEvent));

    if (FAILED(Result)) {

        //
        // STATUS_IN_PAGE_ERROR occurred whilst copying the last event.
        //

        goto Finalize;
    }

    //
    // Calculate the elapsed time relative to the last event's timestamp and
    // then update its elapsed microsecond field.
    //

    Elapsed.QuadPart -= LastEvent.Timestamp.QuadPart;
    LastEvent.ElapsedMicroseconds = Elapsed.LowPart;

    if (LastEvent.IsLine) {

        //
        // If the last event's line number was greater than this line
        // number, we've jumped backwards, presumably as part of a loop.
        //

        if (LastEvent.LineNumber > Event.LineNumber) {
            Event.IsReverseJump = TRUE;
        }
    }

    //
    // Calculate deltas for memory, I/O and handle counts, if applicable.
    //

    if (Flags.TraceMemory) {

        //
        // Calculate memory counter deltas.
        //

        LastEvent.WorkingSetDelta = (LONG)(
            Event.WorkingSetSize -
            LastEvent.WorkingSetSize
        );

        LastEvent.PageFaultDelta = (USHORT)(
            Event.PageFaultCount -
            LastEvent.PageFaultCount
        );

        LastEvent.CommittedDelta = (LONG)(
            Event.CommittedSize -
            LastEvent.CommittedSize
        );

    }

    if (Flags.TraceIoCounters) {

        //
        // Calculate IO counter deltas.
        //

        LastEvent.ReadTransferDelta = (ULONG)(
            Event.ReadTransferCount -
            LastEvent.ReadTransferCount
        );

        LastEvent.WriteTransferDelta = (ULONG)(
            Event.WriteTransferCount -
            LastEvent.WriteTransferCount
        );

    }

    if (Flags.TraceHandleCount) {

        //
        // Calculate handle count delta.
        //

        LastEvent.HandleDelta = (SHORT)(
            Event.HandleCount -
            LastEvent.HandleDelta
        );

    }

    //
    // Copy the last event back, ignoring the return value.
    //

    Rtl->RtlCopyMappedMemory(LastEventPointer,
                             &LastEvent,
                             sizeof(LastEvent));

Finalize:

    //
    // Allocate a new event record, then copy our temporary event over.
    //

    ThisEvent = AllocatePythonTraceEvent(EventStore);

    if (!ThisEvent) {
        return 0;
    }

    Rtl->RtlCopyMappedMemory(ThisEvent,
                             &Event,
                             sizeof(Event));

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


BOOL
InitializePythonTraceContext(
    _In_ PRTL Rtl,
    _Out_bytecap_(*SizeOfContext) PPYTHON_TRACE_CONTEXT Context,
    _Inout_ PULONG SizeOfContext,
    _In_ PPYTHON Python,
    _In_ PTRACE_CONTEXT TraceContext,
    _In_opt_ PPYTRACEFUNC PythonTraceFunction,
    _In_opt_ PVOID UserData
    )
{
    PTRACE_STORES TraceStores;
    PTRACE_STORE EventStore;
    PTRACE_STORE StringStore;
    PTRACE_STORE StringBufferStore;
    PTRACE_STORE HashedStringStore;
    PTRACE_STORE HashedStringBufferStore;
    PTRACE_STORE BufferStore;
    PTRACE_STORE FunctionTableStore;
    PTRACE_STORE FunctionTableEntryStore;
    PTRACE_STORE PathTableStore;
    PTRACE_STORE PathTableEntryStore;
    PTRACE_STORE FilenameStringStore;
    PTRACE_STORE FilenameStringBufferStore;
    PTRACE_STORE DirectoryStringStore;
    PTRACE_STORE DirectoryStringBufferStore;
    PTRACE_STORE StringArrayStore;
    PTRACE_STORE StringTableStore;
    PYTHON_ALLOCATORS Allocators;
    ULONG NumberOfAllocators = 0;
    USHORT TraceStoreIndex;

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

    if (!ARGUMENT_PRESENT(Python)) {
        return FALSE;
    };

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceContext)) {
        return FALSE;
    }

    SecureZeroMemory(Context, sizeof(*Context));

    Context->Size = *SizeOfContext;
    Context->Rtl = Rtl;
    Context->Python = Python;
    Context->TraceContext = TraceContext;
    Context->PythonTraceFunction = PythonTraceFunction;
    Context->UserData = UserData;

    if (!Context->PythonTraceFunction) {
        Context->PythonTraceFunction = (PPYTRACEFUNC)PyTraceCallback;
    }

    Context->SkipFrames = 1;

    TraceStores = TraceContext->TraceStores;

#define INIT_STORE_ALLOCATOR(Name)                                       \
    TraceStoreIndex = TraceStore##Name##Index;                           \
    Name##Store = &TraceStores->Stores[TraceStore##Name##Index];         \
    Name##Store->NoRetire = TRUE;                                        \
    Allocators.##Name##.AllocationRoutine = TraceStoreAllocationRoutine; \
    Allocators.##Name##.AllocationContext = ##Name##Store;               \
    Allocators.##Name##.FreeRoutine = TraceStoreFreeRoutine;             \
    Allocators.##Name##.FreeContext = ##Name##Store;                     \
    NumberOfAllocators++;

    INIT_STORE_ALLOCATOR(String);
    INIT_STORE_ALLOCATOR(StringBuffer);
    INIT_STORE_ALLOCATOR(HashedString);
    INIT_STORE_ALLOCATOR(HashedStringBuffer);
    INIT_STORE_ALLOCATOR(Buffer);
    INIT_STORE_ALLOCATOR(FunctionTable);
    INIT_STORE_ALLOCATOR(FunctionTableEntry);
    INIT_STORE_ALLOCATOR(PathTable);
    INIT_STORE_ALLOCATOR(PathTableEntry);
    INIT_STORE_ALLOCATOR(FilenameString);
    INIT_STORE_ALLOCATOR(FilenameStringBuffer);
    INIT_STORE_ALLOCATOR(DirectoryString);
    INIT_STORE_ALLOCATOR(DirectoryStringBuffer);
    INIT_STORE_ALLOCATOR(StringArray);
    INIT_STORE_ALLOCATOR(StringTable);

    EventStore = &TraceStores->Stores[TraceStoreEventIndex];
    EventStore->NoRetire = FALSE;
    EventStore->NoPreferredAddressReuse = TRUE;

    Allocators.NumberOfAllocators = NumberOfAllocators;
    Allocators.SizeInBytes = sizeof(Allocators);

    if (!Python->SetPythonAllocators(Python, &Allocators)) {
        return FALSE;
    }

    Python->InitializePythonRuntimeTables(Python);

    Context->FirstFunction = NULL;
    QueryPerformanceFrequency(&Context->Frequency);

    Rtl->PfxInitialize(&Context->ModuleFilterPrefixTree);

    InitializeListHead(&Context->Functions);

    Context->StartTracing = StartTracing;
    Context->StopTracing = StopTracing;

    Context->StartProfiling = StartProfiling;
    Context->StopProfiling = StopProfiling;

    Context->EnableMemoryTracing = EnableMemoryTracing;
    Context->DisableMemoryTracing = DisableMemoryTracing;

    Context->EnableIoCountersTracing = EnableIoCountersTracing;
    Context->DisableIoCountersTracing = DisableIoCountersTracing;

    Context->EnableHandleCountTracing = EnableHandleCountTracing;
    Context->DisableHandleCountTracing = DisableHandleCountTracing;

    Context->AddModuleName = AddModuleName;
    Context->SetModuleNamesStringTable = SetModuleNamesStringTable;

    return TRUE;
}

_Use_decl_annotations_
BOOL
StartTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PPYTHON Python;
    PPYTRACEFUNC PythonTraceFunction;

    if (!PythonTraceContext) {
        return FALSE;
    }

    Python = PythonTraceContext->Python;

    if (!Python) {
        return FALSE;
    }

    PythonTraceFunction = PythonTraceContext->PythonTraceFunction;

    if (!PythonTraceFunction) {
        return FALSE;
    }

    QueryPerformanceCounter(&PythonTraceContext->StartTimestamp);

    PythonTraceContext->IsProfile = FALSE;

    Python->PyEval_SetTrace(
        PythonTraceFunction,
        (PPYOBJECT)PythonTraceContext
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
StopTracing(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PPYTHON Python;

    if (!PythonTraceContext) {
        return FALSE;
    }

    Python = PythonTraceContext->Python;

    if (!Python) {
        return FALSE;
    }

    QueryPerformanceCounter(&PythonTraceContext->StopTimestamp);

    Python->PyEval_SetTrace(NULL, NULL);

    return TRUE;
}

_Use_decl_annotations_
BOOL
StartProfiling(
    PPYTHON_TRACE_CONTEXT   PythonTraceContext
    )
{
    PPYTHON Python;
    PPYTRACEFUNC PythonTraceFunction;

    if (!PythonTraceContext) {
        return FALSE;
    }

    Python = PythonTraceContext->Python;

    if (!Python) {
        return FALSE;
    }

    PythonTraceFunction = PythonTraceContext->PythonTraceFunction;

    if (!PythonTraceFunction) {
        return FALSE;
    }

    PythonTraceContext->IsProfile = TRUE;

    Python->PyEval_SetProfile(
        PythonTraceFunction,
        (PPYOBJECT)PythonTraceContext
    );

    return TRUE;
}

_Use_decl_annotations_
BOOL
StopProfiling(
    PPYTHON_TRACE_CONTEXT   Context
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

    Context->HasStarted = FALSE;

    Python->PyEval_SetProfile(NULL, NULL);

    return TRUE;
}

BOOL
AddFunction(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_    PVOID                   FunctionObject
    )
{
    if (!PythonTraceContext) {
        return FALSE;
    }

    if (!FunctionObject) {
        return FALSE;
    }

    //PythonTraceContext->FunctionObject = (PPYFUNCTIONOBJECT)FunctionObject;

    return TRUE;
}

_Success_(return != 0)
BOOL
AddPrefixTableEntry(
    _In_  PPYTHON_TRACE_CONTEXT Context,
    _In_  PPYOBJECT             StringObject,
    _In_  PPREFIX_TABLE         PrefixTable,
    _Out_ PPPREFIX_TABLE_ENTRY  EntryPointer
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
        Context->HasModuleFilter = TRUE;
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

    Context->HasModuleFilter = TRUE;

    return TRUE;
}

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
