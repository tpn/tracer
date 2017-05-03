/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreMemoryMap.c

Abstract:

    This module implements functionality related to trace store memory maps.
    These are the workhorse of the trace store functionality.  Functions are
    provided for preparing, consuming, flushing, retiring, closing, and
    releasing memory maps.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
GetNumberOfMemoryMapsRequiredByTraceStore(
    PTRACE_STORE TraceStore,
    PULONG NumberOfMapsPointer
    )
/*++

Routine Description:

    This routine calculates the number of memory maps required for a trace
    store.  The trace store traits and whether or not it is readonly are used
    to infer the number of trace stores to create.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        number of memory maps required will be calculated.

    NumberOfMapsPointer - Supplies the address of a ULONG variable that will
        receive the number of maps to create.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL IsReadonly;
    BOOL IsMetadata;
    ULONG NumberOfMaps;
    ULONG Multiplier;
    TRACE_STORE_TRAITS Traits;

    Traits = *TraceStore->pTraits;

    //
    // Clear the caller's pointer up-front.
    //

    *NumberOfMapsPointer = 0;

    //
    // Initialize aliases.
    //

    IsReadonly = TraceStore->IsReadonly;
    IsMetadata = IsMetadataTraceStore(TraceStore);

    //
    // Initialize multiplier.
    //

    if (IsFrequentAllocator(Traits)) {
        Multiplier = InitialFreeMemoryMapMultiplierForFrequentAllocators;
    } else {
        Multiplier = 0;
    }

    if (IsSingleRecord(Traits) || (IsReadonly && IsMetadata)) {

        //
        // Make sure a memory map hasn't already been assigned.
        //

        if (TraceStore->TotalNumberOfMemoryMaps > 0) {
            __debugbreak();
            return FALSE;
        }

        NumberOfMaps = 1;
        goto End;
    }

    if (IsReadonly) {
        if (IsStreamingRead(Traits)) {
            NumberOfMaps = InitialFreeMemoryMapsForStreamingReaders;
        } else if (IsMetadata) {
            NumberOfMaps = InitialFreeMemoryMapsForNonStreamingMetadataReaders;
        } else {
            NumberOfMaps = InitialFreeMemoryMapsForNonStreamingReaders;
        }
    } else {
        if (IsStreamingWrite(Traits)) {
            NumberOfMaps = InitialFreeMemoryMapsForStreamingWriters;
        } else if (IsMetadata) {
            NumberOfMaps = InitialFreeMemoryMapsForNonStreamingMetadataWriters;
        } else {
            NumberOfMaps = InitialFreeMemoryMapsForNonStreamingWriters;
        }
    }

    //
    // Sanity check the final number isn't zero, apply the multiplier if
    // applicable, then verify our final number is a power of two.
    //

    if (NumberOfMaps == 0) {
        __debugbreak();
        return FALSE;
    }

    if (Multiplier) {
        NumberOfMaps *= Multiplier;
    }

    if (!IsPowerOf2(NumberOfMaps)) {
        __debugbreak();
        return FALSE;
    }

End:

    //
    // Update the caller's pointer and return success.
    //

    *NumberOfMapsPointer = NumberOfMaps;

    return TRUE;
}

_Use_decl_annotations_
BOOL
CreateMemoryMapsForTraceStore(
    PTRACE_STORE TraceStore,
    PPTRACE_STORE_MEMORY_MAP MemoryMapPointer,
    PULONG NumberOfMemoryMapsPointer
    )
/*++

Routine Description:

    This routine creates memory maps for a given trace store.  It is
    called at least once per trace store, during the initial binding.
    If the value pointed to by NumberOfMemoryMapsPointer is 0, the
    routine will call GetNumberOfMemoryMapsRequiredByTraceStore() to
    obtain the number of maps to create.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        memory maps are to be created.

    MemoryMapPointer - Supplies the address of a variable that will receive
        the address of a usable TRACE_STORE_MEMORY_MAP provided no errors were
        encountered.  All callers of this routine will be in a position where
        they need a memory map, so having this as an output parameter saves the
        caller from popping a memory map off the free list once this routine
        returns successfully.

    NumberOfMemoryMapsPointer - Supplies a pointer to a ULONG variable that
        that indicates the desired number of memory maps to create.  If 0,
        the GetNumberOfMemoryMapsRequiredByTraceStore() method will be called
        to obtain the number of memory maps to create.  The pointer is updated
        with the actual number of maps created.

Return Value:

    TRUE on success, FALSE on failure.  If TRUE, *MemoryMapPointer will be
    non-NULL.  If FALSE, *MemoryMapPointer will be NULL.

--*/
{
    BOOL Success;
    ULONG NumberOfMaps;
    ULONG Index;
    TRACE_STORE_TRAITS Traits;
    PALLOCATOR Allocator;
    PTRACE_STORE_MEMORY_MAP MemoryMapToReturn;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_MEMORY_MAP NextMemoryMap;
    PTRACE_STORE_MEMORY_MAP LastMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMaps;

    Traits = *TraceStore->pTraits;

    //
    // Clear the caller's pointer up-front.
    //

    *MemoryMapPointer = NULL;

    //
    // If the caller hasn't specified the number of memory maps to create,
    // call GetNumberOfMemoryMapsRequiredByTraceStore() to obtain the number.
    //

    NumberOfMaps = *NumberOfMemoryMapsPointer;
    if (!NumberOfMaps) {
        Success = GetNumberOfMemoryMapsRequiredByTraceStore(
            TraceStore,
            NumberOfMemoryMapsPointer
        );

        //
        // Update our local copy of the number of maps.
        //

        NumberOfMaps = *NumberOfMemoryMapsPointer;
    }

    //
    // If only a single memory map is required, we can use the SingleMemoryMap
    // structure embedded within every trace store, removing the need to call
    // the allocator.
    //

    if (NumberOfMaps == 1) {
        MemoryMapToReturn = &TraceStore->SingleMemoryMap;
        goto End;
    }

    //
    // Allocate space for an array of memory maps.
    //

    Allocator = TraceStore->pAllocator;
    MemoryMaps = (PTRACE_STORE_MEMORY_MAP)(
        Allocator->Calloc(
            Allocator->Context,
            NumberOfMaps,
            sizeof(TRACE_STORE_MEMORY_MAP)
        )
    );

    if (!MemoryMaps) {
        __debugbreak();
        return FALSE;
    }

#define IS_ALIGNED(Address) (((ULONG_PTR)(Address) & 15) == 0)

    //
    // Alignment sanity check.
    //

    if (!IS_ALIGNED(MemoryMaps)) {
        __debugbreak();
        Allocator->Free(Allocator->Context, MemoryMaps);
        return FALSE;
    }

    //
    // Reserve the first memory map in the array to return to the caller.
    //

    MemoryMapToReturn = MemoryMaps;

    //
    // The remaining maps need to be pushed onto the interlocked free list.
    // If there is only one map remaining (i.e. the total number of maps was
    // 2), we just do an interlocked push.  Otherwise, we loop through and wire
    // up all the SLIST_ENTRY.Next pointers such that each one points to the
    // element after it, then call InterlockedPushListSListEx() to push the
    // entire list in one operation.
    //

    FirstMemoryMap = &MemoryMaps[1];

    if (NumberOfMaps == 2) {
        PushTraceStoreMemoryMap(&TraceStore->FreeMemoryMaps, FirstMemoryMap);
        goto End;
    }

    //
    // Enumerate the remaining maps, excluding the last one, linking each one
    // to the one after it.
    //

    for (Index = 1; Index < (NumberOfMaps-1); Index++) {
        NextMemoryMap = &MemoryMaps[Index];
        NextMemoryMap->ListEntry.Next = &((NextMemoryMap + 1)->ListEntry);
    }

    LastMemoryMap = &MemoryMaps[NumberOfMaps-1];

    //
    // Push the singly-linked list onto the free list.
    //

    InterlockedPushListSListEx(&TraceStore->FreeMemoryMaps,
                               &FirstMemoryMap->ListEntry,
                               &LastMemoryMap->ListEntry,
                               NumberOfMaps - 1);

End:

    //
    // Because we return a free memory map to the caller, we need to manually
    // increment the number of active memory maps in use.  (This is normally
    // handled by PopFreeTraceStoreMemoryMap().)
    //

    InterlockedIncrement(&TraceStore->NumberOfActiveMemoryMaps);
    InterlockedAdd(&TraceStore->TotalNumberOfMemoryMaps, NumberOfMaps);
    *MemoryMapPointer = MemoryMapToReturn;

    return TRUE;
}

