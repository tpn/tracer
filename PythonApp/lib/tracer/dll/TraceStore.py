#===============================================================================
# Imports
#===============================================================================

from itertools import chain

from ..util import (
    memoize,
)

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

    BYTE,
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
    SRWLOCK,
    FILETIME,
    PTP_WORK,
    PTP_TIMER,
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
    CRITICAL_SECTION,
    PROCESSOR_NUMBER,
    PTP_CLEANUP_GROUP,
    TP_CALLBACK_ENVIRON,
    PTP_CALLBACK_ENVIRON,
    FILE_COMPRESSION_INFO,
    PSAPI_WS_WATCH_INFORMATION_EX,
    PPSAPI_WS_WATCH_INFORMATION_EX,
    PSAPI_WORKING_SET_EX_INFORMATION,
    PPSAPI_WORKING_SET_EX_INFORMATION,
)

from .Rtl import (
    PRTL,
)

from .TracerConfig import (
    PTRACER_CONFIG,
)

from .Allocator import (
    ALLOCATOR,
    PALLOCATOR,
)

from .Python import (
    PYTHON_PATH_TABLE_ENTRY,
    PYTHON_FUNCTION_TABLE_ENTRY,
)

from .PythonTracer import (
    PYTHON_TRACE_EVENT1,
    PYTHON_TRACE_EVENT2,
    PYTHON_EVENT_TRAITS_EX,
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
PALLOCATE_RECORDS_WITH_TIMESTAMP = PVOID
PBIND_COMPLETE = PVOID

IS_BOUND = False

#===============================================================================
# Enums/Indexes/Constants
#===============================================================================

TraceStoreNullId                        =   0
TraceStoreEventId                       =   1
TraceStoreStringBufferId                =   2
TraceStoreFunctionTableId               =   3
TraceStoreFunctionTableEntryId          =   4
TraceStorePathTableId                   =   5
TraceStorePathTableEntryId              =   6
TraceStoreSessionId                     =   7
TraceStoreStringArrayId                 =   8
TraceStoreStringTableId                 =   9
TraceStoreEventTraitsExId               =  10
TraceStoreWsWatchInfoExId               =  11
TraceStoreWsWorkingSetExInfoId          =  12
TraceStoreCCallStackTableId             =  13
TraceStoreCCallStackTableEntryId        =  14
TraceStoreCModuleTableId                =  15
TraceStoreCModuleTableEntryId           =  16
TraceStorePythonCallStackTableId        =  17
TraceStorePythonCallStackTableEntryId   =  18
TraceStorePythonModuleTableId           =  19
TraceStorePythonModuleTableEntryId      =  20
TraceStoreLineTableId                   =  21
TraceStoreLineTableEntryId              =  22
TraceStoreLineStringBufferId            =  23
TraceStoreCallStackId                   =  24
TraceStorePerformanceId                 =  25
TraceStorePerformanceDeltaId            =  26
TraceStoreSourceCodeId                  =  27
TraceStoreBitmapId                      =  28
TraceStoreImageFileId                   =  29
TraceStoreUnicodeStringBufferId         =  30
TraceStoreLineId                        =  31
TraceStoreObjectId                      =  32
TraceStoreModuleLoadEventId             =  33
TraceStoreSymbolTableId                 =  34
TraceStoreSymbolTableEntryId            =  35
TraceStoreSymbolModuleInfoId            =  36
TraceStoreSymbolFileId                  =  37
TraceStoreSymbolInfoId                  =  38
TraceStoreSymbolLineId                  =  39
TraceStoreSymbolTypeId                  =  40
TraceStoreStackFrameId                  =  41
TraceStoreTypeInfoTableId               =  42
TraceStoreTypeInfoTableEntryId          =  43
TraceStoreTypeInfoStringBufferId        =  44
TraceStoreFunctionTableId               =  45
TraceStoreFunctionTableEntryId          =  46
TraceStoreFunctionAssemblyId            =  47
TraceStoreFunctionSourceCodeId          =  48
TraceStoreExamineSymbolsLineId          =  49
TraceStoreExamineSymbolsTextId          =  50
TraceStoreExaminedSymbolId              =  51
TraceStoreExaminedSymbolSecondaryId     =  52
TraceStoreUnassembleFunctionLineId      =  53
TraceStoreUnassembleFunctionTextId      =  54
TraceStoreUnassembledFunctionId         =  55
TraceStoreUnassembledFunctionSecondaryId=  56
TraceStoreDisplayTypeLineId             =  57
TraceStoreDisplayTypeTextId             =  58
TraceStoreDisplayedTypeId               =  59
TraceStoreDisplayedTypeSecondaryId      =  60
TraceStoreInvalidId                     =  61

TraceStoreIdToName = {
    TraceStoreNullId: 'Null',
    TraceStoreEventId: 'Event',
    TraceStoreStringBufferId: 'StringBuffer',
    TraceStoreFunctionTableId: 'FunctionTable',
    TraceStoreFunctionTableEntryId: 'FunctionTableEntry',
    TraceStorePathTableId: 'PathTable',
    TraceStorePathTableEntryId: 'PathTableEntry',
    TraceStoreSessionId: 'Session',
    TraceStoreStringArrayId: 'StringArray',
    TraceStoreStringTableId: 'StringTable',
    TraceStoreEventTraitsExId: 'EventTraitsEx',
    TraceStoreWsWatchInfoExId: 'WsWatchInfoEx',
    TraceStoreWsWorkingSetExInfoId: 'WsWorkingSetExInfo',
    TraceStoreCCallStackTableId: 'CCallStackTable',
    TraceStoreCCallStackTableEntryId: 'CCallStackTableEntry',
    TraceStoreCModuleTableId: 'CModuleTable',
    TraceStoreCModuleTableEntryId: 'CModuleTableEntry',
    TraceStorePythonCallStackTableId: 'PythonCallStackTable',
    TraceStorePythonCallStackTableEntryId: 'PythonCallStackTableEntry',
    TraceStorePythonModuleTableId: 'PythonModuleTable',
    TraceStorePythonModuleTableEntryId: 'PythonModuleTableEntry',
    TraceStoreLineTableId: 'LineTable',
    TraceStoreLineTableEntryId: 'LineTableEntry',
    TraceStoreLineStringBufferId: 'LineStringBuffer',
    TraceStoreCallStackId: 'CallStack',
    TraceStorePerformanceId: 'Performance',
    TraceStorePerformanceDeltaId: 'PerformanceDelta',
    TraceStoreSourceCodeId: 'SourceCode',
    TraceStoreBitmapId: 'Bitmap',
    TraceStoreImageFileId: 'ImageFile',
    TraceStoreUnicodeStringBufferId: 'UnicodeStringBuffer',
    TraceStoreLineId: 'Line',
    TraceStoreObjectId: 'Object',
    TraceStoreModuleLoadEventId: 'ModuleLoadEvent',
    TraceStoreSymbolTableId: 'SymbolTable',
    TraceStoreSymbolTableEntryId: 'SymbolTableEntry',
    TraceStoreSymbolModuleInfoId: 'SymbolModuleInfo',
    TraceStoreSymbolFileId: 'SymbolFile',
    TraceStoreSymbolInfoId: 'SymbolInfo',
    TraceStoreSymbolLineId: 'SymbolLine',
    TraceStoreSymbolTypeId: 'SymbolType',
    TraceStoreStackFrameId: 'StackFrame',
    TraceStoreTypeInfoTableId: 'TypeInfoTable',
    TraceStoreTypeInfoTableEntryId: 'TypeInfoTableEntry',
    TraceStoreTypeInfoStringBufferId: 'TypeInfoStringBuffer',
    TraceStoreFunctionTableId: 'FunctionTable',
    TraceStoreFunctionTableEntryId: 'FunctionTableEntry',
    TraceStoreFunctionAssemblyId: 'FunctionAssembly',
    TraceStoreFunctionSourceCodeId: 'FunctionSourceCode',
    TraceStoreExamineSymbolsLineId: 'ExamineSymbolsLine',
    TraceStoreExamineSymbolsTextId: 'ExamineSymbolsText',
    TraceStoreExaminedSymbolId: 'ExaminedSymbol',
    TraceStoreExaminedSymbolSecondaryId: 'ExaminedSymbolSecondary',
    TraceStoreUnassembleFunctionLineId: 'UnassembleFunctionLine',
    TraceStoreUnassembleFunctionTextId: 'UnassembleFunctionText',
    TraceStoreUnassembledFunctionId: 'UnassembledFunction',
    TraceStoreUnassembledFunctionSecondaryId: 'UnassembledFunctionSecondary',
    TraceStoreDisplayTypeLineId: 'DisplayTypeLine',
    TraceStoreDisplayTypeTextId: 'DisplayTypeText',
    TraceStoreDisplayedTypeId: 'DisplayedType',
    TraceStoreDisplayedTypeSecondaryId: 'DisplayedTypeSecondary',
    TraceStoreInvalidId: 'Invalid',
}

TraceStoreMetadataNullId                        = 0
TraceStoreMetadataMetadataInfoId                = 1
TraceStoreMetadataAllocationId                  = 2
TraceStoreMetadataRelocationId                  = 3
TraceStoreMetadataAddressId                     = 4
TraceStoreMetadataAddressRangeId                = 5
TraceStoreMetadataAllocationTimestampId         = 6
TraceStoreMetadataAllocationTimestampDeltaId    = 7
TraceStoreMetadataSynchronizationId             = 8
TraceStoreMetadataInfoId                        = 9
TraceStoreMetadataInvalidId                     = 10

MAX_TRACE_STORE_IDS = TraceStoreInvalidId - 1
TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS = 1
NUMBER_OF_METADATA_STORES = TraceStoreMetadataInvalidId - 1
ELEMENTS_PER_TRACE_STORE = NUMBER_OF_METADATA_STORES + 1
MAX_TRACE_STORE_INDEX = MAX_TRACE_STORE_IDS * ELEMENTS_PER_TRACE_STORE
NUM_TRACE_STORES = MAX_TRACE_STORE_IDS

TraceStoreMetadataIdToName = {
    TraceStoreMetadataNullId: 'Null',
    TraceStoreMetadataMetadataInfoId: 'MetadataInfo',
    TraceStoreMetadataAllocationId: 'Allocation',
    TraceStoreMetadataRelocationId: 'Relocation',
    TraceStoreMetadataAddressId: 'Address',
    TraceStoreMetadataAddressRangeId: 'AddressRange',
    TraceStoreMetadataAllocationTimestampId: 'AllocationTimestamp',
    TraceStoreMetadataAllocationTimestampDeltaId: 'AllocationTimestampDelta',
    TraceStoreMetadataSynchronizationId: 'Synchronization',
    TraceStoreMetadataInfoId: 'Info',
    TraceStoreMetadataInvalidId: 'Invalid',
}

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
        ('EnableWorkingSetTracing', ULONG, 1),
        ('EnablePerformanceTracing', ULONG, 1),
    ]
PTRACE_FLAGS = POINTER(TRACE_FLAGS)

