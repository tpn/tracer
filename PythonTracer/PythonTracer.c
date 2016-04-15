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

VOID
TraceStoreFreeRoutine(
    _In_opt_ PVOID AllocationContext,
    _In_     PVOID Buffer
    )
{
    //
    // Trace stores don't currently have a free routine.
    //

    return;
}

LONG
PyTraceCallbackDummy(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
)
{
    return 0;
}

LONG
PyTraceCallbackBasic(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
)
{
    BOOL Success;
    PPYTHON Python;
    PPYOBJECT CodeObject;
    PTRACE_CONTEXT TraceContext;
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction;
    PTRACE_STORES TraceStores;
    PTRACE_STORE Events;
    PTRACE_EVENT EventRecord = NULL, LastEvent = NULL;
    TRACE_EVENT  Event = { 0 };
    ULARGE_INTEGER RecordSize = { sizeof(Event) };
    ULARGE_INTEGER NumberOfRecords = { 1 };

    if (!PythonTraceContext) {
        return 1;
    }

    Python = PythonTraceContext->Python;
    if (!Python) {
        return 1;
    }

    TraceContext = PythonTraceContext->TraceContext;
    if (!TraceContext) {
        return 1;
    }

    TraceStores = TraceContext->TraceStores;
    if (!TraceStores) {
        return 1;
    }

    Events = &TraceStores->Stores[0];

    if (!Events || !Events->AllocateRecords) {
        return 1;
    }

    Success = Python->ResolveFrameObjectDetails(Python,
                                                FrameObject,
                                                &CodeObject,
                                                (PPPYOBJECT)&Event.ModulePointer,
                                                (PPPYOBJECT)&Event.FuncPointer,
                                                (PULONG)&Event.LineNumber);

    if (!Success) {
        return 1;
    }

    Event.Version = 1;
    Event.EventType = (USHORT)EventType;
    Event.FramePointer = (ULONG_PTR)FrameObject;
    Event.ObjPointer = (ULONG_PTR)ArgObject;

    if (EventType == TraceEventType_PyTrace_LINE) {
        //
        // Get the actual line number if we're a trace event.
        //
        Event.LineNumber = Python->PyFrame_GetLineNumber(FrameObject);

        LastEvent = (PTRACE_EVENT)Events->PrevAddress;

        if (LastEvent &&
            LastEvent->EventType == TraceEventType_PyTrace_LINE &&
            LastEvent->LineNumber == Event.LineNumber &&
            LastEvent->FramePointer == (ULONG_PTR)FrameObject &&
            LastEvent->FuncPointer == (ULONG_PTR)Event.FuncPointer &&
            LastEvent->ModulePointer == (ULONG_PTR)Event.ModulePointer) {

            ++LastEvent->LineCount;
            Event.SequenceId = ++TraceContext->SequenceId;
            return 0;
        }
    }

    SystemTimerFunction = TraceContext->SystemTimerFunction;

    if (SystemTimerFunction->GetSystemTimePreciseAsFileTime) {
        SystemTimerFunction->GetSystemTimePreciseAsFileTime(&Event.ftTimeStamp);
    } else if (SystemTimerFunction->NtQuerySystemTime) {
        SystemTimerFunction->NtQuerySystemTime(&Event.liTimeStamp);
    }

#ifdef _M_X64
    Event.ProcessId = __readgsdword(0x40);
    Event.ThreadId = __readgsdword(0x48);
#elif _M_X86
    // 32-bit
    Event.ProcessId = __readgsdword(0x20);
    Event.ThreadId = __readgsdword(0x24);
#else
#error Unsupported architecture.
#endif

    switch (EventType) {
        case TraceEventType_PyTrace_CALL:
            break;
        case TraceEventType_PyTrace_EXCEPTION:
            break;
        case TraceEventType_PyTrace_LINE:
            Event.LineCount = 1;
            break;
        case TraceEventType_PyTrace_RETURN:
            break;
        case TraceEventType_PyTrace_C_CALL:
            break;
        case TraceEventType_PyTrace_C_EXCEPTION:
            break;
        case TraceEventType_PyTrace_C_RETURN:
            break;
    };

    EventRecord = (PTRACE_EVENT)Events->AllocateRecords(TraceContext, Events, &RecordSize, &NumberOfRecords);
    if (!EventRecord) {
        return 0;
    }

    Event.SequenceId = ++TraceContext->SequenceId;

    RtlCopyMemory(EventRecord, &Event, sizeof(Event));

    return 0;
}

