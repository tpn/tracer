/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreWorkingSet.c

Abstract:

    This module implements functionality related to tracing working set
    information about a process.  It pairs the working set monitoring
    provided by NT with our trace store functionality, allowing us to
    efficiently record runtime working set information about a process.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
WsWatchInfoExStoreBindComplete(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine overrides the normal trace store bind complete routine for
    the WsWatchInfo trace store.  It calls the normal trace store bind complete
    routine, runs through one iteration of GetWorkingSetChanges(), and, if that
    completes successfully, submits the threadpool timer that is responsible for
    periodically calling GetWorkingSetChanges().

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
        return FALSE;
    }

    //
    // Resolve the runtime parameters.
    //

    TracerConfig = TraceContext->TracerConfig;
    RuntimeParameters = &TracerConfig->RuntimeParameters;

    //
    // Allocate a temporary buffer for the working set changes.
    //

    TraceContext->WsWatchInfoExBuffer = (PPSAPI_WS_WATCH_INFORMATION_EX)(
        TraceContext->Allocator->Calloc(
            TraceContext->Allocator,
            RuntimeParameters->WsWatchInfoExInitialBufferNumberOfElements,
            sizeof(PSAPI_WS_WATCH_INFORMATION_EX)
        )
    );

    if (!TraceContext->WsWatchInfoExBuffer) {
        return FALSE;
    }

    TraceContext->WsWatchInfoExCurrentBufferNumberOfElements = (
        RuntimeParameters->WsWatchInfoExInitialBufferNumberOfElements
    );

    //
    // Allocate a temporary buffer with sufficient space for the same number
    // of initial elements for the working set ex information.
    //

    TraceContext->WsWorkingSetExInfoBuffer = (
        (PPSAPI_WORKING_SET_EX_INFORMATION)(
            TraceContext->Allocator->Calloc(
                TraceContext->Allocator,
                RuntimeParameters->WsWatchInfoExInitialBufferNumberOfElements,
                sizeof(PSAPI_WORKING_SET_EX_INFORMATION)
            )
        )
    );

    if (!TraceContext->WsWorkingSetExInfoBuffer) {
        return FALSE;
    }

    //
    // Drive one iteration of the GetWorkingSetChanges() routine to verify
    // it is behaving properly before submitting a threadpool timer for it.
    // (We acquire and release the lock here to keep SAL happy.)
    //

    AcquireWorkingSetChangesLock(TraceContext);
    TRY_MAPPED_MEMORY_OP{
        Success = GetWorkingSetChanges(TraceContext);
    } CATCH_STATUS_IN_PAGE_ERROR{
        Success = FALSE;
    }
    ReleaseWorkingSetChangesLock(TraceContext);

    if (!Success) {
        return FALSE;
    }

    //
    // Initialize the due time in FILETIME format based on the configured
    // interval.
    //

    Interval = RuntimeParameters->GetWorkingSetChangesIntervalInMilliseconds;

    WindowLength = (
        RuntimeParameters->GetWorkingSetChangesWindowLengthInMilliseconds
    );

    //
    // Convert our interval into a relative threadpool time, and use this as
    // the timer's due time.
    //

    MillisecondsToRelativeThreadpoolTime(Interval, &DueTime);

    //
    // Start a timer that calls GetWorkingSetChanges() periodically.
    //

    Timer = TraceContext->GetWorkingSetChangesTimer;

    SetThreadpoolTimer(Timer, &DueTime, Interval, WindowLength);

    return TRUE;
}

_Use_decl_annotations_
BOOL
GetWorkingSetChanges(
    PTRACE_CONTEXT TraceContext
    )
