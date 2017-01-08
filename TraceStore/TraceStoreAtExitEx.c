/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreAtExitEx.c

Abstract:

    This module implements the "at exit" routine for the trace store component
    that will automatically be called as the final step in process shutdown.
    A routine is provided to save various metrics about the trace session to
    the registry.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
TraceStoreAtExitEx(
    BOOL IsProcessTerminating,
    PVOID ContextPointer
    )
/*++

Routine Description:

    This routine is responsible for writing runtime information about a trace
    session, such as working set final buffer sizes, and timer contention, to
    the registry on process exit.

    It is called by the Rtl AtExitEx rundown functionality.

Arguments:

    IsProcessTerminating - Supplies a boolean value that indicates whether or
        not the process is terminating.  If FALSE, indicates that the library
        has been unloaded via FreeLibrary().

    ContextPointer - Supplies a pointer to the PTRACE_CONTEXT structure that
        was registered when AtExitEx() was called.

Return Value:

    None.

--*/
{
    HKEY RegistryKey;
    PTRACE_CONTEXT Context = (PTRACE_CONTEXT)ContextPointer;

    //
    // Validate arguments.
    //

    if (!Context) {
        return;
    }

    //
    // Load the registry key and flags.
    //

    RegistryKey = Context->RunHistoryRegistryKey;
    if (!RegistryKey) {
        return;
    }

    //
    // Define convenience macros.
    //

#define WRITE_ULONG(Name)              \
    WRITE_REG_DWORD(RegistryKey,       \
                    Name,              \
                    Context->##Name##)

    if (Context->GetWorkingSetChangesTimer) {
        WRITE_ULONG(WorkingSetTimerContention);
        WRITE_ULONG(WsWatchInfoExInitialBufferNumberOfElements);
        WRITE_ULONG(WsWatchInfoExCurrentBufferNumberOfElements);
        WRITE_ULONG(GetWorkingSetChangesIntervalInMilliseconds);
        WRITE_ULONG(GetWorkingSetChangesWindowLengthInMilliseconds);
    }

    if (Context->CapturePerformanceMetricsTimer) {
        WRITE_ULONG(CapturePerformanceMetricsTimerContention);
        WRITE_ULONG(CapturePerformanceMetricsIntervalInMilliseconds);
        WRITE_ULONG(CapturePerformanceMetricsWindowLengthInMilliseconds);
    }

    WRITE_ULONG(NumberOfStoresWithMultipleRelocationDependencies);

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
