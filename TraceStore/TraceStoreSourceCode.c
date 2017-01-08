/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSourceCode.c

Abstract:

    This module implements functionality related to source code that
    participates in a tracing session.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
RegisterSourceCode(
    PTRACE_CONTEXT TraceContext,
    PTRACE_FILE TraceFile
    )
/*++

Routine Description:

    This routine registers a new source code (e.g. plaintext) file.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_STORE structure for which the
        number of memory maps required will be calculated.

    TraceFile - Supplies a pointer to a TRACE_FILE structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    //
    // Load the file, create bitmaps, etc.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
