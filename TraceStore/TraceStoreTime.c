/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreTime.c

Abstract:

    This module implements functionality related to trace store timing
    mechanisms.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeTraceStoreTime(
    PRTL Rtl,
    PTRACE_STORE_TIME Time
    )
/*++

Routine Description:

    This routine initializes a TRACE_STORE_TIME structure.  It is called when
    a trace store is being created, and captures the start time in numerous
    formats (file time and system time, both in UTC and local), as well as the
    performance counter frequency and initial performance counter value, which
    is used in subsequent high-precision microsecond timestamping logic (see
    TraceStoreQueryPerformanceCounter() for more information).

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.  This routine
        uses the Rtl->RtlTimeToSecondsSince1970() routine.

    Time - Supplies a pointer to a TRACE_STORE_TIME structure that is
        initialized by this routine.

Return Value:

    TRUE on success, FALSE on failure.  Failure can be caused by invalid
    arugments, or failure of the FileTimeToLocalFileTime() function or
    Rtl->RtlTimeToSecondsSince1970() function.

--*/
{
    BOOL Success;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Time)) {
        return FALSE;
    }

    //
    // Get the performance counter frequency and fill in the default
    // multiplicand that allows us to convert timestamp ticks to seconds.
    //

    QueryPerformanceFrequency(&Time->Frequency);
    Time->Multiplicand.QuadPart = TIMESTAMP_TO_SECONDS;

    //
    // Query the performance counter.  High-precision tick-based elapsed timings
    // can be made in the future by comparing the counter's value at this point
    // to the value at a subsequent point in the future.
    //

    QueryPerformanceCounter(&Time->StartTime.PerformanceCounter);

    //
    // Load the system time as file time, system time as UTC, system time local,
    // and file time UTC to local.
    //

    GetSystemTimeAsFileTime(&Time->StartTime.FileTimeUtc);
    GetSystemTime(&Time->StartTime.SystemTimeUtc);
    GetLocalTime(&Time->StartTime.SystemTimeLocal);

    Success = FileTimeToLocalFileTime(
        &Time->StartTime.FileTimeUtc,
        &Time->StartTime.FileTimeLocal
    );

    if (!Success) {
        return FALSE;
    }

    //
    // Various downstream libraries (such as NumPy, Pandas) have support for
    // 64-bit microsecond-precision timestamps against UNIX C time, so we
    // capture that time here now too.
    //

    Time->StartTime.SecondsSince1970.HighPart = 0;

    Success = Rtl->RtlTimeToSecondsSince1970(
        (PLARGE_INTEGER)&Time->StartTime.FileTimeLocal,
        &Time->StartTime.SecondsSince1970.LowPart
    );

    if (!Success) {
        return FALSE;
    }

    //
    // Convert to a microsecond representation.
    //

    Time->StartTime.MicrosecondsSince1970.QuadPart = (
        UInt32x32To64(
            Time->StartTime.SecondsSince1970.LowPart,
            SECONDS_TO_MICROSECONDS
        )
    );

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
