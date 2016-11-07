/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreContext.c

Abstract:

    This module implements trace context functionality.  Functions are provided
    for initializing a trace context record, as well as binding a trace store to
    a trace context.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeTraceContext(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACE_CONTEXT TraceContext,
    PULONG SizeOfTraceContext,
    PTRACE_SESSION TraceSession,
    PTRACE_STORES TraceStores,
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment,
    PTRACE_CONTEXT_FLAGS TraceContextFlags,
    PVOID UserData
    )
/*++

Routine Description:

    This routine initializes a TRACE_CONTEXT structure.  This involves setting
    relevant fields in the structure then binding the context to the trace
    stores.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    SizeOfTraceContext - Supplies a pointer to a variable that contains the
        buffer size allocated for the TraceContext parameter.  The actual size
        of the structure will be written to this variable.

    TraceSession - Supplies a pointer to a TRACE_SESSION structure.

    TraceStores - Supplies a pointer to a TRACE_STORES structure to bind the
        trace context to.

    ThreadpoolCallbackEnvironment - Supplies a pointer to a threadpool callback
        environment to use for the trace context.  This threadpool will be used
        to submit various asynchronous thread pool memory map operations.

    TraceContextFlags - Supplies an optional pointer to a TRACE_CONTEXT_FLAGS
        structure to use for the trace context.

    UserData - Supplies an optional pointer to user data that can be used by
        a caller to track additional context information per TRACE_CONTEXT
        structure.

Return Value:

    TRUE on success, FALSE on failure.  The required buffer size for the
    TRACE_CONTEXT structure can be obtained by passing in a valid pointer
    for SizeOfTraceContext and NULL for the remaining parameters.

--*/
{
    BOOL IsReadonly;
    BOOL ManualReset;
    USHORT Index;
    USHORT StoreIndex;
    USHORT NumberOfTraceStores;
    USHORT NumberOfRemainingMetadataStores;
    ULONG NumberOfBytesToZero;
    DWORD Result;
    TRACE_CONTEXT_FLAGS ContextFlags;
    HANDLE Event;
    PTRACE_STORE_WORK Work;
    PTRACE_STORE TraceStore;

    //
    // Validate size parameters.
    //

    if (!TraceContext) {
        if (SizeOfTraceContext) {
            *SizeOfTraceContext = sizeof(*TraceContext);
        }
        return FALSE;
    }

    if (!SizeOfTraceContext) {
        return FALSE;
    }

    if (*SizeOfTraceContext < sizeof(*TraceContext)) {
        *SizeOfTraceContext = sizeof(*TraceContext);
        return FALSE;
    }

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(TraceStores)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ThreadpoolCallbackEnvironment)) {
        return FALSE;
    }

    if (ARGUMENT_PRESENT(TraceContextFlags)) {
        ContextFlags = *TraceContextFlags;
        ContextFlags.Valid = FALSE;
    } else {
        SecureZeroMemory(&ContextFlags, sizeof(ContextFlags));
    }

    //
    // We zero the entire trace context structure, unless the caller has set the
    // IgnorePreferredAddresses context flag, in which case, we zero up to but
    // not including the first field related to the ignore bitmap.
    //

    NumberOfBytesToZero = sizeof(*TraceContext);

    //
    // Test the following invariants of the context flags:
    //  - If TraceContextFlags indicates readonly, TraceStores should as well.
    //  - If not readonly:
    //      - Ensure InitializeAsync is not set.
    //      - Ensure IgnorePreferredAddresses is not set.
    //      - Ensure TraceSession is not NULL.
    //  - Else:
    //      - If IgnorePreferredAddresses is set, ensure the bitmap is valid.
    //

    if (ContextFlags.Readonly) {

        IsReadonly = TRUE;

        //
        // Verify TraceStores is also readonly.
        //

        if (!TraceStores->Flags.Readonly) {
            return FALSE;
        }

        //
        // Verify the ignore bitmap if applicable.
        //

        if (ContextFlags.IgnorePreferredAddresses) {
            ULONG BitmapIndex;
            ULONG HintIndex;
            ULONG PreviousIndex;
            ULONG NumberOfSetBits;
            TRACE_STORE_ID TraceStoreId;
            PRTL_BITMAP Bitmap;

            //
            // Initialize bitmap alias.
            //

            Bitmap = &TraceContext->IgnorePreferredAddressesBitmap;

            //
            // The caller is responsible for initializing SizeOfBitMap.  Verify
            // it matches what we expect.
            //

            if (Bitmap->SizeOfBitMap != MAX_TRACE_STORE_IDS) {
                return FALSE;
            }

            //
            // Ensure the bitmap's buffer field is NULL; we set this ourselves.
            //

            if (Bitmap->Buffer) {
                return FALSE;
            }

            //
            // Initialize buffer pointer.
            //

            Bitmap->Buffer = (PULONG)&TraceContext->BitmapBuffer[0];

            //
            // Zero variables before loop.
            //

            BitmapIndex = 0;
            HintIndex = 0;
            PreviousIndex = 0;
            NumberOfSetBits = 0;

            //
            // Walk the bitmap and extract each bit, validate it is within
            // range, convert into a trace store ID, resolve the corresponding
            // trace store pointer, and set the store's IgnorePreferredAddresses
            // flag.  Fail early by returning FALSE on any erroneous conditions.
            //

            do {

                //
                // Extract the next bit from the bitmap.
                //

                BitmapIndex = Rtl->RtlFindSetBits(Bitmap, 1, HintIndex);

                //
                // Verify we got a sane index back.
                //

                if (BitmapIndex == BITS_NOT_FOUND ||
                    BitmapIndex >= TraceStoreInvalidId) {
                    return FALSE;
                }

                if (BitmapIndex <= PreviousIndex) {

                    //
                    // The search has wrapped, so exit the loop.
                    //

                    break;
                }

                //
                // The index is valid.  Convert to trace store ID, then resolve
                // the store's pointer and set the flag.  Update previous index
                // and hint index.
                //

                TraceStoreId = (TRACE_STORE_ID)BitmapIndex;
                TraceStore = TraceStoreIdToTraceStore(TraceStores,
                                                      TraceStoreId);
                TraceStore->IgnorePreferredAddresses = TRUE;

                PreviousIndex = BitmapIndex;
                HintIndex = BitmapIndex + 1;
                NumberOfSetBits++;

            } while (1);

            //
            // Sanity check that we saw at least one bit.
            //

            if (!NumberOfSetBits) {
                __debugbreak();
                return FALSE;
            }

            //
            // Adjust the number of bytes to zero such that we exclude the first
            // bitmap related field onward.
            //

            NumberOfBytesToZero = (
                FIELD_OFFSET(
                    TRACE_CONTEXT,
                    BitmapBufferSizeInQuadwords
                )
            );

        }

    } else {

        IsReadonly = FALSE;

        if (TraceStores->Flags.Readonly) {

            //
            // TraceStore is set readonly but context indicates otherwise.
            //

            return FALSE;

        } else if (!ARGUMENT_PRESENT(TraceSession)) {

            //
            // If we're not readonly, TraceSession must be provided.
            //

            return FALSE;
        }

        if (ContextFlags.IgnorePreferredAddresses) {

            //
            // IgnorePreferredAddresses only valid when readonly.
            //

            return FALSE;
        }
    }

    //
    // If we're not readonly, make sure the user has provided a trace session.
    //

    //
    // Zero the structure before we start using it.
    //

    SecureZeroMemory(TraceContext, NumberOfBytesToZero);

    TraceContext->TimerFunction = TraceStoreGetTimerFunction();
    if (!TraceContext->TimerFunction) {
        return FALSE;
    }

    //
    // Create a manual reset event for the loading complete state.
    //

    ManualReset = TRUE;
    TraceContext->LoadingCompleteEvent = CreateEvent(NULL,
                                                     ManualReset,
                                                     FALSE,
                                                     NULL);
    if (!TraceContext->LoadingCompleteEvent) {
        return FALSE;
    }

    TraceContext->ThreadpoolCleanupGroup = CreateThreadpoolCleanupGroup();
    if (!TraceContext->ThreadpoolCleanupGroup) {
        return FALSE;
    }

    SetThreadpoolCallbackCleanupGroup(
        ThreadpoolCallbackEnvironment,
        TraceContext->ThreadpoolCleanupGroup,
        NULL
    );

    if (!InitializeTraceStoreTime(Rtl, &TraceContext->Time)) {
        return FALSE;
    }

    TraceContext->SizeOfStruct = (USHORT)(*SizeOfTraceContext);
    TraceContext->TraceSession = TraceSession;
    TraceContext->TraceStores = TraceStores;
    TraceContext->ThreadpoolCallbackEnvironment = ThreadpoolCallbackEnvironment;
    TraceContext->UserData = UserData;
    TraceContext->Allocator = Allocator;

    TraceContext->Flags = ContextFlags;

    NumberOfTraceStores = TraceStores->NumberOfTraceStores;

    //
    // We subtract 2 from ElementsPerTraceStore to account for the normal trace
    // store and :MetadataInfo trace store.
    //

    NumberOfRemainingMetadataStores = (
        (TraceStores->ElementsPerTraceStore - 2) *
        NumberOfTraceStores
    );


