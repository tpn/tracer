#===============================================================================
# Imports
#===============================================================================

from ctypes import (
    Structure,

    POINTER,
    CFUNCTYPE,
)

from ..wintypes import (
    SHORT,
    USHORT,
    LONG,
    ULONG,
    ULONGLONG,
)

from .Python import (
    PPYTHON_FUNCTION,
)

#===============================================================================
# Classes
#===============================================================================

class PYTHON_TRACE_EVENT(Structure):
    pass
PPYTHON_TRACE_EVENT = POINTER(PYTHON_TRACE_EVENT)

PYTHON_TRACE_EVENT._fields_ = [
    ('Timestamp', ULONGLONG),
    ('Function', PPYTHON_FUNCTION),

    ('WorkingSetSize', ULONGLONG),
    ('PageFaultCount', ULONGLONG),
    ('CommittedSize', ULONGLONG),

    ('ReadTransferCount', ULONGLONG),
    ('WriteTransferCount', ULONGLONG),

    ('HandleCount', ULONG),
    ('ThreadId', ULONG),
    ('ElapsedMicroseconds', ULONG),

    ('WorkingSetDelta', LONG),
    ('CommittedDelta', LONG),
    ('ReadTransferDelta', ULONG),
    ('WriteTransferDelta', ULONG),

    ('CodeObjectHash', ULONG),
    ('FunctionHash', ULONG),

    ('PathHash', LONG),
    ('FullNameHash', LONG),
    ('ModuleNameHash', LONG),
    ('ClassNameHash', LONG),
    ('NameHash', LONG),

    ('Flags', ULONG),

    ('HandleDelta', SHORT),
    ('PageFaultDelta', USHORT),

    ('LineNumber', USHORT),
    ('FirstLineNumber', USHORT),
    ('NumberOfLines', USHORT),
    ('NumberOfCodeLines', USHORT),
]

#===============================================================================
# Functions
#===============================================================================


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
