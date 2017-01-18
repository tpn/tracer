/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStorePerformance.c

Abstract:

    This module implements functionality related to capturing performance
    metrics whilst a process is being traced and saving them to a trace store.
    Memory counters, I/O counters and handle counts are currently tracked.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
PerformanceStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the Performance trace store.  It calls the normal trace store bind complete
    routine, runs through one iteration of CapturePerformanceMetrics(), and,
    if that completes successfully, submits the threadpool timer that is
    responsible for periodically calling CapturePerformanceMetrics().

Arguments:

    TraceContext - Supplies a pointer to the TRACE_CONTEXT structure to which
        the trace store was bound.

    TraceStore - Supplies a pointer to the bound TRACE_STORE.

    FirstMemoryMap - Supplies a pointer to the first TRACE_STORE_MEMORY_MAP
        used by the trace store.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    ULONG Interval;
    ULONG WindowLength;
    PTP_TIMER Timer;
    FILETIME DueTime;
    PTRACER_CONFIG TracerConfig;
    PTRACER_RUNTIME_PARAMETERS RuntimeParameters;

    //
    // Complete the normal bind complete routine for the trace store.  This
    // will resume allocations and set the bind complete event.
    //

    if (!TraceStoreBindComplete(TraceContext, TraceStore, FirstMemoryMap)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Resolve the runtime parameters and copy locally.
    //

    TracerConfig = TraceContext->TracerConfig;
    RuntimeParameters = &TracerConfig->RuntimeParameters;

    TraceContext->CapturePerformanceMetricsIntervalInMilliseconds = (
        RuntimeParameters->CapturePerformanceMetricsIntervalInMilliseconds
    );

    TraceContext->CapturePerformanceMetricsWindowLengthInMilliseconds = (
        RuntimeParameters->CapturePerformanceMetricsWindowLengthInMilliseconds
    );

    //
    // Drive one iteration of the CapturePerformanceMetrics() routine to verify
    // it is behaving properly before submitting a threadpool timer for it.
    // (We acquire and release the lock here to keep SAL happy.)
    //

    AcquireCapturePerformanceMetricsLock(TraceContext);
    TRY_MAPPED_MEMORY_OP{
        Success = CapturePerformanceMetrics(TraceContext);
    } CATCH_STATUS_IN_PAGE_ERROR{
        Success = FALSE;
    }
    ReleaseCapturePerformanceMetricsLock(TraceContext);

    if (!Success) {
        __debugbreak();
        return FALSE;
    }

    //
    // Initialize the due time in FILETIME format based on the configured
    // interval.
    //

    Interval = (
        RuntimeParameters->CapturePerformanceMetricsIntervalInMilliseconds
    );

    WindowLength = (
        RuntimeParameters->CapturePerformanceMetricsWindowLengthInMilliseconds
    );

    //
    // Convert our interval into a relative threadpool time, and use this as
    // the timer's due time.
    //

    MillisecondsToRelativeThreadpoolTime(Interval, &DueTime);

    //
    // Start a timer that calls GetPerformanceChanges() periodically.
    //

    Timer = TraceContext->CapturePerformanceMetricsTimer;

    SetThreadpoolTimer(Timer, &DueTime, Interval, WindowLength);

    return TRUE;
}

_Use_decl_annotations_
BOOL
CapturePerformanceMetrics(
    PTRACE_CONTEXT TraceContext
    )
