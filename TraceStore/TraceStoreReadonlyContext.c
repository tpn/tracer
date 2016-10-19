/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreReadonlyContext.c

Abstract:

    This module implements functionality supporting readonly trace contexts.
    Functions are provided for initializing a readonly trace context and
    binding trace stores to a context.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
InitializeReadonlyTraceContext(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PREADONLY_TRACE_CONTEXT ReadonlyTraceContext,
    PULONG SizeOfReadonlyTraceContext,
    PTRACE_STORES TraceStores,
    PTP_CALLBACK_ENVIRON ThreadpoolCallbackEnvironment,
    PVOID UserData
    )
/*--

Routine Description:

    This routine initializes a READONLY_TRACE_CONTEXT structure.  This
    involves setting relevant fields in the structure then binding the
    context to the trace stores.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure.

    TraceContext - Supplies a pointer to a READONLY_TRACE_CONTEXT structure.

    SizeOfReadonlyTraceContext - Supplies a pointer to a variable that
        contains the buffer size allocated for the ReadonlyTraceContext
        parameter.  The actual size of the structure will be written to
        this variable.

    TraceSession - Supplies a pointer to a TRACE_SESSION structure.

    TraceStores - Supplies a pointer to a TRACE_STORES structure to bind the
        readonly trace context to.  TraceStores->Flags.Readonly must be
        TRUE otherwise an error is returned.

    ThreadpoolCallbackEnvironment - Supplies a pointer to a threadpool callback
        environment to use for the readonly trace context.  This threadpool
        will be used to submit various asynchronous thread pool memory map
        operations.

    UserData - Supplies an optional pointer to user data that can be used by
        a caller to track additional context information via the UserData
        parameter of the READONLY_TRACE_CONTEXT structure.

Return Value:

    TRUE on success, FALSE on failure.  The required buffer size for the
    READONLY_TRACE_CONTEXT structure can be obtained by passing in a valid
    pointer for SizeOfReadonlyTraceContext and NULL for the remaining
    parameters.

    This method is asynchronous.  A successful return indicates that the
    loading operations of all trace stores have been initiated.  The event
    ReadonlyTraceContext->LoadingCompleteEvent will be set when the loading
    has completed.  To check whether the loading was successful, a thread
    would wait on this event, then check Flags.IsValid == TRUE to confirm
    the loading occurred successfully.

--*/
{
    //USHORT Index;
    //USHORT StoreIndex;

    //
    // Validate sizes.
    //

    if (!ReadonlyTraceContext) {
        if (SizeOfReadonlyTraceContext) {
            *SizeOfReadonlyTraceContext = sizeof(*ReadonlyTraceContext);
        }
        return FALSE;
    }

    if (!SizeOfReadonlyTraceContext) {
        return FALSE;
    }

    if (*SizeOfReadonlyTraceContext < sizeof(*ReadonlyTraceContext)) {
        *SizeOfReadonlyTraceContext = sizeof(*ReadonlyTraceContext);
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

    //
    // Make sure the trace stores is also readonly.
    //

    if (!TraceStores->Flags.Readonly) {
        return FALSE;
    }

    //
    // Zero memory and initialize fields.
    //

    SecureZeroMemory(ReadonlyTraceContext, sizeof(*ReadonlyTraceContext));
    ReadonlyTraceContext->SizeOfStruct = sizeof(*ReadonlyTraceContext);
    ReadonlyTraceContext->Rtl = Rtl;
    ReadonlyTraceContext->Allocator = Allocator;
    ReadonlyTraceContext->Directory = &TraceStores->BaseDirectory;


    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
