#===============================================================================
# Imports
#===============================================================================
import numpy as np
import pandas as pd

from .path import join_path

#===============================================================================
# Types
#===============================================================================

StringDataType = np.dtype([
    ('Length', np.uint16),
    ('MaximumLength', np.uint16),
    ('Buffer', np.uint64),
])

HashedStringDataType = np.dtype([
    ('Atom', np.uint32),
    ('Hash', np.uint32),
    ('Length', np.uint16),
    ('MaximumLength', np.uint16),
    ('Buffer', np.uint64),
])

PythonPathTableEntryDataType = np.dtype([
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
], align=True)

PythonFunctionDataType = np.dtype([
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
], align=True)

PythonFunctionTableEntryDataType = np.dtype([
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
], align=True)


SlimPythonFunctionTableEntryDataTypeColumns = [
    'PathEntryType',
    'PathLength',
    'PathMaximumLength',
    'PathHash',
    'PathBuffer',

    'FullNameLength',
    'FullNameMaximumLength',
    'FullNameHash',
    'FullNameBuffer',

    'ModuleNameLength',
    'ModuleNameMaximumLength',
    'ModuleNameHash',
    'ModuleNameBuffer',

    'NameLength',
    'NameMaximumLength',
    'NameHash',
    'NameBuffer',

    'ClassNameLength',
    'ClassNameMaximumLength',
    'ClassNameHash',
    'ClassNameBuffer',

    'ReferenceCount',
    'CodeObjectHash',
    'FunctionHash',

    'FirstLineNumber',
    'NumberOfLines',
    'NumberOfCodeLines',
    'SizeOfByteCode',
]

PythonTraceEventDataType = np.dtype([
    ('Timestamp', np.uint64),
    ('Function', np.uint64),

    ('WorkingSetSize', np.uint64),
    ('PageFaultCount', np.uint64),
    ('CommittedSize', np.uint64),

    ('ReadTransferCount', np.uint64),
    ('WriteTransferCount', np.uint64),

    ('HandleCount', np.uint32),
    ('ThreadId', np.uint32),
    ('ElapsedMicroseconds', np.uint32),

    ('WorkingSetDelta', np.int32),
    ('CommittedDelta', np.int32),
    ('ReadTransferDelta', np.uint32),
    ('WriteTransferDelta', np.uint32),

    ('CodeObjectHash', np.uint32),
    ('FunctionHash', np.uint32),

    ('PathHash', np.int32),
    ('FullNameHash', np.int32),
    ('ModuleNameHash', np.int32),
    ('ClassNameHash', np.int32),
    ('NameHash', np.int32),

    ('Flags', np.uint32),

    ('HandleDelta', np.int16),
    ('PageFaultDelta', np.uint16),

    ('LineNumber', np.uint16),
    ('FirstLineNumber', np.uint16),
    ('NumberOfLines', np.uint16),
    ('NumberOfCodeLines', np.uint16),
], align=True)

SlimPythonTraceEventDataTypeColumns = [
    'Timestamp',
    'Function',
    'ElapsedMicroseconds',
    'WorkingSetDelta',
    'CommittedDelta',
    'ReadTransferDelta',
    'WriteTransferDelta',
    'HandleDelta',
    'PageFaultDelta',
    'PathHash',
    'FullNameHash',
    'ModuleNameHash',
    'ClassNameHash',
    'NameHash',
    'LineNumber',
    'NumberOfLines',
    'NumberOfCodeLines'
]

AddressDataType = np.dtype([
    ('PreferredBaseAddress', np.uint64),
    ('BaseAddress', np.uint64),
    ('FileOffset', np.uint64),
    ('MappedSize', np.uint64),
    ('ProcessId', np.uint32),
    ('RequestingThreadId', np.uint32),
    ('RequestedTimestamp', np.uint64),
    ('PreparedTimestamp', np.uint64),
    ('ConsumedTimestamp', np.uint64),
    ('RetiredTimestamp', np.uint64),
    ('ReleasedTimestamp', np.uint64),
    ('AwaitingPreparation', np.uint64),
    ('AwaitingConsumption', np.uint64),
    ('Active', np.uint64),
    ('AwaitingRelease', np.uint64),
    ('MappedSequenceId', np.int32),
    ('RequestingProcessorGroup', np.uint16),
    ('RequestingProcessorNumber', np.uint8),
    ('RequestingNumaNode', np.uint8),
    ('FulfillingProcessorGroup', np.uint16),
    ('FulfillingProcessorNumber', np.uint8),
    ('FulfillingNumaNode', np.uint8),
    ('FulfillingThreadId', np.uint32),
], align=True)