#define INIT_WORK(Name, NumberOfItems)                               \
    Work = &TraceContext->##Name##Work;                              \
                                                                     \
    InitializeSListHead(&Work->ListHead);                            \
                                                                     \
    Work->WorkCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL); \
    if (!Work->WorkCompleteEvent) {                                  \
        goto Error;                                                  \
    }                                                                \
                                                                     \
    Work->ThreadpoolWork = CreateThreadpoolWork(                     \
        Name##Callback,                                              \
        TraceContext,                                                \
        ThreadpoolCallbackEnvironment                                \
    );                                                               \
    if (!Work->ThreadpoolWork) {                                     \
        goto Error;                                                  \
    }                                                                \
                                                                     \
    Work->TotalNumberOfItems = NumberOfItems;                        \
    Work->NumberOfActiveItems = NumberOfItems;                       \
    Work->NumberOfFailedItems = 0

    INIT_WORK(BindMetadataInfoStore, NumberOfTraceStores);
    INIT_WORK(BindRemainingMetadataStores, NumberOfRemainingMetadataStores);
    INIT_WORK(BindTraceStore, NumberOfTraceStores);
    INIT_WORK(ReadonlyNonStreamingBindComplete, NumberOfTraceStores);

    TraceContext->BindsInProgress = NumberOfTraceStores;

    if (IsReadonly) {

        //
        // Enumerate the trace stores and create the relocation complete events
        // first before any threadpool work is submitted.  These are used for
        // coordinating relocation synchronization between stores and must be
        // available for all stores as soon as the binding has been kicked off
        // for one store.  This is because a store could finish mapping itself
        // and be ready to process any relocations before the stores it is
        // dependent upon have finished loading themselves.  By using explicit
        // events and WaitForSingleObject/WaitForMultipleObjects (depending on
        // whether or not we dependent on one or multiple stores), we avoid any
        // race conditions with regards to trace stores not being ready when we
        // want them.
        //

        FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {

            //
            // N.B.: We use ManualReset == TRUE because we want the event to
            //       stay signaled once relocation has been complete.  This
            //       ensures that other stores can call WaitForMultipleObjects
            //       at any time and pick up the signaled event.
            //

            Event = CreateEvent(NULL, ManualReset, FALSE, NULL);
            if (!Event) {
                goto Error;
            }

            TraceStores->RelocationCompleteEvents[Index] = Event;
        }
    }

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {
        TraceStore = &TraceStores->Stores[StoreIndex];
        SubmitBindMetadataInfoWork(TraceContext, TraceStore);
    }

    //
    // If an async initialization has been requested, return now.  Otherwise,
    // wait on the loading complete event.
    //

    if (ContextFlags.AsyncInitialization) {

        //
        // N.B.: We should replace all of the allocator functions at this point
        //       (e.g. AllocateRecords) with a placeholder function that simply
        //       does a WaitForSingleObject() on the trace store's BindComplete
        //       event.  When the binding actually has been completed, we would
        //       set this event but also do an interlocked exchange pointer on
        //       the underlying function and swap in the "live" AllocateRecords.
        //       This would remove the need for callers to check for the state
        //       of the context before attempting to dispatch allocations.
        //

        return TRUE;
    }

    Result = WaitForSingleObject(TraceContext->LoadingCompleteEvent, INFINITE);

    if (Result != WAIT_OBJECT_0) {

        //
        // We don't `goto Error` here because the error handling attempts to
        // close the threadpool work item.  If a wait fails, it may be because
        // the process is being run down (user canceled operation, something
        // else failed, etc), in which case, we don't need to do any threadpool
        // or event cleanup operations.
        //

        OutputDebugStringA("TraceContext: wait for LoadingComplete failed.\n");
        return FALSE;
    }

    //
    // If there were no failures, the result was successful.
    //

    if (TraceContext->FailedCount == 0) {
        return TRUE;
    }