class TRACE_CONTEXT_FLAGS(Structure):
    _fields_ = [
        ('Valid', ULONG, 1),
        ('Readonly', ULONG, 1),
        ('DisableAsynchronousInitialization', ULONG, 1),
        ('IgnorePreferredAddresses', ULONG, 1),
    ]
PTRACE_CONTEXT_FLAGS = POINTER(TRACE_CONTEXT_FLAGS)

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
        ('FrequentAllocations', ULONG, 1),
        ('BlockingAllocations', ULONG, 1),
        ('LinkedStore', ULONG, 1),
        ('CoalesceAllocations', ULONG, 1),
        ('Unused', ULONG, 23),
    ]
PTRACE_STORE_TRAITS = POINTER(TRACE_STORE_TRAITS)

class TRACE_STORE_ALLOCATION(Structure):
    _fields_ = [
        ('NumberOfRecords', ULARGE_INTEGER),
        ('RecordSize', LARGE_INTEGER),
    ]

    __array_interface__ = {
        'version': 3,
        'typestr': '<V16',
        'descr': [
            ('NumberOfRecords', '<u8'),
            ('RecordSize', '<u8'),
        ],
    }
PTRACE_STORE_ALLOCATION = POINTER(TRACE_STORE_ALLOCATION)

class TRACE_STORE_ALLOCATION_TIMESTAMP(Structure):
    _fields_ = [
        ('Timestamp', LARGE_INTEGER),
    ]
PTRACE_STORE_ALLOCATION_TIMESTAMP = POINTER(TRACE_STORE_ALLOCATION_TIMESTAMP)

class TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA(Structure):
    _fields_ = [
        ('Delta', LONG),
    ]
PTRACE_STORE_ALLOCATION_TIMESTAMP_DELTA = (
    POINTER(TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA)
)

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
    def __repr__(self):
        fmt = (
            'W:   %s, '
            'PC:  %s, '
            'LZ:  %s, '
            'TZ:  %s'
        )
        return fmt % (
            str(self.Width).zfill(2),
            str(self.PopulationCount).zfill(2),
            str(self.LeadingZeros).zfill(2),
            str(self.TrailingZeros).zfill(2)
        )

class _ADDRESS_BIT_COUNTS_HIGH(Structure):
    _fields_ = [
        ('HighPopulationCount', ULONG, 5),
        ('LowPopulationCount', ULONG, 5),
        ('HighParity', ULONG, 1),
        ('LowParity', ULONG, 1),
        ('Parity', ULONG, 1),
        ('Unused', ULONG, 19),
    ]
    def __repr__(self):
        fmt = (
            'HPC: %s, '
            'LPC: %s, '
            'HP:  %s, '
            'LP:  %s, '
            'P:   %s'
        )
        return fmt % (
            str(self.HighPopulationCount).zfill(2),
            str(self.LowPopulationCount).zfill(2),
            str(self.HighParity).zfill(2),
            str(self.LowParity).zfill(2),
            str(self.Parity).zfill(2),
        )

class _ADDRESS_BIT_COUNTS(Structure):
    _fields_ = [
        ('l', _ADDRESS_BIT_COUNTS_LOW),
        ('h', _ADDRESS_BIT_COUNTS_HIGH),
    ]
    def __repr__(self):
        return ', '.join((repr(self.l), repr(self.h)))

class ADDRESS_BIT_COUNTS(Union):
    _fields_ = [
        ('AsLongLong', ULONGLONG),
        ('s', _ADDRESS_BIT_COUNTS),
    ]
    def __repr__(self):
        return repr(self.s)
PADDRESS_BIT_COUNTS = POINTER(ADDRESS_BIT_COUNTS)

class _TRACE_STORE_ADDRESS_RANGE_BITCOUNTS_INNER(Structure):
    _fields_ = [
        ('Preferred', ADDRESS_BIT_COUNTS),
        ('Actual', ADDRESS_BIT_COUNTS),
    ]

class TRACE_STORE_ADDRESS_RANGE(Structure):
    @property
    def valid_from(self):
        return self.u1.Timestamp.ValidFrom

    @property
    def valid_to(self):
        return self.u1.Timestamp.ValidTo
PTRACE_STORE_ADDRESS_RANGE = POINTER(TRACE_STORE_ADDRESS_RANGE)

class _TRACE_STORE_ADDRESS_RANGE_TIMESTAMP_INNER(Structure):
    _fields_ = [
        ('ValidFrom', LARGE_INTEGER),
        ('ValidTo', LARGE_INTEGER),
    ]

