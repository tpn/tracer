#===============================================================================
# Imports
#===============================================================================

from itertools import chain

from ..util import memoize

from ..wintypes import (
    cast,
    byref,
    sizeof,
    errcheck,
    create_threadpool,
    create_string_buffer,
    create_threadpool_callback_environment,

    Union,
    Structure,

    SetThreadpoolCallbackPool,
    InitializeThreadpoolEnvironment,

    CDLL,
    BOOL,
    CHAR,
    LONG,
    GUID,
    PWSTR,
    DWORD,
    PVOID,
    ULONG,
    PISID,
    USHORT,
    PULONG,
    PCWSTR,
    HANDLE,
    STRING,
    PSTRING,
    POINTER,
    PHANDLE,
    FILETIME,
    PTP_WORK,
    CFUNCTYPE,
    ULONGLONG,
    RTL_BITMAP,
    SYSTEMTIME,
    LIST_ENTRY,
    SLIST_ENTRY,
    SLIST_HEADER,
    LARGE_INTEGER,
    ULARGE_INTEGER,
    UNICODE_STRING,
    PULARGE_INTEGER,
    PUNICODE_STRING,
    PROCESSOR_NUMBER,
    PTP_CLEANUP_GROUP,
    TP_CALLBACK_ENVIRON,
    PTP_CALLBACK_ENVIRON,
)

from .Rtl import (
    PRTL,
)

from .TracerConfig import (
    PTRACER_CONFIG,
)

from .Allocator import (
    PALLOCATOR,
)

from .Python import (
    PYTHON_PATH_TABLE_ENTRY,
    PYTHON_FUNCTION_TABLE_ENTRY,
)

from .PythonTracer import (
    PYTHON_TRACE_EVENT,
)

#===============================================================================
# Globals
#===============================================================================
TRACE_STORE_ID = ULONG
TRACE_STORE_METADATA_ID = ULONG
TRACE_STORE_INDEX = ULONG
PTIMER_FUNCTION = PVOID

# These should be done as CFUNCTYPE prototypes, but they all take pointers
# to structures we define later, and I can't be bothered figuring out the
# ctypes-fu required to get the forward definitions working at the moment.
PALLOCATE_RECORDS = PVOID
PALLOCATE_ALIGNED_RECORDS = PVOID
PALLOCATE_ALIGNED_OFFSET_RECORDS = PVOID
PBIND_COMPLETE = PVOID

IS_BOUND = False

#===============================================================================
# Enums/Indexes/Constants
#===============================================================================

TraceStoreNullId                    =   0
TraceStoreEventId                   =   1
TraceStoreStringBufferId            =   2
TraceStoreFunctionTableId           =   3
TraceStoreFunctionTableEntryId      =   4
TraceStorePathTableId               =   5
TraceStorePathTableEntryId          =   6
TraceStoreSessionId                 =   7
TraceStoreStringArrayId             =   8
TraceStoreStringTableId             =   9
TraceStoreInvalidId                 =  10

MAX_TRACE_STORE_IDS = TraceStoreInvalidId - 1
TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS = 1

#===============================================================================
# Helpers
#===============================================================================

class NotBoundError(BaseException):
    pass

class ensure_bound(object):
    def __init__(self, func):
        self.func = func
    def __call__(self, *args, **kwds):
        global IS_BOUND
        if not IS_BOUND:
            raise NotBoundError()
        return self.func(*args, **kwds)

#===============================================================================
# Structures
#===============================================================================

class TRACE_FLAGS(Structure):
    _fields_ = [
        ('Readonly', ULONG, 1),
        ('Compress', ULONG, 1),
        ('DisablePrefaultPages', ULONG, 1),
        ('DisableFileFlagOverlapped', ULONG, 1),
        ('DisableFileFlagSequentialScan', ULONG, 1),
        ('EnableFileFlagRandomAccess', ULONG, 1),
        ('EnableFileFlagWriteThrough', ULONG, 1),
        ('NoGlobalRundown', ULONG, 1),
        ('NoTruncate', ULONG, 1),
    ]
PTRACE_FLAGS = POINTER(TRACE_FLAGS)

class TRACE_CONTEXT_FLAGS(Structure):
    _fields_ = [
        ('Valid', ULONG, 1),
        ('Readonly', ULONG, 1),
        ('InitializeAsync', ULONG, 1),
        ('IgnorePreferredAddresses', ULONG, 1),
    ]
PTRACE_CONTEXT_FLAGS = POINTER(TRACE_CONTEXT_FLAGS)

class READONLY_TRACE_CONTEXT_FLAGS(Structure):
    _fields_ = [
        ('Valid', ULONG, 1),
        ('Unused', ULONG, 31),
    ]
PREADONLY_TRACE_CONTEXT_FLAGS = POINTER(READONLY_TRACE_CONTEXT_FLAGS)

class TRACE_STORE_WORK(Structure):
    _fields_ = [
        ('ListHead', SLIST_HEADER),
        ('ThreadpoolWork', PTP_WORK),
        ('WorkCompleteEvent', HANDLE),
        ('Unused1', PVOID),
        ('NumberOfActiveItems', ULONG),
        ('NumberOfFailedItems', ULONG),
        ('TotalNumberOfItems', ULONG),
        ('Unused2', ULONG),
        ('Unused3', ULONGLONG),
    ]
PTRACE_STORE_WORK = POINTER(TRACE_STORE_WORK)

class TRACE_STORE_TRAITS(Structure):
    _fields_ = [
        ('VaryingRecordSize', ULONG, 1),
        ('RecordSizeIsAlwaysPowerOf2', ULONG, 1),
        ('MultipleRecords', ULONG, 1),
        ('StreamingWrite', ULONG, 1),
        ('StreamingRead', ULONG, 1),
        ('Unused', ULONG, 27),
    ]
