////////////////////////////////////////////////////////////////////////////////
// PythonTracer.c
////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif

#include "PythonTracer.h"

PVOID
TraceStoreAllocationRoutine(
    _In_opt_ PVOID AllocationContext,
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
    PTRACE_STORE TraceStore = (PTRACE_STORE)FreeContext;

    TraceStore->FreeRecords(TraceStore->TraceContext,
                            TraceStore,
                            Buffer);
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
    BOOL IsFirstTrace;
    BOOL StartedTracing;
    BOOL IsCall;
    BOOL IsReturn;
    BOOL IsLine;
    BOOL IsException;
    BOOL IsC;

    PRTL Rtl;
    PPYTHON Python;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    PTRACE_STORE Events;
    PYTHON_TRACE_EVENT Event;
    PPYTHON_TRACE_EVENT LastEvent;
    PPYTHON_TRACE_EVENT ThisEvent;
    PPYTHON_FUNCTION Function = NULL;
    PSTRING ModuleName;
    PPREFIX_TABLE Table;
    PPREFIX_TABLE_ENTRY Entry;
    LARGE_INTEGER Elapsed;

    IsFirstTrace = FALSE;
    StartedTracing = (BOOL)Context->StartedTracing;

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

    if (!StartedTracing) {

        if (!IsLine) {

            //
            // If we haven't started tracing yet, we can ignore any event that
            // isn't a line event.  (In practice, there will usually be one
            // return event/frame before we get a line event we're interested
            // in.)
            //

            return 0;
        }

        //
        // We've received our first line event of interest, start tracing.
        //

        StartedTracing = Context->StartedTracing = TRUE;

        IsFirstTrace = TRUE;

        Context->Depth = 1;

    } else {

        //
        // If we're already tracing, just update our counters accordingly.
        // The depth used to be used in conjunction with Context->SkipFrames,
        // but now it's not.  It may be removed altogether.
        //

        if (IsCall) {

            ++Context->Depth;

        } else if (IsReturn) {

            --Context->Depth;

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

    if (!Function || Function->PathEntry.Path.Length == 0) {
        __debugbreak();
        return 0;
    }

    //
    // We obtained the PYTHON_FUNCTION for this frame; use the module name to
    // determine if we should keep tracing by checking for a prefix table entry
    // in our module filter table.
    //

    ModuleName = &Function->PathEntry.ModuleName;

    Table = &Context->ModuleFilterTable;

    Entry = Rtl->PfxFindPrefix(Table, ModuleName);

    if (!Entry) {

        //
        // The function doesn't reside in a module we're tracing, return.
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
    Events = &TraceStores->Stores[TRACE_STORE_EVENTS_INDEX];

    LastEvent = (PPYTHON_TRACE_EVENT)Events->PrevAddress;

    //
    // Save the timestamp for this event.
    //

    QueryPerformanceCounter(&Event.Timestamp);

    //
    // Fill out the function.
    //

    Event.Function = Function;
    Event.Flags = 0;
    Event.IsC = IsC;

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

        Event.LineNumber = Python->PyFrame_GetLineNumber(FrameObject);

        if (LastEvent && LastEvent->IsLine) {

            //
            // Update the duration for the line event.
            //

            Elapsed.QuadPart = (
                Event.Timestamp.QuadPart -
                LastEvent->Timestamp.QuadPart
            );

            //
            // Microseconds to seconds.
            //

            Elapsed.QuadPart *= 1000000;

            //
            // Divide by frequency to get elapsed microseconds.
            //

            Elapsed.QuadPart /= Context->Frequency.QuadPart;

            //
            // Copy the elapsed microsecond value back to the last event.
            //

            LastEvent->Elapsed.QuadPart = Elapsed.QuadPart;

            //
            // If the last event's line number was greater than this line
            // number, we've jumped backwards, presumably as part of a loop.
            //

            if (LastEvent->LineNumber > Event.LineNumber) {
                Event.IsReverseJump = TRUE;
            }

        }
    }

    //
    // Allocate a new event record, then copy our temporary event over.
    //

    ThisEvent = AllocatePythonTraceEvent(Events);

    if (!ThisEvent) {
        return 0;
    }

    Rtl->CopyToMemoryMappedMemory(Rtl, ThisEvent, &Event, sizeof(Event));

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
    PTRACE_STORE FunctionsStore;
    PTRACE_STORE EventsStore;
    PTRACE_STORES TraceStores;

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

    if (!Python) {
        return FALSE;
    };

    if (!Rtl) {
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
    FunctionsStore = &TraceStores->Stores[TRACE_STORE_FUNCTIONS_INDEX];

    FunctionsStore->NoRetire = TRUE;

    Python->InitializePythonRuntimeTables(
        Python,
        TraceStoreAllocationRoutine,
        FunctionsStore,
        TraceStoreFreeRoutine,
        FunctionsStore
    );

    EventsStore = &TraceStores->Stores[TRACE_STORE_EVENTS_INDEX];

    EventsStore->NoPreferredAddressReuse = TRUE;

    Context->FirstFunction = NULL;
    QueryPerformanceFrequency(&Context->Frequency);

    Rtl->PfxInitialize(&Context->ModuleFilterTable);

    InitializeListHead(&Context->Functions);

    return TRUE;
}

BOOL
StartTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
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

    Python->PyEval_SetTrace(
        PythonTraceFunction,
        (PPYOBJECT)PythonTraceContext
    );

    return TRUE;
}

BOOL
StopTracing(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
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

    Python->PyEval_SetTrace(NULL, NULL);

    return TRUE;
}

BOOL
StartProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   PythonTraceContext
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

    Python->PyEval_SetProfile(
        PythonTraceFunction,
        (PPYOBJECT)PythonTraceContext
    );

    return TRUE;
}

BOOL
StopProfiling(
    _In_    PPYTHON_TRACE_CONTEXT   Context
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

    Context->StartedTracing = FALSE;

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

BOOL
AddPrefixTableEntry(
    _In_      PPYTHON_TRACE_CONTEXT   Context,
    _In_      PPYOBJECT               StringObject,
    _In_      PPREFIX_TABLE           PrefixTable,
    _Out_opt_ PPPREFIX_TABLE_ENTRY    EntryPointer
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

    Buffer = Python->AllocationRoutine(Python->AllocationContext, AllocSize);

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
        if (ARGUMENT_PRESENT(EntryPointer)) {
            *EntryPointer = Entry;
        }
    } else {

        //
        // Shouldn't be able to get here.
        //

        __debugbreak();
    }

    return TRUE;
}

BOOL
AddModuleName(
    _In_    PPYTHON_TRACE_CONTEXT   Context,
    _In_    PPYOBJECT               ModuleNameObject
    )
{
    if (!ARGUMENT_PRESENT(Context)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ModuleNameObject)) {
        return FALSE;
    }

    return AddPrefixTableEntry(Context,
                               ModuleNameObject,
                               &Context->ModuleFilterTable,
                               NULL);
}


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