LONG
PyTraceCallbackFast(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
)
{
    PRTL Rtl;
    PPYTHON Python;
    PPYOBJECT CodeObject;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    PTRACE_STORE Events;
    PTRACE_EVENT EventRecord = NULL, LastEvent = NULL;
    TRACE_EVENT  Event = { 0 };
    ULARGE_INTEGER RecordSize = { sizeof(Event) };
    ULARGE_INTEGER NumberOfRecords = { 1 };

    CodeObject = FrameObject->Code;

    if (PythonTraceContext->FunctionObject) {
        if (PythonTraceContext->FunctionObject->Code != CodeObject) {
            return 0;
        }
    }

    Rtl = PythonTraceContext->Rtl;
    TraceContext = PythonTraceContext->TraceContext;
    Python = PythonTraceContext->Python;
    TraceStores = TraceContext->TraceStores;

    Event.Version = 1;
    Event.EventType = (USHORT)EventType;
    Event.FramePointer = (ULONG_PTR)FrameObject;
    Event.ObjPointer = (ULONG_PTR)ArgObject;

    Event.ModulePointer = (ULONG_PTR)*(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->Filename
        )
    );

    Event.FuncPointer = (ULONG_PTR)*(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->Name
        )
    );

    Event.LineNumber = *(
        (PULONG)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->FirstLineNumber
        )
    );

    Events = &TraceStores->Stores[0];

    LastEvent = (PTRACE_EVENT)Events->PrevAddress;

    if (EventType == TraceEventType_PyTrace_LINE) {
        //
        // Get the actual line number if we're a trace event.
        //
        Event.LineNumber = Python->PyFrame_GetLineNumber(FrameObject);

        //
        // List and dict comprehensions register a line event for each iteration.
        // Rather than creating a separate event for each line trace, we increment
        // the line count of the previous event if it's present.
        //
        if (LastEvent &&
            LastEvent->EventType == TraceEventType_PyTrace_LINE &&
            LastEvent->LineNumber == Event.LineNumber &&
            LastEvent->FramePointer == (ULONG_PTR)FrameObject &&
            LastEvent->FuncPointer == (ULONG_PTR)Event.FuncPointer &&
            LastEvent->ModulePointer == (ULONG_PTR)Event.ModulePointer) {

            ++LastEvent->LineCount;
            Event.SequenceId = ++TraceContext->SequenceId;
            return 0;
        }
    }

    QueryPerformanceCounter(&Event.liTimeStamp);

#ifdef _M_X64
    Event.ProcessId = __readgsdword(0x40);
    Event.ThreadId = __readgsdword(0x48);
#elif _M_X86
    // 32-bit
    Event.ProcessId = __readgsdword(0x20);
    Event.ThreadId = __readgsdword(0x24);
#else
#error Unsupported architecture.
#endif

    switch (EventType) {
        case TraceEventType_PyTrace_CALL:
            break;
        case TraceEventType_PyTrace_EXCEPTION:
            break;
        case TraceEventType_PyTrace_LINE:
            Event.LineCount = 1;
            break;
        case TraceEventType_PyTrace_RETURN:
            break;
        case TraceEventType_PyTrace_C_CALL:
            break;
        case TraceEventType_PyTrace_C_EXCEPTION:
            break;
        case TraceEventType_PyTrace_C_RETURN:
            break;
    };

    EventRecord = (PTRACE_EVENT)Events->AllocateRecords(TraceContext, Events, &RecordSize, &NumberOfRecords);
    if (!EventRecord) {
        return 0;
    }

    Event.SequenceId = ++TraceContext->SequenceId;

    if (!Rtl->CopyToMemoryMappedMemory(EventRecord, &Event, sizeof(Event))) {
        ++Events->DroppedRecords;
    }

    return 0;
}

