/*++

Copyright (c) 2015-2017 Trent Nelson <trent@trent.me>.  All Rights Reserved.

Module Name:

    TraceStoreIntervals.c

Abstract:

    This module implements trace store interval functionality.  Functions
    are provided for preparing intervals for trace stores.

--*/

#include "stdafx.h"

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
    BOOL Exclude;
    BOOL Success;
    DOUBLE NumberOfIntervals;
    ULONGLONG StartTicks;
    ULONGLONG EndTicks;
    ULONGLONG ElapsedTicks;
    TRACE_STORE_TRAITS Traits;
    PTRACE_STORE_TOTALS Totals;
    ULONGLONG NumberOfAllocations;
    ULONGLONG RecordSizeInBytes;
    ULONGLONG StartAddress;
    ULONGLONG EndAddress;
    ULONGLONG LastRecordOffset;
    PTRACE_STORE_INTERVALS Intervals;
    PTRACE_STORE AllocationTimestampStore;
    SIZE_T AllocSizeInBytes;
    SIZE_T PageAlignedAllocSizeInBytes;

    Intervals = &TraceStore->Intervals;

    Traits = *TraceStore->pTraits;

    Exclude = (
        !IsFixedRecordSize(Traits)          ||
        !IsRecordSizeAlwaysPowerOf2(Traits) ||
        IsLinkedStore(Traits)
    );

    if (Exclude) {
        Success = FALSE;
        goto End;
    }

    Totals = TraceStore->Totals;
    NumberOfAllocations = Totals->NumberOfAllocations.QuadPart;
    if (NumberOfAllocations == 0) {
        Success = FALSE;
        goto End;
    }

    RecordSizeInBytes = (
        Totals->RecordSize.QuadPart /
        Totals->NumberOfRecords.QuadPart
    );

    if (!IsPowerOf2(RecordSizeInBytes)) {
        __debugbreak();
        Success = FALSE;
        goto End;
    }

    if (RecordSizeInBytes == 0) {
        __debugbreak();
        Success = FALSE;
        goto End;
    }

    Intervals->FramesPerSecond = (DOUBLE)TraceStore->IntervalFramesPerSecond;
    Intervals->Frequency = TraceStore->Time->Frequency.QuadPart;
    Intervals->TicksPerIntervalAsDouble = (
        ((DOUBLE)1.0 / Intervals->FramesPerSecond) /
        ((DOUBLE)1.0 / (DOUBLE)Intervals->Frequency)
    );

    Intervals->TicksPerInterval = (ULONGLONG)(
        Intervals->TicksPerIntervalAsDouble
    );

    Intervals->NumberOfRecords = NumberOfAllocations;
    Intervals->RecordSizeInBytes = RecordSizeInBytes;

    AllocationTimestampStore = TraceStore->AllocationTimestampStore;

    if (AllocationTimestampStore->Totals->NumberOfAllocations.QuadPart !=
        NumberOfAllocations) {
        __debugbreak();
        Success = FALSE;
        goto End;
    }

    Intervals->FirstAllocationTimestamp = (PULONGLONG)(
        AllocationTimestampStore->MemoryMap->BaseAddress
    );

    Intervals->LastAllocationTimestamp = (
        (PULONGLONG)(
            RtlOffsetToPointer(
                Intervals->FirstAllocationTimestamp,
                (NumberOfAllocations - 1) * sizeof(ULONGLONG)
            )
        )
    );

    StartTicks = *Intervals->FirstAllocationTimestamp;
    EndTicks = *Intervals->LastAllocationTimestamp;
    ElapsedTicks = EndTicks - StartTicks;

    NumberOfIntervals = (
        (DOUBLE)ElapsedTicks /
        Intervals->TicksPerIntervalAsDouble
    );
    Intervals->NumberOfIntervals = (ULONGLONG)NumberOfIntervals;

    LastRecordOffset = (NumberOfAllocations - 1) * RecordSizeInBytes;
    StartAddress = (ULONGLONG)TraceStore->FlatMemoryMap.BaseAddress;
    EndAddress = (ULONGLONG)StartAddress + LastRecordOffset;
    Intervals->FirstRecordAddress = StartAddress;
    Intervals->LastRecordAddress = EndAddress;

    AllocSizeInBytes = (
        ALIGN_UP_POINTER(Intervals->NumberOfIntervals) *
        sizeof(TRACE_STORE_INTERVAL)
    );

    PageAlignedAllocSizeInBytes = ROUND_TO_PAGES(AllocSizeInBytes);

    Intervals->FirstInterval = (PTRACE_STORE_INTERVAL)(
        VirtualAlloc(NULL,
                     PageAlignedAllocSizeInBytes,
                     MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE)
    );

    if (!Intervals->FirstInterval) {
        Success = FALSE;
        goto End;
    }

    Success = ExtractIntervals(TraceStore->Rtl, Intervals);

    //
    // Intentional follow-on to End.
    //

