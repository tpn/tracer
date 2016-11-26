#===============================================================================
# Imports
#===============================================================================

from ..wintypes import (
    Structure,

    SHORT,
    USHORT,
    LONG,
    ULONG,
    POINTER,
    CFUNCTYPE,
    ULONGLONG,
)

from .Python import (
    PPYTHON_FUNCTION,
)

#===============================================================================
# Classes
#===============================================================================

class PYTHON_TRACE_EVENT1(Structure):
    pass
PPYTHON_TRACE_EVENT1 = POINTER(PYTHON_TRACE_EVENT1)

PYTHON_TRACE_EVENT1._fields_ = [
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

class PYTHON_TRACE_EVENT2(Structure):
    _fields_ = [
        ('Function', PPYTHON_FUNCTION),
    ]

    __array_interface__ = {
        'version': 3,
        'typestr': '<u8',
        'descr': [
            ('Function', '<u8'),
        ],
    }
PPYTHON_TRACE_EVENT2 = POINTER(PYTHON_TRACE_EVENT2)

class PYTHON_EVENT_TRAITS_EX(Structure):
    _fields_ = [
        ('IsCall', ULONG, 1),
        ('IsException', ULONG, 1),
        ('IsLine', ULONG, 1),
        ('IsReturn', ULONG, 1),
        ('IsC', ULONG, 1),
        ('AsEventType', ULONG, 3),
        ('IsReverseJump', ULONG, 1),
        ('LineNumberOrCallStackDepth', ULONG, 23),
    ]

    __array_interface__ = {
        'version': 3,
        'typestr': '<u4',
        'descr': [
            ('IsCall', '|t1'),
            ('IsException', '|t1'),
            ('IsLine', '|t1'),
            ('IsReturn', '|t1'),
            ('IsC', '|t1'),
            ('AsEventType', '|t3'),
            ('IsReverseJump', '|t1'),
            ('LineNumberOrCallStackDepth', '|t23'),
        ],
    }
PPYTHON_EVENT_TRAITS_EX = POINTER(PYTHON_EVENT_TRAITS_EX)


#===============================================================================
# Functions
#===============================================================================


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