PTRACE_STORE_TRAITS = POINTER(TRACE_STORE_TRAITS)

class TRACE_STORE_ALLOCATION(Structure):
    _fields_ = [
        ('NumberOfRecords', ULARGE_INTEGER),
        ('RecordSize', LARGE_INTEGER),
    ]
PTRACE_STORE_ALLOCATION = POINTER(TRACE_STORE_ALLOCATION)

class _TRACE_STORE_ADDRESS_TIMESTAMP(Structure):
    _fields_ = [
        ('Requested', LARGE_INTEGER),
        ('Prepared',  LARGE_INTEGER),
        ('Consumed',  LARGE_INTEGER),
        ('Retired',   LARGE_INTEGER),
        ('Released',  LARGE_INTEGER),
    ]

class _TRACE_STORE_ADDRESS_ELAPSED(Structure):
    _fields_ = [
        ('AwaitingPreparation', LARGE_INTEGER),
        ('AwaitingConsumption', LARGE_INTEGER),
        ('Active',              LARGE_INTEGER),
        ('AwaitingRelease',     LARGE_INTEGER),
    ]

class TRACE_STORE_ADDRESS(Structure):
    _fields_ = [
        ('PreferredBaseAddress', ULONGLONG),
        ('BaseAddress', ULONGLONG),
        ('FileOffset', LARGE_INTEGER),
        ('MappedSize', LARGE_INTEGER),
        ('ProcessId', DWORD),
        ('RequestingThreadId', DWORD),
        ('Timestamp', _TRACE_STORE_ADDRESS_TIMESTAMP),
        ('Elapsed', _TRACE_STORE_ADDRESS_ELAPSED),
        ('MappedSequenceId', LONG),
        ('RequestingProcessor', PROCESSOR_NUMBER),
        ('FulfillingProcessor', PROCESSOR_NUMBER),
        ('FulfillingThreadId', DWORD),
    ]
PTRACE_STORE_ADDRESS = POINTER(TRACE_STORE_ADDRESS)

class _ADDRESS_BIT_COUNTS_LOW(Structure):
    _fields_ = [
        ('Width', ULONG, 6),
        ('PopulationCount', ULONG, 6),
        ('LeadingZeros', ULONG, 5),
        ('TrailingZeros', ULONG, 5),
    ]

class _ADDRESS_BIT_COUNTS_HIGH(Structure):
    _fields_ = [
        ('HighPopulationCount', ULONG, 5),
        ('LowPopulationCount', ULONG, 5),
        ('HighParity', ULONG, 1),
        ('LowParity', ULONG, 1),
        ('Parity', ULONG, 1),
        ('Unused', ULONG, 19),
    ]

class _ADDRESS_BIT_COUNTS(Structure):
    _fields_ = [
        ('l', _ADDRESS_BIT_COUNTS_LOW),
        ('h', _ADDRESS_BIT_COUNTS_HIGH),
    ]

class ADDRESS_BIT_COUNTS(Union):
    _fields_ = [
        ('AsLongLong', ULONGLONG),
        ('s', _ADDRESS_BIT_COUNTS),
    ]
PADDRESS_BIT_COUNTS = POINTER(ADDRESS_BIT_COUNTS)

class _TRACE_STORE_ADDRESS_RANGE_BITCOUNTS_INNER(Structure):
    _fields_ = [
        ('Preferred', ADDRESS_BIT_COUNTS),
        ('Actual', ADDRESS_BIT_COUNTS),
    ]

class TRACE_STORE_ADDRESS_RANGE(Structure):
    pass
PTRACE_STORE_ADDRESS_RANGE = POINTER(TRACE_STORE_ADDRESS_RANGE)

class _TRACE_STORE_ADDRESS_RANGE_TIMESTAMP_INNER(Union):
    _fields_ = [
        ('ValidFrom', LARGE_INTEGER),
        ('ValidTo', LARGE_INTEGER),
    ]

class _TRACE_STORE_ADDRESS_RANGE_UNION1(Union):
    _fields_ = [
        ('OriginalAddressRange', PTRACE_STORE_ADDRESS_RANGE),
        ('Timestamp', _TRACE_STORE_ADDRESS_TIMESTAMP),
    ]

TRACE_STORE_ADDRESS_RANGE._fields_ = [
        ('PreferredBaseAddress', PVOID),
        ('ActualBaseAddress', PVOID),
        ('EndAddress', PVOID),
        ('BitCounts', _TRACE_STORE_ADDRESS_RANGE_BITCOUNTS_INNER),
        ('MappedSize', ULARGE_INTEGER),
        ('u1', _TRACE_STORE_ADDRESS_RANGE_UNION1),
    ]

class TRACE_STORE_EOF(Structure):
    _fields_ = [
        ('EndOfFile', LARGE_INTEGER),
    ]
PTRACE_STORE_EOF = POINTER(TRACE_STORE_EOF)

class TRACE_STORE_START_TIME(Structure):
    _fields_ = [
        ('FileTimeUtc', FILETIME),
        ('FileTimeLocal', FILETIME),
        ('SystemTimeUtc', SYSTEMTIME),
        ('SystemTimeLocal', SYSTEMTIME),
        ('SecondsSince1970', ULARGE_INTEGER),
        ('MicrosecondsSince1970', ULARGE_INTEGER),
        ('PerformanceCounter', LARGE_INTEGER),
    ]
PTRACE_STORE_START_TIME = POINTER(TRACE_STORE_START_TIME)