class _TRACE_STORE_ADDRESS_RANGE_UNION1(Union):
    _fields_ = [
        ('OriginalAddressRange', PTRACE_STORE_ADDRESS_RANGE),
        ('Timestamp', _TRACE_STORE_ADDRESS_RANGE_TIMESTAMP_INNER),
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

    @property
    def frequency(self):
        return self.Frequency

    @property
    def multiplicand(self):
        return self.Multiplicand

    @property
    def start_time(self):
        return self.StartTime

PTRACE_STORE_TIME = POINTER(TRACE_STORE_TIME)

class TRACE_STORE_STATS(Structure):
    _fields_ = [
        ('DroppedRecords', ULONG),
        ('ExhaustedFreeMemoryMaps', ULONG),
        ('AllocationsOutpacingNextMemoryMapPreparation', ULONG),
        ('PreferredAddressUnavailable', ULONG),
        ('AccessViolationsEncounteredDuringAsyncPrefault', ULONG),
        ('BlockedAllocations', ULONG),
        ('SuspendedAllocations', ULONG),
        ('ElapsedSuspensionTimeInMicroseconds', ULONG),
        ('WastedBytes', ULONGLONG),
        ('PaddedAllocations', ULONG),
        ('Reserved', ULONG * 5),
    ]

    @property
    def dropped_records(self):
        return self.DroppedRecords

    @property
    def exhausted_free_memory_maps(self):
        return self.ExhaustedFreeMemoryMaps

    @property
    def allocations_outpacing_next_memory_map_preparation(self):
        return self.AllocationsOutpacingNextMemoryMapPreparation

    @property
    def preferred_address_unavailable(self):
        return self.PreferredAddressUnavailable

    @property
    def access_violations_encountered_during_async_prefault(self):
        return self.AccessViolationsEncounteredDuringAsyncPrefault

    @property
    def blocked_allocations(self):
        return self.BlockedAllocations

    @property
    def suspended_allocations(self):
        return self.SuspendedAllocations

    @property
    def elapsed_suspension_time(self):
        return self.ElapsedSuspensionTimeInMicroseconds

PTRACE_STORE_STATS = POINTER(TRACE_STORE_STATS)

class TRACE_STORE_TOTALS(Structure):
    _fields_ = [
        ('NumberOfAllocations', ULARGE_INTEGER),
        ('AllocationSize', ULARGE_INTEGER),
    ]

    @property
    def num_allocations(self):
        return self.NumberOfAllocations

    @property
    def allocation_size(self):
        return self.AllocationSize

PTRACE_STORE_TOTALS = POINTER(TRACE_STORE_TOTALS)

class TRACE_STORE_INFO(Structure):
    _fields_ = [
        ('Eof', TRACE_STORE_EOF),
        ('Time', TRACE_STORE_TIME),
        ('Stats', TRACE_STORE_STATS),
        ('Totals', TRACE_STORE_TOTALS),
        ('Traits', TRACE_STORE_TRAITS),
        ('Reserved', CHAR * 76),
    ]
PTRACE_STORE_INFO = POINTER(TRACE_STORE_INFO)

class TRACE_STORE_METADATA_INFO(Structure):
    _fields_ = [
        ('MetadataInfo', TRACE_STORE_INFO),
        ('Allocation', TRACE_STORE_INFO),
        ('Relocation', TRACE_STORE_INFO),
        ('Address', TRACE_STORE_INFO),
        ('AddressRange', TRACE_STORE_INFO),
        ('AllocationTimestamp', TRACE_STORE_INFO),
        ('AllocationTimestampDelta', TRACE_STORE_INFO),
        ('Synchronization', TRACE_STORE_INFO),
        ('Info', TRACE_STORE_INFO),
        ('Reserved', TRACE_STORE_INFO * 7),
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
        ('TraceStoreEventTraitsExId', ULONG, 1),
        ('TraceStoreWsWatchInfoExId', ULONG, 1),
        ('TraceStoreWsWorkingSetExInfoId', ULONG, 1),
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
        ('LastError', ULONG),
        ('Rtl', PRTL),
        ('Allocator', PALLOCATOR),
        ('TracerConfig', PTRACER_CONFIG),
        ('TraceSession', PTRACE_SESSION),
        ('TraceStores', PTRACE_STORES),
        ('TimerFunction', PVOID),
        ('UserData', PVOID),
        ('InitializeAllocatorFromTraceStore', PVOID),
        ('ThreadpoolCallbackEnvironment', PTP_CALLBACK_ENVIRON),
        ('CancellationThreadpoolCallbackEnvironment', PTP_CALLBACK_ENVIRON),
        ('LoadingCompleteEvent', HANDLE),
        ('BindMetadataInfoStoreWork', TRACE_STORE_WORK),
        ('BindRemainingMetadataStoresWork', TRACE_STORE_WORK),
        ('BindTraceStoreWork', TRACE_STORE_WORK),
        ('ReadonlyNonStreamingBindCompleteWork', TRACE_STORE_WORK),
        ('NewModuleEntryWork', TRACE_STORE_WORK),
        ('FailedListHead', SLIST_HEADER),
        ('ThreadpoolCleanupGroup', PTP_CLEANUP_GROUP),
        ('CleanupThreadpoolMembersWork', PTP_WORK),
        ('WorkingSetChangesLock', SRWLOCK),
        ('GetWorkingSetChangesTimer', PTP_TIMER),
        ('WorkingSetTimerContention', ULONG),
        ('WsWatchInfoExInitialBufferNumberOfElements', ULONG),
        ('WsWatchInfoExCurrentBufferNumberOfElements', ULONG),
        ('GetWorkingSetChangesIntervalInMilliseconds', ULONG),
        ('GetWorkingSetChangesWindowLengthInMilliseconds', ULONG),
        ('WsWatchInfoExBuffer', PPSAPI_WS_WATCH_INFORMATION_EX),
        ('WsWorkingSetExInfoBuffer', PPSAPI_WORKING_SET_EX_INFORMATION),
        ('CapturePerformanceMetricsLock', SRWLOCK),
        ('CapturePerformanceMetricsTimer', PTP_TIMER),
        ('CapturePerformanceMetricsTimerContention', ULONG),
        ('CapturePerformanceMetricsIntervalInMilliseconds', ULONG),
        ('CapturePerformanceMetricsWindowLengthInMilliseconds', ULONG),
        ('ActiveWorkItems', ULONG),
        ('BindsInProgress', ULONG),
        ('PrepareReadonlyNonStreamingMapsInProgress', ULONG),
        ('ReadonlyNonStreamingBindCompletesInProgress', ULONG),
        ('NumberOfStoresWithMultipleRelocationDependencies', ULONG),
        ('Time', TRACE_STORE_TIME),
        ('RunHistoryRegistryKey', PVOID),
        ('ModuleNamePrefixTableLock', SRWLOCK),
        ('BitmapAllocator', ALLOCATOR),
        ('UnicodeStringBufferAllocator', ALLOCATOR),
        ('ImageFileAllocator', ALLOCATOR),
        ('AtExitExEntry', PVOID),
        ('SymbolContext', PVOID),
        ('DllNotificationCookie', PVOID),
        ('BitmapBufferSizeInQuadwords', ULONG),
        ('IgnorePreferredAddressesBitmap', RTL_BITMAP),
        ('BitmapBuffer', _TRACE_CONTEXT_IGNORE_PREFERRED_ADDRESS_BITMAP),
        ('Dummy1', PVOID),
        ('Dummy2', PVOID),
    ]

    def set_ignore_preferred_addresses_bitmap(self, ignore):
        self.BitmapBufferSizeInQuadwords = TRACE_STORE_BITMAP_SIZE_IN_QUADWORDS
        bitmap = self.IgnorePreferredAddressesBitmap
        bitmap.SizeOfBitMap = MAX_TRACE_STORE_IDS
        self.BitmapBuffer.TraceStoreBitmap.Raw = ignore.Raw

PTRACE_CONTEXT = POINTER(TRACE_CONTEXT)

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
    is_metadata = False

    @property
    @memoize
    def size(self):
        return self.Totals.contents.AllocationSize

    @property
    @memoize
    def num_allocations(self):
        return self.Totals.contents.NumberOfAllocations

    @property
    @memoize
    def file_compression_info(self):
        return FILE_COMPRESSION_INFO.get(self.FileHandle)

    @property
    @memoize
    def compressed_size(self):
        return self.file_compression_info.CompressedFileSize

    @property
    @memoize
    def compression_ratio(self):
        try:
            return float(self.size) / float(self.compressed_size)
        except ZeroDivisionError:
            return 0.0

    @property
    @memoize
    def space_saved(self):
        try:
            return 1.0 - (float(self.compressed_size) / float(self.size))
        except ZeroDivisionError:
            return 0.0

    @property
    @memoize
    def space_saved_percent(self):
        return self.space_saved * 100.0

    @property
    def address_store(self):
        return self.AddressStore.contents

    @property
    def num_addresses(self):
        return self.address_store.totals.NumberOfAllocations

    @property
    def address_range_store(self):
        return self.AddressRangeStore.contents

    @property
    @memoize
    def num_address_ranges(self):
        return self.address_range_store.totals.NumberOfAllocations

    @property
    def allocation_timestamp_store(self):
        return self.AllocationTimestampStore.contents

    @property
    def num_allocation_timestamps(self):
        return self.allocation_timestamp_store.totals.NumberOfAllocations

    @property
    def allocation_timestamps(self):
        return self.AllocationTimestamp

    @property
    def allocation_timestamps_array(self):
        import numpy as np
        from numpy.ctypeslib import as_array
        ctypes_array = self.AllocationTimestamp
        size = self.num_allocation_timestamps
        numpy_array = as_array(ctypes_array, shape=(size,))
        numpy_array.dtype = np.uint64
        return numpy_array

    @property
    def allocation_timestamp_delta_store(self):
        return self.AllocationTimestampStore.contents

    @property
    def num_allocation_timestamp_deltas(self):
        return self.allocation_timestamp_delta_store.totals.NumberOfAllocations

    @property
    def allocation_timestamp_deltas(self):
        return self.AllocationTimestamp

    @property
    def allocation_timestamp_deltas_array(self):
        import numpy as np
        from numpy.ctypeslib import as_array
        ctypes_array = self.AllocationTimestampDelta
        size = self.num_allocation_timestamp_deltas
        numpy_array = as_array(ctypes_array, shape=(size,))
        numpy_array.dtype = np.uint32
        return numpy_array

    @property
    def as_numpy_array(self):
        import numpy as np
        from numpy.ctypeslib import as_array
        ctypes_array = self.as_array
        size = self.number_of_records
        numpy_array = as_array(ctypes_array, shape=(size,))
        numpy_array.dtype = self.numpy_dtype
        return numpy_array

    @property
    def as_array(self):
        assert self.struct_type

        return cast(
            self.base_address,
            POINTER(self.struct_type * self.number_of_records),
        ).contents

    @property
    def stats(self):
        return self.Stats.contents

    @property
    def totals(self):
        return self.Totals.contents

    @property
    def info(self):
        return self.Info.contents

    @property
    def mapping_size(self):
        return self.MappingSize

    @property
    def dropped_records(self):
        return self.stats.DroppedRecords

    @property
    def exhausted_free_memory_maps(self):
        return self.stats.ExhaustedFreeMemoryMaps

    @property
    def allocations_outpacing_next_memory_map_preparation(self):
        return self.stats.AllocationsOutpacingNextMemoryMapPreparation

    @property
    def preferred_address_unavailable(self):
        return self.stats.PreferredAddressUnavailable

    @property
    def preferred_address_unavailable(self):
        return self.stats.PreferredAddressUnavailable

    @property
    def access_violations_encountered_during_async_prefault(self):
        return self.stats.AccessViolationsEncounteredDuringAsyncPrefault

    @property
    def blocked_allocations(self):
        return self.stats.BlockedAllocations

    @property
    def suspended_allocations(self):
        return self.stats.SuspendedAllocations

    @property
    def elapsed_suspension_time(self):
        return self.stats.ElapsedSuspensionTimeInMicroseconds

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
    def readonly_arrays(self):
        return chain(
            self.address_range_to_array(address_range)
                for address_range in self.readonly_address_ranges
        )

    @property
    def address_ranges(self):
        return cast(
            self.AddressRange,
            POINTER(
                TRACE_STORE_ADDRESS_RANGE *
                self.NumberOfAddressRanges
            )
        ).contents

    @property
    def readonly_address_ranges(self):
        return cast(
            self.ReadonlyAddressRanges,
            POINTER(
                TRACE_STORE_ADDRESS_RANGE *
                self.NumberOfReadonlyAddressRanges
            )
        ).contents

    @property
    def addresses(self):
        return cast(
            self.Address,
            POINTER(
                TRACE_STORE_ADDRESS *
                self.address_store.totals.NumberOfAllocations
            )
        ).contents

    @property
    def mapped_size(self):
        return self.MemoryMap.contents.MappingSize

    @property
    def end_address(self):
        return self.base_address + self.mapped_size

    @property
    def number_of_records(self):
        assert self.struct_type
        return self.mapped_size / sizeof(self.struct_type)

    @property
    def as_array(self):
        assert self.struct_type

        return cast(
            self.base_address,
            POINTER(self.struct_type * self.number_of_records),
        ).contents


PTRACE_STORE = POINTER(TRACE_STORE)
PPTRACE_STORE = POINTER(PTRACE_STORE)

class TRACE_STORE_LIST_ENTRY(Structure):
    _fields_ = [
        ('Flink', PTRACE_STORE),
        ('Blink', PTRACE_STORE),
    ]
PTRACE_STORE_LIST_ENTRY = POINTER(TRACE_STORE_LIST_ENTRY)

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
    ('TotalNumberOfMemoryMaps', LONG),
    ('NumberOfActiveMemoryMaps', LONG),
    ('NumberOfNonRetiredMemoryMaps', LONG),
    ('Padding1', LONG),
    ('Padding2', ULONGLONG),
    ('CloseMemoryMaps', SLIST_HEADER),
    ('PrepareMemoryMaps', SLIST_HEADER),
    ('PrepareReadonlyMemoryMaps', SLIST_HEADER),
    ('NextMemoryMaps', SLIST_HEADER),
    ('FreeMemoryMaps', SLIST_HEADER),
    ('PrefaultMemoryMaps', SLIST_HEADER),
    ('NonRetiredMemoryMaps', SLIST_HEADER),
    ('SingleMemoryMap', TRACE_STORE_MEMORY_MAP),
    ('StoresListEntry', TRACE_STORE_LIST_ENTRY),
    ('MetadataListHeadOrEntry', TRACE_STORE_LIST_ENTRY),
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
    ('BindCompleteEvent', HANDLE),
    ('ResumeAllocationsEvent', HANDLE),
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
    ('AllocationTimestampStore', PTRACE_STORE),
    ('AllocationTimestampDeltaStore', PTRACE_STORE),
    ('SynchronizationStore', PTRACE_STORE),
    ('InfoStore', PTRACE_STORE),
    ('RelocationDependency', _TRACE_STORE_RELOC_DEP),
    ('AllocateRecords', PALLOCATE_RECORDS),
    ('AllocateRecordsWithTimestamp', PALLOCATE_RECORDS_WITH_TIMESTAMP),
    ('TryAllocateRecords', PVOID),
    ('TryAllocateRecordsWithTimestamp', PVOID),
    ('SuspendedAllocateRecordsWithTimestamp', PALLOCATE_RECORDS_WITH_TIMESTAMP),
    ('AllocateRecordsWithTimestampImpl1', PALLOCATE_RECORDS_WITH_TIMESTAMP),
    ('AllocateRecordsWithTimestampImpl2', PALLOCATE_RECORDS_WITH_TIMESTAMP),
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
    ('Allocation', PTRACE_STORE_ALLOCATION),
    ('AllocationTimestamp', PTRACE_STORE_ALLOCATION_TIMESTAMP),
    ('AllocationTimestampDelta', PTRACE_STORE_ALLOCATION_TIMESTAMP_DELTA),
    ('Address', PTRACE_STORE_ADDRESS),
    ('AddressRange', PTRACE_STORE_ADDRESS_RANGE),
    ('Sync', PVOID),
    ('NumberOfAllocations', ULARGE_INTEGER),
    ('NumberOfAddresses', ULARGE_INTEGER),
    ('NumberOfAddressRanges', ULARGE_INTEGER),
    ('NumberOfReadonlyAddressRanges', ULARGE_INTEGER),
    ('ReadonlyAddresses', PTRACE_STORE_ADDRESS),
    ('ReadonlyMemoryMaps', PTRACE_STORE_MEMORY_MAP),
    ('ReadonlyAddressRanges', PTRACE_STORE_ADDRESS_RANGE),
    ('ReadonlyMappingSizes', PULARGE_INTEGER),
    ('ReadonlyPreferredAddressUnavailable', ULONG),
    ('Reserved', BYTE * 172),
]
assert sizeof(TRACE_STORE) == 1024, sizeof(TRACE_STORE)

class METADATA_STORE(TRACE_STORE):
    is_metadata = True

    @property
    def base_address(self):
        return self.MemoryMap.contents.BaseAddress

    @property
    def mapped_size(self):
        return self.MemoryMap.contents.MappingSize

    @property
    def end_address(self):
        return self.base_address + self.mapped_size

    @property
    def number_of_records(self):
        assert self.struct_type
        return self.mapped_size / sizeof(self.struct_type)

    @property
    def as_array(self):
        assert self.struct_type

        return cast(
            self.base_address,
            POINTER(self.struct_type * self.number_of_records),
        ).contents

    @property
    def as_numpy_array(self):
        import numpy as np
        from numpy.ctypeslib import as_array
        ctypes_array = self.as_array
        size = self.number_of_records
        numpy_array = as_array(ctypes_array, shape=(size,))
        numpy_array.dtype = self.numpy_dtype
        return numpy_array

    @property
    def as_numpy_array_old(self):
        import numpy as np
        from numpy.ctypeslib import as_array
        ctypes_array = self.as_array
        size = self.number_of_records
        numpy_array = as_array(ctypes_array, shape=(size,))
        numpy_array.dtype = self.numpy_dtype
        return numpy_array


