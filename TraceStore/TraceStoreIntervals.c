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
    TRACE_STORE_INTERVALS Intervals;
    DOUBLE TicksPerInterval;
    DOUBLE NumberOfIntervals;
    ULONGLONG StartTicks;
    ULONGLONG EndTicks;
    ULONGLONG ElapsedTicks;
    TRACE_STORE_TRAITS Traits;
    PTRACE_STORE_TOTALS Totals;
    ULONGLONG NumberOfAllocations;
    ULONGLONG RecordSizeInBytes;
    ULONG_PTR LastRecordOffset;
    ULONG_PTR StartAddress;
    ULONG_PTR EndAddress;
    PTRACE_STORE AllocationTimestampStore;
    PVOID AllocationTimestampBaseAddress;

    Traits = *TraceStore->pTraits;

    Exclude = (
        !IsFixedRecordSize(Traits)          ||
        !IsRecordSizeAlwaysPowerOf2(Traits) ||
        IsLinkedStore(Traits)
    );

    if (Exclude) {
        return FALSE;
    }

    Totals = TraceStore->Totals;
    NumberOfAllocations = Totals->NumberOfAllocations.QuadPart;
    if (NumberOfAllocations == 0) {
        return FALSE;
    }

    RecordSizeInBytes = (
        Totals->AllocationSize.QuadPart /
        Totals->NumberOfAllocations.QuadPart
    );

    if (!IsPowerOf2(RecordSizeInBytes)) {
        __debugbreak();
    }

    ZeroStruct(Intervals);

    //Intervals.FramesPerSecond = TraceStore->IntervalFramesPerSecond;
    Intervals.FramesPerSecond = 240;
    TicksPerInterval = (
        ((DOUBLE)1.0 / (DOUBLE)Intervals.FramesPerSecond) /
        ((DOUBLE)1.0 / (DOUBLE)TraceStore->Time->Frequency.QuadPart)
    );

    Intervals.TicksPerInterval = (ULONGLONG)TicksPerInterval;

    AllocationTimestampStore = TraceStore->AllocationTimestampStore;

    if (AllocationTimestampStore->Totals->NumberOfAllocations.QuadPart !=
        NumberOfAllocations) {
        __debugbreak();
        return FALSE;
    }

    AllocationTimestampBaseAddress = (
        AllocationTimestampStore->MemoryMap->BaseAddress
    );

    StartTicks = *((PULONGLONG)AllocationTimestampBaseAddress);
    EndTicks = *(
        (PULONGLONG)(
            RtlOffsetToPointer(
                AllocationTimestampBaseAddress,
                (NumberOfAllocations - 1) * sizeof(ULONGLONG)
            )
        )
    );

    ElapsedTicks = EndTicks - StartTicks;

    NumberOfIntervals = (DOUBLE)ElapsedTicks / (DOUBLE)TicksPerInterval;
    Intervals.NumberOfIntervals = (ULONGLONG)NumberOfIntervals;

    LastRecordOffset = (NumberOfAllocations - 1) * RecordSizeInBytes;
    StartAddress = (ULONG_PTR)TraceStore->FlatMemoryMap.BaseAddress;
    EndAddress = (ULONG_PTR)StartAddress + LastRecordOffset;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
