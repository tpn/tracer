/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    CreatePerfectHashTableContext.c

Abstract:

    This module implements the runtime context creation routine for the
    PerfectHashTable component.  The context encapsulates threadpool resources
    in order to support finding perfect hash table solutions in parallel.

--*/

#include "stdafx.h"

//
// Forward definition of various callbacks we implement in this module.
// These are needed up-front in order to create and register the various
// work and cleanup callbacks as part of context creation.
//
// (Annoyingly, winnt.h only defines PTP_WORK_CALLBACK, not the underlying
//  raw function type TP_WORK_CALLBACK, so, do that now.  Ditto for the
//  PTP_CLEANUP_GROUP_CANCEL_CALLBACK signature.)
//

typedef
VOID
(NTAPI TP_WORK_CALLBACK)(
    _Inout_     PTP_CALLBACK_INSTANCE Instance,
    _Inout_opt_ PVOID                 Context,
    _Inout_     PTP_WORK              Work
    );


typedef
VOID
(NTAPI TP_CLEANUP_GROUP_CANCEL_CALLBACK)(
    _Inout_opt_ PVOID ObjectContext,
    _Inout_opt_ PVOID CleanupContext
    );

TP_WORK_CALLBACK MainCallback;
TP_WORK_CALLBACK ErrorCallback;
TP_WORK_CALLBACK FinishedCallback;
TP_CLEANUP_GROUP_CANCEL_CALLBACK CleanupCallback;

//
// Main context creation routine.
//

_Use_decl_annotations_
BOOLEAN
CreatePerfectHashTableContext(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PPERFECT_HASH_TABLE_ANY_API AnyApi,
    PULONG MaximumConcurrencyPointer,
    PPERFECT_HASH_TABLE_CONTEXT *ContextPointer
    )
