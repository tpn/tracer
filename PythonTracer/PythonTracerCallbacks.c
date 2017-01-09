/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    PythonTracerCallbacks.c

Abstract:

    This module contains all callbacks used by the PythonTracer component.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
CALLBACK
NewPythonPathTableEntryCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PPYTHON_TRACE_CONTEXT Context,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine is the callback target for the new Python path table entry
    event.

Arguments:

    Instance - Not used.

    Context - Supplies a pointer to a PYTHON_TRACE_CONTEXT structure.

    Work - Not used.

Return Value:

    None.

--*/
{
    PPYTHON_PATH_TABLE_ENTRY Entry;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Context)) {
        return;
    }


    if (!PopNewPathTableEntry(Context, &Entry)) {
        return;
    }

    InterlockedIncrement(&Context->ActiveNewPythonPathTableEntryCallbacks);

#ifdef _DEBUG
    OutputDebugStringA("NewPythonPathTableEntryCallback");
    OutputDebugStringA(Entry->Path.Buffer);
#endif

    InterlockedDecrement(&Context->ActiveNewPythonPathTableEntryCallbacks);

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