class TRACE_STORE_TIME(Structure):
    _fields_ = [
        ('Frequency', LARGE_INTEGER),
        ('Multiplicand', LARGE_INTEGER),
        ('StartTime', TRACE_STORE_START_TIME),
    ]
PTRACE_STORE_TIME = POINTER(TRACE_STORE_TIME)

class TRACE_STORE_STATS(Structure):
    _fields_ = [
        ('DroppedRecords', ULONG),
        ('ExhaustedFreeMemoryMaps', ULONG),
        ('AllocationsOutpacingNextMemoryMapPreparation', ULONG),
        ('PreferredAddressUnavailable', ULONG),
    ]
PTRACE_STORE_STATS = POINTER(TRACE_STORE_STATS)

class TRACE_STORE_TOTALS(Structure):
    _fields_ = [
        ('NumberOfAllocations', ULARGE_INTEGER),
        ('AllocationSize', ULARGE_INTEGER),
    ]
PTRACE_STORE_TOTALS = POINTER(TRACE_STORE_TOTALS)

class TRACE_STORE_INFO(Structure):
    _fields_ = [
        ('Eof', TRACE_STORE_EOF),
        ('Time', TRACE_STORE_TIME),
        ('Stats', TRACE_STORE_STATS),
        ('Totals', TRACE_STORE_TOTALS),
        ('Traits', TRACE_STORE_TRAITS),
        ('Unused', CHAR * 124),
    ]
PTRACE_STORE_INFO = POINTER(TRACE_STORE_INFO)

class TRACE_STORE_METADATA_INFO(Structure):
    _fields_ = [
        ('MetadataInfo', TRACE_STORE_INFO),
        ('Allocation', TRACE_STORE_INFO),
        ('Relocation', TRACE_STORE_INFO),
        ('Address', TRACE_STORE_INFO),
        ('AddressRange', TRACE_STORE_INFO),
        ('Bitmap', TRACE_STORE_INFO),
        ('Info', TRACE_STORE_INFO),
        ('Unused', TRACE_STORE_INFO),
    ]
PTRACE_STORE_METADATA_INFO = POINTER(TRACE_STORE_METADATA_INFO)

class TRACE_STORE_FIELD_RELOC(Structure):
    _fields_ = [
        ('Offset', ULONG),
        ('TraceStoreId', TRACE_STORE_ID),
    ]
PTRACE_STORE_FIELD_RELOC = POINTER(TRACE_STORE_FIELD_RELOC)

class TRACE_STORE_FIELD_RELOCS(Structure):
    _fields_ = [
        ('TraceStoreId', TRACE_STORE_ID),
        ('Relocations', PTRACE_STORE_FIELD_RELOC),
    ]
PTRACE_STORE_FIELD_RELOCS = POINTER(TRACE_STORE_FIELD_RELOCS)

class TRACE_STORE_BITMAP(Structure):
    _fields_ = [
        ('SizeOfBitMap', ULONG),
        ('Granularity', USHORT),
        ('Shift', USHORT),
        ('Buffer', PVOID),
    ]
PTRACE_STORE_BITMAP = POINTER(TRACE_STORE_BITMAP)

class TRACE_STORE_RELOC(Structure):
    _fields_ = [
        ('SizeOfStruct', ULONG),
        ('NumberOfRelocations', ULONG),
        ('NumberOfRelocationBackReferences', ULONG),
        ('BitmapBufferSizeInQuadwords', ULONG),
        ('Relocations', PTRACE_STORE_FIELD_RELOC),
        ('ForwardRefBitmap', TRACE_STORE_BITMAP),
        ('ForwardRefBitmapBuffer',
            ULONGLONG * TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS
        ),
        ('BackwardRefBitmap', TRACE_STORE_BITMAP),
        ('BackwardRefBitmapBuffer',
            ULONGLONG * TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS
        ),
    ]
PTRACE_STORE_RELOC = POINTER(TRACE_STORE_RELOC)

class TRACE_SESSION(Structure):
    _fields_ = [
        ('Rtl', PRTL),
        ('SessionId', LARGE_INTEGER),
        ('MachineGuid', GUID),
        ('Sid', PISID),
        ('UserName', PCWSTR),
        ('ComputerName', PCWSTR),
        ('DomainName', PCWSTR),
        ('SystemTime', FILETIME),
    ]
PTRACE_SESSION = POINTER(TRACE_SESSION)

class _TRACE_STORE_MEMORY_MAP_OVERLAY(Structure):
    _fields_ = [
        ('PrevAddress', PVOID),
        ('pAddress', PTRACE_STORE_ADDRESS),
    ]

class _TRACE_STORE_MEMORY_MAP_INNER1(Union):
    _fields_ = [
        ('ListEntry', SLIST_ENTRY),
        ('Overlay', _TRACE_STORE_MEMORY_MAP_OVERLAY),
    ]

class _TRACE_STORE_MEMORY_MAP_INNER2(Union):
    _fields_ = [
        ('PreferredBaseAddress', PVOID),
        ('NextAddress', PVOID),
    ]

class TRACE_STORE_MEMORY_MAP(Structure):
    _fields_ = [
        ('u1', _TRACE_STORE_MEMORY_MAP_INNER1),
        ('FileHandle', HANDLE),
        ('MappingHandle', HANDLE),
        ('FileOffset', LARGE_INTEGER),
        ('MappingSize', LARGE_INTEGER),
        ('BaseAddress', PVOID),
        ('u2', _TRACE_STORE_MEMORY_MAP_INNER2),
    ]
PTRACE_STORE_MEMORY_MAP = POINTER(TRACE_STORE_MEMORY_MAP)

