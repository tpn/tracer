/*++

Copyright (c) 2015-2017 Trent Nelson <trent@trent.me>.  All Rights Reserved.

Module Name:

    TraceStoreIntervals.c

Abstract:

    This module implements trace store interval functionality.  Functions
    are provided for preparing intervals for trace stores.

--*/

#include "stdafx.h"

typedef struct _TRACE_STORE_INTERVAL {
    ULONGLONG IntervalNumber;
    ULONGLONG RecordNumber;
    ULONGLONG AllocationTimestamp;
    ULONGLONG Address;
} TRACE_STORE_INTERVAL;
typedef TRACE_STORE_INTERVAL *PTRACE_STORE_INTERVAL;

typedef struct _TRACE_STORE_INTERVALS {
    ULONGLONG FramesPerSecond;
    ULONGLONG IntervalInNanoseconds;
    ULONGLONG TicksPerInterval;
    ULONGLONG NumberOfIntervals;
    TRACE_STORE_INTERVAL Interval[ANYSIZE_ARRAY];
} TRACE_STORE_INTERVALS;
typedef TRACE_STORE_INTERVALS *PTRACE_STORE_INTERVALS;

_Use_decl_annotations_
BOOL
PrepareTraceStoreIntervals(
    PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    WIP.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which
        intervals are to be prepared.

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    TRACE_STORE_INTERVALS Intervals;

    ZeroStruct(Intervals);

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