_Use_decl_annotations_
BOOL
CreateMemoryMapsForReadonlyTraceStore(
    PTRACE_STORE TraceStore,
    PPTRACE_STORE_MEMORY_MAP MemoryMapPointer,
    PULONG NumberOfMemoryMapsPointer
    )
/*++

Routine Description:

    This routine creates memory maps for readonly trace stores.  This is a
    specialized version of CreateMemoryMapsForTraceStore() that does not push
    the memory maps to the trace store's free list or adjust the relevant
    counters.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        memory maps are to be created.

    MemoryMapPointer - Supplies the address of a variable that will receive
        the address of the first memory map in the array of memory maps
        created by this routine.

    NumberOfMemoryMapsPointer - Supplies a pointer to a ULONG variable that
        that indicates the desired number of memory maps to create.  This value
        cannot be 0.

Return Value:

    TRUE on success, FALSE on failure.  If TRUE, *MemoryMapPointer will be
    non-NULL.  If FALSE, *MemoryMapPointer will be NULL.

--*/
{
    ULONG NumberOfMaps;
    TRACE_STORE_TRAITS Traits;
    PALLOCATOR Allocator;
    PTRACE_STORE_MEMORY_MAP MemoryMaps;

    Traits = *TraceStore->pTraits;

    //
    // Clear the caller's pointer up-front.
    //

    *MemoryMapPointer = NULL;

    //
    // Verify number of maps is non-zero.
    //

    NumberOfMaps = *NumberOfMemoryMapsPointer;
    if (!NumberOfMaps) {
        __debugbreak();
        return FALSE;
    }

    //
    // If only a single memory map is required, we can use the SingleMemoryMap
    // structure embedded within every trace store, removing the need to call
    // the allocator.
    //

    if (NumberOfMaps == 1) {
        *MemoryMapPointer = &TraceStore->SingleMemoryMap;
        return TRUE;
    }

    //
    // Allocate space for an array of memory maps.
    //

    Allocator = TraceStore->pAllocator;
    MemoryMaps = (PTRACE_STORE_MEMORY_MAP)(
        Allocator->Calloc(
            Allocator->Context,
            NumberOfMaps,
            sizeof(TRACE_STORE_MEMORY_MAP)
        )
    );

    if (!MemoryMaps) {
        __debugbreak();
        return FALSE;
    }

    //
    // If the allocation isn't aligned on a 16 byte boundary, bump it forward
    // 8 bytes.
    //

#define IS_ALIGNED(Address) (((ULONG_PTR)(Address) & 15) == 0)

    //
    // Alignment sanity check.
    //

    if (!IS_ALIGNED(MemoryMaps)) {
        __debugbreak();
        Allocator->Free(Allocator->Context, MemoryMaps);
        return FALSE;
    }

    *MemoryMapPointer = MemoryMaps;
    return TRUE;
}