class TRACE_STORES(Structure):
    pass
PTRACE_STORES = POINTER(TRACE_STORES)

class _TRACE_STORE_BITMAP(Structure):
    _fields_ = [
        ('TraceStoreNullId', ULONG, 1),
        ('TraceStoreEventId', ULONG, 1),
        ('TraceStoreStringBufferId', ULONG, 1),
        ('TraceStoreFunctionTableId', ULONG, 1),
        ('TraceStoreFunctionTableEntryId', ULONG, 1),
        ('TraceStorePathTableId', ULONG, 1),
        ('TraceStorePathTableEntryId', ULONG, 1),
        ('TraceStoreSessionId', ULONG, 1),
        ('TraceStoreStringArrayId', ULONG, 1),
        ('TraceStoreStringTableId', ULONG, 1),
    ]

class TRACE_STORE_BITMAP(Union):
    _fields_ = [
        ('Raw', ULONGLONG),
        ('BitFields', _TRACE_STORE_BITMAP),
    ]
PTRACE_STORE_BITMAP = POINTER(TRACE_STORE_BITMAP)

class _TRACE_CONTEXT_IGNORE_PREFERRED_ADDRESS_BITMAP(Union):
    _fields_ = [
        ('Buffer', ULONGLONG * TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS),
        ('TraceStoreBitmap', TRACE_STORE_BITMAP),
    ]

class TRACE_CONTEXT(Structure):
    _fields_ = [
        ('SizeOfStruct', ULONG),
        ('Flags', TRACE_CONTEXT_FLAGS),
        ('FailedCount', ULONG),
        ('Padding1', ULONG),
        ('Rtl', PRTL),
        ('Allocator', PALLOCATOR),
        ('TraceSession', PTRACE_SESSION),
        ('TraceStores', PTRACE_STORES),
        ('TimerFunction', PVOID),
        ('UserData', PVOID),
        ('ThreadpoolCallbackEnvironment', PTP_CALLBACK_ENVIRON),
        ('LoadingCompleteEvent', HANDLE),
        ('ThreadpoolCleanupGroup', PTP_CLEANUP_GROUP),
        ('Padding2', PVOID),
        ('BindMetadataInfoStoreWork', TRACE_STORE_WORK),
        ('BindRemainingMetadataStoresWork', TRACE_STORE_WORK),
        ('BindTraceStoreWork', TRACE_STORE_WORK),
        ('ReadonlyNonStreamingBindCompleteWork', TRACE_STORE_WORK),
        ('FailedListHead', SLIST_HEADER),
        ('ActiveWorkItems', ULONG),
        ('BindsInProgress', ULONG),
        ('PrepareReadonlyNonStreamingMapsInProgress', ULONG),
        ('ReadonlyNonStreamingBindCompletesInProgress', ULONG),
        ('NumberOfStoresWithMultipleRelocationDependencies', ULONG),
        ('Padding3', ULONG),
        ('Time', TRACE_STORE_TIME),
        ('BitmapBufferSizeInQuadwords', ULONG),
        ('Padding4', ULONG),
        ('IgnorePreferredAddressesBitmap', RTL_BITMAP),
        ('BitmapBuffer', _TRACE_CONTEXT_IGNORE_PREFERRED_ADDRESS_BITMAP),
    ]

    def set_ignore_preferred_addresses_bitmap(self, ignore):
        self.BitmapBufferSizeInQuadwords = TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS
        bitmap = self.IgnorePreferredAddressesBitmap
        bitmap.SizeOfBitMap = MAX_TRACE_STORE_IDS
        self.BitmapBuffer.TraceStoreBitmap.Raw = ignore.Raw

PTRACE_CONTEXT = POINTER(TRACE_CONTEXT)

class READONLY_TRACE_CONTEXT(Structure):
    _fields_ = [
        ('SizeOfStruct', USHORT),
        ('Padding1', USHORT),
        ('Flags', READONLY_TRACE_CONTEXT_FLAGS),
        ('Rtl', PRTL),
        ('Allocator', PALLOCATOR),
        ('Directory', PUNICODE_STRING),
        ('UserData', PVOID),
        ('ThreadpoolCallbackEnvironment', PTP_CALLBACK_ENVIRON),
        ('TraceStores', PTRACE_STORES),
        ('LoadingCompleteEvent', HANDLE),
        ('TraceStoresLoadingCompleteCallback', PVOID),
        ('TraceStoresLoadingCompleteWork', PTP_WORK),
        ('LoadTraceStoreMaps', SLIST_HEADER),
        ('LoadTraceStoreWork', PTP_WORK),
        ('Padding2', CHAR * 24),
        ('ConcurrentLoadsInProgress', ULONG),
        ('Padding3', CHAR * 60),
        ('Padding4', ULONGLONG * 8),
    ]
PREADONLY_TRACE_CONTEXT = POINTER(READONLY_TRACE_CONTEXT)

class _TRACE_STORE_INNER_FLAGS(Structure):
    _fields_ = [
        ('NoRetire', ULONG, 1),
        ('NoPrefaulting', ULONG, 1),
        ('IgnorePreferredAddresses', ULONG, 1),
        ('IsReadonly', ULONG, 1),
        ('SetEndOfFileOnClose', ULONG, 1),
        ('IsMetadata', ULONG, 1),
        ('HasRelocations', ULONG, 1),
        ('NoTruncate', ULONG, 1),
        ('IsRelocationTarget', ULONG, 1),
        ('HasSelfRelocations', ULONG, 1),
        ('OnlyRelocationIsToSelf', ULONG, 1),
        ('HasMultipleRelocationWaits', ULONG, 1),
        ('RequiresSelfRelocation', ULONG, 1),
        ('HasNullStoreRelocations', ULONG, 1),
    ]