BOOL
PyTracePrepareTraceEvent(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
    )
{
    PPYOBJECT CodeObject = FrameObject->Code;
    PPYTHON Python = PythonTraceContext->Python;

    TraceEvent->Version = 1;
    TraceEvent->EventType = (USHORT)EventType;
    TraceEvent->FramePointer = (ULONG_PTR)FrameObject;
    TraceEvent->ObjPointer = (ULONG_PTR)ArgObject;

    CodeObject = FrameObject->Code;

    TraceEvent->ModulePointer = (ULONG_PTR)*(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->Filename
        )
    );

    TraceEvent->FuncPointer = (ULONG_PTR)*(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->Name
        )
    );

    TraceEvent->LineNumber = *(
        (PULONG)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->FirstLineNumber
        )
    );

    return TRUE;
}

VOID
PyTraceContinueTraceEvent(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    )
{
    PPYTHON Python = PythonTraceContext->Python;

    QueryPerformanceCounter(&TraceEvent->liTimeStamp);

    if (TraceEvent->EventType == TraceEventType_PyTrace_LINE) {
        //
        // Get the actual line number if we're tracing a line.
        //
        TraceEvent->LineNumber = Python->PyFrame_GetLineNumber(FrameObject);
    }

#ifdef _M_X64
    TraceEvent->ProcessId = __readgsdword(0x40);
    TraceEvent->ThreadId = __readgsdword(0x48);
#elif _M_X86
    // 32-bit
    TraceEvent->ProcessId = __readgsdword(0x20);
    TraceEvent->ThreadId = __readgsdword(0x24);
#else
#error Unsupported architecture.
#endif

}

LONG
PyTraceCallback(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _In_        PPYFRAMEOBJECT          FrameObject,
    _In_opt_    LONG                    EventType,
    _In_opt_    PPYOBJECT               ArgObject
)
{
    PRTL Rtl;
    PPYTHON Python;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    TRACE_EVENT  Event = { 0 };
    ULARGE_INTEGER RecordSize = { sizeof(Event) };
    ULARGE_INTEGER NumberOfRecords = { 1 };
    PPYTHON_TRACE_CONTEXT Context = PythonTraceContext;
    BOOLEAN Success;

    Rtl = Context->Rtl;
    TraceContext = Context->TraceContext;
    Python = Context->Python;
    TraceStores = TraceContext->TraceStores;

    Success = Context->PrepareTraceEvent(Context,
                                         &Event,
                                         FrameObject,
                                         EventType,
                                         ArgObject);

    if (!Success) {
        return 0;
    }

    switch (EventType) {
        case TraceEventType_PyTrace_CALL:

            Success = (
                ++Context->Depth > Context->SkipFrames &&
                Context->RegisterPythonFunction(Context, &Event, FrameObject)
            );

            if (Success) {
                Context->ContinueTraceEvent(Context, &Event, FrameObject);
                Context->TraceCall(Context, &Event, FrameObject);
            }
            break;

        case TraceEventType_PyTrace_EXCEPTION:
            break;

        case TraceEventType_PyTrace_LINE:
            Event.LineCount = 1;
            break;

        case TraceEventType_PyTrace_RETURN:

            Success = (
                Context->Depth &&
                --Context->Depth > Context->SkipFrames &&
                Context->RegisterPythonFunction(Context, &Event, FrameObject)
            );

            if (Success) {
                Context->ContinueTraceEvent(Context, &Event, FrameObject);
                Context->TraceReturn(Context, &Event, FrameObject);
            }

            break;

        case TraceEventType_PyTrace_C_CALL:
            break;
        case TraceEventType_PyTrace_C_EXCEPTION:
            break;
        case TraceEventType_PyTrace_C_RETURN:
            break;
    };

    Event.SequenceId = ++TraceContext->SequenceId;

    return 0;
}