End:

    SetEvent(TraceStore->Intervals.LoadingCompleteEvent);

    return Success;
}

_Use_decl_annotations_
BOOL
ExtractIntervals(
    PRTL Rtl,
    PTRACE_STORE_INTERVALS Intervals
    )
/*++

Routine Description:

    WIP.

Arguments:

    WIP

Return Value:

    TRUE on success, FALSE otherwise.

--*/
{
    ULONGLONG Address;
    ULONGLONG NextTimestamp;
    ULONGLONG RecordIndex;
    ULONGLONG IntervalIndex;
    ULONGLONG RecordSizeInBytes;
    ULONGLONG TicksPerInterval;
    LARGE_INTEGER Start;
    LARGE_INTEGER End;
    LARGE_INTEGER Elapsed;
    PULONGLONG AllocationTimestamp;
    PULONGLONG LastAllocationTimestamp;
    PTRACE_STORE_INTERVAL Interval;

    //
    // Initialize local variables and aliases.
    //

    Interval = Intervals->FirstInterval;
    Address = Intervals->FirstRecordAddress;
    TicksPerInterval = Intervals->TicksPerInterval;
    RecordSizeInBytes = Intervals->RecordSizeInBytes;
    AllocationTimestamp = Intervals->FirstAllocationTimestamp;
    LastAllocationTimestamp = Intervals->LastAllocationTimestamp;
    NextTimestamp = *Intervals->FirstAllocationTimestamp + TicksPerInterval;

    //
    // Wire up the first interval.
    //

    Interval->IntervalIndex = IntervalIndex = 0;
    Interval->RecordIndex = RecordIndex = 0;
    Interval->AllocationTimestamp = *AllocationTimestamp;
    Interval->Address = Address;
    Interval++;

    NextTimestamp = *AllocationTimestamp + TicksPerInterval;

    QueryPerformanceCounter(&Start);

    //
    // Stream through the allocation timestamps and wire up interval records
    // each time we detect a new interval.
    //

    while (++AllocationTimestamp <= LastAllocationTimestamp) {
        ++RecordIndex;
        if (*AllocationTimestamp >= NextTimestamp) {
            Interval->IntervalIndex = ++IntervalIndex;
            Interval->RecordIndex = RecordIndex;
            Interval->AllocationTimestamp = *AllocationTimestamp;
            Interval->Address = (ULONGLONG)(
                RtlOffsetToPointer(
                    Address,
                    RecordIndex * RecordSizeInBytes
                )
            );
            NextTimestamp = *AllocationTimestamp + TicksPerInterval;
            Interval++;
        }
    }

    QueryPerformanceCounter(&End);

    Intervals->LastInterval = Interval - 1;

    Elapsed.QuadPart = End.QuadPart - Start.QuadPart;
    Elapsed.QuadPart *= TIMESTAMP_TO_SECONDS;
    Elapsed.QuadPart /= Intervals->Frequency;

    Intervals->IntervalExtractionTimeInMicroseconds = Elapsed.QuadPart;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
