#===============================================================================
# Imports
#===============================================================================

from ..util import (
    Constant,
)

from ..wintypes import (
    sizeof,

    Structure,
    Union,

    BOOL,
    SHORT,
    USHORT,
    PUSHORT,
    LONG,
    ULONG,
    PULONG,
    PVOID,
    STRING,
    PSTRING,
    POINTER,
    CFUNCTYPE,

    HMODULE,
    ULONGLONG,
    ULONG_PTR,

    LIST_ENTRY,
    RTL_SPLAY_LINKS,
    RTL_GENERIC_TABLE,
)

from Rtl import (
    PRTL,
)

#===============================================================================
# Enums/Constants
#===============================================================================

class PythonPathEntryType(Constant):
    IsModuleDirectory       =       1
    IsNonModuleDirectory    = (1 << 1)
    IsFile                  = (1 << 2)
    IsClass                 = (1 << 3)
    IsFunction              = (1 << 4)
    IsSpecial               = (1 << 5)
    IsValid                 = (1 << 6)

class PYTHON_PATH_ENTRY_TYPE(Structure):
    _fields_ = [
        ('IsModuleDirectory', ULONG, 1),
        ('IsNonModuleDirectory', ULONG, 1),
        ('IsFile', ULONG, 1),
        ('IsClass', ULONG, 1),
        ('IsFunction', ULONG, 1),
        ('IsSpecial', ULONG, 1),
        ('IsValid', ULONG, 1),
        ('IsDll', ULONG, 1),
        ('IsC', ULONG, 1),
        ('IsBuiltin', ULONG, 1),
    ]

#===============================================================================
# Classes
#===============================================================================

class PYTHON_PATH_TABLE_ENTRY(Structure):
    @classmethod
    def _get_dtype(cls):
        import numpy as np
        return np.dtype([
            # PATH_TABLE_ENTRY
            # PREFIX_TABLE_ENTRY
            ('PrefixTreeNodeTypeCode', np.int16),       #   2       40      42
            ('PrefixTreeNameLength', np.int16),         #   2       42      44
            ('PathEntryType', np.uint32),               #   4       44      48
            ('NextPrefixTree', np.uint64),              #   8       48      56
            # RTL_SPLAY_LINKS
            ('SplayParent', np.uint64),                 #   8       16      24
            ('SplayLeftChild', np.uint64),              #   8       24      32
            ('SplayRightChild', np.uint64),             #   8       32      40
            ('Prefix', np.uint64),                      #   8       40      48

            ('PathLength', np.uint16),
            ('PathMaximumLength', np.uint16),
            ('PathHash', np.uint32),
            ('PathBuffer', np.uint64),

            ('FullNameLength', np.uint16),
            ('FullNameMaximumLength', np.uint16),
            ('FullNameHash', np.uint32),
            ('FullNameBuffer', np.uint64),

            ('ModuleNameLength', np.uint16),
            ('ModuleNameMaximumLength', np.uint16),
            ('ModuleNameHash', np.uint32),
            ('ModuleNameBuffer', np.uint64),

            ('NameLength', np.uint16),
            ('NameMaximumLength', np.uint16),
            ('NameHash', np.uint32),
            ('NameBuffer', np.uint64),

            ('ClassNameLength', np.uint16),
            ('ClassNameMaximumLength', np.uint16),
            ('ClassNameHash', np.uint32),
            ('ClassNameBuffer', np.uint64),
        ])

PPYTHON_PATH_TABLE_ENTRY = POINTER(PYTHON_PATH_TABLE_ENTRY)

class PYTHON_PATH_TABLE(Structure):
    pass
PPYTHON_PATH_TABLE = POINTER(PYTHON_PATH_TABLE)

class _PYTHON_PATH_TABLE_ENTRY(Structure):
    pass

class PYTHON_PATH_TABLE_ENTRY(Structure):
    pass
PPYTHON_PATH_TABLE_ENTRY = POINTER(PYTHON_PATH_TABLE_ENTRY)

