#===============================================================================
# Imports
#===============================================================================

from tracer.dbgeng import (
    Struct,
)

#===============================================================================
# Constants
#===============================================================================

struct__TEST1 = """\
struct _TEST1, 6 elements, 0x20 bytes
   +0x000 Char             : Char
   +0x008 LongLong         : Int8B
   +0x010 WideChar         : Wchar
   +0x018 Bitfield1        : Bitfield Pos 0, 1 Bit
   +0x018 VoidPointer      : Ptr64 to Void
   +0x018 Bitfield2        : Bitfield Pos 0, 1 Bit"""

struct__TEST2 = """\
struct _TEST1, 13 elements, 0x4c0 bytes
   +0x000 Foo              : [1] UChar
   +0x001 Bar              : [2] [3] UChar
   +0x008 Moo              : [4] [5] [6] Uint2B
   +0x0f8 Guid             : struct _GUID, 4 elements, 0x10 bytes
   +0x110 Xmm0             : union __m128i, 8 elements, 0x10 bytes
   +0x120 Ymm0             : union __m256i, 8 elements, 0x20 bytes
   +0x140 Float            : Float
   +0x148 Double           : Float
   +0x150 Bitfield1        : Bitfield Pos 0, 1 Bit
   +0x150 VoidPointer      : Ptr64 to Void
   +0x150 Bitfield2        : Bitfield Pos 0, 1 Bit
   +0x158 Allocator3DArray : [1] [2] [3] struct _ALLOCATOR, 28 elements, 0x90 bytes
   +0x4b8 Char             : Char
   +0x4c0 NamedUnion1      : union <unnamed-tag>, 2 elements, 0x8 bytes
   +0x4c8 NamedStruct2     : struct <unnamed-tag>, 2 elements, 0x4 bytes"""

struct__TEST3 = """\
struct _TEST3, 3 elements, 0x10 bytes
   +0x000 ListEntry        : struct _SLIST_ENTRY, 1 elements, 0x10 bytes
   +0x000 Next             : Ptr64 to struct _SLIST_ENTRY, 1 elements, 0x10 bytes
   +0x008 Unused           : Ptr64 to Void"""

