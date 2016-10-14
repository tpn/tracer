#===============================================================================
# Imports
#===============================================================================

from ..wintypes import (
    cast,
    byref,
    create_string_buffer,

    Structure,

    BOOL,
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
    prtl = cast(byref(buf), PRTL)
    success = InitializeRtl(prtl, byref(size))
    assert success

    Rtl = cast(byref(buf), PRTL).contents
    return Rtl

# vim:set ts=8 sw=4 sts=4 tw=80 ai et                                          :
