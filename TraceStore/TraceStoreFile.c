/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreFile.c

Abstract:

    This module implements functionality related to TRACE_FILE structures.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
RegisterTraceFile(
    PTRACE_CONTEXT TraceContext,
    PTRACE_FILE TraceFile
    )
/*++

Routine Description:

    This routine registers a new trace file.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_STORE structure for which the
        number of memory maps required will be calculated.

    TraceFile - Supplies a pointer to a TRACE_FILE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    //
    // Determine what type of file this is (source code or image file), then
    // dispatch the relevant threadpool work.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
