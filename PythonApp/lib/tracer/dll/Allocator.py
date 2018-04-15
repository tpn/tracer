#===============================================================================
# Imports
#===============================================================================

from ..wintypes import (
    Union,
    Structure,

    BOOL,
    ULONG,
    PVOID,
    SIZE_T,
    HANDLE,
    POINTER,
    CFUNCTYPE,
)

#===============================================================================
# Function Prototypes
#===============================================================================

MALLOC = CFUNCTYPE(PVOID, PVOID, SIZE_T)
PMALLOC = POINTER(MALLOC)

CALLOC = CFUNCTYPE(PVOID, PVOID, SIZE_T, SIZE_T)
PCALLOC = POINTER(CALLOC)

REALLOC = CFUNCTYPE(PVOID, PVOID, PVOID, SIZE_T)
PREALLOC = POINTER(REALLOC)

FREE = CFUNCTYPE(None, PVOID, PVOID)
PFREE = POINTER(FREE)

FREE_POINTER = CFUNCTYPE(None, PVOID, PVOID)
PFREE_POINTER = POINTER(FREE_POINTER)

class ALLOCATOR(Structure):
    pass
PALLOCATOR = POINTER(ALLOCATOR)
PPALLOCATOR = POINTER(PALLOCATOR)

INITIALIZE_ALLOCATOR = CFUNCTYPE(BOOL, PALLOCATOR)
PINITIALIZE_ALLOCATOR = POINTER(INITIALIZE_ALLOCATOR)

DESTROY_ALLOCATOR = CFUNCTYPE(None, PALLOCATOR)
PDESTROY_ALLOCATOR = POINTER(DESTROY_ALLOCATOR)

class ALLOCATOR_FLAGS(Structure):
    _fields_ = [
        ('IsTlsAware', ULONG, 1),
        ('IsTlsRedirectionEnabled', ULONG, 1),
    ]
PALLOCATOR_FLAGS = POINTER(ALLOCATOR_FLAGS)

class _ALLOCATOR_CONTEXT1(Union):
    _fields_ = [
        ('Context', PVOID),
        ('Allocator', PALLOCATOR),
    ]

class _ALLOCATOR_CONTEXT2(Union):
    _fields_ = [
        ('Context2', PVOID),
        ('HeapHandle', HANDLE),
    ]

ALLOCATOR._fields_ = [
    ('Context', _ALLOCATOR_CONTEXT1),
    ('Malloc', PMALLOC),
    ('Calloc', PCALLOC),
    ('Realloc', PREALLOC),
    ('Free', PFREE),
    ('FreePointer', PFREE_POINTER),
    ('Initialize', PINITIALIZE_ALLOCATOR),
    ('Destroy', PDESTROY_ALLOCATOR),
    ('WriteBytes', PVOID),
    ('Context2', _ALLOCATOR_CONTEXT2),
    ('Parent', PALLOCATOR),
    ('TryMalloc', PVOID),
    ('TryCalloc', PVOID),
    ('MallocWithTimestamp', PVOID),
    ('CallocWithTimestamp', PVOID),
    ('TryMallocWithTimestamp', PVOID),
    ('TryCallocWithTimestamp', PVOID),
    ('AlignedMalloc', PVOID),
    ('TryAlignedMalloc', PVOID),
    ('AlignedMallocWithTimestamp', PVOID),
    ('TryAlignedMallocWithTimestamp', PVOID),
    ('AlignedOffsetMalloc', PVOID),
    ('TryAlignedOffsetMalloc', PVOID),
    ('AlignedOffsetMallocWithTimestamp', PVOID),
    ('TryAlignedOffsetMallocWithTimestamp', PVOID),
    ('AlignedCalloc', PVOID),
    ('TryAlignedCalloc', PVOID),
    ('AlignedCallocWithTimestamp', PVOID),
    ('TryAlignedCallocWithTimestamp', PVOID),
    ('AlignedOffsetCalloc', PVOID),
    ('TryAlignedOffsetCalloc', PVOID),
    ('AlignedOffsetCallocWithTimestamp', PVOID),
    ('TryAlignedOffsetCallocWithTimestamp', PVOID),
    ('AlignedFree', PVOID),
    ('AlignedFreePointer', PVOID),
    ('ThreadId', ULONG),
    ('TlsIndex', ULONG),
    ('Flags', ALLOCATOR_FLAGS),
    ('NumberOfThreads', ULONG),
]

GET_OR_CREATE_GLOBAL_ALLOCATOR = CFUNCTYPE(BOOL, PPALLOCATOR)


# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
