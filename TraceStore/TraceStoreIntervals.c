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
    DOUBLE TicksPerInterval;
    DOUBLE NumberOfIntervals;
    DOUBLE FramesPerSecond;
    ULONG NumberOfPages;
    ULONGLONG StartTicks;
    ULONGLONG EndTicks;
    ULONGLONG ElapsedTicks;
    TRACE_STORE_TRAITS Traits;
    PTRACE_STORE_TOTALS Totals;
    ULONGLONG NumberOfAllocations;
    ULONGLONG RecordSizeInBytes;
    ULONG_PTR LastRecordOffset;
    PTRACE_STORE_INTERVALS Intervals;
    PTRACE_STORE AllocationTimestampStore;
    PVOID RecordBaseAddress;
    SIZE_T AllocSizeInBytes;
    SIZE_T PageAlignedAllocSizeInBytes;

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
        Totals->RecordSize.QuadPart /
        Totals->NumberOfRecords.QuadPart
    );

    if (!IsPowerOf2(RecordSizeInBytes)) {
        __debugbreak();
        return FALSE;
    }

    if (RecordSizeInBytes == 0) {
        __debugbreak();
        return FALSE;
    }
    
    ZeroStruct(Intervals);

    //FramesPerSecond = (DOUBLE)TraceStore->IntervalFramesPerSecond;
    FramesPerSecond = 240.0;
    TicksPerInterval = (
        ((DOUBLE)1.0 / FramesPerSecond) /
        ((DOUBLE)1.0 / (DOUBLE)TraceStore->Time->Frequency.QuadPart)
    );

    Intervals->TicksPerInterval = (ULONGLONG)TicksPerInterval;

    AllocationTimestampStore = TraceStore->AllocationTimestampStore;

    if (AllocationTimestampStore->Totals->NumberOfAllocations.QuadPart !=
        NumberOfAllocations) {
        __debugbreak();
        return FALSE;
    }

    FirstAllocationTimestamp = (PULONGLONG)(
        AllocationTimestampStore->MemoryMap->BaseAddress
    );

    LastAllocationTimestamp = (
        (PULONGLONG)(
            RtlOffsetToPointer(
                FirstAllocationTimestamp,
                (NumberOfAllocations - 1) * sizeof(ULONGLONG)
            )
        )
    );

    StartTicks = *FirstAllocationTimestamp;
    EndTicks = *LastAllocationTimestamp;
    ElapsedTicks = EndTicks - StartTicks;

    NumberOfIntervals = (DOUBLE)ElapsedTicks / (DOUBLE)TicksPerInterval;
    Intervals->NumberOfIntervals = (ULONGLONG)NumberOfIntervals;

    LastRecordOffset = (NumberOfAllocations - 1) * RecordSizeInBytes;
    StartAddress = (ULONG_PTR)TraceStore->FlatMemoryMap.BaseAddress;
    EndAddress = (ULONG_PTR)StartAddress + LastRecordOffset;
    RecordBaseAddress = TraceStore->FlatMemoryMap.BaseAddress;

    AllocSizeInBytes = (
        ALIGN_UP_POINTER(Intervals->NumberOfIntervals) *
        sizeof(TRACE_STORE_INTERVAL)
    );

    PageAlignedAllocSizeInBytes = ROUND_TO_PAGES(AllocSizeInBytes);
    
    Intervals->Intervals = (PTRACE_STORE_INTERVAL)(
        VirtualAlloc(NULL,
                     PageAlignedAllocSizeInBytes,
                     MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE)
    );

    if (!Intervals->Intervals) {
        return FALSE;
    }

    Intervals->FramesPerSecond = 240;

    NumberOfPages = (ULONG)(PageAlignedAllocSizeInBytes >> PAGE_SHIFT);

    TraceStore->Rtl->FillPages((PBYTE)Intervals->Intervals,
                               0,
                               NumberOfPages);

    Success = ExtractIntervals(Intervals->NumberOfIntervals,
                               Intervals->TicksPerInterval,
                               NumberOfAllocations,
                               RecordSizeInBytes,
                               FirstAllocationTimestamp,
                               LastAllocationTimestamp,
                               RecordBaseAddress,
                               Intervals->Intervals,
                               &Intervals->ExtractionDurationInTicks);
                               
    return Success;                    
}

_Use_decl_annotations_
BOOL
ExtractIntervals(
    ULONGLONG NumberOfIntervals,
    ULONGLONG TicksPerInterval,
    ULONGLONG NumberOfRecords,
    ULONGLONG RecordSizeInBytes,
    PULONGLONG FirstAllocationTimestamp,
    PULONGLONG LastAllocationTimestamp,
    PVOID RecordBaseAddress,
    PTRACE_STORE_INTERVAL IntervalBase,
    PULONGLONG ExtractionTicksPointer
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
    BOOL MoreIntervalsThanRecords = FALSE;
    ULONGLONG IntervalCount = 0;
    ULONGLONG RecordCount = 0;
    PULONGLONG AllocationTimestamp = (PULONGLONG)FirstAllocationTimestamp;
    ULONGLONG Address = (ULONGLONG)RecordBaseAddress;
    ULONGLONG NextTimestamp = *AllocationTimestamp + TicksPerInterval;
    PTRACE_STORE_INTERVAL Interval = IntervalBase;
    LARGE_INTEGER Start;
    LARGE_INTEGER End;
    LARGE_INTEGER Elapsed;

    QueryPerformanceCounter(&Start);

    while (AllocationTimestamp++ < LastAllocationTimestamp) {
        ++RecordCount;
        if (*AllocationTimestamp >= NextTimestamp) {
            Interval->IntervalNumber = ++IntervalCount;
            Interval->RecordNumber = RecordCount;
            Interval->AllocationTimestamp = *AllocationTimestamp;
            Interval->Address = (ULONGLONG)(
                RtlOffsetToPointer(
                    Address,
                    (RecordCount - 1) * RecordSizeInBytes
                )
            );
            NextTimestamp = Interval->AllocationTimestamp + TicksPerInterval;
            Interval++;
        }
    }

    QueryPerformanceCounter(&End);

    Elapsed.QuadPart = End.QuadPart - Start.QuadPart;

    *ExtractionTicksPointer = Elapsed.QuadPart;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
