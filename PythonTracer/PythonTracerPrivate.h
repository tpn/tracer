/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerPrivate.h

Abstract:

    This is the private header file for the PythonTracer component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#ifndef _PYTHON_TRACER_INTERNAL_BUILD
#error PythonTracerPrivate.h being included but _PYTHON_TRACER_INTERNAL_BUILD
#error not set.
#endif

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Registry-related strings.
//

#define ROOT_REGISTRY_PATH L"Software\\Tracer\\PythonTracer"

#define RUN_HISTORY_DATE_FORMAT L"YYYY-MM-DD_hhmmss.SSS"

#define RUN_HISTORY_REGISTRY_PATH_PREFIX          \
    L"Software\\Tracer\\PythonTracer\\RunHistory"

#define RUN_HISTORY_REGISTRY_PATH_FORMAT \
    RUN_HISTORY_REGISTRY_PATH_PREFIX     \
    L"\\"                                \
    RUN_HISTORY_DATE_FORMAT


//
// AtExitEx-related definitions.
//

ATEXITEX_CALLBACK SaveMaxRefCountsAtExit;
ATEXITEX_CALLBACK SaveCountsToRunHistoryAtExit;
ATEXITEX_CALLBACK SavePerformanceMetricsAtExit;

//
// CallbackWorker-related functions.
//

typedef enum _PYTHON_TRACE_CALLBACK_WORKER_TYPE {
    PythonTraceCallbackWorkerNull = 0,
    PythonTraceCallbackWorker1 = 1,
    PythonTraceCallbackWorker2,
    PythonTraceCallbackWorkerInvalid
} PYTHON_TRACE_CALLBACK_WORKER_TYPE, *PPYTHON_TRACE_CALLBACK_WORKER_TYPE;

PY_TRACE_CALLBACK PyTraceCallbackWorker1;
PY_TRACE_CALLBACK PyTraceCallbackWorker2;