/*++

Routine Description:

    This routine captures performance metrics.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    BOOL Success;
    FILETIME ExitTime;
    FILETIME CreationTime;
    HANDLE CurrentProcess;
    LARGE_INTEGER Elapsed;
    LARGE_INTEGER Timestamp;
    PTRACE_STORES TraceStores;
    ULONG_PTR RecordSize;
    ULONG_PTR NumberOfRecords;
    PTRACE_STORE PerformanceStore;
    PTRACE_STORE PerformanceDeltaStore;
    PTRACE_PERFORMANCE Last;
    PTRACE_PERFORMANCE Perf;
    PTRACE_PERFORMANCE Delta = NULL;

    //
    // Initialize variables.
    //

    Rtl = TraceContext->Rtl;
    CurrentProcess = GetCurrentProcess();
    TraceStores = TraceContext->TraceStores;

    //
    // Resolve the performance and delta trace stores.
    //

    PerformanceStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStorePerformanceId
        )
    );

    PerformanceDeltaStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStorePerformanceDeltaId
        )
    );

    //
    // Save the timestamp for this event.
    //

    TraceContextQueryPerformanceCounter(TraceContext, &Elapsed, &Timestamp);

    //
    // Load the existing TRACE_PERFORMANCE record, if there is one.
    //

    Last = (PTRACE_PERFORMANCE)PerformanceStore->PrevAddress;

    //
    // Allocate a new TRACE_PERFORMANCE record from the performance store.
    //

    RecordSize = sizeof(*Perf);
    NumberOfRecords = 1;

    Perf = (PTRACE_PERFORMANCE)(
        PerformanceStore->AllocateRecordsWithTimestamp(
            TraceContext,
            PerformanceStore,
            NumberOfRecords,
            RecordSize,
            &Timestamp
        )
    );

    if (!Perf) {
        goto Error;
    }

    //
    // If there was a previous record, also allocate a record from the
    // performance delta store.
    //

    if (!Last) {
        goto InitializePerf;
    }

    Delta = (PTRACE_PERFORMANCE)(
        PerformanceDeltaStore->AllocateRecordsWithTimestamp(
            TraceContext,
            PerformanceDeltaStore,
            NumberOfRecords,
            RecordSize,
            &Timestamp
        )
    );

    if (!Delta) {
        goto Error;
    }

    //
    // Initialize the performance record.
    //

InitializePerf:

    Perf->SizeOfStruct = sizeof(*Perf);
    Perf->ProcessMemoryCountersExSize = sizeof(Perf->MemoryCountersEx);
    Perf->MemoryStatusExLength = sizeof(Perf->MemoryStatusEx);
    Perf->PerformanceInfoSize = sizeof(Perf->PerformanceInfo);
    Perf->IntervalInMilliseconds = (
        TraceContext->CapturePerformanceMetricsIntervalInMilliseconds
    );
    Perf->WindowLengthInMilliseconds = (
        TraceContext->CapturePerformanceMetricsWindowLengthInMilliseconds
    );
    Perf->Timestamp.QuadPart = Timestamp.QuadPart;

    //
    // Get memory information for the process.
    //

    Success = Rtl->K32GetProcessMemoryInfo(CurrentProcess,
                                           &Perf->MemoryCounters,
                                           Perf->ProcessMemoryCountersExSize);
    if (!Success) {
        TraceContext->LastError = GetLastError();
        __debugbreak();
        goto Error;
    }

    //
    // Get global system memory information.
    //

    Success = GlobalMemoryStatusEx(&Perf->MemoryStatusEx);
    if (!Success) {
        TraceContext->LastError = GetLastError();
        __debugbreak();
        goto Error;
    }

    //
    // Get performance information.
    //

    Success = Rtl->K32GetPerformanceInfo(&Perf->PerformanceInfo,
                                         Perf->PerformanceInfoSize);
    if (!Success) {
        TraceContext->LastError = GetLastError();
        goto Error;
    }

    //
    // Get process times.
    //

    Success = GetProcessTimes(CurrentProcess,
                              &CreationTime,
                              &ExitTime,
                              &Perf->KernelTime,
                              &Perf->UserTime);
    if (!Success) {
        TraceContext->LastError = GetLastError();
        goto Error;
    }

    //
    // Get I/O counter information.
    //

    Success = Rtl->GetProcessIoCounters(CurrentProcess, &Perf->IoCounters);
    if (!Success) {
        TraceContext->LastError = GetLastError();
        goto Error;
    }

    //
    // Get handle count information.
    //

    Success = Rtl->GetProcessHandleCount(CurrentProcess, &Perf->HandleCount);
    if (!Success) {
        TraceContext->LastError = GetLastError();
        goto Error;
    }

    //
    // Query cycle time.
    //

    Success = QueryProcessCycleTime(CurrentProcess, &Perf->ProcessCycles);
    if (!Success) {
        TraceContext->LastError = GetLastError();
        goto Error;
    }

    //
    // If this is the first performance record we've captured, we can exit here.
    //

    if (!Last) {
        Success = TRUE;
        goto End;
    }

    //
    // Fill out delta information.
    //

    Delta->SizeOfStruct = sizeof(*Perf);

    //
    // Convenience macros.
    //

#define SUBTRACT(Name) Perf->##Name - Last->##Name
#define DELTA(Name) Delta->##Name##Delta = SUBTRACT(Name)

    DELTA(ProcessHandleCount);

    //
    // PROCESS_MEMORY_COUNTERS_EX
    //

    DELTA(PageFaultCount);
    DELTA(WorkingSetSize);
    DELTA(QuotaPeakPagedPoolUsage);
    DELTA(QuotaPagedPoolUsage);
    DELTA(QuotaPeakNonPagedPoolUsage);
    DELTA(QuotaNonPagedPoolUsage);
    DELTA(PagefileUsage);
    DELTA(PeakPagefileUsage);
    DELTA(PrivateUsage);

    //
    // MEMORYSTATUSEX
    //

    DELTA(dwMemoryLoad);
    DELTA(ullTotalPhys);
    DELTA(ullAvailPhys);
    DELTA(ullTotalPageFile);
    DELTA(ullAvailPageFile);
    DELTA(ullTotalVirtual);
    DELTA(ullAvailVirtual);
    DELTA(ullAvailExtendedVirtual);

    //
    // IO_COUNTERS
    //

    DELTA(ReadOperationCount);
    DELTA(WriteOperationCount);
    DELTA(OtherOperationCount);
    DELTA(ReadTransferCount);
    DELTA(WriteTransferCount);
    DELTA(OtherTransferCount);

    //
    // PERFORMANCE_INFORMATION
    //

    DELTA(CommitTotal);
    DELTA(CommitLimit);
    DELTA(CommitPeak);
    DELTA(PhysicalTotal);
    DELTA(PhysicalAvailable);
    DELTA(SystemCache);
    DELTA(KernelTotal);
    DELTA(KernelPaged);
    DELTA(KernelNonpaged);
    DELTA(PageSize);
    DELTA(HandleCount);
    DELTA(ProcessCount);
    DELTA(ThreadCount);

    Success = TRUE;
    goto End;

Error:
    Success = FALSE;

End:
    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