BOOL
PyTraceRegisterPythonFunction(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
)
{
    PRTL Rtl;
    PPYTHON Python;
    PPYOBJECT CodeObject = FrameObject->Code;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;
    //PTRACE_STORE Events;
    PTRACE_EVENT EventRecord = NULL, LastEvent = NULL;
    TRACE_EVENT  Event = { 0 };
    PPYTHON_FUNCTION Function;
    PYTHON_FUNCTION FunctionRecord = { 0 };
    ULARGE_INTEGER EventRecordSize = { sizeof(Event) };
    ULARGE_INTEGER FunctionRecordSize = { sizeof(FunctionRecord) };
    ULARGE_INTEGER OneRecord = { 1 };
    BOOLEAN NewFunction = FALSE, NewModule = FALSE;
    PPYTHON_MODULE Module;
    PYTHON_MODULE ModuleRecord = { 0 };
    ULARGE_INTEGER ModuleRecordSize = { sizeof(ModuleRecord) };
    PTRACE_STORE Strings;
    BOOL Success = FALSE;
    PPYTHON_TRACE_CONTEXT Context = PythonTraceContext;
    PFUNCTIONS_TABLE FunctionsTable;
    PMODULES_TABLE ModulesTable;

    CodeObject = FrameObject->Code;

    Python = PythonTraceContext->Python;

    //
    // Make sure we've been passed a Python code object.
    //

    if (CodeObject->TypeObject != Python->PyCode_Type) {
        return FALSE;
    }

    Rtl = PythonTraceContext->Rtl;
    TraceContext = PythonTraceContext->TraceContext;
    TraceStores = TraceContext->TraceStores;

    FunctionRecord.CodeObject = CodeObject;

    //
    // Insert the function into our table if it's not already present.
    //

    FunctionsTable = &Context->FunctionsTable;

    Function = Rtl->RtlInsertElementGenericTable(
        FunctionsTable,
        &FunctionRecord,
        FunctionRecordSize.LowPart,
        &NewFunction
        );

    if (!NewFunction) {

        //
        // We've already seen this function before, no need to do anything else.
        //

        return TRUE;
    }

    //
    // We haven't seen this function before.  Determine if we've seen the
    // module filename.
    //

    ModuleRecord.ModuleFilenameObject = *(
        (PPPYOBJECT)RtlOffsetToPointer(
            CodeObject,
            Python->PyCodeObjectOffsets->Filename
        )
    );

    ModulesTable = &Context->ModulesTable;

    Module = Rtl->RtlInsertElementGenericTable(
        ModulesTable,
        &ModuleRecord,
        ModuleRecordSize.LowPart,
        &NewModule
    );

    if (!NewModule) {

        //
        // We've already seen this module before.  Fill in the function details
        // accordingly.
        //
        Function->Module = Module;

    } else {

        //Python->Py_IncRef(Module->ModuleFilenameObject);

        //
        // This is the first time we've seen this module.  We need to save
        // UNICODE_STRING instances of the qualified path and the module
        // name.
        //
        Strings = &TraceStores->Stores[TRACE_STORE_STRINGS_INDEX];

        Success = Python->GetModuleNameAndQualifiedPathFromModuleFilename(
            Python,
            Module->ModuleFilenameObject,
            &Module->Path,
            &Module->Name,
            TraceStoreAllocationRoutine,
            Strings,
            TraceStoreFreeRoutine,
            NULL
            );

        if (!Success) {

            //
            // Remove the function from the table and return.  (This may not
            // be the best way to handle this -- not sure what circumstances
            // we're going to fail to copy a Python string.)
            //

            Python->Py_DecRef(Module->ModuleFilenameObject);
            Rtl->RtlDeleteElementGenericTable(ModulesTable, Module);
            Rtl->RtlDeleteElementGenericTable(FunctionsTable, Function);
            return 0;
        }

        //Success = Rtl->EnsureQualifiedPath(


    }

    return FALSE;
}

VOID
PyTraceCall(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    )
{
    return;
}

VOID
PyTraceLine(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    )
{
    return;
}