/*++

Routine Description:

    Creates and initializes a PERFECT_HASH_TABLE_CONTEXT structure, which
    creates a main threadpool with a maximum of threads indicated by the
    MaximumConcurrency parameter, if applicable.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an initialized ALLOCATOR structure that
        will be used for all memory allocations.

    AnyApi - Supplies a pointer to the active API structure in use.

    MaximumConcurrency - Optionally supplies a pointer to a variable that
        contains the desired maximum concurrency to be used for the underlying
        threadpool.  If NULL, or non-NULL but points to a value of 0, then the
        number of system processors * 2 will be used as a default value.

        N.B. This value is passed directly to SetThreadpoolThreadMaximum().

    ContextPointer - Supplies the address of a variable that receives the
        address of the newly created PERFECT_HASH_TABLE_CONTEXT structure on
        success, NULL on failure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    USHORT Index;
    USHORT NumberOfEvents;
    ULONG LastError;
    PHANDLE Event;
    BOOLEAN Success;
    PBYTE Buffer;
    PBYTE ExpectedBuffer;
    ULONG MaximumConcurrency;
    ULONG SizeOfNamesWideBuffer;
    PWSTR NamesWideBuffer;
    PTP_POOL Threadpool;
    ULARGE_INTEGER AllocSize;
    ULARGE_INTEGER ObjectNameArraySize;
    PUNICODE_STRING Name;
    PPUNICODE_STRING Prefixes;
    PPERFECT_HASH_TABLE_API Api;
    PPERFECT_HASH_TABLE_CONTEXT Context = NULL;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ContextPointer)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AnyApi)) {

        return FALSE;

    } else {

        //
        // Initialize Api alias.  We'll use this during error handling in order
        // to call the context destroy routine.
        //

        Api = &AnyApi->Api;
    }

    //
    // Default the maximum concurrency to 0 if a NULL pointer was provided.
    //

    if (ARGUMENT_PRESENT(MaximumConcurrencyPointer)) {

        MaximumConcurrency = *MaximumConcurrencyPointer;

    } else {

        MaximumConcurrency = 0;

    }

    //
    // Argument validation complete.  Clear the caller's pointer up-front and
    // continue with creation.
    //

    *ContextPointer = NULL;

    //
    // Calculate the size required by the array of object names that will
    // trail the context structure, filled in by Rtl->CreateRandomObjectNames().
    //

    ObjectNameArraySize.QuadPart = (
        NumberOfContextObjectPrefixes * sizeof(PPUNICODE_STRING)
    );

    ASSERT(!ObjectNameArraySize.HighPart);

    //
    // Calculate allocation size required by the structure.
    //

    AllocSize.QuadPart = (

        //
        // Account for the size of the context structure itself.
        //

        sizeof(PERFECT_HASH_TABLE_CONTEXT) +

        //
        // Account for the array of object names trailing the structure.
        //

        ObjectNameArraySize.QuadPart

    );

    //
    // Sanity check we haven't overflowed.
    //

    ASSERT(!AllocSize.HighPart);

    //
    // Allocate space for the context.
    //

    Context = (PPERFECT_HASH_TABLE_CONTEXT)(
        Allocator->Calloc(Allocator->Context,
                          1,
                          AllocSize.LowPart)
    );

    if (!Context) {
        return FALSE;
    }

    //
    // Allocation was successful, continue with initialization.
    //

    Context->SizeOfStruct = sizeof(*Context);
    Context->Rtl = Rtl;
    Context->Allocator = Allocator;
    Context->Flags.AsULong = 0;
    Context->AnyApi = AnyApi;

    //
    // Wire up the array of PUNICODE_STRING pointers that will be filled in by
    // Rtl->CreateRandomObjectNames().
    //

    Buffer = (PBYTE)Context;
    Buffer += sizeof(*Context);
    Context->ObjectNames = (PPUNICODE_STRING)Buffer;

    Buffer += ObjectNameArraySize.LowPart;

    //
    // If our pointer arithmetic was correct, Buffer should match the base
    // address of the context plus the total allocation size at this point.
    // Assert this invariant now.
    //

    ExpectedBuffer = RtlOffsetToPointer(Context, AllocSize.LowPart);
    ASSERT(Buffer == ExpectedBuffer);

    //
    // Create the random object names for our underlying events.
    //

    Prefixes = (PPUNICODE_STRING)ContextObjectPrefixes;


    Success = Rtl->CreateRandomObjectNames(Rtl,
                                           Allocator,
                                           Allocator,
                                           NumberOfContextObjectPrefixes,
                                           64,
                                           NULL,
                                           Context->ObjectNames,
                                           Prefixes,
                                           &SizeOfNamesWideBuffer,
                                           &NamesWideBuffer);

    if (!Success) {
        goto Error;
    }

    //
    // Wire up the wide buffer pointer and containing size.
    //

    Context->ObjectNamesWideBuffer = NamesWideBuffer;
    Context->SizeOfObjectNamesWideBuffer = SizeOfNamesWideBuffer;
    Context->NumberOfObjects = NumberOfContextObjectPrefixes;

    //
    // Calculate the number of event handles based on the first and last event
    // indicators in the context structure.
    //

    NumberOfEvents = (USHORT)(
        RtlOffsetFromPointer(
            &Context->FirstEvent,
            &Context->LastEvent
        )
    );

    NumberOfEvents /= sizeof(HANDLE);

    //
    // Sanity check the number of events matches the number of event prefixes.
    //

    ASSERT(NumberOfEvents == NumberOfContextEventPrefixes);

    //
    // Initialize the event pointer to the first handle, and the name pointer
    // to the first UNICODE_STRING pointer.
    //

    Event = (PHANDLE)&Context->FirstEvent;
    Name = Context->ObjectNames[0];

    for (Index = 0; Index < NumberOfEvents; Index++, Event++, Name++) {

        //
        // We want all of our events to be manual reset, such that they stay
        // signalled even after they've satisfied a wait.
        //

        BOOLEAN ManualReset = TRUE;

        *Event = Rtl->CreateEventW(NULL,
                                   ManualReset,
                                   FALSE,
                                   Name->Buffer);

        LastError = GetLastError();

        if (*Event || LastError == ERROR_ALREADY_EXISTS) {

            //
            // As the event names are random, a last error that indicates the
            // name already exists is evident of a pretty serious problem.
            // Treat this as a fatal.
            //

            goto Error;
        }
    }

    //
    // If the maximum concurrency is 0, default to the number of processors.
    //

    if (MaximumConcurrency == 0) {
        MaximumConcurrency = GetMaximumProcessorCount(ALL_PROCESSOR_GROUPS);
    }

    Context->MinimumConcurrency = MaximumConcurrency;
    Context->MaximumConcurrency = MaximumConcurrency;

    //
    // Create the Main threadpool structures.  This threadpool creates a fixed
    // number of threads equal to the maximum concurrency specified by the user
    // (e.g. min threads is set to the same value as max threads).
    //

    Threadpool = Context->MainThreadpool = CreateThreadpool(NULL);
    if (!Threadpool) {
        goto Error;
    }

    if (!SetThreadpoolThreadMinimum(Threadpool, MaximumConcurrency)) {
        goto Error;
    }

    SetThreadpoolThreadMaximum(Threadpool, MaximumConcurrency);

    //
    // Initialize the Main threadpool environment and associate it with the
    // Main threadpool.
    //

    InitializeThreadpoolEnvironment(&Context->MainCallbackEnv);
    SetThreadpoolCallbackPool(&Context->MainCallbackEnv,
                              Context->MainThreadpool);

    //
    // Create a cleanup group for the Main threadpool and register it.
    //

    Context->MainCleanupGroup = CreateThreadpoolCleanupGroup();
    if (!Context->MainCleanupGroup) {
        goto Error;
    }

    SetThreadpoolCallbackCleanupGroup(&Context->MainCallbackEnv,
                                      Context->MainCleanupGroup,
                                      CleanupCallback);

    //
    // Create a work object for the Main threadpool.
    //

    Context->MainWork = CreateThreadpoolWork(MainCallback,
                                             Context,
                                             &Context->MainCallbackEnv);

    if (!Context->MainWork) {
        goto Error;
    }

    //
    // Initialize the main list head.
    //

    InitializeSListHead(&Context->MainListHead);

    //
    // Create the Finished and Error threadpools and associated resources.
    // These are slightly easier as we only have 1 thread maximum for each
    // pool and no cleanup group is necessary.
    //

    Context->FinishedThreadpool = CreateThreadpool(NULL);
    if (!Context->FinishedThreadpool) {
        goto Error;
    }

    if (!SetThreadpoolThreadMinimum(Context->FinishedThreadpool, 1)) {
        goto Error;
    }

    SetThreadpoolThreadMaximum(Context->FinishedThreadpool, 1);

    Context->FinishedWork = CreateThreadpoolWork(FinishedCallback,
                                                 Context,
                                                 &Context->FinishedCallbackEnv);
    if (!Context->FinishedWork) {
        goto Error;
    }

    //
    // Initialize the finished list head.
    //

    InitializeSListHead(&Context->FinishedListHead);

    //
    // Create the Error threadpool.
    //

    Context->ErrorThreadpool = CreateThreadpool(NULL);
    if (!Context->ErrorThreadpool) {
        goto Error;
    }

    if (!SetThreadpoolThreadMinimum(Context->ErrorThreadpool, 1)) {
        goto Error;
    }

    SetThreadpoolThreadMaximum(Context->ErrorThreadpool, 1);

    Context->ErrorWork = CreateThreadpoolWork(ErrorCallback,
                                              Context,
                                              &Context->ErrorCallbackEnv);
    if (!Context->ErrorWork) {
        goto Error;
    }


    //
    // We're done!  Jump to the end of the routine to finish up.
    //

    Success = TRUE;
    goto End;

Error:

    Success = FALSE;

    //
    // Call the destroy routine on the context if it is non-NULL.
    //

    if (Context) {

        if (!Api->DestroyPerfectHashTableContext(&Context, NULL)) {

            //
            // There's nothing we can do here.
            //

            NOTHING;
        }

        //
        // N.B. DestroyPerfectHashTableContext() should clear the Context
        //      pointer.  Assert that invariant now.
        //

        ASSERT(Context == NULL);
    }

    //
    // Intentional follow-on to End.
    //

End:

    //
    // Update the caller's pointer and return.
    //
    // N.B. Context could be NULL here, which is fine.
    //

    *ContextPointer = Context;

    return Success;
}

//
// Implement callback routines.
//

_Use_decl_annotations_
VOID
MainCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Ctx,
    PTP_WORK Work
    )
/*++

Routine Description:

    This is the callback routine for the Main threadpool's work.  It will be
    invoked by a thread in the Main group whenever SubmitThreadpoolWork()
    is called against Context->MainWork.  The caller is responsible for pushing
    a work item to Context->MainListHead prior to submission.

    This routine pops an item off Context->MainListHead, then calls the worker
    routine that was registered with the context.

Arguments:

    Instance - Supplies a pointer to the callback instance responsible for this
        threadpool callback invocation.

    Ctx - Supplies a pointer to the owning PERFECT_HASH_TABLE_CONTEXT.

    Work - Supplies a pointer to the TP_WORK object for this routine.

Return Value:

    None.

--*/
{
    PSLIST_ENTRY ListEntry;
    PPERFECT_HASH_TABLE_CONTEXT Context;

    //
    // Cast the Ctx variable into a suitable type, then pop a list entry off
    // the main work list head.
    //

    Context = (PPERFECT_HASH_TABLE_CONTEXT)Ctx;
    ListEntry = InterlockedPopEntrySList(&Context->MainListHead);

    if (!ListEntry) {

        //
        // A spurious work item was requested but no corresponding element was
        // pushed to the list.  This typically indicates API misuse.  We could
        // terminate here, however, that's pretty drastic, so let's just ignore
        // it.
        //

        return;
    }

    //
    // Dispatch the work item to the routine registered with the context.
    //

    Context->MainCallback(Context, ListEntry);

    return;
}