struct__TRACE_STORE = """\
struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x000 TraceStoreId     : Enum _TRACE_STORE_ID,  62 total enums

   +0x004 TraceStoreMetadataId : Enum _TRACE_STORE_METADATA_ID,  11 total enums

   +0x008 TraceStoreIndex  : Enum _TRACE_STORE_INDEX,  601 total enums

   +0x00c NoRetire         : Bitfield Pos 0, 1 Bit
   +0x00c NoPrefaulting    : Bitfield Pos 1, 1 Bit
   +0x00c IgnorePreferredAddresses : Bitfield Pos 2, 1 Bit
   +0x00c IsReadonly       : Bitfield Pos 3, 1 Bit
   +0x00c SetEndOfFileOnClose : Bitfield Pos 4, 1 Bit
   +0x00c IsMetadata       : Bitfield Pos 5, 1 Bit
   +0x00c HasRelocations   : Bitfield Pos 6, 1 Bit
   +0x00c NoTruncate       : Bitfield Pos 7, 1 Bit
   +0x00c IsRelocationTarget : Bitfield Pos 8, 1 Bit
   +0x00c HasSelfRelocations : Bitfield Pos 9, 1 Bit
   +0x00c OnlyRelocationIsToSelf : Bitfield Pos 10, 1 Bit
   +0x00c OnlyRelocationIsToNull : Bitfield Pos 11, 1 Bit
   +0x00c HasMultipleRelocationWaits : Bitfield Pos 12, 1 Bit
   +0x00c RequiresSelfRelocation : Bitfield Pos 13, 1 Bit
   +0x00c HasNullStoreRelocations : Bitfield Pos 14, 1 Bit
   +0x00c BeginningRundown : Bitfield Pos 15, 1 Bit
   +0x00c RundownComplete  : Bitfield Pos 16, 1 Bit
   +0x00c HasFlatMapping   : Bitfield Pos 17, 1 Bit
   +0x00c FlatMappingLoaded : Bitfield Pos 18, 1 Bit
   +0x00c UnusedStoreFlagBits : Bitfield Pos 19, 13 Bits
   +0x00c StoreFlags       : struct _TRACE_STORE_FLAGS, 20 elements, 0x4 bytes
   +0x010 TraceFlags       : struct _TRACE_FLAGS, 20 elements, 0x4 bytes
   +0x014 InitialTraits    : struct _TRACE_STORE_TRAITS, 17 elements, 0x4 bytes
   +0x018 TracerConfig     : Ptr64 to struct _TRACER_CONFIG, 15 elements, 0x1c0 bytes
   +0x020 LastError        : Uint4B
   +0x024 TotalNumberOfMemoryMaps : Int4B
   +0x028 NumberOfActiveMemoryMaps : Int4B
   +0x02c NumberOfNonRetiredMemoryMaps : Int4B
   +0x030 CloseMemoryMaps  : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x040 PrepareMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x050 PrepareReadonlyMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x060 NextMemoryMaps   : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x070 FreeMemoryMaps   : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x080 PrefaultMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x090 NonRetiredMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x0a0 SingleMemoryMap  : struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x0e0 StoresListEntry  : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x0f0 MetadataListHead : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x0f0 MetadataListEntry : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x100 ListEntry        : struct _SLIST_ENTRY, 1 elements, 0x10 bytes
   +0x100 Next             : Ptr64 to struct _SLIST_ENTRY, 1 elements, 0x10 bytes
   +0x108 Unused           : Ptr64 to Void
   +0x110 Rtl              : Ptr64 to struct _RTL, 483 elements, 0x1020 bytes
   +0x118 pAllocator       : Ptr64 to struct _ALLOCATOR, 28 elements, 0x90 bytes
   +0x120 TraceContext     : Ptr64 to struct _TRACE_CONTEXT, 73 elements, 0x520 bytes
   +0x128 InitialSize      : union _LARGE_INTEGER, 4 elements, 0x8 bytes
   +0x130 ExtensionSize    : union _LARGE_INTEGER, 4 elements, 0x8 bytes
   +0x138 MappingSize      : union _LARGE_INTEGER, 4 elements, 0x8 bytes
   +0x140 PrefaultFuturePageWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x148 PrepareNextMemoryMapWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x150 PrepareReadonlyNonStreamingMemoryMapWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x158 CloseMemoryMapWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x160 NextMemoryMapAvailableEvent : Ptr64 to Void
   +0x168 AllMemoryMapsAreFreeEvent : Ptr64 to Void
   +0x170 ReadonlyMappingCompleteEvent : Ptr64 to Void
   +0x178 BindCompleteEvent : Ptr64 to Void
   +0x180 ResumeAllocationsEvent : Ptr64 to Void
   +0x188 RelocationCompleteWaitEvent : Ptr64 to Void
   +0x190 RelocationCompleteWaitEvents : Ptr64 to Ptr64 to Void
   +0x198 PrevMemoryMap    : Ptr64 to struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x1a0 MemoryMap        : Ptr64 to struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x1a8 NumberOfRelocationBackReferences : Uint4B
   +0x1ac OutstandingRelocationBinds : Uint4B
   +0x1b0 MappedSequenceId : Int4B
   +0x1b4 MetadataBindsInProgress : Int4B
   +0x1b8 PrepareReadonlyNonStreamingMapsInProgress : Int4B
   +0x1bc ReadonlyNonStreamingBindCompletesInProgress : Int4B
   +0x1c0 ActiveAllocators : Int4B
   +0x1c4 NumberOfRelocationDependencies : Uint4B
   +0x1c8 NumberOfRelocationsRequired : Uint4B
   +0x1cc SequenceId       : Uint4B
   +0x1d0 NumaNode         : Uint4B
   +0x1d4 CreateFileDesiredAccess : Uint4B
   +0x1d8 CreateFileCreationDisposition : Uint4B
   +0x1dc CreateFileMappingProtectionFlags : Uint4B
   +0x1e0 CreateFileFlagsAndAttributes : Uint4B
   +0x1e4 MapViewOfFileDesiredAccess : Uint4B
   +0x1e8 MappingHandle    : Ptr64 to Void
   +0x1f0 FileHandle       : Ptr64 to Void
   +0x1f8 PrevAddress      : Ptr64 to Void
   +0x200 FlatMappingHandle : Ptr64 to Void
   +0x208 FlatAddress      : struct _TRACE_STORE_ADDRESS, 20 elements, 0x80 bytes
   +0x288 FlatAddressRange : struct _TRACE_STORE_ADDRESS_RANGE, 7 elements, 0x40 bytes
   +0x2c8 Padding4         : Uint8B
   +0x2d0 FlatMemoryMap    : struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x310 TraceStore       : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x318 MetadataInfoStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x320 AllocationStore  : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x328 RelocationStore  : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x330 AddressStore     : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x338 AddressRangeStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x340 AllocationTimestampStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x348 AllocationTimestampDeltaStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x350 SynchronizationStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x358 InfoStore        : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x360 RelocationDependencyStores : Ptr64 to Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x360 RelocationDependencyStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x368 AllocateRecords  : Ptr64 to     void*
   +0x370 AllocateRecordsWithTimestamp : Ptr64 to     void*
   +0x378 TryAllocateRecords : Ptr64 to     void*
   +0x380 TryAllocateRecordsWithTimestamp : Ptr64 to     void*
   +0x388 SuspendedAllocateRecordsWithTimestamp : Ptr64 to     void*
   +0x390 AllocateRecordsWithTimestampImpl1 : Ptr64 to     void*
   +0x398 AllocateRecordsWithTimestampImpl2 : Ptr64 to     void*
   +0x3a0 BindComplete     : Ptr64 to     int
   +0x3a8 pReloc           : Ptr64 to struct _TRACE_STORE_RELOC, 9 elements, 0x48 bytes
   +0x3b0 pTraits          : Ptr64 to struct _TRACE_STORE_TRAITS, 17 elements, 0x4 bytes
   +0x3b8 Eof              : Ptr64 to struct _TRACE_STORE_EOF, 1 elements, 0x8 bytes
   +0x3c0 Time             : Ptr64 to struct _TRACE_STORE_TIME, 3 elements, 0x58 bytes
   +0x3c8 Stats            : Ptr64 to struct _TRACE_STORE_STATS, 11 elements, 0x40 bytes
   +0x3d0 Totals           : Ptr64 to struct _TRACE_STORE_TOTALS, 4 elements, 0x20 bytes
   +0x3d8 Traits           : Ptr64 to struct _TRACE_STORE_TRAITS, 17 elements, 0x4 bytes
   +0x3e0 Info             : Ptr64 to struct _TRACE_STORE_INFO, 6 elements, 0x100 bytes
   +0x3e8 Reloc            : Ptr64 to struct _TRACE_STORE_RELOC, 9 elements, 0x48 bytes
   +0x3f0 Allocation       : Ptr64 to struct _TRACE_STORE_ALLOCATION, 2 elements, 0x10 bytes
   +0x3f8 AllocationTimestamp : Ptr64 to struct _TRACE_STORE_ALLOCATION_TIMESTAMP, 1 elements, 0x8 bytes
   +0x400 AllocationTimestampDelta : Ptr64 to struct _TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA, 1 elements, 0x4 bytes
   +0x408 Address          : Ptr64 to struct _TRACE_STORE_ADDRESS, 20 elements, 0x80 bytes
   +0x410 AddressRange     : Ptr64 to struct _TRACE_STORE_ADDRESS_RANGE, 7 elements, 0x40 bytes
   +0x418 Sync             : Ptr64 to struct _TRACE_STORE_SYNC, 6 elements, 0x100 bytes
   +0x420 NumberOfAllocations : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x428 NumberOfAddresses : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x430 NumberOfAddressRanges : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x438 NumberOfReadonlyAddressRanges : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x440 ReadonlyAddresses : Ptr64 to struct _TRACE_STORE_ADDRESS, 20 elements, 0x80 bytes
   +0x448 ReadonlyMemoryMaps : Ptr64 to struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x450 ReadonlyAddressRanges : Ptr64 to struct _TRACE_STORE_ADDRESS_RANGE, 7 elements, 0x40 bytes
   +0x458 ReadonlyMappingSizes : Ptr64 to union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x460 ReadonlyPreferredAddressUnavailable : Uint4B
   +0x468 DataType         : struct <unnamed-tag>, 2 elements, 0x10 bytes
   +0x478 Allocator        : struct _ALLOCATOR, 28 elements, 0x90 bytes
   +0x508 IntervalFramesPerSecond : Uint8B
   +0x510 Intervals        : struct _TRACE_STORE_INTERVALS, 16 elements, 0x80 bytes
   +0x590 Db               : Ptr64 to struct _TRACE_STORE_SQLITE3_DB, 40 elements, 0x4b0 bytes
   +0x598 Sqlite3Schema    : Ptr64 to Char
   +0x5a0 Sqlite3VirtualTableName : Ptr64 to Char
   +0x5a8 Sqlite3Column    : Ptr64 to     long
   +0x5b0 Sqlite3Module    : struct _SQLITE3_MODULE, 24 elements, 0xb8 bytes
   +0x668 Sqlite3VirtualTable : struct _TRACE_STORE_SQLITE3_VTAB, 6 elements, 0x18 bytes
   +0x680 Sqlite3IntervalSchema : Ptr64 to Char
   +0x688 Sqlite3IntervalVirtualTableName : Ptr64 to Char
   +0x690 Sqlite3IntervalColumn : Ptr64 to     long
   +0x698 Sqlite3IntervalModule : struct _SQLITE3_MODULE, 24 elements, 0xb8 bytes
   +0x750 Sqlite3IntervalVirtualTable : struct _TRACE_STORE_SQLITE3_VTAB, 6 elements, 0x18 bytes
   +0x768 Padding5         : [18] Uint8B"""