/*++

Routine Description:

    This routine calls GetWsChangesEx() and saves the results to the
    WsWatchInfo trace store.  Additionally, for each faulting address
    reported, QueryWorkingSetEx() is called to obtain further information
    about the backing memory.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    BOOL Success;
    ULONG LastError;
    ULONG Index;
    ULONG MaxCount;
    ULONG BufferSizeInBytes;
    ULONGLONG RecordsLost;
    ULARGE_INTEGER BufferSize;
    ULARGE_INTEGER NumberOfElements;
    ULARGE_INTEGER WsWatchInfoExSize;
    ULARGE_INTEGER WsWorkingSetExInfoSize;
    ULARGE_INTEGER NumberOfActualElements;
    ULARGE_INTEGER TempWsWorkingSetExBufferSizeInBytes;
    PPSAPI_WS_WATCH_INFORMATION_EX WsWatchInfoEx;
    PPSAPI_WS_WATCH_INFORMATION_EX DestWsWatchInfoEx;
    PPSAPI_WS_WATCH_INFORMATION_EX WsWatchInfoExBuffer;
    PPSAPI_WS_WATCH_INFORMATION_EX TempWsWatchInfoExBuffer;
    PPSAPI_WORKING_SET_EX_INFORMATION WsWorkingSetExInfo;
    PPSAPI_WORKING_SET_EX_INFORMATION DestWsWorkingSetExInfo;
    PPSAPI_WORKING_SET_EX_INFORMATION WsWorkingSetExInfoBuffer;
    PPSAPI_WORKING_SET_EX_INFORMATION TempWsWorkingSetExInfoBuffer;
    HANDLE CurrentProcess;
    PTRACE_STORE WsWatchInfoExStore;
    PTRACE_STORE WsWorkingSetExInfoStore;
    PTRACE_STORES TraceStores;

    //
    // Initialize variables prior to getting the working set changes.
    //

    Rtl = TraceContext->Rtl;
    CurrentProcess = GetCurrentProcess();
    TempWsWatchInfoExBuffer = TraceContext->WsWatchInfoExBuffer;
    TempWsWorkingSetExInfoBuffer = TraceContext->WsWorkingSetExInfoBuffer;
    WsWatchInfoExSize.QuadPart = sizeof(PSAPI_WS_WATCH_INFORMATION_EX);
    WsWorkingSetExInfoSize.QuadPart = sizeof(PSAPI_WORKING_SET_EX_INFORMATION);
    NumberOfElements.QuadPart = (
        TraceContext->WsWatchInfoExCurrentBufferNumberOfElements
    );
    BufferSize.QuadPart = (
        NumberOfElements.QuadPart *
        WsWatchInfoExSize.QuadPart
    );

    //
    // Get the working set changes into our temporary heap allocated buffer.
    //

TryGetWsChangesEx:

    if (BufferSize.HighPart) {

        //
        // A buffer size >= 8GB is going to be a logic error somewhere.
        //

        __debugbreak();
        return FALSE;
    }

    BufferSizeInBytes = BufferSize.LowPart;

    Success = Rtl->K32GetWsChangesEx(CurrentProcess,
                                     TempWsWatchInfoExBuffer,
                                     &BufferSizeInBytes);

    if (!Success) {
        LastError = GetLastError();

        if (LastError == ERROR_NO_MORE_ITEMS) {

            //
            // Nothing to do, return success.
            //

            return TRUE;
        }

        if (LastError != ERROR_INSUFFICIENT_BUFFER) {

            //
            // Anything other than an insufficient buffer error is fatal.
            //

            __debugbreak();
            TraceContext->LastError = LastError;
            return FALSE;
        }

        //
        // Double our number of elements, update the buffer size, check that the
        // new buffer size is sane, then reallocate the buffer.
        //

        NumberOfElements.QuadPart = NumberOfElements.QuadPart << 1;
        BufferSize.QuadPart = (
            NumberOfElements.QuadPart *
            WsWatchInfoExSize.QuadPart
        );

        if (BufferSize.HighPart) {

            //
            // A buffer size >= 8GB is going to be a logic error somewhere.
            //

            __debugbreak();
            return FALSE;
        }

        TraceContext->WsWatchInfoExBuffer = (PPSAPI_WS_WATCH_INFORMATION_EX)(
            TraceContext->Allocator->Realloc(
                TraceContext->Allocator,
                TempWsWatchInfoExBuffer,
                BufferSize.LowPart
            )
        );

        if (!TraceContext->WsWatchInfoExBuffer) {
            return FALSE;
        }

        TempWsWatchInfoExBuffer = TraceContext->WsWatchInfoExBuffer;
        TraceContext->WsWatchInfoExCurrentBufferNumberOfElements = (ULONG)(
            NumberOfElements.QuadPart
        );

        //
        // We need to keep the WorkingSetExInfo buffer sized with the same
        // number of elements, so reallocate that buffer now.
        //

        TraceContext->WsWorkingSetExInfoBuffer = (
            (PPSAPI_WORKING_SET_EX_INFORMATION)(
                TraceContext->Allocator->Realloc(
                    TraceContext->Allocator,
                    TraceContext->WsWorkingSetExInfoBuffer,
                    NumberOfElements.QuadPart * WsWorkingSetExInfoSize.QuadPart
                )
            )
        );

        if (!TraceContext->WsWorkingSetExInfoBuffer) {
            return FALSE;
        }

        TempWsWorkingSetExInfoBuffer = TraceContext->WsWorkingSetExInfoBuffer;

        //
        // Try the K32WsChangesEx() call again.
        //

        goto TryGetWsChangesEx;
    }

    //
    // The working set changes were successfully copied into our buffer.
    // Initialize a pointer to the first element and calculate the maximum
    // possible count of elements.
    //

    WsWatchInfoEx = TempWsWatchInfoExBuffer;
    MaxCount = BufferSize.LowPart / sizeof(PSAPI_WS_WATCH_INFORMATION_EX);

    //
    // Loop through the elements in the returned array and kill birds with one
    // stone: 1) priming the working set ex information buffer with virtual
    // addresses to dispatch to a QueryWorkingSetEx() call, and 2) verifying
    // the final NULL-terminated array element is where we expect.
    //

    for (Index = 0; Index < MaxCount; Index++) {
        WsWatchInfoEx = &TempWsWatchInfoExBuffer[Index];
        WsWorkingSetExInfo = &TempWsWorkingSetExInfoBuffer[Index];

        if (!WsWatchInfoEx->BasicInfo.FaultingPc) {

            //
            // This is the final element in the array.  Capture the number of
            // records lost, if any, then break out of the loop.  (The number
            // of records lost is captured in the FaultingVa member of the last
            // array element.)
            //

            RecordsLost = (ULONGLONG)WsWatchInfoEx->BasicInfo.FaultingVa;

            break;
        }

        //
        // Copy the faulting address.
        //

        WsWorkingSetExInfo->VirtualAddress = (
            WsWatchInfoEx->BasicInfo.FaultingVa
        );
    }

    //
    // Ensure we captured the expected number of events.
    //

    if (Index > MaxCount) {
        __debugbreak();
        return FALSE;
    } else {

        //
        // The last array element should be NULL.
        //

        if (TempWsWatchInfoExBuffer[Index].BasicInfo.FaultingPc != NULL) {
            __debugbreak();
            return FALSE;
        }
    }

    //
    // Capture the count of elements and calculate the buffer size of the
    // working set ex info, then call QueryWorkingSetEx() against the array
    // of faulting virtual addresses.
    //

    NumberOfActualElements.QuadPart = Index - 1;

    //
    // If there were no elements, we're done.
    //

    if (!NumberOfActualElements.QuadPart) {
        return TRUE;
    }

    TempWsWorkingSetExBufferSizeInBytes.QuadPart = (
        NumberOfActualElements.QuadPart *
        WsWorkingSetExInfoSize.QuadPart
    );

    //
    // Ensure we've got a sane buffer size.
    //

    if (TempWsWorkingSetExBufferSizeInBytes.HighPart) {
        __debugbreak();
        return FALSE;
    }

    Success = QueryWorkingSetEx(CurrentProcess,
                                TempWsWorkingSetExInfoBuffer,
                                TempWsWorkingSetExBufferSizeInBytes.LowPart);

    if (!Success) {
        TraceContext->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    //
    // Allocate an appropriately sized buffer from each trace store.
    //

    TraceStores = TraceContext->TraceStores;

    WsWatchInfoExStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreWsWatchInfoExId
        )
    );

    WsWorkingSetExInfoStore = (
        TraceStoreIdToTraceStore(
            TraceStores,
            TraceStoreWsWorkingSetExInfoId
        )
    );

    WsWatchInfoExBuffer = (PPSAPI_WS_WATCH_INFORMATION_EX)(
        WsWatchInfoExStore->AllocateRecords(
            TraceContext,
            WsWatchInfoExStore,
            &WsWatchInfoExSize,
            &NumberOfActualElements
        )
    );

    if (!WsWatchInfoExBuffer) {
        __debugbreak();
        return FALSE;
    }

    WsWorkingSetExInfoBuffer = (PPSAPI_WORKING_SET_EX_INFORMATION)(
        WsWorkingSetExInfoStore->AllocateRecords(
            TraceContext,
            WsWorkingSetExInfoStore,
            &WsWorkingSetExInfoSize,
            &NumberOfActualElements
        )
    );

    if (!WsWorkingSetExInfoBuffer) {
        __debugbreak();
        return FALSE;
    }

    //
    // Loop through the temporary buffers and copy information into the
    // trace store buffers.
    //

    for (Index = 0; Index < NumberOfActualElements.LowPart; Index++) {

        //
        // Copy the working set watch information.
        //

        WsWatchInfoEx = &TempWsWatchInfoExBuffer[Index];
        DestWsWatchInfoEx = &WsWatchInfoExBuffer[Index];

        DestWsWatchInfoEx->BasicInfo.FaultingPc = (
            WsWatchInfoEx->BasicInfo.FaultingPc
        );

        DestWsWatchInfoEx->BasicInfo.FaultingVa = (
            WsWatchInfoEx->BasicInfo.FaultingVa
        );

        DestWsWatchInfoEx->FaultingThreadId = (
            WsWatchInfoEx->FaultingThreadId
        );

        DestWsWatchInfoEx->Flags = (
            WsWatchInfoEx->Flags
        );

        //
        // Copy the working set extended information.
        //

        WsWorkingSetExInfo = &TempWsWorkingSetExInfoBuffer[Index];
        DestWsWorkingSetExInfo = &WsWorkingSetExInfoBuffer[Index];

        DestWsWorkingSetExInfo->VirtualAddress = (
            WsWorkingSetExInfo->VirtualAddress
        );

        DestWsWorkingSetExInfo->VirtualAttributes.Flags = (
            WsWorkingSetExInfo->VirtualAttributes.Flags
        );
    }

    //
    // We're done, return success to the caller.
    //

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