_Use_decl_annotations_
VOID
FinishedCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Ctx,
    PTP_WORK Work
    )
/*++

Routine Description:

    This routine will be called when a Main thread successfully finds a perfect
    hash table solution.  It is responsible for signaling the ShutdownEvent and
    FinishedEvent.  It will also call CloseThreadpoolCleanupGroupMembers() to
    cancel pending callbacks.

Arguments:

    Instance - Supplies a pointer to the callback instance responsible for this
        threadpool callback invocation.

    Ctx - Supplies a pointer to the owning PERFECT_HASH_TABLE_CONTEXT.

    Work - Supplies a pointer to the TP_WORK object for this routine.

Return Value:

    None.

--*/
{
    PPERFECT_HASH_TABLE_CONTEXT Context;

    //
    // Cast the Ctx variable into a suitable type.
    //

    Context = (PPERFECT_HASH_TABLE_CONTEXT)Ctx;

    //
    // Signal the shutdown and succeeded events.
    //

    SetEvent(Context->ShutdownEvent);
    SetEvent(Context->SucceededEvent);

    //
    // Cancel the main thread work group members.  This should block until all
    // the workers have returned.
    //

    CloseThreadpoolCleanupGroupMembers(Context->MainCleanupGroup,
                                       TRUE,
                                       NULL);

    //
    // Clear the MainWork member to make it explicit that it has been closed.
    //

    Context->MainWork = NULL;

    //
    // Set the completed event and return.
    //

    SetEvent(Context->CompletedEvent);

    return;
}