FORCEINLINE
_Check_return_
_Success_(return != 0)
PPY_TRACE_EVENT
GetFunctionPointerForTraceEventType(
    _In_ PPYTHON_TRACE_CONTEXT Context
    )
{
    PYTHON_TRACE_EVENT_TYPE TraceEventType;

    TraceEventType = Context->RuntimeParameters.TraceEventType;

    if (TraceEventType == PythonTraceEventNull ||
        TraceEventType >= PythonTraceEventInvalid) {
        OutputDebugStringA("Invalid PYTHON_TRACE_EVENT_TYPE.\n");
        return NULL;
    }

    return PythonTraceEventTypeToFunctionPointer[TraceEventType-1];
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
PPY_TRACE_CALLBACK
GetFunctionPointerForCallbackWorkerType(
    _In_ PPYTHON_TRACE_CONTEXT Context
    )
{
    PYTHON_TRACE_CALLBACK_WORKER_TYPE WorkerType;

    WorkerType = Context->RuntimeParameters.CallbackWorkerType;

    if (WorkerType == PythonTraceCallbackWorkerNull ||
        WorkerType >= PythonTraceCallbackWorkerInvalid) {
        OutputDebugStringA("Invalid PYTHON_TRACE_CALLBACK_WORKER_TYPE.\n");
        return NULL;
    }

    return PythonTraceCallbackWorkerTypeToFunctionPointer[WorkerType-1];
}

//
// PythonPathEntry-related functions.
//

REGISTER_NEW_PATH_ENTRY PyTraceRegisterNewPathEntry;

FORCEINLINE
VOID
PushPythonPathTableEntry(
    _In_ PSLIST_HEADER ListHead,
    _In_ PPYTHON_PATH_TABLE_ENTRY Entry
    )
{
    InterlockedPushEntrySList(ListHead, &Entry->File.ListEntry);
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PopPythonPathTableEntry(
    _In_ PSLIST_HEADER ListHead,
    _Out_ PPPYTHON_PATH_TABLE_ENTRY EntryPointer
    )
{
    PSLIST_ENTRY ListEntry;
    PPYTHON_PATH_TABLE_ENTRY PathTableEntry;

    ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    PathTableEntry = CONTAINING_RECORD(ListEntry,
                                       PYTHON_PATH_TABLE_ENTRY,
                                       File.ListEntry);

    *EntryPointer = PathTableEntry;
    return TRUE;
}

#define PushNewPythonPathTableEntry(Context, Entry) \
    PushPythonPathTableEntry(                       \
        &Context->NewPythonPathTableEntryListHead,  \
        Entry                                       \
    )

#define SubmitNewPythonPathTableEntryWork(Context, Entry)       \
    PushNewPythonPathTableEntry(Context, Entry);                \
    SubmitThreadpoolWork(Context->NewPythonPathTableEntryWork);

#define PopNewPathTableEntry(Context, Entry)       \
    PopPythonPathTableEntry(                       \
        &Context->NewPythonPathTableEntryListHead, \
        Entry                                      \
    )

typedef
_Check_return_
_Success_(return != 0)
BOOL
(NEW_PYTHON_PATH_TABLE_ENTRY)(
    _In_ PPYTHON_TRACE_CONTEXT Context,
    _In_ PPYTHON_PATH_TABLE_ENTRY Entry
    );
typedef NEW_PYTHON_PATH_TABLE_ENTRY *PNEW_PYTHON_PATH_TABLE_ENTRY;
NEW_PYTHON_PATH_TABLE_ENTRY NewPythonPathTableEntry;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(UPDATE_RTL_PATH_FROM_PATH_ENTRY)(
    _In_ PPYTHON_TRACE_CONTEXT Context,
    _In_ PPYTHON_PATH_TABLE_ENTRY Entry
    );
typedef UPDATE_RTL_PATH_FROM_PATH_ENTRY *PUPDATE_RTL_PATH_FROM_PATH_ENTRY;
UPDATE_RTL_PATH_FROM_PATH_ENTRY UpdateRtlPathFromPathEntry;

//
// PythonTracerCallback-related functions.
//

typedef
VOID
(CALLBACK NEW_PYTHON_PATH_TABLE_ENTRY_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PPYTHON_TRACE_CONTEXT Context,
    _In_     PTP_WORK Work
    );
typedef NEW_PYTHON_PATH_TABLE_ENTRY_CALLBACK \
      *PNEW_PYTHON_PATH_TABLE_ENTRY_CALLBACK;
NEW_PYTHON_PATH_TABLE_ENTRY_CALLBACK NewPythonPathTableEntryCallback;

////////////////////////////////////////////////////////////////////////////////
// Inline functions.
////////////////////////////////////////////////////////////////////////////////

FORCEINLINE
BOOL
IsFunctionOfInterestStringTable(
    _In_    PRTL                    Rtl,
    _In_    PPYTHON_TRACE_CONTEXT   Context,
    _In_    PPYTHON_FUNCTION        Function
    )
{
    PSTRING ModuleName;
    PSTRING_TABLE StringTable = Context->ModuleFilterStringTable;
    STRING_TABLE_INDEX Index;
    PSTRING_TABLE_API StringTableApi;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

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

    if (!StringTable || !ModuleName || ModuleName->Length <= 1) {
        return FALSE;
    }

    StringTableApi = Context->StringTableApi;
    IsPrefixOfStringInTable = StringTableApi->IsPrefixOfStringInTable;
    Index = IsPrefixOfStringInTable(StringTable, ModuleName, NULL);

    return (Index != NO_MATCH_FOUND);
}

#define IsFunctionOfInterest IsFunctionOfInterestStringTable

FORCEINLINE
VOID
InitializeEventTraits(
    _In_ LONG EventType,
    _Out_ PPYTHON_EVENT_TRAITS EventTraitsPointer
    )
{
    BOOL IsCall;
    BOOL IsReturn;
    BOOL IsLine;
    BOOL IsException;
    BOOL IsC;
    PYTHON_EVENT_TRAITS EventTraits;

    IsCall = (
        EventType == TraceEventType_PyTrace_CALL        ||
        EventType == TraceEventType_PyTrace_C_CALL
    );

    IsException = (
        EventType == TraceEventType_PyTrace_EXCEPTION   ||
        EventType == TraceEventType_PyTrace_C_EXCEPTION
    );

    IsLine = (
        EventType == TraceEventType_PyTrace_LINE
    );

    IsReturn = (
        EventType == TraceEventType_PyTrace_RETURN      ||
        EventType == TraceEventType_PyTrace_C_RETURN
    );

    IsC = (
        EventType == TraceEventType_PyTrace_C_CALL      ||
        EventType == TraceEventType_PyTrace_C_RETURN    ||
        EventType == TraceEventType_PyTrace_C_EXCEPTION
    );

    EventTraits.IsCall = IsCall;
    EventTraits.IsException = IsException;
    EventTraits.IsLine = IsLine;
    EventTraits.IsReturn = IsReturn;
    EventTraits.IsC = IsC;
    EventTraits.AsEventType = (BYTE)EventType;

    EventTraitsPointer->AsByte = EventTraits.AsByte;
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
InitializePythonTraceEvent(
    _In_ PPYTHON_TRACE_CONTEXT Context,
    _In_ LONG EventType,
    _Out_ PPYTHON_EVENT_TRAITS EventTraitsPointer
    )
{
    PYTHON_EVENT_TRAITS EventTraits;

    InitializeEventTraits(EventType, EventTraitsPointer);
    EventTraits.AsByte = EventTraitsPointer->AsByte;

    if (!Context->RuntimeState.HasStarted) {

        if (!EventTraits.IsCall) {

            //
            // If we haven't started profiling yet, we can ignore any event that
            // isn't a call event.  (In practice, there will usually be one
            // return event/frame before we get a call event we're interested
            // in.)
            //

            return TRUE;
        }

        //
        // We've received our first profile/trace event of interest.  Toggle
        // our 'HasStarted' flag and set our context depth to 1.
        //

        Context->RuntimeState.HasStarted = TRUE;
        Context->Depth = 1;

    } else {

        //
        // We're already tracing/profiling, so just update our depth counter
        // accordingly if we're a call/return.
        //

        if (EventTraits.IsCall) {
            Context->Depth++;
        } else if (EventTraits.IsReturn) {
            Context->Depth--;
        }
    }

    //
    // Abuse the fact we can index into our counter array using the event type
    // directly.
    //

    if (Context->Flags.CountEvents) {
        ++(Context->Counters[EventTraits.AsEventType]);
    }

    //
    // If we've been configured to track maximum reference counts, do that now.
    //

    if (Context->Flags.TrackMaxRefCounts) {
        UpdateMaxRefCounts(Context);
    }

    //
    // Update our maximum depth, if applicable.
    //

    if (Context->Depth > Context->MaxDepth.QuadPart) {
        Context->MaxDepth.QuadPart = Context->Depth;
        if (Context->MaxDepth.HighPart) {
            __debugbreak();
        }
    }

    return TRUE;
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
CopyPythonTraceEvent1(
    _Out_ PPYTHON_TRACE_EVENT1 DestEvent,
    _In_ _Const_ PPYTHON_TRACE_EVENT1 SourceEvent
    )
/*++

Routine Description:

    This is a helper routine that can be used to safely copy a trace event
    structure when either the source or destination is backed by memory
    mapped memory.  Internally, it is simply a __movsq() wrapped in a
    __try/__except block that catches STATUS_IN_PAGE_ERROR exceptions.

Arguments:

    DestEvent - Supplies a pointer to the destination event to which the
        source event will be copied.

    SourceAddress - Supplies a pointer to the source event to copy into
        the destination event.

Return Value:

    TRUE on success, FALSE if a STATUS_IN_PAGE_ERROR occurred.

--*/
{
    TRY_MAPPED_MEMORY_OP {
        __movsq((PDWORD64)DestEvent,
                (PDWORD64)SourceEvent,
                sizeof(*DestEvent) >> 3);
        return TRUE;
    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }
}

//
// Registry-related typedefs and inline functions.
//

typedef enum _PYTHON_TRACER_REGISTRY_KEY_TYPE {
    RootRegistryKey = 0,
    LastRunRegistryKey,
    InvalidRegistryKey
} PYTHON_TRACER_REGISTRY_KEY_TYPE;

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
OpenRegistryKey(
    _In_ PCUNICODE_STRING RegistryPath,
    _Out_ PHKEY RegistryKey
    )
{
    ULONG Result;

    Result = RegCreateKeyExW(
        HKEY_CURRENT_USER,
        RegistryPath->Buffer,
        0,          // Reserved
        NULL,       // Class
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        RegistryKey,
        NULL
    );

    if (Result != ERROR_SUCCESS) {
        OutputDebugStringW(L"PythonTracer!RegCreateKeyExW() failed for: ");
        OutputDebugStringW(RegistryPath->Buffer);
        return FALSE;
    }

    return TRUE;
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
OpenRootRegistryKey(
    _Out_ PHKEY RegistryKey
    )
{
    return OpenRegistryKey(&RootRegistryPath, RegistryKey);
}

/*++

    VOID
    READ_REG_DWORD_FLAG(
        Name,
        Default
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry
    into a PYTHON_TRACE_CONTEXT structure (implicitly referred to as 'Context').
    If the registry key isn't present, an attempt will be made to write the
    default value.

Arguments:

    Name - Name of the flag to read (e.g. ProfileOnly).  The macro
        resolves this to Context->Flags.Name (e.g. Context->Flags.ProfileOnly).

    Default - Default value to assign to the flag (Context->Flags.Name) if the
        registry key couldn't be read successfully (because it was not present,
        or was an incorrect type).

Return Value:

    None.

--*/
#define READ_REG_DWORD_FLAG(Name, Default) do { \
    ULONG Value;                                \
    ULONG ValueLength = sizeof(Value);          \
    Result = RegGetValueW(                      \
        RegistryKey,                            \
        NULL,                                   \
        L#Name,                                 \
        RRF_RT_REG_DWORD,                       \
        NULL,                                   \
        (PVOID)&Value,                          \
        &ValueLength                            \
    );                                          \
    if (Result == ERROR_SUCCESS) {              \
        Context->Flags.Name = Value;            \
    } else {                                    \
        Value = Context->Flags.Name = Default;  \
        RegSetValueExW(                         \
            RegistryKey,                        \
            L#Name,                             \
            0,                                  \
            REG_DWORD,                          \
            (const BYTE*)&Value,                \
            ValueLength                         \
        );                                      \
    }                                           \
} while (0)

/*++

    VOID
    READ_REG_DWORD_RUNTIME_PARAM(
        Name,
        Default
        );

Routine Description:

    This is a helper macro for reading REG_DWORD values from the registry
    into a PYTHON_TRACER_RUNTIME_PARAMETERS structure.  If the registry key
    isn't present, an attempt will be made to write the default value.

Arguments:

    Name - Name of the parameter to read.

    Default - Default value to assign to the parameter if the registry key
        couldn't be read successfully (because it was not present, or was an
        incorrect type).

Return Value:

    None.

--*/
#define READ_REG_DWORD_RUNTIME_PARAM(Name, Default) do {   \
    ULONG Value;                                           \
    ULONG ValueLength = sizeof(Value);                     \
    Result = RegGetValueW(                                 \
        RegistryKey,                                       \
        NULL,                                              \
        L#Name,                                            \
        RRF_RT_REG_DWORD,                                  \
        NULL,                                              \
        (PVOID)&Value,                                     \
        &ValueLength                                       \
    );                                                     \
    if (Result == ERROR_SUCCESS) {                         \
        Context->RuntimeParameters.Name = Value;           \
    } else {                                               \
        Value = Context->RuntimeParameters.Name = Default; \
        RegSetValueExW(                                    \
            RegistryKey,                                   \
            L#Name,                                        \
            0,                                             \
            REG_DWORD,                                     \
            (const BYTE*)&Value,                           \
            ValueLength                                    \
        );                                                 \
    }                                                      \
} while (0)

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
