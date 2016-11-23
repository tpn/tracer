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

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;

    if (!Context->Flags.HasModuleFilter) {

        //
        // Trace everything.
        //

        return TRUE;
    }

    ModuleName = &Function->PathEntry.ModuleName;

    if (!StringTable || !ModuleName || ModuleName->Length <= 1) {
        return FALSE;
    }

    IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;
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

    if (!Context->Flags.HasStarted) {

        if (!EventTraits.IsCall) {

            //
            // If we haven't started profiling yet, we can ignore any
            // event that isn't a call event.  (In practice, there will
            // usually be one return event/frame before we get a call
            // event we're interested in.)
            //

            return FALSE;
        }

        //
        // We've received our first profile/trace event of interest.  Toggle
        // our 'HasStarted' flag and set our context depth to 1.
        //

        Context->Flags.HasStarted = TRUE;
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

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