_Use_decl_annotations_
VOID
ErrorCallback(
    PTP_CALLBACK_INSTANCE Instance,
    PVOID Ctx,
    PTP_WORK Work
    )
/*++

Routine Description:

    Not yet implemented.

Arguments:

    Instance - Supplies a pointer to the callback instance responsible for this
        threadpool callback invocation.

    Ctx - Supplies a pointer to the owning PERFECT_HASH_TABLE_CONTEXT.

    Work - Supplies a pointer to the TP_WORK object for this routine.

Return Value:

    None.

--*/
{
    //
    // N.B. This routine has not been implemented yet.  Main algorithm worker
    //      threads cannot cause termination of the entire context.
    //

    return;
}

_Use_decl_annotations_
VOID
CleanupCallback(
    PVOID ObjectContext,
    PVOID CleanupContext
    )
/*++

Routine Description:

    This method is called when the Main threadpool context is closed via the
    CloseThreadpoolCleanupGroupMembers() routine, which will be issued by either
    the Finished or Error threadpool.

Arguments:

    ObjectContext - Supplies a pointer to the PERFECT_HASH_TABLE_CONTEXT
        structure for which this cleanup was associated.

    CleanupContext - Optionally supplies per-cleanup context information at the
        time CloseThreadpoolCleanupGroupMembers() was called.  Not currently
        used.

Return Value:

    None.

--*/
{
    PPERFECT_HASH_TABLE_CONTEXT Context;


    Context = (PPERFECT_HASH_TABLE_CONTEXT)ObjectContext;

    //
    // (This is placeholder scaffolding at the moment.  We don't do anything
    //  explicit in this callback currently.)
    //

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
