#===============================================================================
# Imports
#===============================================================================

from ..wintypes import (
    cast,
    byref,
    sizeof,
    errcheck,
    create_threadpool,
    create_string_buffer,
    create_threadpool_callback_environment,

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
    FILETIME,
    PTP_WORK,
    CFUNCTYPE,
    ULONGLONG,
    SYSTEMTIME,
    LIST_ENTRY,
    SLIST_HEADER,
    LARGE_INTEGER,
    ULARGE_INTEGER,
    UNICODE_STRING,
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

#===============================================================================
# Globals
#===============================================================================
TRACE_STORE_ID = ULONG
TRACE_STORE_INDEX = ULONG
PTIMER_FUNCTION = PVOID

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
        ('Unused', ULONG, 29),
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

class ADDRESS_BIT_COUNTS(Structure):
    _fields_ = [
        ('RightShift', ULONG, 6),
        ('PopulationCount', ULONG, 6),
        ('LeadingZeros', ULONG, 5),
        ('TrailingZeros', ULONG, 5),
    ]
PADDRESS_BIT_COUNTS = POINTER(ADDRESS_BIT_COUNTS)

class _TRACE_STORE_ADDRESS_RANGE_BITCOUNTS_INNER(Structure):
    _fields_ = [
        ('Preferred', ADDRESS_BIT_COUNTS),
        ('Actual', ADDRESS_BIT_COUNTS),
        ('End', ADDRESS_BIT_COUNTS),
    ]

class TRACE_STORE_ADDRESS_RANGE(Structure):
    _fields_ = [
        ('PreferredBaseAddress', PVOID),
        ('ActualBaseAddress', PVOID),
        ('NumberOfMaps', ULONG),
        ('BitCounts', _TRACE_STORE_ADDRESS_RANGE_BITCOUNTS_INNER)
    ]
PTRACE_STORE_ADDRESS_RANGE = POINTER(TRACE_STORE_ADDRESS_RANGE)

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

class TRACE_STORE_RELOC(Structure):
    _fields_ = [
        ('SizeOfStruct', USHORT),
        ('NumberOfRelocations', USHORT),
        ('Unused1', ULONG),
        ('Relocations', PTRACE_STORE_FIELD_RELOC),
    ]
PTRACE_STORE_RELOC = POINTER(TRACE_STORE_RELOC)

class TRACE_STORE_BITMAP(Structure):
    _fields_ = [
        ('SizeOfBitMap', ULONG),
        ('Granularity', USHORT),
        ('Shift', USHORT),
        ('Buffer', PVOID),
    ]
PTRACE_STORE_BITMAP = TRACE_STORE_BITMAP

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

class TRACE_STORE_MEMORY_MAP(Structure):
    pass
PTRACE_STORE_MEMORY_MAP = POINTER(TRACE_STORE_MEMORY_MAP)

class TRACE_STORES(Structure):
    pass
PTRACE_STORES = POINTER(TRACE_STORES)

class TRACE_CONTEXT(Structure):
    _fields_ = [
        ('SizeOfStruct', USHORT),
        ('Flags', TRACE_CONTEXT_FLAGS),
        ('FailedCount', ULONG),
        ('Rtl', PRTL),
        ('Allocator', PALLOCATOR),
        ('TraceSession', PTRACE_SESSION),
        ('TraceStores', PTRACE_STORES),
        ('TimerFunction', PVOID),
        ('UserData', PVOID),
        ('ThreadpoolCallbackEnvironment', PTP_CALLBACK_ENVIRON),
        ('LoadingCompleteEvent', HANDLE),
        ('ThreadpoolCleanupGroup', PTP_CLEANUP_GROUP),
        ('BindMetadataInfoStoreWork', TRACE_STORE_WORK),
        ('BindRemainingMetadataStoresWork', TRACE_STORE_WORK),
        ('BindTraceStoreWork', TRACE_STORE_WORK),
        ('FailedListHead', SLIST_HEADER),
        ('ActiveWorkItems', ULONG),
        ('BindsInProgress', ULONG),
        ('Time', TRACE_STORE_TIME),
    ]
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

class TRACE_STORE(Structure):
    pass
PTRACE_STORE = POINTER(TRACE_STORE)

class TRACE_STORES_RUNDOWN(Structure):
    pass
PTRACE_STORES_RUNDOWN = POINTER(TRACE_STORES_RUNDOWN)

TRACE_STORES._fields_ = [
    ('SizeOfStruct', USHORT),
    ('SizeOfAllocation', USHORT),
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
]

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
InitializeReadonlyTraceStores = None
InitializeReadonlyTraceContext = None
UpdateTracerConfigWithTraceStoreInfo = None

def bind(path=None, dll=None):
    global TraceStoreDll
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

    assert size.value > 0

    buf = create_string_buffer(size.value)
    ptrace_stores = cast(buf, PTRACE_STORES)
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

    ts = ptrace_stores.contents
    return ts

def create_and_initialize_readonly_trace_context(rtl, allocator,
                                                 trace_stores,
                                                 trace_context_flags,
                                                 num_cpus=None,
                                                 threadpool=None,
                                                 tp_callback_env=None):
    if not threadpool:
        threadpool = create_threadpool(num_cpus=num_cpus)

    if not tp_callback_env:
        tp_callback_env = TP_CALLBACK_ENVIRON()
        InitializeThreadpoolEnvironment(tp_callback_env)
        SetThreadpoolCallbackPool(tp_callback_env, threadpool)

    if not trace_context_flags:
        trace_context_flags = TRACE_CONTEXT_FLAGS()

    trace_context = TRACE_CONTEXT()
    size = ULONG(sizeof(TRACE_CONTEXT))

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

    return trace_context.contents

def is_readonly_trace_context_ready(readonly_trace_context):
    return is_signaled(readonly_trace_context.LoadingCompleteEvent)

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