class PYTHON_FUNCTION(Structure):

    _exclude = (
        'Unused',
        'CodeObject',
        'CodeLineNumbers',
        'Histogram',
        'LineNumbersBitmap',
    )

    @classmethod
    def _get_dtype(cls):
        import numpy as np
        return np.dtype([
            # PATH_TABLE_ENTRY
            # PREFIX_TABLE_ENTRY
            ('PrefixTreeNodeTypeCode', np.int16),       #   2       40      42
            ('PrefixTreeNameLength', np.int16),         #   2       42      44
            ('PathEntryType', np.uint32),               #   4       44      48
            ('NextPrefixTree', np.uint64),              #   8       48      56
            # RTL_SPLAY_LINKS
            ('SplayParent', np.uint64),                 #   8       16      24
            ('SplayLeftChild', np.uint64),              #   8       24      32
            ('SplayRightChild', np.uint64),             #   8       32      40
            ('Prefix', np.uint64),                      #   8       40      48

            ('PathLength', np.uint16),
            ('PathMaximumLength', np.uint16),
            ('PathHash', np.int32),
            ('PathBuffer', np.uint64),

            ('FullNameLength', np.uint16),
            ('FullNameMaximumLength', np.uint16),
            ('FullNameHash', np.int32),
            ('FullNameBuffer', np.uint64),

            ('ModuleNameLength', np.uint16),
            ('ModuleNameMaximumLength', np.uint16),
            ('ModuleNameHash', np.int32),
            ('ModuleNameBuffer', np.uint64),

            ('NameLength', np.uint16),
            ('NameMaximumLength', np.uint16),
            ('NameHash', np.int32),
            ('NameBuffer', np.uint64),

            ('ClassNameLength', np.uint16),
            ('ClassNameMaximumLength', np.uint16),
            ('ClassNameHash', np.int32),
            ('ClassNameBuffer', np.uint64),
            # End of PYTHON_PATH_TABLE_ENTRY

            ('ParentPathEntry', np.uint64),
            ('CodeObject', np.uint64),
            ('LineNumbersBitmap', np.uint64),
            ('Histogram', np.uint64),
            ('CodeLineNumbers', np.uint64),

            ('ReferenceCount', np.uint32),
            ('CodeObjectHash', np.uint32),
            ('FunctionHash', np.uint32),
            ('Unused1', np.uint32),

            ('FirstLineNumber', np.uint16),
            ('NumberOfLines', np.uint16),
            ('NumberOfCodeLines', np.uint16),
            ('SizeOfByteCode', np.uint16),

            ('Unused2', np.uint64),
            ('Unused3', np.uint64),
            ('Unused4', np.uint64),
        ])

    @property
    def is_valid(self):
        return self.PathEntry.PrefixTableEntry.PathEntryType.IsValid

PPYTHON_FUNCTION = POINTER(PYTHON_FUNCTION)

class PYTHON_FUNCTION_TABLE_ENTRY(Structure):
    _exclude = (
        'Unused',
        'NextPrefixTree',
    )
    @classmethod
    def _get_dtype(cls):
        import numpy as np
        return np.dtype([
            # TABLE_ENTRY_HEADER
            ('HeaderSplayParent', np.uint64),           #   8       0       8
            ('HeaderSplayLeftChild', np.uint64),        #   8       8       16
            ('HeaderSplayRightChild', np.uint64),       #   8       16      24
            ('HeaderFlink', np.uint64),                 #   8       24      32
            ('HeaderBlink', np.uint64),                 #   8       32      40
            # PATH_TABLE_ENTRY
            # PREFIX_TABLE_ENTRY
            ('PrefixTreeNodeTypeCode', np.int16),       #   2       40      42
            ('PrefixTreeNameLength', np.int16),         #   2       42      44
            ('PathEntryType', np.uint32),               #   4       44      48
            ('NextPrefixTree', np.uint64),              #   8       48      56
            # RTL_SPLAY_LINKS
            ('SplayParent', np.uint64),                 #   8       16      24
            ('SplayLeftChild', np.uint64),              #   8       24      32
            ('SplayRightChild', np.uint64),             #   8       32      40
            ('Prefix', np.uint64),                      #   8       40      48

            ('PathLength', np.uint16),
            ('PathMaximumLength', np.uint16),
            ('PathHash', np.int32),
            ('PathBuffer', np.uint64),

            ('FullNameLength', np.uint16),
            ('FullNameMaximumLength', np.uint16),
            ('FullNameHash', np.int32),
            ('FullNameBuffer', np.uint64),

            ('ModuleNameLength', np.uint16),
            ('ModuleNameMaximumLength', np.uint16),
            ('ModuleNameHash', np.int32),
            ('ModuleNameBuffer', np.uint64),

            ('NameLength', np.uint16),
            ('NameMaximumLength', np.uint16),
            ('NameHash', np.int32),
            ('NameBuffer', np.uint64),

            ('ClassNameLength', np.uint16),
            ('ClassNameMaximumLength', np.uint16),
            ('ClassNameHash', np.int32),
            ('ClassNameBuffer', np.uint64),
            # End of PYTHON_PATH_TABLE_ENTRY

            ('ParentPathEntry', np.uint64),
            ('CodeObject', np.uint64),
            ('LineNumbersBitmap', np.uint64),
            ('Histogram', np.uint64),
            ('CodeLineNumbers', np.uint64),

            ('ReferenceCount', np.uint32),
            ('CodeObjectHash', np.int32),
            ('FunctionHash', np.uint32),
            ('Unused1', np.uint32),

            ('FirstLineNumber', np.uint16),
            ('NumberOfLines', np.uint16),
            ('NumberOfCodeLines', np.uint16),
            ('SizeOfByteCode', np.uint16),

            ('Unused2', np.uint64),
            ('Unused3', np.uint64),
            ('Unused4', np.uint64),
        ])

    @property
    def is_valid(self):
        return self.Function.PathEntry.PrefixTableEntry.PathEntryType.IsValid

