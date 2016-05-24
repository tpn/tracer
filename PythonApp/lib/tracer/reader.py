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
    ('PathAtom', np.uint32),
    ('PathBuffer', np.uint64),

    ('FullNameLength', np.uint16),
    ('FullNameMaximumLength', np.uint16),
    ('FullNameAtom', np.uint32),
    ('FullNameBuffer', np.uint64),

    ('ModuleNameLength', np.uint16),
    ('ModuleNameMaximumLength', np.uint16),
    ('ModuleNameAtom', np.uint32),
    ('ModuleNameBuffer', np.uint64),

    ('NameLength', np.uint16),
    ('NameMaximumLength', np.uint16),
    ('NameAtom', np.uint32),
    ('NameBuffer', np.uint64),

    ('ClassNameLength', np.uint16),
    ('ClassNameMaximumLength', np.uint16),
    ('ClassNameAtom', np.uint32),
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
    ('PathAtom', np.uint32),
    ('PathBuffer', np.uint64),

    ('FullNameLength', np.uint16),
    ('FullNameMaximumLength', np.uint16),
    ('FullNameAtom', np.uint32),
    ('FullNameBuffer', np.uint64),

    ('ModuleNameLength', np.uint16),
    ('ModuleNameMaximumLength', np.uint16),
    ('ModuleNameAtom', np.uint32),
    ('ModuleNameBuffer', np.uint64),

    ('NameLength', np.uint16),
    ('NameMaximumLength', np.uint16),
    ('NameAtom', np.uint32),
    ('NameBuffer', np.uint64),

    ('ClassNameLength', np.uint16),
    ('ClassNameMaximumLength', np.uint16),
    ('ClassNameAtom', np.uint32),
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
    ('PathAtom', np.uint32),
    ('PathBuffer', np.uint64),

    ('FullNameLength', np.uint16),
    ('FullNameMaximumLength', np.uint16),
    ('FullNameAtom', np.uint32),
    ('FullNameBuffer', np.uint64),

    ('ModuleNameLength', np.uint16),
    ('ModuleNameMaximumLength', np.uint16),
    ('ModuleNameAtom', np.uint32),
    ('ModuleNameBuffer', np.uint64),

    ('NameLength', np.uint16),
    ('NameMaximumLength', np.uint16),
    ('NameAtom', np.uint32),
    ('NameBuffer', np.uint64),

    ('ClassNameLength', np.uint16),
    ('ClassNameMaximumLength', np.uint16),
    ('ClassNameAtom', np.uint32),
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

PythonTraceEventDataType = np.dtype([
    ('Timestamp', np.uint64),
    ('Function', np.uint64),

    ('WorkingSetSize', np.uint64),
    ('PageFaultCount', np.uint64),
    ('CommittedSize', np.uint64),

    ('ReadTransferCount', np.uint64),
    ('WriteTransferCount', np.uint64),

    ('HandleCount', np.uint32),
    ('TimestampDelta', np.uint32),
    ('ElapsedMicroseconds', np.uint32),

    ('WorkingSetDelta', np.int32),
    ('CommittedDelta', np.int32),
    ('ReadTransferDelta', np.uint32),
    ('WriteTransferDelta', np.uint32),

    ('CodeObjectHash', np.uint32),
    ('FunctionHash', np.uint32),

    ('PathAtom', np.uint32),
    ('FullNameAtom', np.uint32),
    ('ModuleNameAtom', np.uint32),
    ('ClassNameAtom', np.uint32),
    ('NameAtom', np.uint32),

    ('Flags', np.uint32),

    ('HandleDelta', np.int16),
    ('PageFaultDelta', np.uint16),

    ('LineNumber', np.uint16),
    ('FirstLineNumber', np.uint16),
    ('NumberOfLines', np.uint16),
    ('NumberOfCodeLines', np.uint16),
], align=True)

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

MetadataDataType = np.dtype([
    ('NumberOfRecords', np.uint64),
    ('RecordSize', np.uint64),
], align=True)

#===============================================================================
# Classes
#===============================================================================
class TraceStore:
    dtype = None
    filename = None

    def __init__(self, basedir, filename=None):
        if not filename:
            filename = self.filename
        self.path = join_path(basedir, filename)
        self.metadata_path = '%s:metadata' % self.path
        self.address_path = '%s:address' % self.path
        self.eof_path = '%s:eof' % self.path

        with open(self.metadata_path, 'rb') as f:
            self.metadata_np = np.fromfile(f, dtype=MetadataDataType)

        with open(self.address_path, 'rb') as f:
            self.address_np = np.fromfile(f, dtype=AddressDataType)

        with open(self.eof_path, 'rb') as f:
            self.eof_data = f.read()

        self.metadata_df = pd.DataFrame(self.metadata_np)
        self.address_df = pd.DataFrame(self.address_np)

        self.load_data()

    def load_data(self):
        with open(self.path, 'rb') as f:
            self.data_np = np.fromfile(f, dtype=self.dtype)

        self.data_df = pd.DataFrame(self.data_np)

class PythonTraceEventStore(TraceStore):
    dtype = PythonTraceEventDataType
    filename = 'TraceEvent.dat'

class PythonFunctionTableEntryStore(TraceStore):
    dtype = PythonFunctionTableEntryDataType
    filename = 'TraceFunctionTableEntry.dat'

class PythonPathTableEntryStore(TraceStore):
    dtype = PythonPathTableEntryDataType
    filename = 'TracePathTableEntry.dat'

class StringStore(TraceStore):
    dtype = StringDataType
    filename = 'TraceString.dat'

class StringBufferStore(TraceStore):
    dtype = StringDataType
    filename = 'TraceStringBuffer.dat'

    def load_data(self):
        with open(self.path, 'rb') as f:
            self.data = f.read()



        self.data_df = pd.DataFrame(self.data_np)

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