class TRACE_STORE(Structure):
    __array = None
    struct_type = None

    @property
    def base_address(self):
        return self.ReadonlyMemoryMaps.contents.BaseAddress

    def __len__(self):
        return self.Totals.contents.NumberOfAllocations

    def address_range_to_array(self, address_range):
        if not self.struct_type:
            return

        base_address = address_range.ActualBaseAddress
        record_size = sizeof(self.struct_type)
        number_of_records = address_range.MappedSize / record_size

        return cast(
            base_address,
            POINTER(self.struct_type * number_of_records),
        ).contents

    @property
    def arrays(self):
        return chain(
            self.address_range_to_array(address_range)
                for address_range in self.address_ranges
        )

    @property
    def address_ranges(self):
        return cast(
            self.ReadonlyAddressRanges,
            POINTER(
                TRACE_STORE_ADDRESS_RANGE *
                self.NumberOfReadonlyAddressRanges
            )
        ).contents

PTRACE_STORE = POINTER(TRACE_STORE)
PPTRACE_STORE = POINTER(PTRACE_STORE)

class _TRACE_STORE_RELOC_DEP(Union):
    _fields_ = [
        ('Stores', PPTRACE_STORE),
        ('Store', PTRACE_STORE),
    ]

TRACE_STORE._fields_ = [
    ('TraceStoreId', TRACE_STORE_ID),
    ('TraceStoreMetadataId', TRACE_STORE_METADATA_ID),
    ('TraceStoreIndex', TRACE_STORE_INDEX),
    ('StoreFlags', _TRACE_STORE_INNER_FLAGS),
    ('TraceFlags', TRACE_FLAGS),
    ('LastError', DWORD),
    ('TotalNumberOfMemoryMaps', ULONG),
    ('NumberOfActiveMemoryMaps', ULONG),
    ('CloseMemoryMaps', SLIST_HEADER),
    ('PrepareMemoryMaps', SLIST_HEADER),
    ('PrepareReadonlyMemoryMaps', SLIST_HEADER),
    ('NextMemoryMaps', SLIST_HEADER),
    ('FreeMemoryMaps', SLIST_HEADER),
    ('PrefaultMemoryMaps', SLIST_HEADER),
    ('SingleMemoryMap', TRACE_STORE_MEMORY_MAP),
    ('ListEntry', SLIST_ENTRY),
    ('Unused', PVOID),
    ('Rtl', PRTL),
    ('Allocator', PALLOCATOR),
    ('TraceContext', PTRACE_CONTEXT),
    ('InitialSize', LARGE_INTEGER),
    ('ExtensionSize', LARGE_INTEGER),
    ('MappingSize', LARGE_INTEGER),
    ('PrefaultFuturePageWork', PTP_WORK),
    ('PrepareNextMemoryMapWork', PTP_WORK),
    ('PrepareReadonlyMemoryMapWork', PTP_WORK),
    ('CloseMemoryMapWork', PTP_WORK),
    ('NextMemoryMapAvailableEvent', HANDLE),
    ('AllMemoryMapsAreFreeEvent', HANDLE),
    ('ReadonlyMappingCompleteEvent', HANDLE),
    ('RelocationCompleteWaitEvent', HANDLE),
    ('RelocationCompleteWaitEvents', PHANDLE),
    ('PrevMemoryMap', PTRACE_STORE_MEMORY_MAP),
    ('MemoryMap', PTRACE_STORE_MEMORY_MAP),
    ('NumberOfRelocationBackReferences', ULONG),
    ('OutstandingRelocationBinds', ULONG),
    ('MappedSequenceId', LONG),
    ('MetadataBindsInProgress', LONG),
    ('PrepareReadonlyNonStreamingMapsInProgress', LONG),
    ('ReadonlyNonStreamingBindCompletesInProgress', LONG),
    ('NumberOfRelocationDependencies', ULONG),
    ('NumberOfRelocationsRequired', ULONG),
    ('SequenceId', ULONG),
    ('NumaNode', ULONG),
    ('CreateFileDesiredAccess', DWORD),
    ('CreateFileCreationDisposition', DWORD),
    ('CreateFileMappingProtectionFlags', DWORD),
    ('CreateFileFlagsAndAttributes', DWORD),
    ('MapViewOfFileDesiredAccess', DWORD),
    ('MappingHandle', HANDLE),
    ('FileHandle', HANDLE),
    ('PrevAddress', PVOID),
    ('TraceStore', PTRACE_STORE),
    ('MetadataInfoStore', PTRACE_STORE),
    ('AllocationStore', PTRACE_STORE),
    ('RelocationStore', PTRACE_STORE),
    ('AddressStore', PTRACE_STORE),
    ('AddressRangeStore', PTRACE_STORE),
    ('BitmapStore', PTRACE_STORE),
    ('InfoStore', PTRACE_STORE),
    ('RelocationDependency', _TRACE_STORE_RELOC_DEP),
    ('AllocateRecords', PALLOCATE_RECORDS),
    ('AllocateAlignedRecords', PALLOCATE_ALIGNED_RECORDS),
    ('AllocateAlignedOffsetRecords', PALLOCATE_ALIGNED_OFFSET_RECORDS),
    ('BindComplete', PBIND_COMPLETE),
    ('pReloc', PTRACE_STORE_RELOC),
    ('pTraits', PTRACE_STORE_TRAITS),
    ('Eof', PTRACE_STORE_EOF),
    ('Time', PTRACE_STORE_TIME),
    ('Stats', PTRACE_STORE_STATS),
    ('Totals', PTRACE_STORE_TOTALS),
    ('Traits', PTRACE_STORE_TRAITS),
    ('Info', PTRACE_STORE_INFO),
    ('Reloc', PTRACE_STORE_RELOC),
    ('Bitmap', PTRACE_STORE_BITMAP),
    ('Allocation', PTRACE_STORE_ALLOCATION),
    ('Address', PTRACE_STORE_ADDRESS),
    ('AddressRange', PTRACE_STORE_ADDRESS_RANGE),
    ('NumberOfAllocations', ULARGE_INTEGER),
    ('NumberOfAddresses', ULARGE_INTEGER),
    ('NumberOfAddressRanges', ULARGE_INTEGER),
    ('NumberOfReadonlyAddressRanges', ULARGE_INTEGER),
    ('ReadonlyAddresses', PTRACE_STORE_ADDRESS),
    ('ReadonlyMemoryMaps', PTRACE_STORE_MEMORY_MAP),
    ('ReadonlyAddressRanges', PTRACE_STORE_ADDRESS_RANGE),
    ('ReadonlyMappingSizes', PULARGE_INTEGER),
    ('ReadonlyPreferredAddressUnavailable', ULONG),
    ('Dummy', PVOID),
]