_Use_decl_annotations_
BOOL
PrepareNextTraceStoreMemoryMap(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine is responsible for preparing the next memory map for a trace
    store given the current state of the underlying file (in terms of where
    the current file pointer is).  It is also called to initialize the first
    memory map used by a trace store.

    The routine will query the current file offset and adjust the file pointer
    if necessary.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        memory map is to be prepared.

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure to
        be used for the memory map preparation.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsFirstMap;
    BOOL IsMetadata;
    BOOL PreferredAddressUnavailable;
    USHORT NumaNode;
    PVOID EndAddress;
    PVOID PreferredBaseAddress;
    PVOID OriginalPreferredBaseAddress;
    TRACE_STORE_ADDRESS Address;
    TRACE_STORE_ADDRESS_RANGE AddressRange;
    PTRACE_STORE_ADDRESS AddressPointer;
    FILE_STANDARD_INFO FileInfo;
    LARGE_INTEGER CurrentFileOffset;
    LARGE_INTEGER NewFileOffset;
    LARGE_INTEGER DistanceToMove;
    LARGE_INTEGER Timestamp;
    LARGE_INTEGER Elapsed;
    PTRACE_STORE_STATS Stats;
    TRACE_STORE_STATS DummyStats = { 0 };

    //
    // Readonly trace stores are handled by a separate routine.  Ensure
    // we're not readonly now.
    //

    if (TraceStore->IsReadonly) {
        __debugbreak();
        return FALSE;
    }

    //
    // Initialize aliases.
    //

    IsMetadata = IsMetadataTraceStore(TraceStore);
    Stats = TraceStore->Stats;

    if (!Stats) {
        Stats = &DummyStats;
    }

    //
    // Make a note if this is the first memory map we've been requested to
    // prepare.  We use this information down the track with regards to the
    // preparation of the address range structure.
    //

    IsFirstMap = (MemoryMap->FileOffset.QuadPart == 0);

    //
    // Get the size of the file from the memory map.
    //

    if (!GetTraceStoreMemoryMapFileInfo(MemoryMap, &FileInfo)) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    //
    // Get the current file offset.
    //

    DistanceToMove.QuadPart = 0;

    Success = SetFilePointerEx(MemoryMap->FileHandle,
                               DistanceToMove,
                               &CurrentFileOffset,
                               FILE_CURRENT);
    if (!Success) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    if (CurrentFileOffset.QuadPart != MemoryMap->FileOffset.QuadPart) {

        //
        // This shouldn't occur if all our memory map machinery is working
        // correctly, as maps are prepared and consumed in-order.
        //

        __debugbreak();
        return FALSE;
    }

    //
    // Determine the distance we need to move the file pointer.
    //

    DistanceToMove.QuadPart = (
        MemoryMap->FileOffset.QuadPart +
        MemoryMap->MappingSize.QuadPart
    );

    //
    // Adjust the file pointer from the current position.
    //

    Success = SetFilePointerEx(MemoryMap->FileHandle,
                               DistanceToMove,
                               &NewFileOffset,
                               FILE_BEGIN);
    if (!Success) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    //
    // XXX: skip SetEndOfFile() whilst evaluating larger address ranges.
    //

    goto CreateSection;

    //
    // If the new file offset is past the end of the file, extend it.
    //
    // N.B. This will be a synchronous (blocking) I/O call.
    //

    if (FileInfo.EndOfFile.QuadPart < NewFileOffset.QuadPart) {
        if (!SetEndOfFile(MemoryMap->FileHandle)) {
            TraceStore->LastError = GetLastError();
            __debugbreak();
            return FALSE;
        }
    }

    //
    // Create a new file mapping for this memory map's slice of data.
    //

CreateSection:

    MemoryMap->MappingHandle = CreateFileMappingNuma(
        MemoryMap->FileHandle,
        NULL,
        TraceStore->CreateFileMappingProtectionFlags,
        NewFileOffset.HighPart,
        NewFileOffset.LowPart,
        NULL,
        TraceStore->NumaNode
    );

    if (MemoryMap->MappingHandle == NULL ||
        MemoryMap->MappingHandle == INVALID_HANDLE_VALUE) {
        TraceStore->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    OriginalPreferredBaseAddress = NULL;
    PreferredAddressUnavailable = FALSE;

    //
    // MemoryMap->BaseAddress will be the next contiguous address for this
    // mapping if we're in write mode.  If we're readonly and the value is
    // non-NULL, it is the address we need to map the view at in order to
    // avoid relocations.
    //
    // In both cases, our first call to MapViewOfFileExNuma() attempts to
    // honor this preferred base address.
    //

    PreferredBaseAddress = MemoryMap->BaseAddress;
    AddressPointer = MemoryMap->pAddress;

    if (IsMetadata) {
        goto TryMapMemory;
    }

    //
    // Take a local copy of the address.
    //

    if (!CopyTraceStoreAddress(&Address, AddressPointer)) {
        __debugbreak();
        return FALSE;
    }

TryMapMemory:

    MemoryMap->BaseAddress = MapViewOfFileExNuma(
        MemoryMap->MappingHandle,
        TraceStore->MapViewOfFileDesiredAccess,
        MemoryMap->FileOffset.HighPart,
        MemoryMap->FileOffset.LowPart,
        MemoryMap->MappingSize.QuadPart,
        PreferredBaseAddress,
        TraceStore->NumaNode
    );

    if (!MemoryMap->BaseAddress) {
        if (PreferredBaseAddress) {

            //
            // Make a note of the original preferred base address, clear it,
            // then attempt the mapping again.
            //

            OriginalPreferredBaseAddress = PreferredBaseAddress;
            PreferredBaseAddress = NULL;
            PreferredAddressUnavailable = TRUE;
            Stats->PreferredAddressUnavailable++;
            goto TryMapMemory;
        }

        //
        // The map view attempt failed for some reason other than the base
        // address being unavailable.
        //

        TraceStore->LastError = GetLastError();
        __debugbreak();
        return FALSE;
    }

    if (IsMetadata) {
        goto Finalize;
    }

    if (!PreferredBaseAddress) {
        PreferredBaseAddress = MemoryMap->BaseAddress;
    } else if (OriginalPreferredBaseAddress) {
        PreferredBaseAddress = OriginalPreferredBaseAddress;
    }

    //
    // Record all of the mapping information in our address record.
    //

    Address.PreferredBaseAddress = PreferredBaseAddress;
    Address.BaseAddress = MemoryMap->BaseAddress;
    Address.FileOffset.QuadPart = MemoryMap->FileOffset.QuadPart;
    Address.MappedSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    //
    // Fill in the thread and processor information.
    //

    Address.FulfillingThreadId = FastGetCurrentThreadId();
    GetCurrentProcessorNumberEx(&Address.FulfillingProcessor);
    Success = GetNumaProcessorNodeEx(&Address.FulfillingProcessor, &NumaNode);
    Address.FulfillingNumaNode = (Success ? (UCHAR)NumaNode : 0);

    //
    // Take a local copy of the timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed, &Timestamp);

    //
    // Copy it to the Prepared timestamp.
    //

    Address.Timestamp.Prepared.QuadPart = Elapsed.QuadPart;

    //
    // Calculate the elapsed time spent awaiting preparation.
    //

    Elapsed.QuadPart -= Address.Timestamp.Requested.QuadPart;
    Address.Elapsed.AwaitingPreparation.QuadPart = Elapsed.QuadPart;

    //
    // Finally, copy the updated record back to the memory-mapped backing
    // store.
    //

    if (!CopyTraceStoreAddress(AddressPointer, &Address)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Update the address range details.  If this is the first map, or the
    // preferred address wasn't available, fill out a new local address range
    // structure and register it.  Otherwise, update the existing address
    // range's number of maps counter and mapped size.
    //

    EndAddress = (PVOID)(
        RtlOffsetToPointer(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.QuadPart
        )
    );

    if (IsFirstMap || PreferredAddressUnavailable) {

        AddressRange.PreferredBaseAddress = PreferredBaseAddress;
        AddressRange.ActualBaseAddress = MemoryMap->BaseAddress;
        AddressRange.EndAddress = EndAddress;
        AddressRange.MappedSize.QuadPart = MemoryMap->MappingSize.QuadPart;

        //
        // Update the bit counts.
        //

        AddressRange.BitCounts.Preferred = (
            GetTraceStoreAddressBitCounts(
                AddressRange.PreferredBaseAddress
            )
        );

        AddressRange.BitCounts.Actual = (
            GetTraceStoreAddressBitCounts(
                AddressRange.ActualBaseAddress
            )
        );

        //
        // Register this new address range.
        //

        Success = RegisterNewTraceStoreAddressRange(TraceStore, &AddressRange);

        if (!Success) {
            __debugbreak();
            return FALSE;
        }

    } else {

        TRY_MAPPED_MEMORY_OP {

            TraceStore->AddressRange->EndAddress = EndAddress;
            TraceStore->AddressRange->MappedSize.QuadPart += (
                MemoryMap->MappingSize.QuadPart
            );

        } CATCH_STATUS_IN_PAGE_ERROR {

            __debugbreak();
            return FALSE;
        }
    }

    //
    // Intentional follow-on.
    //

Finalize:

    //
    // Initialize the next address to the base address.
    //

    MemoryMap->NextAddress = MemoryMap->BaseAddress;

    if (!TraceStore->NoPrefaulting) {

        //
        // Make sure we don't prefault a page past the end of the file.
        // This can happen with the smaller metadata stores that only memory
        // map their exact structure size.
        //

        ULONG_PTR BaseAddress = (ULONG_PTR)MemoryMap->BaseAddress;
        ULONG_PTR PrefaultPage = BaseAddress + (PAGE_SIZE << 1);
        ULONG_PTR EndPage = BaseAddress + (ULONG_PTR)NewFileOffset.QuadPart;

        if (PrefaultPage < EndPage) {

            //
            // Prefault the first two pages.  The AllocateRecords function will
            // take care of prefaulting subsequent pages.  This will be the
            // first point of failure if we've run out of storage space on the
            // device.
            //

            if (!TraceStore->Rtl->PrefaultPages((PVOID)BaseAddress, 2)) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

_Use_decl_annotations_
BOOL
PrepareReadonlyTraceStoreMemoryMap(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine is responsible for preparing a readonly memory map for a trace
    store.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure for which the
        memory map is to be prepared.

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure to
        be used for the memory map preparation.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsReadonly;
    BOOL PreferredAddressUnavailable;
    BOOL IgnorePreferredAddresses;
    USHORT NumaNode;
    PVOID EndAddress;
    PVOID PreferredBaseAddress;
    PVOID OriginalPreferredBaseAddress;
    TRACE_STORE_ADDRESS Address;
    TRACE_STORE_ADDRESS_RANGE AddressRange;
    PTRACE_STORE_ADDRESS AddressPointer;
    LARGE_INTEGER Elapsed;
    LARGE_INTEGER Timestamp;

    //
    // Initialize aliases.
    //

    IsReadonly = IsReadonlyTraceStore(TraceStore);
    IgnorePreferredAddresses = TraceStore->IgnorePreferredAddresses;

    //
    // Ensure we're readonly.
    //

    if (!IsReadonly) {
        __debugbreak();
        return FALSE;
    }

    //
    // Ensure we've got a mapping handle.
    //

    if (!MemoryMap->MappingHandle) {
        __debugbreak();
        return FALSE;
    }

    //
    // If we're configured to ignore preferred base addresses, capture the
    // original preferred base address now, and clear the preferred base
    // address pointer we pass to MapViewOfFileExNuma().  Otherwise, use
    // the preferred base address stored in the memory map.
    //

    if (IgnorePreferredAddresses) {
        OriginalPreferredBaseAddress = MemoryMap->PreferredBaseAddress;
        PreferredBaseAddress = NULL;
        PreferredAddressUnavailable = TRUE;
        TraceStore->ReadonlyPreferredAddressUnavailable++;
    } else {
        OriginalPreferredBaseAddress = NULL;
        PreferredBaseAddress = MemoryMap->PreferredBaseAddress;
        PreferredAddressUnavailable = FALSE;
    }

    //
    // Initialize the address pointer alias and make sure it has a value.
    //

    AddressPointer = MemoryMap->pAddress;

    if (!AddressPointer) {
        __debugbreak();
        return FALSE;
    }

    //
    // Take a local copy of the address.
    //

    if (!CopyTraceStoreAddress(&Address, AddressPointer)) {
        return FALSE;
    }

TryMapMemory:

    MemoryMap->BaseAddress = MapViewOfFileExNuma(
        MemoryMap->MappingHandle,
        TraceStore->MapViewOfFileDesiredAccess,
        MemoryMap->FileOffset.HighPart,
        MemoryMap->FileOffset.LowPart,
        MemoryMap->MappingSize.QuadPart,
        PreferredBaseAddress,
        TraceStore->NumaNode
    );

    //
    // Verify the base address and potentially retry mapping again if it looks
    // like the mapping failed because the address couldn't be assigned.
    //

    if (IgnorePreferredAddresses) {

        if (!MemoryMap->BaseAddress) {
            TraceStore->LastError = GetLastError();
            return FALSE;
        }

        //
        // Restore the original preferred base address.
        //

        PreferredBaseAddress = OriginalPreferredBaseAddress;

    } else if (!MemoryMap->BaseAddress) {

        if (PreferredBaseAddress) {

            //
            // Make a note of the original preferred base address, clear it,
            // then attempt the mapping again.
            //

            OriginalPreferredBaseAddress = PreferredBaseAddress;
            PreferredBaseAddress = NULL;
            PreferredAddressUnavailable = TRUE;
            TraceStore->ReadonlyPreferredAddressUnavailable++;
            goto TryMapMemory;
        }

        //
        // The map view attempt failed for some reason other than the base
        // address being unavailable.
        //

        TraceStore->LastError = GetLastError();
        return FALSE;

    } else {

        //
        // The mapping was successful and we weren't ignoring preferred
        // addresses.  Restore the original preferred address from either
        // the memory map or the original value.
        //

        if (!PreferredBaseAddress) {
            PreferredBaseAddress = MemoryMap->BaseAddress;
        } else if (OriginalPreferredBaseAddress) {
            PreferredBaseAddress = OriginalPreferredBaseAddress;
        }
    }

    //
    // Record all of the mapping information in our address record.
    //

    Address.PreferredBaseAddress = PreferredBaseAddress;
    Address.BaseAddress = MemoryMap->BaseAddress;
    Address.FileOffset.QuadPart = MemoryMap->FileOffset.QuadPart;
    Address.MappedSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    //
    // Fill in the thread and processor information.
    //

    Address.FulfillingThreadId = FastGetCurrentThreadId();
    GetCurrentProcessorNumberEx(&Address.FulfillingProcessor);
    Success = GetNumaProcessorNodeEx(&Address.FulfillingProcessor, &NumaNode);
    Address.FulfillingNumaNode = (Success ? (UCHAR)NumaNode : 0);

    //
    // Take a local copy of the timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed, &Timestamp);

    //
    // Copy it to the Prepared timestamp.
    //

    Address.Timestamp.Prepared.QuadPart = Timestamp.QuadPart;

    //
    // Calculate the elapsed time spent awaiting preparation.
    //

    Elapsed.QuadPart -= Address.Timestamp.Requested.QuadPart;
    Address.Elapsed.AwaitingPreparation.QuadPart = Elapsed.QuadPart;

    //
    // Finally, copy the updated record back to the memory-mapped backing
    // store.
    //

    if (!CopyTraceStoreAddress(AddressPointer, &Address)) {
        return FALSE;
    }

    //
    // Update the address range details.  When we're readonly, there's a 1:1
    // map between memory maps and address ranges, so we record the address
    // range details for every successful memory map that's been prepared.
    //

    EndAddress = (PVOID)(
        RtlOffsetToPointer(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.QuadPart
        )
    );

    AddressRange.PreferredBaseAddress = PreferredBaseAddress;
    AddressRange.ActualBaseAddress = MemoryMap->BaseAddress;
    AddressRange.EndAddress = EndAddress;
    AddressRange.MappedSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    //
    // Update the bit counts.
    //

    AddressRange.BitCounts.Preferred = (
        GetTraceStoreAddressBitCounts(
            AddressRange.PreferredBaseAddress
        )
    );

    AddressRange.BitCounts.Actual = (
        GetTraceStoreAddressBitCounts(
            AddressRange.ActualBaseAddress
        )
    );

    //
    // Register this new address range.
    //

    Success = (
        RegisterNewReadonlyTraceStoreAddressRange(
            TraceStore,
            &AddressRange,
            MemoryMap
        )
    );

    return Success;
}

_Use_decl_annotations_
BOOL
CloseTraceStoreMemoryMap(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine closes a trace store memory map.  This flushes the view of
    the file at the memory map's base address, then unmaps it and clears the
    base address pointer, then closes the corresponding memory map handle
    and clears that pointer.

    This routine is forgiving: it can be called with a NULL pointer, a NULL
    BaseAddress pointer, or a NULL MappingHandle, simplifying error handling
    logic.

    BaseAddress and MappingHandle will be cleared after the flushing/unmapping
    and handle closing has been done, respectively.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    None.

--*/
{
    if (!UnmapTraceStoreMemoryMap(MemoryMap)) {
        __debugbreak();
    }
    FinalizeTraceStoreAddressTimes(TraceStore, MemoryMap->pAddress);
    ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);
    return TRUE;
}


_Use_decl_annotations_
VOID
RundownTraceStoreMemoryMap(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine runs down a trace store memory map.  This flushes the view of
    the file at the memory map's base address, then unmaps it and clears the
    base address pointer, then closes the corresponding memory map handle
    and clears that pointer.

    This routine is forgiving: it can be called with a NULL pointer, a NULL
    BaseAddress pointer, or a NULL MappingHandle, simplifying error handling
    logic.

    BaseAddress and MappingHandle will be cleared after the flushing/unmapping
    and handle closing has been done, respectively.

Arguments:

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    None.

--*/
{
    //
    // Validate arguments.
    //

    if (!MemoryMap) {
        //__debugbreak();
        return;
    }

    TraceStore->BeginningRundown = TRUE;

    if (!FinalizeTraceStoreAddressTimes(TraceStore, MemoryMap->pAddress)) {
        //__debugbreak();
    }

    if (MemoryMap->BaseAddress) {

        //
        // Flush the view, unmap it, then clear the base address pointer.
        //

        if (!FlushViewOfFile(MemoryMap->BaseAddress, 0)) {
            TraceStore->LastError = GetLastError();
            //__debugbreak();
        }

        if (!UnmapViewOfFile(MemoryMap->BaseAddress)) {
            TraceStore->LastError = GetLastError();
            //__debugbreak();
        }

        MemoryMap->BaseAddress = NULL;
    }

    if (MemoryMap->MappingHandle) {

        //
        // Close the memory mapping handle and clear the pointer.
        //

        if (!CloseHandle(MemoryMap->MappingHandle)) {
            TraceStore->LastError = GetLastError();
            //__debugbreak();
        }
        MemoryMap->MappingHandle = NULL;
    }

    TraceStore->RundownComplete = TRUE;
}

_Use_decl_annotations_
BOOL
UnmapTraceStoreMemoryMap(
    PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This routine unmaps a trace store memory map's view and closes the memory
    mapping handle.

Arguments:

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    None.

--*/
{
    DWORD LastError;

    if (MemoryMap->BaseAddress) {
        if (!UnmapViewOfFile(MemoryMap->BaseAddress)) {
            LastError = GetLastError();
            __debugbreak();
        }
        //MemoryMap->BaseAddress = NULL;
    }

    if (MemoryMap->MappingHandle) {
        if (!CloseHandle(MemoryMap->MappingHandle)) {
            LastError = GetLastError();
            __debugbreak();
        }
        //MemoryMap->MappingHandle = NULL;
    }

    return TRUE;
}

_Use_decl_annotations_
VOID
SubmitCloseMemoryMapThreadpoolWork(
    PTRACE_STORE TraceStore,
    PPTRACE_STORE_MEMORY_MAP MemoryMapPointer
    )
/*++

Routine Description:

    This routine submits a memory map to a trace store's close memory map
    thread pool work routine.  It is used to asynchronously close trace store
    memory maps.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    MemoryMapPointer - Supplies a pointer to an address that contains a pointer
        to the trace store memory map to close.  The pointer will be cleared as
        the last step of this routine.

Return Value:

    None.

--*/
{
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TraceStore)) {
        return;
    }

    if (!ARGUMENT_PRESENT(MemoryMapPointer)) {
        return;
    }

    MemoryMap = *MemoryMapPointer;

    if (!MemoryMap) {
        return;
    }

    //*MemoryMapPointer = NULL;

    //
    // Push the referenced memory map onto the trace store's close memory map
    // list and submit a close memory map threadpool work item, then clear the
    // memory map pointer.
    //

    if (TRUE) {
        PushTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, MemoryMap);
        SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);
    } else {
        CloseTraceStoreMemoryMap(TraceStore, MemoryMap);
    }
}

_Use_decl_annotations_
BOOL
ConsumeNextTraceStoreMemoryMap(
    PTRACE_STORE TraceStore,
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap
    )
/*++

Routine Description:

    This routine consumes the next trace store memory map for a trace store.

    It is called via two paths: a) once, by BindStore(), when a trace store's
    first memory map needs to be activated, and b) by a trace store's
    AllocateRecords() function, when the current memory map has been exhausted
    and it needs the next one in order to satisfy the memory allocation.

    A central design tenet of the tracing machinery is that it should be as
    low-latency as possible, with a dropped trace record being preferable to a
    thread stalled waiting for backing memory maps to be available.  Thus, if
    this routine cannot immediately satisfy the steps required to perform its
    duty without blocking, it will immediately return FALSE, indicating that
    the next memory map is not yet ready.  If a trace store's traits indicate
    BlockingAllocations, though, this routine will block until the next memory
    map is available.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

    FirstMemoryMap - Supplies an optional pointer to an explicit memory map to
        consume.  If NULL, the memory map will be obtained by popping the trace
        store's NextMemoryMap list.  FirstMemoryMap is currently only provided
        via the BindStore() path when the first memory map is being consumed.
        This is because some metadata trace stores have the "single record"
        trait, which means they only need one memory map for the entire session,
        and have no need for prefaulting or retiring machinery.

        N.B.: There may be other uses for consuming an explicit memory map,
              in which case, this parameter may be renamed to NextMemoryMap
              to better reflect its purpose.

Return Value:

    TRUE if the next memory map was successfully "consumed", FALSE otherwise.

--*/
{
    BOOL Success = TRUE;
    BOOL IsMetadata;
    BOOL IsReadonly;
    BOOL HasRelocations;
    HRESULT Result;
    TRACE_STORE_TRAITS Traits;
    PRTL Rtl;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORE_MEMORY_MAP PrevPrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap = NULL;
    PTRACE_STORE_MEMORY_MAP PrepareMemoryMap = NULL;
    TRACE_STORE_ADDRESS Address;
    PTRACE_STORE_ADDRESS AddressPointer;
    LARGE_INTEGER RequestedTimestamp;
    LARGE_INTEGER Elapsed;
    LARGE_INTEGER Timestamp;
    PTRACE_STORE_STATS Stats;
    TRACE_STORE_STATS DummyStats = { 0 };
    PRTL_COPY_MAPPED_MEMORY RtlCopyMappedMemory;

    //
    // Load traits and initialize aliases.
    //

    Traits = *TraceStore->pTraits;
    IsReadonly = IsReadonlyTraceStore(TraceStore);
    IsMetadata = IsMetadataTraceStore(TraceStore);
    HasRelocations = TraceStoreHasRelocations(TraceStore);

    Rtl = TraceStore->Rtl;
    RtlCopyMappedMemory = Rtl->RtlCopyMappedMemory;
    TraceContext = TraceStore->TraceContext;

    //
    // We may not have a stats struct available yet if this is the first
    // call to ConsumeNextTraceStoreMemoryMap().  If that's the case, just
    // point the pointer at a dummy one.  This simplifies the rest of the
    // code in the function.
    //

    Stats = TraceStore->Stats;
    if (!Stats) {
        Stats = &DummyStats;
    }

    //
    // Fast-path for first memory map; we can avoid the logic that checks to
    // see if we need to close any existing maps.
    //

    if (FirstMemoryMap) {
        MemoryMap = FirstMemoryMap;
        if (IsSingleRecord(Traits)) {
            goto ConsumeMap;
        } else {
            goto StartPreparation;
        }
    }

    //
    // Invariant check: all single record trace stores should be handled by the
    // first memory map check above.
    //

    if (IsSingleRecord(Traits)) {
        __debugbreak();
        return FALSE;
    }

    //
    // The previous memory map becomes the previous-previous memory map.
    //

    PrevPrevMemoryMap = TraceStore->PrevMemoryMap;

    //
    // Retire the previous previous memory map if it exists.
    //

    if (!PrevPrevMemoryMap) {

        //
        // No previous previous memory map needs to be retired, so we can jump
        // straight into preparation.
        //

        goto StartPreparation;
    }

    TraceStore->PrevMemoryMap = NULL;

    if (TraceStore->NoRetire) {
        PushNonRetiredMemoryMap(TraceStore, PrevPrevMemoryMap);
        goto StartPreparation;
    }

    //
    // We need to retire (close) this memory map.  If we're metadata or there's
    // no underlying address record for this memory map, we can go straight to
    // the logic that closes the map.  Otherwise, we need to update the various
    // timestamps.
    //

    AddressPointer = PrevPrevMemoryMap->pAddress;
    if (IsMetadata || (AddressPointer == NULL)) {
        goto CloseOldMemoryMap;
    }

    //
    // Take a local copy of the address record.
    //

    Result = RtlCopyMappedMemory(&Address, AddressPointer, sizeof(Address));

    if (FAILED(Result)) {
        __debugbreak();
        PrevPrevMemoryMap->pAddress = NULL;
        goto CloseOldMemoryMap;
    }

    //
    // Take a local timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed, &Timestamp);

    //
    // Copy to the Retired timestamp.
    //

    Address.Timestamp.Retired.QuadPart = Elapsed.QuadPart;

    //
    // Calculate the memory map's active elapsed time.
    //

    Elapsed.QuadPart -= Address.Timestamp.Consumed.QuadPart;
    Address.Elapsed.Active.QuadPart = Elapsed.QuadPart;

    //
    // Copy back to the memory mapped backing store.  If it fails, set the
    // pAddress pointer to NULL and continue submitting the close operation.
    //

    Result = RtlCopyMappedMemory(AddressPointer, &Address, sizeof(Address));
    if (FAILED(Result)) {
        __debugbreak();
        PrevPrevMemoryMap->pAddress = NULL;
        goto Error;
    }

CloseOldMemoryMap:
    PushTraceStoreMemoryMap(&TraceStore->CloseMemoryMaps, PrevPrevMemoryMap);
    SubmitThreadpoolWork(TraceStore->CloseMemoryMapWork);

StartPreparation:

    //
    // Pop a memory map descriptor off the free list to use for the
    // PrepareMemoryMap.  If there are no free memory maps, attempt to create
    // a new set.  If this fails, increment the dropped record and exhausted
    // free memory maps counter and return FALSE indicating that we failed to
    // prepare the next memory map.
    //

    Success = PopFreeTraceStoreMemoryMap(TraceStore, &PrepareMemoryMap);
    if (!Success) {
        ULONG NumberOfMaps = 0;
        Success = CreateMemoryMapsForTraceStore(TraceStore,
                                                &PrepareMemoryMap,
                                                &NumberOfMaps);
        if (!Success) {
            __debugbreak();
            Stats->DroppedRecords++;
            Stats->ExhaustedFreeMemoryMaps++;
            return FALSE;
        }
    }

    if (FirstMemoryMap) {
        goto ConsumeMap;
    }

    //
    // Invariant check: MemoryMap should be NULL here and PrepareMemoryMap
    // should be non-NULL;
    //

    if (MemoryMap || !PrepareMemoryMap) {
        __debugbreak();
        return FALSE;
    }

PopNextMap:

    if (!PopTraceStoreMemoryMap(&TraceStore->NextMemoryMaps, &MemoryMap)) {

        //
        // Our allocations are outpacing the next memory map preparation
        // being done asynchronously in the threadpool.  If the trace store
        // has the BlockingAllocations trait set, wait on the next memory map
        // available event -- otherwise, increment the dropped record counter
        // and return immediately.
        //

        Stats->AllocationsOutpacingNextMemoryMapPreparation++;

        if (IsBlockingAllocator(Traits)) {
            HANDLE Event;
            HRESULT WaitResult;

            //
            // Increment the blocked allocations counter and wait on the next
            // memory map available event.  If the wait is satisfied, jump back
            // and attempt to pop the next memory map again.  If it fails, fall
            // through to the dropping logic.
            //

            Stats->BlockedAllocations++;
            Event = TraceStore->NextMemoryMapAvailableEvent;
            WaitResult = WaitForSingleObject(Event, INFINITE);
            if (WaitResult == WAIT_OBJECT_0) {
                goto PopNextMap;
            }

            //
            // Intentional fall-through to non-blocking logic.  (If the wait
            // result is anything other than WAIT_OBJECT_0, it indicates an
            // error condition -- and is usually catastrophic (i.e. wait
            // abandoned as part of process/thread rundown).)
            //
        }

        //
        // Increment the dropped record counter, return the PrepareMemoryMap
        // back to the free list, and return failure to the caller.
        //

        __debugbreak();
        Stats->DroppedRecords++;
        ReturnFreeTraceStoreMemoryMap(TraceStore, PrepareMemoryMap);
        return FALSE;
    }

    //
    // We've now got the two things we need: a free memory map to fill in with
    // the details of the next memory map to prepare (PrepareMemoryMap), and
    // the ready memory map (MemoryMap) that contains an active mapping ready
    // for use.
    //

    if (TraceStore->MemoryMap) {

        //
        // If there's an active memory map, it now becomes the previous one.
        //

        TraceStore->PrevAddress = TraceStore->MemoryMap->PrevAddress;
        MemoryMap->PrevAddress = TraceStore->MemoryMap->PrevAddress;
        TraceStore->PrevMemoryMap = TraceStore->MemoryMap;
    }

ConsumeMap:

    TraceStore->MemoryMap = MemoryMap;

    //
    // Fast-path exit if we're a single record trace store.
    //

    if (IsSingleRecord(Traits)) {
        goto End;
    }

    //
    // Take a local copy of the timestamp.  We'll use this for both the "next"
    // memory map's Consumed timestamp and the "prepare" memory map's Requested
    // timestamp.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed, &Timestamp);

    //
    // Save the timestamp before we start fiddling with it.
    //

    RequestedTimestamp.QuadPart = Elapsed.QuadPart;

    if (IsMetadata) {

        //
        // Skip the TRACE_STORE_ADDRESS logic for metadata stores.
        //

        goto PrepareMemoryMap;
    }

    if (!MemoryMap->pAddress) {
        __debugbreak();
        goto Error;
    }

    //
    // Take a local copy of the address record, update timestamps and
    // calculate elapsed time, then save the local record back to the
    // backing TRACE_STORE_ADDRESS struct.
    //

    Result = RtlCopyMappedMemory(&Address,
                                 MemoryMap->pAddress,
                                 sizeof(Address));

    if (FAILED(Result)) {
        __debugbreak();
        goto Error;
    }

    //
    // Update the Consumed timestamp.
    //

    Address.Timestamp.Consumed.QuadPart = Elapsed.QuadPart;

    //
    // Calculate elapsed time between Prepared and Consumed.
    //

    Elapsed.QuadPart -= Address.Timestamp.Prepared.QuadPart;

    //
    // Save as the AwaitingConsumption timestamp.
    //

    Address.Elapsed.AwaitingConsumption.QuadPart = Elapsed.QuadPart;

    //
    // Copy the local record back to the backing store.
    //

    RtlCopyMappedMemory(MemoryMap->pAddress,
                        &Address,
                        sizeof(Address));

    if (FAILED(Result)) {
        __debugbreak();
        goto Error;
    }

PrepareMemoryMap:

    //
    // Single record trace stores do not need the preparation machinery.
    // If we've gotten to this point and we're still a single record,
    // we've got a logic error somewhere.
    //

    if (IsSingleRecord(Traits)) {
        __debugbreak();
        goto Error;
    }

    //
    // Prepare the next memory map with the relevant offset details based
    // on the new memory map and submit it to the threadpool.
    //

    PrepareMemoryMap->FileHandle = MemoryMap->FileHandle;
    PrepareMemoryMap->MappingSize.QuadPart = MemoryMap->MappingSize.QuadPart;

    PrepareMemoryMap->FileOffset.QuadPart = (
        MemoryMap->FileOffset.QuadPart +
        MemoryMap->MappingSize.QuadPart
    );

    //
    // Set the base address to the next contiguous address for this mapping.
    // PrepareNextTraceStoreMemoryMap() will attempt to honor this if it can.
    //

    PrepareMemoryMap->BaseAddress = (
        (PVOID)RtlOffsetToPointer(
            MemoryMap->BaseAddress,
            MemoryMap->MappingSize.LowPart
        )
    );

    if (IsMetadata) {
        goto SubmitPreparedMemoryMap;
    }

    //
    // Attempt to load the next address record and fill in the relevant details.
    // This will become the prepared memory map's address record if everything
    // goes successfully.
    //

    Success = LoadNextTraceStoreAddress(TraceStore, &AddressPointer);
    if (!Success) {
        __debugbreak();
        goto Error;
    }

    //
    // Take a local copy.
    //

    Result = RtlCopyMappedMemory(&Address, AddressPointer, sizeof(Address));
    if (FAILED(Result)) {
        __debugbreak();
        goto Error;
    }

    //
    // Copy the timestamp we took earlier to the Requested timestamp.
    //

    Address.Timestamp.Requested.QuadPart = RequestedTimestamp.QuadPart;

    //
    // Copy the local record back to the backing store.
    //

    Result = RtlCopyMappedMemory(AddressPointer, &Address, sizeof(Address));

    if (SUCCEEDED(Result)) {

        //
        // Update the memory map to point at the address struct.
        //

        PrepareMemoryMap->pAddress = AddressPointer;

    } else if (PrepareMemoryMap->pAddress != NULL) {

        //
        // Invariant check: this should never get hit.
        //

        __debugbreak();
        goto Error;
    }

SubmitPreparedMemoryMap:

    //
    // Push the prepare memory map to the relevant list and submit a threadpool
    // work item.
    //

    if (!PrepareMemoryMap) {
        __debugbreak();
    }

    PushTraceStoreMemoryMap(&TraceStore->PrepareMemoryMaps, PrepareMemoryMap);
    SubmitThreadpoolWork(TraceStore->PrepareNextMemoryMapWork);

    Success = TRUE;
    goto End;

Error:

    if (PrepareMemoryMap) {
        ReturnFreeTraceStoreMemoryMap(TraceStore, PrepareMemoryMap);
    }

    if (MemoryMap) {
        ReturnFreeTraceStoreMemoryMap(TraceStore, MemoryMap);
    }

    Success = FALSE;

End:
    return Success;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