Error:

#define CLEANUP_WORK(Name)                                       \
    Work = &TraceContext->##Name##Work;                          \
                                                                 \
    if (Work->ThreadpoolWork) {                                  \
        CloseThreadpoolWork(Work->ThreadpoolWork);               \
    }                                                            \
                                                                 \
    if (Work->WorkCompleteEvent) {                               \
        CloseHandle(Work->WorkCompleteEvent);                    \
    }                                                            \
                                                                 \
    SecureZeroMemory(Work, sizeof(*Work));

    CLEANUP_WORK(BindMetadataInfoStore);
    CLEANUP_WORK(BindRemainingMetadataStores);
    CLEANUP_WORK(BindTraceStore);
    CLEANUP_WORK(ReadonlyNonStreamingBindComplete);

    return FALSE;
}

_Use_decl_annotations_
BOOL
InitializeReadonlyTraceContext(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACE_CONTEXT TraceContext,
    PULONG SizeOfTraceContext,
    PTRACE_SESSION TraceSession,
    PTRACE_STORES TraceStores,
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment,
    PTRACE_CONTEXT_FLAGS TraceContextFlags,
    PVOID UserData
    )
/*++

Routine Description:

    This routine initializes a readonly TRACE_CONTEXT structure.  It is a
    convenience method that is equivalent to calling InitializeTraceContext()
    with TraceContextFlags->Readonly set to TRUE.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    SizeOfTraceContext - Supplies a pointer to a variable that contains the
        buffer size allocated for the TraceContext parameter.  The actual size
        of the structure will be written to this variable.

    TraceSession - This parameter is ignored and a NULL value is passed to
        InitializeTraceContext() instead.

    TraceStores - Supplies a pointer to a TRACE_STORES structure to bind the
        trace context to.

    ThreadpoolCallbackEnvironment - Supplies a pointer to a threadpool callback
        environment to use for the trace context.  This threadpool will be used
        to submit various asynchronous thread pool memory map operations.

    TraceContextFlags - Supplies an optional pointer to a TRACE_CONTEXT_FLAGS
        structure to use for the trace context.  The Readonly flag will always
        be set on the flags passed over to InitializeTraceContext().

    UserData - Supplies an optional pointer to user data that can be used by
        a caller to track additional context information per TRACE_CONTEXT
        structure.

Return Value:

    TRUE on success, FALSE on failure.  The required buffer size for the
    TRACE_CONTEXT structure can be obtained by passing in a valid pointer
    for SizeOfTraceContext and NULL for the remaining parameters.

--*/
{
    TRACE_CONTEXT_FLAGS Flags = { 0 };

    //
    // Load the caller's flags if the pointer is non-NULL.
    //

    if (ARGUMENT_PRESENT(TraceContextFlags)) {
        Flags = *TraceContextFlags;
    }

    //
    // Set the readonly flag.
    //

    Flags.Readonly = TRUE;

    return InitializeTraceContext(Rtl,
                                  Allocator,
                                  TraceContext,
                                  SizeOfTraceContext,
                                  NULL,
                                  TraceStores,
                                  ThreadpoolCallbackEnvironment,
                                  &Flags,
                                  UserData);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