AllocationDataType = np.dtype([
    ('NumberOfRecords', np.uint64),
    ('RecordSize', np.uint64),
], align=True)

InfoDataType = np.dtype([
    # TRACE_STORE_EOF
    ('EndOfFile', np.uint64),
    # TRACE_STORE_TIME
    ('Frequency', np.uint64),
    ('Multiplicand', np.uint64),
    ('StartTime', np.uint64),
    ('StartCounter', np.uint64),
    # TRACE_STORE_STATS
    ('DroppedRecords', np.uint32),
    ('ExhaustedFreeMemoryMaps', np.uint32),
    ('AllocationsOutpacingNextMemoryMapPreparation', np.uint32),
    ('PreferredAddressUnavailable', np.uint32),
], align=True)

DefaultHashedStringColumns = [
    'Path',
    'FullName',
    'ModuleName',
    'Name',
    'ClassName',
]

#===============================================================================
# Classes
#===============================================================================
class TraceStore:
    slim = None
    dtype = None
    filename = None
    hashed_string_columns = None

    def __init__(self, basedir, filename=None, string_buffer_store=None,
                                slim=True):
        if not filename:
            filename = self.filename
        self.path = join_path(basedir, filename)
        self.allocation_path = '%s:allocation' % self.path
        self.address_path = '%s:address' % self.path
        self.info_path = '%s:info' % self.path

        with open(self.allocation_path, 'rb') as f:
            self.allocation_np = np.fromfile(f, dtype=AllocationDataType)

        with open(self.address_path, 'rb') as f:
            self.address_np = np.fromfile(f, dtype=AddressDataType)

        with open(self.info_path, 'rb') as f:
            self.info_np = np.fromfile(f, dtype=InfoDataType)

        self.allocation_df = pd.DataFrame(self.allocation_np)
        self.address_df = pd.DataFrame(self.address_np)
        self.info_df = pd.DataFrame(self.info_np)
        self.string_buffer_store = string_buffer_store

        self.load_data(slim=slim)

    def load_data(self, slim=None):
        with open(self.path, 'rb') as f:
            self.data_np = np.fromfile(f, dtype=self.dtype)

        self.data_df = pd.DataFrame(self.data_np)

        if slim:
            self.data_df = self.data_df[self.slim]

        if self.hashed_string_columns and self.string_buffer_store:
            sb = self.string_buffer_store
            for column in self.hashed_string_columns:
                self.resolve_string_hashes(sb.string_items, column)

    def resolve_string_hashes(self, string_items, column_prefix):
        hash_col = '%sHash' % column_prefix
        string_col = '%sString' % column_prefix

        columns = [ hash_col, string_col ]
        df = pd.DataFrame(string_items, columns=columns)

        self.data_df = self.data_df.merge(df, on=hash_col, how='left')

class PythonTraceEventStore(TraceStore):
    slim = SlimPythonTraceEventDataTypeColumns
    dtype = PythonTraceEventDataType
    filename = 'TraceEvent.dat'
    hashed_string_columns = DefaultHashedStringColumns

class PythonFunctionTableEntryStore(TraceStore):
    slim = SlimPythonFunctionTableEntryDataTypeColumns
    dtype = PythonFunctionTableEntryDataType
    filename = 'TraceFunctionTableEntry.dat'
    hashed_string_columns = DefaultHashedStringColumns

class PythonPathTableEntryStore(TraceStore):
    dtype = PythonPathTableEntryDataType
    filename = 'TracePathTableEntry.dat'
    has_hashed_strings = True

class StringStore(TraceStore):
    dtype = StringDataType
    filename = 'TraceString.dat'

class StringBufferStore(TraceStore):
    dtype = StringDataType
    filename = 'TraceStringBuffer.dat'

    def load_data(self, slim=None):
        with open(self.path, 'rb') as f:
            self.data = f.read()

        self.strings = [ s for s in self.data.split('\x00') if s ]
        self.hashed_strings = { hash(s): s for s in self.strings }
        self.hashed_strings[0] = '';
        self.hashed_strings[-1] = '';
        self.hashed_strings[-2] = '';
        self.string_items = self.hashed_strings.items()

    @classmethod
    def to_hash_map(self, tracer, strings):
        hashes = {}
        failed = []
        for string in strings:
            (hash_value, _) = tracer.hash_and_atomize_string(string)
            if hash_value != hash(string):
                failed.append((hash_value, hash(string), string))
            else:
                hashes[hash_value] = string


        return (hashes, failed)

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
