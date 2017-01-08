/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreAddress.c

Abstract:

    This module implements functionality related to the trace store address
    structure.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
LoadNextTraceStoreAddress(
    PTRACE_STORE TraceStore,
    PPTRACE_STORE_ADDRESS AddressPointer
    )
/*++

Routine Description:

    This routine loads a trace store's next address into a TRACE_STORE_ADDRESS
    structure.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that the address
        is to be loaded for.

    AddressPointer - Supplies a pointer to a variable that will receive the
        address of the loaded TRACE_STORE_ADDRESS structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PRTL Rtl;
    BOOL Success;
    PVOID Buffer;
    USHORT NumaNode;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS pAddress;
    PALLOCATE_RECORDS AllocateRecords;
    PTRACE_STORE AddressStore;
    PTRACE_CONTEXT Context;

    ULONG_PTR AddressRecordSize = sizeof(Address);
    ULONG_PTR NumberOfAddressRecords = 1;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AddressPointer)) {
        return FALSE;
    }

    if (TraceStore->IsReadonly) {
        __debugbreak();
        return FALSE;
    }

    //
    // Clear the caller's pointer up front.
    //

    *AddressPointer = NULL;

    if (IsMetadataTraceStore(TraceStore)) {

        //
        // Should never be called against a metadata store.
        //

        __debugbreak();

        return FALSE;
    }

    if (PAGE_SIZE % AddressRecordSize) {

        //
        // The record isn't evenly divisible by PAGE_SIZE.
        //

        __debugbreak();
        return FALSE;
    }

    Rtl = TraceStore->Rtl;
    Context = TraceStore->TraceContext;
    AddressStore = TraceStore->AddressStore;
    AllocateRecords = AddressStore->AllocateRecords;

    Buffer = AllocateRecords(Context,
                             AddressStore,
                             NumberOfAddressRecords,
                             AddressRecordSize);

    if (!Buffer) {
        return FALSE;
    }

    pAddress = (PTRACE_STORE_ADDRESS)Buffer;

    SecureZeroMemory(&Address, sizeof(Address));

    Address.MappedSequenceId = (
        InterlockedIncrement(&TraceStore->MappedSequenceId)
    );

    Address.ProcessId = FastGetCurrentProcessId();
    Address.RequestingThreadId = FastGetCurrentThreadId();

    GetCurrentProcessorNumberEx(&Address.RequestingProcessor);

    Success = GetNumaProcessorNodeEx(&Address.RequestingProcessor, &NumaNode);

    if (Success) {
        Address.RequestingNumaNode = (UCHAR)NumaNode;
    } else {
        Address.RequestingNumaNode = 0;
    }

    if (!CopyTraceStoreAddress(pAddress, &Address)) {
        return FALSE;
    }

    TraceStore->Address = pAddress;

    *AddressPointer = (PTRACE_STORE_ADDRESS)Buffer;
    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