class METADATA_STORE(TRACE_STORE):
    @property
    def base_address(self):
        return self.MemoryMap.contents.BaseAddress

    @property
    def end_address(self):
        return (
            self.MemoryMap.contents.BaseAddress +
            self.MemoryMap.contents.MappingSize.QuadPart
        )

class ADDRESS_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ADDRESS

class ADDRESS_RANGE_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ADDRESS_RANGE

class ALLOCATION_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ALLOCATION

class PYTHON_TRACE_EVENT_STORE(TRACE_STORE):
    struct_type = PYTHON_TRACE_EVENT

class PYTHON_PATH_TABLE_ENTRY_STORE(TRACE_STORE):
    struct_type = PYTHON_PATH_TABLE_ENTRY

class PYTHON_FUNCTION_TABLE_ENTRY_STORE(TRACE_STORE):
    struct_type = PYTHON_FUNCTION_TABLE_ENTRY

    def get_valid_functions(self):
        funcs = []
        for array in self.arrays:
            for func in array:
                if not func.is_valid:
                    continue
                funcs.append(func)
        return funcs


class TRACE_STORES_RUNDOWN(Structure):
    pass
PTRACE_STORES_RUNDOWN = POINTER(TRACE_STORES_RUNDOWN)

TRACE_STORES._fields_ = [
    ('SizeOfStruct', ULONG),
    ('SizeOfAllocation', ULONG),
    ('NumberOfTraceStore', USHORT),
    ('ElementsPerTraceStore', USHORT),
    ('NumberOfFieldRelocationsElements', USHORT),
    ('Padding1', USHORT),
    ('Padding2', ULONG),
    ('Flags', TRACE_FLAGS),
    ('BaseDirectory', UNICODE_STRING),
    ('Rtl', PRTL),
    ('Allocator', PALLOCATOR),
    ('RundownListEntry', LIST_ENTRY),
    ('Rundown', PTRACE_STORES_RUNDOWN),
    # Start of RelocationCompleteEvents[MAX_TRACE_STORE_IDS].
    ('EventRelocationCompleteEvent', HANDLE),
    ('StringBufferRelocationCompleteEvent', HANDLE),
    ('FunctionTableRelocationCompleteEvent', HANDLE),
    ('FunctionTableEntryRelocationCompleteEvent', HANDLE),
    ('PathTableRelocationCompleteEvent', HANDLE),
    ('PathTableEntryRelocationCompleteEvent', HANDLE),
    ('SessionRelocationCompleteEvent', HANDLE),
    ('StringArrayRelocationCompleteEvent', HANDLE),
    ('StringTableRelocationCompleteEvent', HANDLE),
    # Start of Relocations[MAX_TRACE_STORE_IDS].
    ('EventReloc', TRACE_STORE_RELOC),
    ('StringBufferReloc', TRACE_STORE_RELOC),
    ('FunctionTableReloc', TRACE_STORE_RELOC),
    ('FunctionTableEntryReloc', TRACE_STORE_RELOC),
    ('PathTableReloc', TRACE_STORE_RELOC),
    ('PathTableEntryReloc', TRACE_STORE_RELOC),
    ('SessionReloc', TRACE_STORE_RELOC),
    ('StringArrayReloc', TRACE_STORE_RELOC),
    ('StringTableReloc', TRACE_STORE_RELOC),
    # Start of Stores[MAX_TRACE_STORES].
    ('EventStore', PYTHON_TRACE_EVENT_STORE),
    ('EventMetadataInfoStore', TRACE_STORE),
    ('EventAllocationStore', ALLOCATION_STORE),
    ('EventRelocationStore', TRACE_STORE),
    ('EventAddressStore', ADDRESS_STORE),
    ('EventAddressRangeStore', ADDRESS_RANGE_STORE),
    ('EventBitmapStore', TRACE_STORE),
    ('EventInfoStore', TRACE_STORE),
    ('StringBufferStore', TRACE_STORE),
    ('StringBufferMetadataInfoStore', TRACE_STORE),
    ('StringBufferAllocationStore', ALLOCATION_STORE),
    ('StringBufferRelocationStore', TRACE_STORE),
    ('StringBufferAddressStore', ADDRESS_STORE),
    ('StringBufferAddressRangeStore', ADDRESS_RANGE_STORE),
    ('StringBufferBitmapStore', TRACE_STORE),
    ('StringBufferInfoStore', TRACE_STORE),
    ('FunctionTableStore', TRACE_STORE),
    ('FunctionTableMetadataInfoStore', TRACE_STORE),
    ('FunctionTableAllocationStore', ALLOCATION_STORE),
    ('FunctionTableRelocationStore', TRACE_STORE),
    ('FunctionTableAddressStore', ADDRESS_STORE),
    ('FunctionTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionTableBitmapStore', TRACE_STORE),
    ('FunctionTableInfoStore', TRACE_STORE),
    ('FunctionTableEntryStore', PYTHON_FUNCTION_TABLE_ENTRY_STORE),
    ('FunctionTableEntryMetadataInfoStore', TRACE_STORE),
    ('FunctionTableEntryAllocationStore', ALLOCATION_STORE),
    ('FunctionTableEntryRelocationStore', TRACE_STORE),
    ('FunctionTableEntryAddressStore', ADDRESS_STORE),
    ('FunctionTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionTableEntryBitmapStore', TRACE_STORE),
    ('FunctionTableEntryInfoStore', TRACE_STORE),
    ('PathTableStore', TRACE_STORE),
    ('PathTableMetadataInfoStore', TRACE_STORE),
    ('PathTableAllocationStore', ALLOCATION_STORE),
    ('PathTableRelocationStore', TRACE_STORE),
    ('PathTableAddressStore', ADDRESS_STORE),
    ('PathTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PathTableBitmapStore', TRACE_STORE),
    ('PathTableInfoStore', TRACE_STORE),
    ('PathTableEntryStore', PYTHON_PATH_TABLE_ENTRY_STORE),
    ('PathTableEntryMetadataInfoStore', TRACE_STORE),
    ('PathTableEntryAllocationStore', ALLOCATION_STORE),
    ('PathTableEntryRelocationStore', TRACE_STORE),
    ('PathTableEntryAddressStore', ADDRESS_STORE),
    ('PathTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PathTableEntryBitmapStore', TRACE_STORE),
    ('PathTableEntryInfoStore', TRACE_STORE),
    ('SessionStore', TRACE_STORE),
    ('SessionMetadataInfoStore', TRACE_STORE),
    ('SessionAllocationStore', ALLOCATION_STORE),
    ('SessionRelocationStore', TRACE_STORE),
    ('SessionAddressStore', ADDRESS_STORE),
    ('SessionAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SessionBitmapStore', TRACE_STORE),
    ('SessionInfoStore', TRACE_STORE),
    ('StringArrayStore', TRACE_STORE),
    ('StringArrayMetadataInfoStore', TRACE_STORE),
    ('StringArrayAllocationStore', ALLOCATION_STORE),
    ('StringArrayRelocationStore', TRACE_STORE),
    ('StringArrayAddressStore', ADDRESS_STORE),
    ('StringArrayAddressRangeStore', ADDRESS_RANGE_STORE),
    ('StringArrayBitmapStore', TRACE_STORE),
    ('StringArrayInfoStore', TRACE_STORE),
    ('StringTableStore', TRACE_STORE),
    ('StringTableMetadataInfoStore', TRACE_STORE),
    ('StringTableAllocationStore', ALLOCATION_STORE),
    ('StringTableRelocationStore', TRACE_STORE),
    ('StringTableAddressStore', ADDRESS_STORE),
    ('StringTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('StringTableBitmapStore', TRACE_STORE),
    ('StringTableInfoStore', TRACE_STORE),
]

class TRACE_STORE_STRUCTURE_SIZES(Structure):
    _fields_ = [
        ('TraceStore', ULONG),
        ('TraceStores', ULONG),
        ('TraceContext', ULONG),
        ('TraceStoreStartTime', ULONG),
        ('TraceStoreInfo', ULONG),
        ('TraceStoreMetadataInfo', ULONG),
        ('TraceStoreReloc', ULONG),
        ('TraceStoreAddress', ULONG),
        ('TraceStoreAddressRange', ULONG),
        ('AddressBitCounts', ULONG),
    ]
PTRACE_STORE_STRUCTURE_SIZES = POINTER(TRACE_STORE_STRUCTURE_SIZES);

OurTraceStoreStructureSizes = {
    'TraceStore': sizeof(TRACE_STORE),
    'TraceStores': sizeof(TRACE_STORES),
    'TraceContext': sizeof(TRACE_CONTEXT),
    'TraceStoreStartTime': sizeof(TRACE_STORE_START_TIME),
    'TraceStoreInfo': sizeof(TRACE_STORE_INFO),
    'TraceStoreMetadataInfo': sizeof(TRACE_STORE_METADATA_INFO),
    'TraceStoreReloc': sizeof(TRACE_STORE_RELOC),
    'TraceStoreAddress': sizeof(TRACE_STORE_ADDRESS),
    'TraceStoreAddressRange': sizeof(TRACE_STORE_ADDRESS_RANGE),
    'AddressBitCounts': sizeof(ADDRESS_BIT_COUNTS),
}

#===============================================================================
# Function Types
#===============================================================================

UPDATE_TRACER_CONFIG_WITH_TRACE_STORE_INFO = CFUNCTYPE(
    BOOL,
    PTRACER_CONFIG
)
UPDATE_TRACER_CONFIG_WITH_TRACE_STORE_INFO.errcheck = errcheck

INITIALIZE_READONLY_TRACE_STORES = CFUNCTYPE(
    BOOL,
    PRTL,
    PALLOCATOR,
    PWSTR,
    PTRACE_STORES,
    PULONG,
    PTRACE_FLAGS,
)

INITIALIZE_TRACE_CONTEXT = CFUNCTYPE(
    BOOL,
    PRTL,
    PALLOCATOR,
    PTRACE_CONTEXT,
    PULONG,
    PTRACE_SESSION,
    PTRACE_STORES,
    PTP_CALLBACK_ENVIRON,
    PTRACE_CONTEXT_FLAGS,
    PVOID,
)

#===============================================================================
# Binding
#===============================================================================

TracerConfig = None
TraceStoreDll = None
TraceStoreStructureSizesRaw = None
InitializeReadonlyTraceStores = None
InitializeReadonlyTraceContext = None
UpdateTracerConfigWithTraceStoreInfo = None

def bind(path=None, dll=None):
    global IS_BOUND
    global TraceStoreDll
    global TraceStoreStructureSizes
    global TraceStoreStructureSizesRaw
    global InitializeReadonlyTraceStores
    global InitializeReadonlyTraceContext
    global UpdateTracerConfigWithTraceStoreInfo

    assert path or dll
    if not dll:
        dll = TraceStoreDll
        if not dll:
            dll = CDLL(path)
            TraceStoreDll = dll

    InitializeReadonlyTraceStores = INITIALIZE_READONLY_TRACE_STORES(
        ('InitializeReadonlyTraceStores', dll),
        (
            (1, 'Rtl'),
            (1, 'Allocator'),
            (1, 'BaseDirectory'),
            (1, 'TraceStores'),
            (1, 'SizeOfTraceStores'),
            (1, 'TraceFlags'),
        )
    )

    UpdateTracerConfigWithTraceStoreInfo = (
        UPDATE_TRACER_CONFIG_WITH_TRACE_STORE_INFO(
            ('UpdateTracerConfigWithTraceStoreInfo', dll),
            (
                (1, 'TracerConfig'),
            )
        )
    )

    InitializeReadonlyTraceContext = INITIALIZE_TRACE_CONTEXT(
        ('InitializeReadonlyTraceContext', dll),
        (
            (1, 'Rtl'),
            (1, 'Allocator'),
            (1, 'TraceContext'),
            (1, 'SizeOfTraceContext'),
            (1, 'TraceSession'),
            (1, 'TraceStores'),
            (1, 'ThreadpoolCallbackEnvironment'),
            (1, 'TraceContextFlags'),
            (1, 'UserData'),
        )
    )

    TraceStoreStructureSizesRaw = dll.TraceStoreStructureSizes
    TraceStoreStructureSizes = cast(
        TraceStoreStructureSizesRaw,
        PTRACE_STORE_STRUCTURE_SIZES
    ).contents

    IS_BOUND = True


#===============================================================================
# Python Functions
#===============================================================================

def update_tracer_config_with_trace_store_info(tracer_config):
    global TracerConfig
    if not TracerConfig:
        TracerConfig = tracer_config
        UpdateTracerConfigWithTraceStoreInfo(tracer_config)

def create_and_initialize_readonly_trace_stores(rtl, allocator, basedir,
                                                tracer_config):

    update_tracer_config_with_trace_store_info(tracer_config)

    size = ULONG()

    success = InitializeReadonlyTraceStores(
        cast(0, PRTL),
        cast(0, PALLOCATOR),
        cast(0, PWSTR),
        cast(0, PTRACE_STORES),
        byref(size),
        cast(0, PTRACE_FLAGS),
    )

    expected = sizeof(TRACE_STORES)
    assert size.value == expected, (size.value, expected)

    #buf = create_string_buffer(size.value)
    #ptrace_stores = cast(buf, PTRACE_STORES)

    trace_stores = TRACE_STORES()
    ptrace_stores = byref(trace_stores)
    flags = TRACE_FLAGS()
    prtl = byref(rtl)
    pallocator = byref(allocator)

    success = InitializeReadonlyTraceStores(
        prtl,
        pallocator,
        basedir,
        ptrace_stores,
        byref(size),
        byref(flags),
    )
    assert success

    return trace_stores

def create_and_initialize_readonly_trace_context(
        rtl,
        allocator,
        trace_stores,
        trace_context=None,
        trace_context_flags=None,
        ignore_preferred_addresses_bitmap=None,
        num_cpus=None,
        threadpool=None,
        tp_callback_env=None):

    if not threadpool:
        threadpool = create_threadpool(num_cpus=num_cpus)

    if not tp_callback_env:
        tp_callback_env = TP_CALLBACK_ENVIRON()
        InitializeThreadpoolEnvironment(tp_callback_env)
        SetThreadpoolCallbackPool(tp_callback_env, threadpool)

    if trace_context_flags is None:
        trace_context_flags = TRACE_CONTEXT_FLAGS()
        trace_context_flags.AsyncInitialization = True

    if not trace_context:
        trace_context = TRACE_CONTEXT()

    size = ULONG(sizeof(TRACE_CONTEXT))

    if ignore_preferred_addresses_bitmap is not None:
        trace_context_flags.IgnorePreferredAddressesBitmap = True
        trace_context.set_ignore_preferred_addresses_bitmap(
            ignore_preferred_addresses_bitmap
        )

    success = InitializeReadonlyTraceContext(
        byref(rtl),
        byref(allocator),
        byref(trace_context),
        byref(size),
        cast(0, PTRACE_SESSION),
        byref(trace_stores),
        byref(tp_callback_env),
        byref(trace_context_flags),
        cast(0, PVOID),
    )

    assert success

    return trace_context

def is_readonly_trace_context_ready(readonly_trace_context):
    return is_signaled(readonly_trace_context.LoadingCompleteEvent)

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