struct__TRACE_STORES = """\
struct _TRACE_STORES, 18 elements, 0x12e000 bytes
   +0x000 SizeOfStruct     : Uint4B
   +0x004 SizeOfAllocation : Uint4B
   +0x008 NumberOfTraceStores : Uint2B
   +0x00a ElementsPerTraceStore : Uint2B
   +0x00c NumberOfFieldRelocationsElements : Uint2B
   +0x00e Padding1         : Uint2B
   +0x010 Padding2         : Uint4B
   +0x014 Flags            : struct _TRACE_FLAGS, 20 elements, 0x4 bytes
   +0x018 BaseDirectory    : struct _UNICODE_STRING, 5 elements, 0x10 bytes
   +0x028 Rtl              : Ptr64 to struct _RTL, 483 elements, 0x1020 bytes
   +0x030 Allocator        : Ptr64 to struct _ALLOCATOR, 28 elements, 0x90 bytes
   +0x038 TracerConfig     : Ptr64 to struct _TRACER_CONFIG, 15 elements, 0x1c0 bytes
   +0x040 RundownListEntry : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x050 Rundown          : Ptr64 to struct _TRACE_STORES_RUNDOWN, 5 elements, 0x40 bytes
   +0x058 StoresListHead   : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x080 RelocationCompleteEvents : [60] Ptr64 to Void
   +0x400 Relocations      : [60] struct _TRACE_STORE_RELOC, 9 elements, 0x48 bytes
   +0x2000 Stores           : [600] struct _TRACE_STORE, 148 elements, 0x800 bytes"""

#===============================================================================
# Tests
#===============================================================================

def test_struct_load_1():
    s = Struct.load(struct__TEST1)
    assert s.has_implicit_padding
    assert s.has_trailing_padding

def test_struct_load_2():
    s = Struct.load(struct__TEST2)
    assert s.has_implicit_padding
    assert s.has_trailing_padding

def test_struct_load_3():
    s = Struct.load(struct__TEST3)

def test_struct_load_trace_store():
    s = Struct.load(struct__TRACE_STORE)

def test_struct_load_trace_stores():
    s = Struct.load(struct__TRACE_STORES)

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
