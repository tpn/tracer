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
/*--

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
    BOOL IsPageAligned;
    PVOID Buffer;
    HRESULT Result;
    USHORT NumaNode;
    TRACE_STORE_ADDRESS Address;
    PALLOCATE_RECORDS AllocateRecords;
    PTRACE_STORE AddressStore;
    PTRACE_CONTEXT Context;

    ULARGE_INTEGER AddressRecordSize = { sizeof(Address) };
    ULARGE_INTEGER NumberOfAddressRecords = { 1 };

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(AddressPointer)) {
        return FALSE;
    }

    if (IsMetadataTraceStore(TraceStore)) {

        //
        // Should never be called against a metadata store.
        //

        __debugbreak();

        return FALSE;
    }

    IsPageAligned = (BOOL)(
        AddressRecordSize.QuadPart & (PAGE_SIZE-1)
    );

    if (!IsPageAligned) {
        __debugbreak();
        return FALSE;
    }

    Rtl = TraceStore->Rtl;
    Context = TraceStore->TraceContext;
    AddressStore = TraceStore->AddressStore;
    AllocateRecords = AddressStore->AllocateRecords;

    Buffer = AllocateRecords(Context,
                             AddressStore,
                             &AddressRecordSize,
                             &NumberOfAddressRecords);

    if (!Buffer) {
        return FALSE;
    }

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


    Result = Rtl->RtlCopyMappedMemory(Buffer,
                                      &Address,
                                      sizeof(Address));

    if (SUCCEEDED(Result)) {
        *AddressPointer = (PTRACE_STORE_ADDRESS)Buffer;
        return TRUE;
    }

    return FALSE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