VOID
PyTraceReturn(
    _In_        PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_     PTRACE_EVENT            TraceEvent,
    _In_        PPYFRAMEOBJECT          FrameObject
    )
{
    return;
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

RTL_GENERIC_COMPARE_RESULTS
NTAPI
CodeObjectCompare(
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

BOOL
InitializePythonTraceContext(
    _In_                                        PRTL                    Rtl,
    _Out_bytecap_(*SizeOfPythonTraceContext)    PPYTHON_TRACE_CONTEXT   PythonTraceContext,
    _Inout_                                     PULONG                  SizeOfPythonTraceContext,
    _In_                                        PPYTHON                 Python,
    _In_                                        PTRACE_CONTEXT          TraceContext,
    _In_opt_                                    PPYTRACEFUNC            PythonTraceFunction,
    _In_opt_                                    PVOID                   UserData
)
{
    if (!PythonTraceContext) {
        if (SizeOfPythonTraceContext) {
            *SizeOfPythonTraceContext = sizeof(*PythonTraceContext);
        }
        return FALSE;
    }

    if (!SizeOfPythonTraceContext) {
        return FALSE;
    }

    if (*SizeOfPythonTraceContext < sizeof(*PythonTraceContext)) {
        *SizeOfPythonTraceContext = sizeof(*PythonTraceContext);
        return FALSE;
    }

    if (!Python) {
        return FALSE;
    };

    if (!Rtl) {
        return FALSE;
    }

    //PythonTraceContext->CodeObjectsHeap = HeapCreate(HEAP_NO_SERIALIZE | HEAP_GENERATE_EXCEPTIONS, 0, 0);
    //if (!PythonTraceContext->CodeObjectsHeap) {
    //    return FALSE;
    //}

    PythonTraceContext->Size = *SizeOfPythonTraceContext;
    PythonTraceContext->Rtl = Rtl;
    PythonTraceContext->Python = Python;
    PythonTraceContext->TraceContext = TraceContext;
    PythonTraceContext->PythonTraceFunction = PythonTraceFunction;
    PythonTraceContext->UserData = UserData;

    if (!PythonTraceContext->PythonTraceFunction) {
        PythonTraceContext->PythonTraceFunction = (PPYTRACEFUNC)PyTraceCallback;
    }

    PythonTraceContext->PrepareTraceEvent = (PPREPARE_TRACE_EVENT)PyTracePrepareTraceEvent;
    PythonTraceContext->ContinueTraceEvent = (PCONTINUE_TRACE_EVENT)PyTraceContinueTraceEvent;

    PythonTraceContext->RegisterPythonFunction = (PREGISTER_PYTHON_FUNCTION)PyTraceRegisterPythonFunction;

    PythonTraceContext->TraceCall = (PPYTHON_TRACE_CALL)PyTraceCall;
    PythonTraceContext->TraceLine = (PPYTHON_TRACE_LINE)PyTraceLine;
    PythonTraceContext->TraceReturn = (PPYTHON_TRACE_RETURN)PyTraceReturn;

    PythonTraceContext->SkipFrames = 1;

    Rtl->RtlInitializeGenericTable(
        &PythonTraceContext->FunctionsTable,
        CodeObjectCompare,
        CodeObjectAllocateFromHeap,
        CodeObjectFreeFromHeap,
        PythonTraceContext
    );

    Rtl->RtlInitializeGenericTable(
        &PythonTraceContext->ModulesTable,
        CodeObjectCompare,
        CodeObjectAllocateFromHeap,
        CodeObjectFreeFromHeap,
        PythonTraceContext
    );

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

    Python->PyEval_SetTrace(PythonTraceFunction, (PPYOBJECT)PythonTraceContext);

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

    Python->PyEval_SetProfile(PythonTraceFunction, (PPYOBJECT)PythonTraceContext);

    return TRUE;
}

BOOL
StopProfiling(
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

    PythonTraceContext->FunctionObject = (PPYFUNCTIONOBJECT)FunctionObject;

    return TRUE;
}

#ifdef __cplusplus
} // extern "C"
#endif
