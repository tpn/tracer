#===============================================================================
# Imports
#===============================================================================

from ..wintypes import (
    cast,
    byref,
    create_string_buffer,

    Structure,

    BOOL,
    BYTE,
    CDLL,
    ULONG,
    PVOID,
    PULONG,
    HANDLE,
    HMODULE,
    POINTER,
    CFUNCTYPE,
)

#===============================================================================
# Structures
#===============================================================================

class RTL(Structure):
    _fields_ = [
        ('Size', ULONG),
        ('NtdllModule', HMODULE),
        ('Kernel32Module', HMODULE),
        ('NtosKrnlModule', HMODULE),
        ('ShlwapiModule', HMODULE),
        ('__C_specific_handler', PVOID),
        ('__security_init_cookie', PVOID),
        ('HeapHandle', HANDLE),
    ]
PRTL = POINTER(RTL)
PPRTL = POINTER(PRTL)

class RTL_FILE(Structure):
    _fields_ = [
        ('Reserved', BYTE * 512),
    ]
PRTL_FILE = POINTER(RTL_FILE)

#

from ctypes import *
from ..wintypes import *

class _LINKED_STRING_INNER_UNION_1(Union):
    pass

class _LINKED_STRING_INNER_STRUCT_1(Structure):
    _anonymous_ = ('u1',)

class _LINKED_STRING_INNER_UNION_2(Union):
    _anonymous_ = ('s1',)

class LINKED_STRING(Structure):
    _anonymous_ = ('u1',)
PLINKED_STRING = POINTER(LINKED_STRING)

_LINKED_STRING_INNER_UNION_1._fields_ = [
    ('Buffer', PCHAR),
    ('WideBuffer', PWCHAR),
]

_LINKED_STRING_INNER_STRUCT_1._fields_ = [
    ('Length', USHORT),
    ('MaximumLength', USHORT),
    ('Hash', LONG),
    ('u1', _LINKED_STRING_INNER_UNION_1),
]

_LINKED_STRING_INNER_UNION_2._fields_ = [
    ('s1', _LINKED_STRING_INNER_STRUCT_1),
    ('String', STRING),
    ('Unicode', UNICODE_STRING),
]

LINKED_STRING._fields_ = [
    ('ListEntry', LIST_ENTRY),
    ('u1', _LINKED_STRING_INNER_UNION_2),
]

#===============================================================================
# Functions
#===============================================================================

INITIALIZE_RTL = CFUNCTYPE(BOOL, PRTL, PULONG)

#===============================================================================
# Binding
#===============================================================================

Rtl = None
RtlDll = None
InitializeRtl = None

def bind(path=None, dll=None):
    global RtlDll
    global InitializeRtl

    assert path or dll
    if not dll:
        dll = RtlDll
        if not dll:
            dll = CDLL(path)
            RtlDll = dll

    InitializeRtl = INITIALIZE_RTL(
        ('InitializeRtl', dll),
        (
            (1, 'Rtl'),
            (1, 'SizeOfRtl'),
        ),
    )

#===============================================================================
# Python Functions
#===============================================================================

def create_and_initialize_rtl():
    global Rtl
    if Rtl:
        return Rtl

    size = ULONG()

    null = cast(0, PRTL)
    success = InitializeRtl(null, byref(size))
    assert size.value > 0

    buf = create_string_buffer(size.value)
    prtl = cast(buf, PRTL)
    success = InitializeRtl(prtl, byref(size))
    assert success

    Rtl = cast(buf, PRTL).contents
    return Rtl

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