class INFO_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_INFO

class ADDRESS_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ADDRESS

class ADDRESS_RANGE_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ADDRESS_RANGE

class ALLOCATION_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ALLOCATION

    @property
    def numpy_dtype(self):
        return self.struct_type._get_numpy_dtype()

class RELOCATION_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_RELOC

class ALLOCATION_TIMESTAMP_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ALLOCATION_TIMESTAMP

class ALLOCATION_TIMESTAMP_DELTA_STORE(METADATA_STORE):
    struct_type = TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA

class WS_WATCH_INFO_EX_STORE(TRACE_STORE):
    struct_type = PSAPI_WS_WATCH_INFORMATION_EX

class WS_WORKING_SET_EX_INFO_STORE(TRACE_STORE):
    struct_type = PSAPI_WORKING_SET_EX_INFORMATION

class PYTHON_TRACE_EVENT1_STORE(TRACE_STORE):
    struct_type = PYTHON_TRACE_EVENT1

class PYTHON_TRACE_EVENT2_STORE(TRACE_STORE):
    struct_type = PYTHON_TRACE_EVENT2

class PYTHON_EVENT_TRAITS_EX_STORE(TRACE_STORE):
    struct_type = PYTHON_EVENT_TRAITS_EX

class PYTHON_PATH_TABLE_ENTRY_STORE(TRACE_STORE):
    struct_type = PYTHON_PATH_TABLE_ENTRY

    @property
    def entries(self):
        return [
            pte for pte in (array for array in self.readonly_arrays)
        ]

class PYTHON_FUNCTION_TABLE_ENTRY_STORE(TRACE_STORE):
    struct_type = PYTHON_FUNCTION_TABLE_ENTRY

    def get_valid_functions(self):
        funcs = []
        for array in self.readonly_arrays:
            for func in array:
                if not func.is_valid:
                    continue
                funcs.append(func)
        return funcs


class TRACE_STORES_RUNDOWN(Structure):
    pass
PTRACE_STORES_RUNDOWN = POINTER(TRACE_STORES_RUNDOWN)

#
# To add new metadata stores:
#
# :'a,'b s/('\(.\+\)InfoStore', INFO_STORE),/('\1SynchronizationStore', TRACE_STORE),\r    ('\1InfoStore', INFO_STORE),/g
#
# ^ Adds SynchronizationStore before InfoStore.
#