PPYTHON_FUNCTION_TABLE_ENTRY = POINTER(PYTHON_FUNCTION_TABLE_ENTRY)

class PYTHON_FUNCTION_TABLE(Structure):
    _fields_ = [
        ('GenericTable', RTL_GENERIC_TABLE),
    ]
PPYTHON_FUNCTION_TABLE = POINTER(PYTHON_FUNCTION_TABLE)

class PYTHON_TRACE_EVENT(Structure):
    pass
PPYTHON_TRACE_EVENT = POINTER(PYTHON_TRACE_EVENT)

_PYTHON_PATH_TABLE_ENTRY._fields_ = [
    ('NodeTypeCode', SHORT),
    ('PrefixNameLength', SHORT),
    ('PathEntryType', PYTHON_PATH_ENTRY_TYPE),
    ('NextPrefixTree', PPYTHON_PATH_TABLE_ENTRY),
    ('Links', RTL_SPLAY_LINKS),
    ('Prefix', PSTRING),
]

PYTHON_PATH_TABLE_ENTRY._fields_ = [
    ('PrefixTableEntry', _PYTHON_PATH_TABLE_ENTRY),
    ('Path', STRING),
    ('FullName', STRING),
    ('ModuleName', STRING),
    ('Name', STRING),
    ('ClassName', STRING),
]

class _PYTHON_FUNCTION_CODEOBJECT_INNER(Structure):
    _fields_ = [
        ('CodeLineNumbers', PUSHORT),
        ('FirstLineNumber', ULONG),
        ('NumberOfLines', ULONG),
        ('NumberOfCodeLines', ULONG),
        ('SizeOfByteCode', ULONG),
    ]

class _PYTHON_FUNCTION_PYCFUNCTION_INNER(Structure):
    _fields_ = [
        ('ModuleHandle', HMODULE),
    ]

class _PYTHON_FUNCTION_INNER_UNION(Union):
    _fields_ = [
        ('PythonCodeObject', _PYTHON_FUNCTION_CODEOBJECT_INNER),
        ('PyCFunction', _PYTHON_FUNCTION_PYCFUNCTION_INNER),
    ]


PYTHON_FUNCTION._fields_ = [
    ('PathEntry', PYTHON_PATH_TABLE_ENTRY),
    ('ParentPathEntry', PPYTHON_PATH_TABLE_ENTRY),
    ('Key', ULONG_PTR),
    ('Signature', ULONG_PTR),
    ('CodeObject', PVOID),
    ('PyCFunctionObject', PVOID),
    ('MaxCallStackDepth', ULONG),
    ('CallCount', ULONG),
    ('u', _PYTHON_FUNCTION_INNER_UNION),
    ('ListEntry', LIST_ENTRY),
]

PYTHON_FUNCTION_TABLE_ENTRY._fields_ = [
    ('Links', RTL_SPLAY_LINKS),
    ('ListEntry', LIST_ENTRY),
    ('Function', PYTHON_FUNCTION),
]

SizeOfPythonFunctionTableEntry = sizeof(PYTHON_FUNCTION_TABLE_ENTRY)

class PYTHON(Structure):
    pass
PPYTHON = POINTER(PYTHON)

#===============================================================================
# Functions
#===============================================================================

INITIALIZE_PYTHON = CFUNCTYPE(BOOL, PRTL, HMODULE, PPYTHON, PULONG)

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
