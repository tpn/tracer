/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>
Copyright (c) Microsoft Corporation.

Module Name:

    SALExamples.h

Abstract:

    SAL is easier to grok when you can see examples, especially more advanced
    and esoteric examples.  This header file serves as a dumping ground for
    interesting SAL annotations one may come across.  Most, if not all, are
    copy-and-pasted from various Windows SDK/DDK headers.

    This file isn't included in any projects, nor is it expected to be in a
    state where a C compiler or pre-processor could parse it.

--*/
_Must_inspect_result_
_IRQL_requires_max_(DISPATCH_LEVEL)
NTKERNELAPI
_When_ (return != NULL, _Post_writable_byte_size_ (NumberOfBytes)) PVOID
MmAllocateContiguousMemorySpecifyCacheNode (
    _In_ SIZE_T NumberOfBytes,
    _In_ PHYSICAL_ADDRESS LowestAcceptableAddress,
    _In_ PHYSICAL_ADDRESS HighestAcceptableAddress,
    _In_opt_ PHYSICAL_ADDRESS BoundaryAddressMultiple,
    _In_ MEMORY_CACHING_TYPE CacheType,
    _In_ NODE_REQUIREMENT PreferredNode
    );

NTKERNELAPI
HANDLE
MmSecureVirtualMemory (
    __in_data_source(USER_MODE) _In_reads_bytes_ (Size) PVOID Address,
    _In_  __in_data_source(USER_MODE) SIZE_T Size,
    _In_ ULONG ProbeMode
    );

NTKERNELAPI
NTSTATUS
IoVolumeDeviceToDosName(
    _In_  PVOID           VolumeDeviceObject,
    _Out_ _When_(return==0,
          _At_(DosName->Buffer, __drv_allocatesMem(Mem)))
          PUNICODE_STRING DosName
    );

_Must_inspect_result_
_Success_(return != 0)
NTSYSAPI
BOOLEAN
NTAPI
RtlCreateHashTableEx(
    _Inout_ _When_(NULL == *HashTable, _At_(*HashTable, __drv_allocatesMem(Mem)))
        PRTL_DYNAMIC_HASH_TABLE *HashTable,
    _In_ ULONG InitialSize,
    _In_ ULONG Shift,
    _Reserved_ ULONG Flags
    );

NTSYSAPI
VOID
NTAPI
RtlDeleteHashTable(
    _In_ _When_((HashTable->Flags & RTL_HASH_ALLOCATED_HEADER), __drv_freesMem(Mem) _Post_invalid_)
        PRTL_DYNAMIC_HASH_TABLE HashTable
    );


NTSYSAPI
ULONG
NTAPI
RtlWalkFrameChain (
    _Out_writes_(Count - (Flags >> RTL_STACK_WALKING_MODE_FRAMES_TO_SKIP_SHIFT)) PVOID *Callers,
    _In_ ULONG Count,
    _In_ ULONG Flags
    );

_IRQL_requires_max_(PASSIVE_LEVEL)
_When_(AllocateDestinationString, _Must_inspect_result_)
NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    _When_(AllocateDestinationString, _Out_ _At_(DestinationString->Buffer, __drv_allocatesMem(Mem)))
    _When_(!AllocateDestinationString, _Inout_)
        PUNICODE_STRING DestinationString,
    _In_ PCUNICODE_STRING SourceString,
    _In_ BOOLEAN AllocateDestinationString
    );

_IRQL_requires_max_(DISPATCH_LEVEL)
_Success_(return == 0)
EXPORT
NDIS_STATUS
NdisAllocateMemoryWithTag(
    _At_(*VirtualAddress, __drv_allocatesMem(Mem))
    _Outptr_result_bytebuffer_(Length)
          PVOID *                 VirtualAddress,
    _In_  UINT                    Length,
    _In_  ULONG                   Tag
    );

_When_(MemoryFlags==0,
    _IRQL_requires_max_(DISPATCH_LEVEL))
_When_(MemoryFlags==NDIS_MEMORY_NONCACHED,
    _IRQL_requires_max_(APC_LEVEL))
_When_(MemoryFlags==NDIS_MEMORY_CONTIGUOUS,
    _IRQL_requires_(PASSIVE_LEVEL))
EXPORT
VOID
NdisFreeMemory(
    _In_reads_bytes_(Length) __drv_freesMem(Mem)
            PVOID           VirtualAddress,
    _In_    UINT            Length,
    _In_ _Pre_satisfies_(MemoryFlags ==0 || MemoryFlags == NDIS_MEMORY_NONCACHED || MemoryFlags ==NDIS_MEMORY_CONTIGUOUS)
            UINT            MemoryFlags
    );


_IRQL_requires_max_(DISPATCH_LEVEL)
_At_(AlignOffset, _In_range_(0, AlignMultiple-1))
_Pre_satisfies_(AlignMultiple == 1 || AlignMultiple == 2 || AlignMultiple == 4 ||
AlignMultiple == 8 || AlignMultiple == 16 || AlignMultiple == 32 || AlignMultiple == 64 ||
AlignMultiple ==128 || AlignMultiple ==256 || AlignMultiple ==512 || AlignMultiple == 1024 ||
AlignMultiple == 2048 || AlignMultiple == 4096 || AlignMultiple == 8192)
_Must_inspect_result_
EXPORT
PVOID
NdisGetDataBuffer(
    _In_      PNET_BUFFER  NetBuffer,
    _In_      ULONG        BytesNeeded,
    _Out_writes_bytes_all_opt_(BytesNeeded) PVOID Storage,
    _In_      UINT         AlignMultiple,
    _In_      UINT         AlignOffset
    );


_When_(TokenInformationClass == TokenAccessInformation,
       _At_(TokenInformationLength,
            _In_range_(>=, sizeof(TOKEN_ACCESS_INFORMATION))))
_Must_inspect_result_
__kernel_entry NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationToken (
    _In_ HANDLE TokenHandle,
    _In_ TOKEN_INFORMATION_CLASS TokenInformationClass,
    _Out_writes_bytes_to_opt_(TokenInformationLength, *ReturnLength) PVOID TokenInformation,
    _In_ ULONG TokenInformationLength,
    _Out_ PULONG ReturnLength
    );


BOOLEAN
_interlockedbittestandset64 (
    _Inout_updates_bytes_((Offset/8)+1) _Interlocked_operand_ LONG64 volatile *Base,
    _In_range_(>=,0) LONG64 Offset
    );

BOOLEAN
_interlockedbittestandreset64 (
    _Inout_updates_bytes_((Offset/8)+1) _Interlocked_operand_ LONG64 volatile *Base,
    _In_range_(>=,0) LONG64 Offset
    );