TRACE_STORES._fields_ = [
    ('SizeOfStruct', ULONG),
    ('SizeOfAllocation', ULONG),
    ('NumberOfTraceStores', USHORT),
    ('ElementsPerTraceStore', USHORT),
    ('NumberOfFieldRelocationsElements', USHORT),
    ('Padding1', USHORT),
    ('Padding2', ULONG),
    ('Flags', TRACE_FLAGS),
    ('BaseDirectory', UNICODE_STRING),
    ('Rtl', PRTL),
    ('Allocator', PALLOCATOR),
    ('TracerConfig', PTRACER_CONFIG),
    ('RundownListEntry', LIST_ENTRY),
    ('Rundown', PTRACE_STORES_RUNDOWN),
    ('StoresListHead', TRACE_STORE_LIST_ENTRY),

    (
        '__Reserved1__',
        BYTE * 24
    ),

    # Start of RelocationCompleteEvents[MAX_TRACE_STORE_IDS].
    # Aligned @ 128 bytes.
    ('EventRelocationCompleteEvent', HANDLE),
    ('StringBufferRelocationCompleteEvent', HANDLE),
    ('FunctionTableRelocationCompleteEvent', HANDLE),
    ('FunctionTableEntryRelocationCompleteEvent', HANDLE),
    ('PathTableRelocationCompleteEvent', HANDLE),
    ('PathTableEntryRelocationCompleteEvent', HANDLE),
    ('SessionRelocationCompleteEvent', HANDLE),
    ('StringArrayRelocationCompleteEvent', HANDLE),
    ('StringTableRelocationCompleteEvent', HANDLE),
    ('EventTraitsExRelocationCompleteEvent', HANDLE),
    ('WsWatchInfoExRelocationCompleteEvent', HANDLE),
    ('WsWorkingSetExInfoRelocationCompleteEvent', HANDLE),
    ('CCallStackTableRelocationCompleteEvent', HANDLE),
    ('CCallStackTableEntryRelocationCompleteEvent', HANDLE),
    ('CModuleTableRelocationCompleteEvent', HANDLE),
    ('CModuleTableEntryRelocationCompleteEvent', HANDLE),
    ('PythonCallStackTableRelocationCompleteEvent', HANDLE),
    ('PythonCallStackTableEntryRelocationCompleteEvent', HANDLE),
    ('PythonModuleTableRelocationCompleteEvent', HANDLE),
    ('PythonModuleTableEntryRelocationCompleteEvent', HANDLE),
    ('LineTableRelocationCompleteEvent', HANDLE),
    ('LineTableEntryRelocationCompleteEvent', HANDLE),
    ('LineStringBufferRelocationCompleteEvent', HANDLE),
    ('CallStackRelocationCompleteEvent', HANDLE),
    ('PerformanceRelocationCompleteEvent', HANDLE),
    ('PerformanceDeltaRelocationCompleteEvent', HANDLE),
    ('SourceCodeRelocationCompleteEvent', HANDLE),
    ('BitmapRelocationCompleteEvent', HANDLE),
    ('ImageFileRelocationCompleteEvent', HANDLE),
    ('UnicodeStringBufferRelocationCompleteEvent', HANDLE),
    ('LineRelocationCompleteEvent', HANDLE),
    ('ObjectRelocationCompleteEvent', HANDLE),
    ('ModuleLoadEventRelocationCompleteEvent', HANDLE),
    ('SymbolTableRelocationCompleteEvent', HANDLE),
    ('SymbolTableEntryRelocationCompleteEvent', HANDLE),
    ('SymbolModuleInfoRelocationCompleteEvent', HANDLE),
    ('SymbolFileRelocationCompleteEvent', HANDLE),
    ('SymbolInfoRelocationCompleteEvent', HANDLE),
    ('SymbolLineRelocationCompleteEvent', HANDLE),
    ('SymbolTypeRelocationCompleteEvent', HANDLE),
    ('StackFrameRelocationCompleteEvent', HANDLE),

    (
        '__Reserved2__',
        BYTE * (512 - 128 - (sizeof(HANDLE) * NUM_TRACE_STORES))
    ),

    # Start of Relocations[MAX_TRACE_STORE_IDS].
    # Aligned @ 512 bytes.
    ('EventReloc', TRACE_STORE_RELOC),
    ('StringBufferReloc', TRACE_STORE_RELOC),
    ('FunctionTableReloc', TRACE_STORE_RELOC),
    ('FunctionTableEntryReloc', TRACE_STORE_RELOC),
    ('PathTableReloc', TRACE_STORE_RELOC),
    ('PathTableEntryReloc', TRACE_STORE_RELOC),
    ('SessionReloc', TRACE_STORE_RELOC),
    ('StringArrayReloc', TRACE_STORE_RELOC),
    ('StringTableReloc', TRACE_STORE_RELOC),
    ('EventTraitsExReloc', TRACE_STORE_RELOC),
    ('WsWatchInfoExReloc', TRACE_STORE_RELOC),
    ('WsWorkingSetExInfoReloc', TRACE_STORE_RELOC),
    ('CCallStackTableReloc', TRACE_STORE_RELOC),
    ('CCallStackTableEntryReloc', TRACE_STORE_RELOC),
    ('CModuleTableReloc', TRACE_STORE_RELOC),
    ('CModuleTableEntryReloc', TRACE_STORE_RELOC),
    ('PythonCallStackTableReloc', TRACE_STORE_RELOC),
    ('PythonCallStackTableEntryReloc', TRACE_STORE_RELOC),
    ('PythonModuleTableReloc', TRACE_STORE_RELOC),
    ('PythonModuleTableEntryReloc', TRACE_STORE_RELOC),
    ('LineTableReloc', TRACE_STORE_RELOC),
    ('LineTableEntryReloc', TRACE_STORE_RELOC),
    ('LineStringBufferReloc', TRACE_STORE_RELOC),
    ('CallStackReloc', TRACE_STORE_RELOC),
    ('PerformanceReloc', TRACE_STORE_RELOC),
    ('PerformanceDeltaReloc', TRACE_STORE_RELOC),
    ('SourceCodeReloc', TRACE_STORE_RELOC),
    ('BitmapReloc', TRACE_STORE_RELOC),
    ('ImageFileReloc', TRACE_STORE_RELOC),
    ('UnicodeStringBufferReloc', TRACE_STORE_RELOC),
    ('LineReloc', TRACE_STORE_RELOC),
    ('ObjectReloc', TRACE_STORE_RELOC),
    ('ModuleLoadEventReloc', TRACE_STORE_RELOC),
    ('SymbolTableReloc', TRACE_STORE_RELOC),
    ('SymbolTableEntryReloc', TRACE_STORE_RELOC),
    ('SymbolModuleInfoReloc', TRACE_STORE_RELOC),
    ('SymbolFileReloc', TRACE_STORE_RELOC),
    ('SymbolInfoReloc', TRACE_STORE_RELOC),
    ('SymbolLineReloc', TRACE_STORE_RELOC),
    ('SymbolTypeReloc', TRACE_STORE_RELOC),
    ('StackFrameReloc', TRACE_STORE_RELOC),

    (
        '__Reserved3__',
        BYTE * (4096 - 512 - (sizeof(TRACE_STORE_RELOC) * NUM_TRACE_STORES))
    ),

    # Start of Stores[MAX_TRACE_STORES].
    # Aligned @ 4096.
    ('EventStore', PYTHON_TRACE_EVENT2_STORE),
    ('EventMetadataInfoStore', TRACE_STORE),
    ('EventAllocationStore', ALLOCATION_STORE),
    ('EventRelocationStore', RELOCATION_STORE),
    ('EventAddressStore', ADDRESS_STORE),
    ('EventAddressRangeStore', ADDRESS_RANGE_STORE),
    ('EventAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('EventAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('EventSynchronizationStore', TRACE_STORE),
    ('EventInfoStore', INFO_STORE),

    ('StringBufferStore', TRACE_STORE),
    ('StringBufferMetadataInfoStore', TRACE_STORE),
    ('StringBufferAllocationStore', ALLOCATION_STORE),
    ('StringBufferRelocationStore', RELOCATION_STORE),
    ('StringBufferAddressStore', ADDRESS_STORE),
    ('StringBufferAddressRangeStore', ADDRESS_RANGE_STORE),
    ('StringBufferAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('StringBufferAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('StringBufferSynchronizationStore', TRACE_STORE),
    ('StringBufferInfoStore', INFO_STORE),

    ('FunctionTableStore', TRACE_STORE),
    ('FunctionTableMetadataInfoStore', TRACE_STORE),
    ('FunctionTableAllocationStore', ALLOCATION_STORE),
    ('FunctionTableRelocationStore', RELOCATION_STORE),
    ('FunctionTableAddressStore', ADDRESS_STORE),
    ('FunctionTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('FunctionTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('FunctionTableSynchronizationStore', TRACE_STORE),
    ('FunctionTableInfoStore', INFO_STORE),

    ('FunctionTableEntryStore', PYTHON_FUNCTION_TABLE_ENTRY_STORE),
    ('FunctionTableEntryMetadataInfoStore', TRACE_STORE),
    ('FunctionTableEntryAllocationStore', ALLOCATION_STORE),
    ('FunctionTableEntryRelocationStore', RELOCATION_STORE),
    ('FunctionTableEntryAddressStore', ADDRESS_STORE),
    ('FunctionTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionTableEntryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('FunctionTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('FunctionTableEntrySynchronizationStore', TRACE_STORE),
    ('FunctionTableEntryInfoStore', INFO_STORE),

    ('PathTableStore', TRACE_STORE),
    ('PathTableMetadataInfoStore', TRACE_STORE),
    ('PathTableAllocationStore', ALLOCATION_STORE),
    ('PathTableRelocationStore', RELOCATION_STORE),
    ('PathTableAddressStore', ADDRESS_STORE),
    ('PathTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PathTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('PathTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PathTableSynchronizationStore', TRACE_STORE),
    ('PathTableInfoStore', INFO_STORE),

    ('PathTableEntryStore', PYTHON_PATH_TABLE_ENTRY_STORE),
    ('PathTableEntryMetadataInfoStore', TRACE_STORE),
    ('PathTableEntryAllocationStore', ALLOCATION_STORE),
    ('PathTableEntryRelocationStore', RELOCATION_STORE),
    ('PathTableEntryAddressStore', ADDRESS_STORE),
    ('PathTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PathTableEntryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('PathTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PathTableEntrySynchronizationStore', TRACE_STORE),
    ('PathTableEntryInfoStore', INFO_STORE),

    ('SessionStore', TRACE_STORE),
    ('SessionMetadataInfoStore', TRACE_STORE),
    ('SessionAllocationStore', ALLOCATION_STORE),
    ('SessionRelocationStore', RELOCATION_STORE),
    ('SessionAddressStore', ADDRESS_STORE),
    ('SessionAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SessionAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SessionAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SessionSynchronizationStore', TRACE_STORE),
    ('SessionInfoStore', INFO_STORE),

    ('StringArrayStore', TRACE_STORE),
    ('StringArrayMetadataInfoStore', TRACE_STORE),
    ('StringArrayAllocationStore', ALLOCATION_STORE),
    ('StringArrayRelocationStore', RELOCATION_STORE),
    ('StringArrayAddressStore', ADDRESS_STORE),
    ('StringArrayAddressRangeStore', ADDRESS_RANGE_STORE),
    ('StringArrayAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('StringArrayAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('StringArraySynchronizationStore', TRACE_STORE),
    ('StringArrayInfoStore', INFO_STORE),

    ('StringTableStore', TRACE_STORE),
    ('StringTableMetadataInfoStore', TRACE_STORE),
    ('StringTableAllocationStore', ALLOCATION_STORE),
    ('StringTableRelocationStore', RELOCATION_STORE),
    ('StringTableAddressStore', ADDRESS_STORE),
    ('StringTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('StringTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('StringTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('StringTableSynchronizationStore', TRACE_STORE),
    ('StringTableInfoStore', INFO_STORE),

    ('EventTraitsExStore', PYTHON_EVENT_TRAITS_EX_STORE),
    ('EventTraitsExMetadataInfoStore', TRACE_STORE),
    ('EventTraitsExAllocationStore', ALLOCATION_STORE),
    ('EventTraitsExRelocationStore', RELOCATION_STORE),
    ('EventTraitsExAddressStore', ADDRESS_STORE),
    ('EventTraitsExAddressRangeStore', ADDRESS_RANGE_STORE),
    ('EventTraitsExAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('EventTraitsExAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('EventTraitsExSynchronizationStore', TRACE_STORE),
    ('EventTraitsExInfoStore', INFO_STORE),

    ('WsWatchInfoExStore', WS_WATCH_INFO_EX_STORE),
    ('WsWatchInfoExMetadataInfoStore', TRACE_STORE),
    ('WsWatchInfoExAllocationStore', ALLOCATION_STORE),
    ('WsWatchInfoExRelocationStore', RELOCATION_STORE),
    ('WsWatchInfoExAddressStore', ADDRESS_STORE),
    ('WsWatchInfoExAddressRangeStore', ADDRESS_RANGE_STORE),
    ('WsWatchInfoExAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('WsWatchInfoExAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('WsWatchInfoExSynchronizationStore', TRACE_STORE),
    ('WsWatchInfoExInfoStore', INFO_STORE),

    ('WsWorkingSetExInfoStore', WS_WORKING_SET_EX_INFO_STORE),
    ('WsWorkingSetExInfoMetadataInfoStore', TRACE_STORE),
    ('WsWorkingSetExInfoAllocationStore', ALLOCATION_STORE),
    ('WsWorkingSetExInfoRelocationStore', RELOCATION_STORE),
    ('WsWorkingSetExInfoAddressStore', ADDRESS_STORE),
    ('WsWorkingSetExInfoAddressRangeStore', ADDRESS_RANGE_STORE),
    ('WsWorkingSetExInfoAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('WsWorkingSetExInfoAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('WsWorkingSetExInfoSynchronizationStore', TRACE_STORE),
    ('WsWorkingSetExInfoInfoStore', INFO_STORE),

    ('CCallStackTableStore', TRACE_STORE),
    ('CCallStackTableMetadataInfoStore', TRACE_STORE),
    ('CCallStackTableAllocationStore', ALLOCATION_STORE),
    ('CCallStackTableRelocationStore', RELOCATION_STORE),
    ('CCallStackTableAddressStore', ADDRESS_STORE),
    ('CCallStackTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('CCallStackTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('CCallStackTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('CCallStackTableSynchronizationStore', TRACE_STORE),
    ('CCallStackTableInfoStore', INFO_STORE),

    ('CCallStackTableEntryStore', TRACE_STORE),
    ('CCallStackTableEntryMetadataInfoStore', TRACE_STORE),
    ('CCallStackTableEntryAllocationStore', ALLOCATION_STORE),
    ('CCallStackTableEntryRelocationStore', RELOCATION_STORE),
    ('CCallStackTableEntryAddressStore', ADDRESS_STORE),
    ('CCallStackTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('CCallStackTableEntryAllocationTimestampStore',
     ALLOCATION_TIMESTAMP_STORE),
    ('CCallStackTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('CCallStackTableEntrySynchronizationStore', TRACE_STORE),
    ('CCallStackTableEntryInfoStore', INFO_STORE),

    ('CModuleTableStore', TRACE_STORE),
    ('CModuleTableMetadataInfoStore', TRACE_STORE),
    ('CModuleTableAllocationStore', ALLOCATION_STORE),
    ('CModuleTableRelocationStore', RELOCATION_STORE),
    ('CModuleTableAddressStore', ADDRESS_STORE),
    ('CModuleTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('CModuleTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('CModuleTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('CModuleTableSynchronizationStore', TRACE_STORE),
    ('CModuleTableInfoStore', INFO_STORE),

    ('CModuleTableEntryStore', TRACE_STORE),
    ('CModuleTableEntryMetadataInfoStore', TRACE_STORE),
    ('CModuleTableEntryAllocationStore', ALLOCATION_STORE),
    ('CModuleTableEntryRelocationStore', RELOCATION_STORE),
    ('CModuleTableEntryAddressStore', ADDRESS_STORE),
    ('CModuleTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('CModuleTableEntryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('CModuleTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('CModuleTableEntrySynchronizationStore', TRACE_STORE),
    ('CModuleTableEntryInfoStore', INFO_STORE),

    ('PythonCallStackTableStore', TRACE_STORE),
    ('PythonCallStackTableMetadataInfoStore', TRACE_STORE),
    ('PythonCallStackTableAllocationStore', ALLOCATION_STORE),
    ('PythonCallStackTableRelocationStore', RELOCATION_STORE),
    ('PythonCallStackTableAddressStore', ADDRESS_STORE),
    ('PythonCallStackTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PythonCallStackTableAllocationTimestampStore',
     ALLOCATION_TIMESTAMP_STORE),
    ('PythonCallStackTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PythonCallStackTableSynchronizationStore', TRACE_STORE),
    ('PythonCallStackTableInfoStore', INFO_STORE),

    ('PythonCallStackTableEntryStore', TRACE_STORE),
    ('PythonCallStackTableEntryMetadataInfoStore', TRACE_STORE),
    ('PythonCallStackTableEntryAllocationStore', ALLOCATION_STORE),
    ('PythonCallStackTableEntryRelocationStore', RELOCATION_STORE),
    ('PythonCallStackTableEntryAddressStore', ADDRESS_STORE),
    ('PythonCallStackTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PythonCallStackTableEntryAllocationTimestampStore',
     ALLOCATION_TIMESTAMP_STORE),
    ('PythonCallStackTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PythonCallStackTableEntrySynchronizationStore', TRACE_STORE),
    ('PythonCallStackTableEntryInfoStore', INFO_STORE),

    ('PythonModuleTableStore', TRACE_STORE),
    ('PythonModuleTableMetadataInfoStore', TRACE_STORE),
    ('PythonModuleTableAllocationStore', ALLOCATION_STORE),
    ('PythonModuleTableRelocationStore', RELOCATION_STORE),
    ('PythonModuleTableAddressStore', ADDRESS_STORE),
    ('PythonModuleTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PythonModuleTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('PythonModuleTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PythonModuleTableSynchronizationStore', TRACE_STORE),
    ('PythonModuleTableInfoStore', INFO_STORE),

    ('PythonModuleTableEntryStore', TRACE_STORE),
    ('PythonModuleTableEntryMetadataInfoStore', TRACE_STORE),
    ('PythonModuleTableEntryAllocationStore', ALLOCATION_STORE),
    ('PythonModuleTableEntryRelocationStore', RELOCATION_STORE),
    ('PythonModuleTableEntryAddressStore', ADDRESS_STORE),
    ('PythonModuleTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PythonModuleTableEntryAllocationTimestampStore',
     ALLOCATION_TIMESTAMP_STORE),
    ('PythonModuleTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PythonModuleTableEntrySynchronizationStore', TRACE_STORE),
    ('PythonModuleTableEntryInfoStore', INFO_STORE),

    ('LineTableStore', TRACE_STORE),
    ('LineTableMetadataInfoStore', TRACE_STORE),
    ('LineTableAllocationStore', ALLOCATION_STORE),
    ('LineTableRelocationStore', RELOCATION_STORE),
    ('LineTableAddressStore', ADDRESS_STORE),
    ('LineTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('LineTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('LineTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('LineTableSynchronizationStore', TRACE_STORE),
    ('LineTableInfoStore', INFO_STORE),

    ('LineTableEntryStore', TRACE_STORE),
    ('LineTableEntryMetadataInfoStore', TRACE_STORE),
    ('LineTableEntryAllocationStore', ALLOCATION_STORE),
    ('LineTableEntryRelocationStore', RELOCATION_STORE),
    ('LineTableEntryAddressStore', ADDRESS_STORE),
    ('LineTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('LineTableEntryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('LineTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('LineTableEntrySynchronizationStore', TRACE_STORE),
    ('LineTableEntryInfoStore', INFO_STORE),

    ('LineStringBufferStore', TRACE_STORE),
    ('LineStringBufferMetadataInfoStore', TRACE_STORE),
    ('LineStringBufferAllocationStore', ALLOCATION_STORE),
    ('LineStringBufferRelocationStore', RELOCATION_STORE),
    ('LineStringBufferAddressStore', ADDRESS_STORE),
    ('LineStringBufferAddressRangeStore', ADDRESS_RANGE_STORE),
    ('LineStringBufferAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('LineStringBufferAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('LineStringBufferSynchronizationStore', TRACE_STORE),
    ('LineStringBufferInfoStore', INFO_STORE),

    ('CallStackStore', TRACE_STORE),
    ('CallStackMetadataInfoStore', TRACE_STORE),
    ('CallStackAllocationStore', ALLOCATION_STORE),
    ('CallStackRelocationStore', RELOCATION_STORE),
    ('CallStackAddressStore', ADDRESS_STORE),
    ('CallStackAddressRangeStore', ADDRESS_RANGE_STORE),
    ('CallStackAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('CallStackAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('CallStackSynchronizationStore', TRACE_STORE),
    ('CallStackInfoStore', INFO_STORE),

    ('PerformanceStore', TRACE_STORE),
    ('PerformanceMetadataInfoStore', TRACE_STORE),
    ('PerformanceAllocationStore', ALLOCATION_STORE),
    ('PerformanceRelocationStore', RELOCATION_STORE),
    ('PerformanceAddressStore', ADDRESS_STORE),
    ('PerformanceAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PerformanceAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('PerformanceAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PerformanceSynchronizationStore', TRACE_STORE),
    ('PerformanceInfoStore', INFO_STORE),

    ('PerformanceDeltaStore', TRACE_STORE),
    ('PerformanceDeltaMetadataInfoStore', TRACE_STORE),
    ('PerformanceDeltaAllocationStore', ALLOCATION_STORE),
    ('PerformanceDeltaRelocationStore', RELOCATION_STORE),
    ('PerformanceDeltaAddressStore', ADDRESS_STORE),
    ('PerformanceDeltaAddressRangeStore', ADDRESS_RANGE_STORE),
    ('PerformanceDeltaAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('PerformanceDeltaAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('PerformanceDeltaSynchronizationStore', TRACE_STORE),
    ('PerformanceDeltaInfoStore', INFO_STORE),

    ('SourceCodeStore', TRACE_STORE),
    ('SourceCodeMetadataInfoStore', TRACE_STORE),
    ('SourceCodeAllocationStore', ALLOCATION_STORE),
    ('SourceCodeRelocationStore', RELOCATION_STORE),
    ('SourceCodeAddressStore', ADDRESS_STORE),
    ('SourceCodeAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SourceCodeAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SourceCodeAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SourceCodeSynchronizationStore', TRACE_STORE),
    ('SourceCodeInfoStore', INFO_STORE),

    ('BitmapStore', TRACE_STORE),
    ('BitmapMetadataInfoStore', TRACE_STORE),
    ('BitmapAllocationStore', ALLOCATION_STORE),
    ('BitmapRelocationStore', RELOCATION_STORE),
    ('BitmapAddressStore', ADDRESS_STORE),
    ('BitmapAddressRangeStore', ADDRESS_RANGE_STORE),
    ('BitmapAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('BitmapAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('BitmapSynchronizationStore', TRACE_STORE),
    ('BitmapInfoStore', INFO_STORE),

    ('ImageFileStore', TRACE_STORE),
    ('ImageFileMetadataInfoStore', TRACE_STORE),
    ('ImageFileAllocationStore', ALLOCATION_STORE),
    ('ImageFileRelocationStore', RELOCATION_STORE),
    ('ImageFileAddressStore', ADDRESS_STORE),
    ('ImageFileAddressRangeStore', ADDRESS_RANGE_STORE),
    ('ImageFileAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('ImageFileAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('ImageFileSynchronizationStore', TRACE_STORE),
    ('ImageFileInfoStore', INFO_STORE),

    ('UnicodeStringBufferStore', TRACE_STORE),
    ('UnicodeStringBufferMetadataInfoStore', TRACE_STORE),
    ('UnicodeStringBufferAllocationStore', ALLOCATION_STORE),
    ('UnicodeStringBufferRelocationStore', RELOCATION_STORE),
    ('UnicodeStringBufferAddressStore', ADDRESS_STORE),
    ('UnicodeStringBufferAddressRangeStore', ADDRESS_RANGE_STORE),
    ('UnicodeStringBufferAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('UnicodeStringBufferAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('UnicodeStringBufferSynchronizationStore', TRACE_STORE),
    ('UnicodeStringBufferInfoStore', INFO_STORE),

    ('LineStore', TRACE_STORE),
    ('LineMetadataInfoStore', TRACE_STORE),
    ('LineAllocationStore', ALLOCATION_STORE),
    ('LineRelocationStore', RELOCATION_STORE),
    ('LineAddressStore', ADDRESS_STORE),
    ('LineAddressRangeStore', ADDRESS_RANGE_STORE),
    ('LineAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('LineAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('LineSynchronizationStore', TRACE_STORE),
    ('LineInfoStore', INFO_STORE),

    ('ObjectStore', TRACE_STORE),
    ('ObjectMetadataInfoStore', TRACE_STORE),
    ('ObjectAllocationStore', ALLOCATION_STORE),
    ('ObjectRelocationStore', RELOCATION_STORE),
    ('ObjectAddressStore', ADDRESS_STORE),
    ('ObjectAddressRangeStore', ADDRESS_RANGE_STORE),
    ('ObjectAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('ObjectAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('ObjectSynchronizationStore', TRACE_STORE),
    ('ObjectInfoStore', INFO_STORE),

    ('ModuleLoadEventStore', TRACE_STORE),
    ('ModuleLoadEventMetadataInfoStore', TRACE_STORE),
    ('ModuleLoadEventAllocationStore', ALLOCATION_STORE),
    ('ModuleLoadEventRelocationStore', RELOCATION_STORE),
    ('ModuleLoadEventAddressStore', ADDRESS_STORE),
    ('ModuleLoadEventAddressRangeStore', ADDRESS_RANGE_STORE),
    ('ModuleLoadEventAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('ModuleLoadEventAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('ModuleLoadEventSynchronizationStore', TRACE_STORE),
    ('ModuleLoadEventInfoStore', INFO_STORE),

    ('SymbolTableStore', TRACE_STORE),
    ('SymbolTableMetadataInfoStore', TRACE_STORE),
    ('SymbolTableAllocationStore', ALLOCATION_STORE),
    ('SymbolTableRelocationStore', RELOCATION_STORE),
    ('SymbolTableAddressStore', ADDRESS_STORE),
    ('SymbolTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SymbolTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SymbolTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SymbolTableSynchronizationStore', TRACE_STORE),
    ('SymbolTableInfoStore', INFO_STORE),

    ('SymbolTableEntryStore', TRACE_STORE),
    ('SymbolTableEntryMetadataInfoStore', TRACE_STORE),
    ('SymbolTableEntryAllocationStore', ALLOCATION_STORE),
    ('SymbolTableEntryRelocationStore', RELOCATION_STORE),
    ('SymbolTableEntryAddressStore', ADDRESS_STORE),
    ('SymbolTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SymbolTableEntryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SymbolTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SymbolTableEntrySynchronizationStore', TRACE_STORE),
    ('SymbolTableEntryInfoStore', INFO_STORE),

    ('SymbolModuleInfoStore', TRACE_STORE),
    ('SymbolModuleInfoMetadataInfoStore', TRACE_STORE),
    ('SymbolModuleInfoAllocationStore', ALLOCATION_STORE),
    ('SymbolModuleInfoRelocationStore', RELOCATION_STORE),
    ('SymbolModuleInfoAddressStore', ADDRESS_STORE),
    ('SymbolModuleInfoAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SymbolModuleInfoAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SymbolModuleInfoAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SymbolModuleInfoSynchronizationStore', TRACE_STORE),
    ('SymbolModuleInfoInfoStore', INFO_STORE),

    ('SymbolFileStore', TRACE_STORE),
    ('SymbolFileMetadataInfoStore', TRACE_STORE),
    ('SymbolFileAllocationStore', ALLOCATION_STORE),
    ('SymbolFileRelocationStore', RELOCATION_STORE),
    ('SymbolFileAddressStore', ADDRESS_STORE),
    ('SymbolFileAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SymbolFileAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SymbolFileAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SymbolFileSynchronizationStore', TRACE_STORE),
    ('SymbolFileInfoStore', INFO_STORE),

    ('SymbolInfoStore', TRACE_STORE),
    ('SymbolInfoMetadataInfoStore', TRACE_STORE),
    ('SymbolInfoAllocationStore', ALLOCATION_STORE),
    ('SymbolInfoRelocationStore', RELOCATION_STORE),
    ('SymbolInfoAddressStore', ADDRESS_STORE),
    ('SymbolInfoAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SymbolInfoAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SymbolInfoAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SymbolInfoSynchronizationStore', TRACE_STORE),
    ('SymbolInfoInfoStore', INFO_STORE),

    ('SymbolLineStore', TRACE_STORE),
    ('SymbolLineMetadataInfoStore', TRACE_STORE),
    ('SymbolLineAllocationStore', ALLOCATION_STORE),
    ('SymbolLineRelocationStore', RELOCATION_STORE),
    ('SymbolLineAddressStore', ADDRESS_STORE),
    ('SymbolLineAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SymbolLineAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SymbolLineAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SymbolLineSynchronizationStore', TRACE_STORE),
    ('SymbolLineInfoStore', INFO_STORE),

    ('SymbolTypeStore', TRACE_STORE),
    ('SymbolTypeMetadataInfoStore', TRACE_STORE),
    ('SymbolTypeAllocationStore', ALLOCATION_STORE),
    ('SymbolTypeRelocationStore', RELOCATION_STORE),
    ('SymbolTypeAddressStore', ADDRESS_STORE),
    ('SymbolTypeAddressRangeStore', ADDRESS_RANGE_STORE),
    ('SymbolTypeAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('SymbolTypeAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('SymbolTypeSynchronizationStore', TRACE_STORE),
    ('SymbolTypeInfoStore', INFO_STORE),

    ('StackFrameStore', TRACE_STORE),
    ('StackFrameMetadataInfoStore', TRACE_STORE),
    ('StackFrameAllocationStore', ALLOCATION_STORE),
    ('StackFrameRelocationStore', RELOCATION_STORE),
    ('StackFrameAddressStore', ADDRESS_STORE),
    ('StackFrameAddressRangeStore', ADDRESS_RANGE_STORE),
    ('StackFrameAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('StackFrameAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('StackFrameSynchronizationStore', TRACE_STORE),
    ('StackFrameInfoStore', INFO_STORE),

    ('TypeInfoTableStore', TRACE_STORE),
    ('TypeInfoTableMetadataInfoStore', TRACE_STORE),
    ('TypeInfoTableAllocationStore', ALLOCATION_STORE),
    ('TypeInfoTableRelocationStore', RELOCATION_STORE),
    ('TypeInfoTableAddressStore', ADDRESS_STORE),
    ('TypeInfoTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('TypeInfoTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('TypeInfoTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('TypeInfoTableSynchronizationStore', TRACE_STORE),
    ('TypeInfoTableInfoStore', INFO_STORE),

    ('TypeInfoTableEntryStore', TRACE_STORE),
    ('TypeInfoTableEntryMetadataInfoStore', TRACE_STORE),
    ('TypeInfoTableEntryAllocationStore', ALLOCATION_STORE),
    ('TypeInfoTableEntryRelocationStore', RELOCATION_STORE),
    ('TypeInfoTableEntryAddressStore', ADDRESS_STORE),
    ('TypeInfoTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('TypeInfoTableEntryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('TypeInfoTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('TypeInfoTableEntrySynchronizationStore', TRACE_STORE),
    ('TypeInfoTableEntryInfoStore', INFO_STORE),

    ('TypeInfoStringBufferStore', TRACE_STORE),
    ('TypeInfoStringBufferMetadataInfoStore', TRACE_STORE),
    ('TypeInfoStringBufferAllocationStore', ALLOCATION_STORE),
    ('TypeInfoStringBufferRelocationStore', RELOCATION_STORE),
    ('TypeInfoStringBufferAddressStore', ADDRESS_STORE),
    ('TypeInfoStringBufferAddressRangeStore', ADDRESS_RANGE_STORE),
    ('TypeInfoStringBufferAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('TypeInfoStringBufferAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('TypeInfoStringBufferSynchronizationStore', TRACE_STORE),
    ('TypeInfoStringBufferInfoStore', INFO_STORE),

    ('FunctionTableStore', TRACE_STORE),
    ('FunctionTableMetadataInfoStore', TRACE_STORE),
    ('FunctionTableAllocationStore', ALLOCATION_STORE),
    ('FunctionTableRelocationStore', RELOCATION_STORE),
    ('FunctionTableAddressStore', ADDRESS_STORE),
    ('FunctionTableAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionTableAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('FunctionTableAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('FunctionTableSynchronizationStore', TRACE_STORE),
    ('FunctionTableInfoStore', INFO_STORE),

    ('FunctionTableEntryStore', TRACE_STORE),
    ('FunctionTableEntryMetadataInfoStore', TRACE_STORE),
    ('FunctionTableEntryAllocationStore', ALLOCATION_STORE),
    ('FunctionTableEntryRelocationStore', RELOCATION_STORE),
    ('FunctionTableEntryAddressStore', ADDRESS_STORE),
    ('FunctionTableEntryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionTableEntryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('FunctionTableEntryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('FunctionTableEntrySynchronizationStore', TRACE_STORE),
    ('FunctionTableEntryInfoStore', INFO_STORE),

    ('FunctionAssemblyStore', TRACE_STORE),
    ('FunctionAssemblyMetadataInfoStore', TRACE_STORE),
    ('FunctionAssemblyAllocationStore', ALLOCATION_STORE),
    ('FunctionAssemblyRelocationStore', RELOCATION_STORE),
    ('FunctionAssemblyAddressStore', ADDRESS_STORE),
    ('FunctionAssemblyAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionAssemblyAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('FunctionAssemblyAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('FunctionAssemblySynchronizationStore', TRACE_STORE),
    ('FunctionAssemblyInfoStore', INFO_STORE),

    ('FunctionSourceCodeStore', TRACE_STORE),
    ('FunctionSourceCodeMetadataInfoStore', TRACE_STORE),
    ('FunctionSourceCodeAllocationStore', ALLOCATION_STORE),
    ('FunctionSourceCodeRelocationStore', RELOCATION_STORE),
    ('FunctionSourceCodeAddressStore', ADDRESS_STORE),
    ('FunctionSourceCodeAddressRangeStore', ADDRESS_RANGE_STORE),
    ('FunctionSourceCodeAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('FunctionSourceCodeAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('FunctionSourceCodeSynchronizationStore', TRACE_STORE),
    ('FunctionSourceCodeInfoStore', INFO_STORE),

    ('ExamineSymbolsLineStore', TRACE_STORE),
    ('ExamineSymbolsLineMetadataInfoStore', TRACE_STORE),
    ('ExamineSymbolsLineAllocationStore', ALLOCATION_STORE),
    ('ExamineSymbolsLineRelocationStore', RELOCATION_STORE),
    ('ExamineSymbolsLineAddressStore', ADDRESS_STORE),
    ('ExamineSymbolsLineAddressRangeStore', ADDRESS_RANGE_STORE),
    ('ExamineSymbolsLineAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('ExamineSymbolsLineAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('ExamineSymbolsLineSynchronizationStore', TRACE_STORE),
    ('ExamineSymbolsLineInfoStore', INFO_STORE),

    ('ExamineSymbolsTextStore', TRACE_STORE),
    ('ExamineSymbolsTextMetadataInfoStore', TRACE_STORE),
    ('ExamineSymbolsTextAllocationStore', ALLOCATION_STORE),
    ('ExamineSymbolsTextRelocationStore', RELOCATION_STORE),
    ('ExamineSymbolsTextAddressStore', ADDRESS_STORE),
    ('ExamineSymbolsTextAddressRangeStore', ADDRESS_RANGE_STORE),
    ('ExamineSymbolsTextAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('ExamineSymbolsTextAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('ExamineSymbolsTextSynchronizationStore', TRACE_STORE),
    ('ExamineSymbolsTextInfoStore', INFO_STORE),

    ('ExaminedSymbolStore', TRACE_STORE),
    ('ExaminedSymbolMetadataInfoStore', TRACE_STORE),
    ('ExaminedSymbolAllocationStore', ALLOCATION_STORE),
    ('ExaminedSymbolRelocationStore', RELOCATION_STORE),
    ('ExaminedSymbolAddressStore', ADDRESS_STORE),
    ('ExaminedSymbolAddressRangeStore', ADDRESS_RANGE_STORE),
    ('ExaminedSymbolAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('ExaminedSymbolAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('ExaminedSymbolSynchronizationStore', TRACE_STORE),
    ('ExaminedSymbolInfoStore', INFO_STORE),

    ('ExaminedSymbolSecondaryStore', TRACE_STORE),
    ('ExaminedSymbolSecondaryMetadataInfoStore', TRACE_STORE),
    ('ExaminedSymbolSecondaryAllocationStore', ALLOCATION_STORE),
    ('ExaminedSymbolSecondaryRelocationStore', RELOCATION_STORE),
    ('ExaminedSymbolSecondaryAddressStore', ADDRESS_STORE),
    ('ExaminedSymbolSecondaryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('ExaminedSymbolSecondaryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('ExaminedSymbolSecondaryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('ExaminedSymbolSecondarySynchronizationStore', TRACE_STORE),
    ('ExaminedSymbolSecondaryInfoStore', INFO_STORE),

    ('UnassembleFunctionLineStore', TRACE_STORE),
    ('UnassembleFunctionLineMetadataInfoStore', TRACE_STORE),
    ('UnassembleFunctionLineAllocationStore', ALLOCATION_STORE),
    ('UnassembleFunctionLineRelocationStore', RELOCATION_STORE),
    ('UnassembleFunctionLineAddressStore', ADDRESS_STORE),
    ('UnassembleFunctionLineAddressRangeStore', ADDRESS_RANGE_STORE),
    ('UnassembleFunctionLineAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('UnassembleFunctionLineAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('UnassembleFunctionLineSynchronizationStore', TRACE_STORE),
    ('UnassembleFunctionLineInfoStore', INFO_STORE),

    ('UnassembleFunctionTextStore', TRACE_STORE),
    ('UnassembleFunctionTextMetadataInfoStore', TRACE_STORE),
    ('UnassembleFunctionTextAllocationStore', ALLOCATION_STORE),
    ('UnassembleFunctionTextRelocationStore', RELOCATION_STORE),
    ('UnassembleFunctionTextAddressStore', ADDRESS_STORE),
    ('UnassembleFunctionTextAddressRangeStore', ADDRESS_RANGE_STORE),
    ('UnassembleFunctionTextAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('UnassembleFunctionTextAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('UnassembleFunctionTextSynchronizationStore', TRACE_STORE),
    ('UnassembleFunctionTextInfoStore', INFO_STORE),

    ('UnassembledFunctionStore', TRACE_STORE),
    ('UnassembledFunctionMetadataInfoStore', TRACE_STORE),
    ('UnassembledFunctionAllocationStore', ALLOCATION_STORE),
    ('UnassembledFunctionRelocationStore', RELOCATION_STORE),
    ('UnassembledFunctionAddressStore', ADDRESS_STORE),
    ('UnassembledFunctionAddressRangeStore', ADDRESS_RANGE_STORE),
    ('UnassembledFunctionAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('UnassembledFunctionAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('UnassembledFunctionSynchronizationStore', TRACE_STORE),
    ('UnassembledFunctionInfoStore', INFO_STORE),

    ('UnassembledFunctionSecondaryStore', TRACE_STORE),
    ('UnassembledFunctionSecondaryMetadataInfoStore', TRACE_STORE),
    ('UnassembledFunctionSecondaryAllocationStore', ALLOCATION_STORE),
    ('UnassembledFunctionSecondaryRelocationStore', RELOCATION_STORE),
    ('UnassembledFunctionSecondaryAddressStore', ADDRESS_STORE),
    ('UnassembledFunctionSecondaryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('UnassembledFunctionSecondaryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('UnassembledFunctionSecondaryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('UnassembledFunctionSecondarySynchronizationStore', TRACE_STORE),
    ('UnassembledFunctionSecondaryInfoStore', INFO_STORE),

    ('DisplayTypeLineStore', TRACE_STORE),
    ('DisplayTypeLineMetadataInfoStore', TRACE_STORE),
    ('DisplayTypeLineAllocationStore', ALLOCATION_STORE),
    ('DisplayTypeLineRelocationStore', RELOCATION_STORE),
    ('DisplayTypeLineAddressStore', ADDRESS_STORE),
    ('DisplayTypeLineAddressRangeStore', ADDRESS_RANGE_STORE),
    ('DisplayTypeLineAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('DisplayTypeLineAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('DisplayTypeLineSynchronizationStore', TRACE_STORE),
    ('DisplayTypeLineInfoStore', INFO_STORE),

    ('DisplayTypeTextStore', TRACE_STORE),
    ('DisplayTypeTextMetadataInfoStore', TRACE_STORE),
    ('DisplayTypeTextAllocationStore', ALLOCATION_STORE),
    ('DisplayTypeTextRelocationStore', RELOCATION_STORE),
    ('DisplayTypeTextAddressStore', ADDRESS_STORE),
    ('DisplayTypeTextAddressRangeStore', ADDRESS_RANGE_STORE),
    ('DisplayTypeTextAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('DisplayTypeTextAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('DisplayTypeTextSynchronizationStore', TRACE_STORE),
    ('DisplayTypeTextInfoStore', INFO_STORE),

    ('DisplayedTypeStore', TRACE_STORE),
    ('DisplayedTypeMetadataInfoStore', TRACE_STORE),
    ('DisplayedTypeAllocationStore', ALLOCATION_STORE),
    ('DisplayedTypeRelocationStore', RELOCATION_STORE),
    ('DisplayedTypeAddressStore', ADDRESS_STORE),
    ('DisplayedTypeAddressRangeStore', ADDRESS_RANGE_STORE),
    ('DisplayedTypeAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('DisplayedTypeAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('DisplayedTypeSynchronizationStore', TRACE_STORE),
    ('DisplayedTypeInfoStore', INFO_STORE),

    ('DisplayedTypeSecondaryStore', TRACE_STORE),
    ('DisplayedTypeSecondaryMetadataInfoStore', TRACE_STORE),
    ('DisplayedTypeSecondaryAllocationStore', ALLOCATION_STORE),
    ('DisplayedTypeSecondaryRelocationStore', RELOCATION_STORE),
    ('DisplayedTypeSecondaryAddressStore', ADDRESS_STORE),
    ('DisplayedTypeSecondaryAddressRangeStore', ADDRESS_RANGE_STORE),
    ('DisplayedTypeSecondaryAllocationTimestampStore', ALLOCATION_TIMESTAMP_STORE),
    ('DisplayedTypeSecondaryAllocationTimestampDeltaStore',
     ALLOCATION_TIMESTAMP_DELTA_STORE),
    ('DisplayedTypeSecondarySynchronizationStore', TRACE_STORE),
    ('DisplayedTypeSecondaryInfoStore', INFO_STORE),

    (
        '__Reserved4__',
        ULONGLONG * 256
    ),
]
assert sizeof(TRACE_STORES) == 0x68000, hex(sizeof(TRACE_STORES))

class TRACE_STORE_ARRAY(Structure):
    _fields_ = [
        ('SizeOfStruct', ULONG),
        ('SizeOfAllocation', ULONG),
        ('NumberOfTraceStores', USHORT),
        ('ElementsPerTraceStore', USHORT),
        ('NumberOfFieldRelocationsElements', USHORT),
        ('Padding1', USHORT),
        ('Padding2', ULONG),
        ('Flags', TRACE_FLAGS),
        ('BaseDirectory', UNICODE_STRING),
        ('Rtl', PRTL),
        ('Allocator', PALLOCATOR),
        ('TracerConfig', PTRACER_CONFIG),
        ('RundownListEntry', LIST_ENTRY),
        ('Rundown', PTRACE_STORES_RUNDOWN),
        ('StoresListHead', TRACE_STORE_LIST_ENTRY),

        (
            '__Reserved1__',
            BYTE * 24
        ),

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
        ('EventTraitsExRelocationCompleteEvent', HANDLE),
        ('WsWatchInfoExRelocationCompleteEvent', HANDLE),
        ('WsWorkingSetExInfoRelocationCompleteEvent', HANDLE),
        ('CCallStackTableRelocationCompleteEvent', HANDLE),
        ('CCallStackTableEntryRelocationCompleteEvent', HANDLE),
        ('CModuleTableRelocationCompleteEvent', HANDLE),
        ('CModuleTableEntryRelocationCompleteEvent', HANDLE),
        ('PythonCallStackTableRelocationCompleteEvent', HANDLE),
        ('PythonCallStackTableEntryRelocationCompleteEvent', HANDLE),
        ('PythonModuleTableRelocationCompleteEvent', HANDLE),
        ('PythonModuleTableEntryRelocationCompleteEvent', HANDLE),
        ('LineTableRelocationCompleteEvent', HANDLE),
        ('LineTableEntryRelocationCompleteEvent', HANDLE),
        ('LineStringBufferRelocationCompleteEvent', HANDLE),
        ('CallStackRelocationCompleteEvent', HANDLE),
        ('PerformanceRelocationCompleteEvent', HANDLE),
        ('PerformanceDeltaRelocationCompleteEvent', HANDLE),
        ('SourceCodeRelocationCompleteEvent', HANDLE),
        ('BitmapRelocationCompleteEvent', HANDLE),
        ('ImageFileRelocationCompleteEvent', HANDLE),
        ('UnicodeStringBufferRelocationCompleteEvent', HANDLE),
        ('LineRelocationCompleteEvent', HANDLE),
        ('ObjectRelocationCompleteEvent', HANDLE),
        ('ModuleLoadEventRelocationCompleteEvent', HANDLE),
        ('SymbolTableRelocationCompleteEvent', HANDLE),
        ('SymbolTableEntryRelocationCompleteEvent', HANDLE),
        ('SymbolModuleInfoRelocationCompleteEvent', HANDLE),
        ('SymbolFileRelocationCompleteEvent', HANDLE),
        ('SymbolInfoRelocationCompleteEvent', HANDLE),
        ('SymbolLineRelocationCompleteEvent', HANDLE),
        ('SymbolTypeRelocationCompleteEvent', HANDLE),
        ('StackFrameRelocationCompleteEvent', HANDLE),

        (
            '__Reserved2__',
            BYTE * (512 - 128 - (sizeof(HANDLE) * NUM_TRACE_STORES))
        ),

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
        ('EventTraitsExReloc', TRACE_STORE_RELOC),
        ('WsWatchInfoExReloc', TRACE_STORE_RELOC),
        ('WsWorkingSetExInfoReloc', TRACE_STORE_RELOC),
        ('CCallStackTableReloc', TRACE_STORE_RELOC),
        ('CCallStackTableEntryReloc', TRACE_STORE_RELOC),
        ('CModuleTableReloc', TRACE_STORE_RELOC),
        ('CModuleTableEntryReloc', TRACE_STORE_RELOC),
        ('PythonCallStackTableReloc', TRACE_STORE_RELOC),
        ('PythonCallStackTableEntryReloc', TRACE_STORE_RELOC),
        ('PythonModuleTableReloc', TRACE_STORE_RELOC),
        ('PythonModuleTableEntryReloc', TRACE_STORE_RELOC),
        ('LineTableReloc', TRACE_STORE_RELOC),
        ('LineTableEntryReloc', TRACE_STORE_RELOC),
        ('LineStringBufferReloc', TRACE_STORE_RELOC),
        ('CallStackReloc', TRACE_STORE_RELOC),
        ('PerformanceReloc', TRACE_STORE_RELOC),
        ('PerformanceDeltaReloc', TRACE_STORE_RELOC),
        ('SourceCodeReloc', TRACE_STORE_RELOC),
        ('BitmapReloc', TRACE_STORE_RELOC),
        ('ImageFileReloc', TRACE_STORE_RELOC),
        ('UnicodeStringBufferReloc', TRACE_STORE_RELOC),
        ('LineReloc', TRACE_STORE_RELOC),
        ('ObjectReloc', TRACE_STORE_RELOC),
        ('ModuleLoadEventReloc', TRACE_STORE_RELOC),
        ('SymbolTableReloc', TRACE_STORE_RELOC),
        ('SymbolTableEntryReloc', TRACE_STORE_RELOC),
        ('SymbolModuleInfoReloc', TRACE_STORE_RELOC),
        ('SymbolFileReloc', TRACE_STORE_RELOC),
        ('SymbolInfoReloc', TRACE_STORE_RELOC),
        ('SymbolLineReloc', TRACE_STORE_RELOC),
        ('SymbolTypeReloc', TRACE_STORE_RELOC),
        ('StackFrameReloc', TRACE_STORE_RELOC),

        (
            '__Reserved3__',
            BYTE * (4096 - 512 - (sizeof(TRACE_STORE_RELOC) * NUM_TRACE_STORES))
        ),

        (
            'TraceStores',
            TRACE_STORE * (MAX_TRACE_STORE_IDS * ELEMENTS_PER_TRACE_STORE)
        ),

        (
            '__Reserved4__',
            ULONGLONG * 256
        ),
    ]

    def _indexes(self):

        assert self.ElementsPerTraceStore == ELEMENTS_PER_TRACE_STORE, \
              (self.ElementsPerTraceStore, ELEMENTS_PER_TRACE_STORE)

        num_metadata_stores = NUMBER_OF_METADATA_STORES

        return [
            (index, [ index+i for i in range(1, num_metadata_stores+1) ])
                for index in range(
                    0,
                    MAX_TRACE_STORE_INDEX,
                    self.ElementsPerTraceStore
                )
        ]

    def _stores(self):
        results = []
        base = self.TraceStores
        for (index, store_indexes) in self._indexes():
            metadata = []
            for store_index in store_indexes:
                store = base[store_index]
                store_id = store.TraceStoreMetadataId
                name = '    :%s' % TraceStoreMetadataIdToName[store_id]
                metadata.append((name, store))

            store = base[index]
            store_id = store.TraceStoreId
            name = TraceStoreIdToName[store.TraceStoreId]

            pair = (name, store)
            result = (pair, metadata)
            results.append(result)
        return results
assert sizeof(TRACE_STORE_ARRAY) == sizeof(TRACE_STORES), (
    sizeof(TRACE_STORE_ARRAY),
    sizeof(TRACE_STORES),
)

PTRACE_STORE_ARRAY = POINTER(TRACE_STORE_ARRAY)

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
    PTRACER_CONFIG,
    PWSTR,
    PTRACE_STORES,
    PULONG,
    PTRACE_FLAGS,
)

INITIALIZE_TRACE_CONTEXT = CFUNCTYPE(
    BOOL,
    PRTL,
    PALLOCATOR,
    PTRACER_CONFIG,
    PTRACE_CONTEXT,
    PULONG,
    PTRACE_SESSION,
    PTRACE_STORES,
    PTP_CALLBACK_ENVIRON,
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
            (1, 'TracerConfig'),
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
            (1, 'TracerConfig'),
            (1, 'TraceContext'),
            (1, 'SizeOfTraceContext'),
            (1, 'TraceSession'),
            (1, 'TraceStores'),
            (1, 'ThreadpoolCallbackEnvironment'),
            (1, 'CancellationThreadpoolCallbackEnvironment'),
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
        cast(0, PTRACER_CONFIG),
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
        byref(tracer_config),
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
        tracer_config,
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

    cancel_tp_callback_env = TP_CALLBACK_ENVIRON()
    InitializeThreadpoolEnvironment(cancel_tp_callback_env)
    SetThreadpoolCallbackPool(cancel_tp_callback_env, threadpool)

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
        byref(tracer_config),
        byref(trace_context),
        byref(size),
        cast(0, PTRACE_SESSION),
        byref(trace_stores),
        byref(tp_callback_env),
        byref(cancel_tp_callback_env),
        byref(trace_context_flags),
        cast(0, PVOID),
    )

    assert success

    return trace_context

def is_readonly_trace_context_ready(readonly_trace_context):
    return is_signaled(readonly_trace_context.LoadingCompleteEvent)

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
