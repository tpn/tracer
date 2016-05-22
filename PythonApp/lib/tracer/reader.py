#===============================================================================
# Imports
#===============================================================================
import numpy as np
import pandas as pd

import struct

#===============================================================================
# Types
#===============================================================================

PythonTraceEventDataType = np.dtype([
    ('Timestamp', np.uint64),
    ('Function', np.uint64),

    ('WorkingSetSize', np.uint64),
    ('PageFaultCount', np.uint64),
    ('CommittedSize', np.uint64),
    ('Unused2', np.uint64),

    ('TimestampDelta', np.uint32),
    ('ElapsedMicroseconds', np.uint32),

    ('WorkingSetDelta', np.uint32),
    ('PageFaultDelta', np.uint32),
    ('CommittedDelta', np.uint32),

    ('Flags', np.uint32),

    ('CodeObjectHash', np.uint32),
    ('FunctionHash', np.uint32),

    ('FunctionReferenceCount', np.uint32),

    ('PathAtom', np.uint32),
    ('FullNameAtom', np.uint32),
    ('ModuleNameAtom', np.uint32),
    ('ClassNameAtom', np.uint32),
    ('NameAtom', np.uint32),

    ('LineNumber', np.uint16),
    ('FirstLineNumber', np.uint16),
    ('LastLineNumber', np.uint16),
    ('NumberOfLines', np.uint16),

    ('Padding1', np.uint64),
    ('Padding2', np.uint64),
])

AddressDataType = np.dtype([
    ('PreferredBaseAddress', np.uint64),
    ('BaseAddress', np.uint64),
    ('FileOffset', np.uint64),
    ('MappedSize', np.uint64),
    ('MappedSequenceId', np.uint64),
    ('RequestedTimestamp', np.uint64),
    ('PreparedTimestamp', np.uint64),
    ('ConsumedTimestamp', np.uint64),
    ('RetiredTimestamp', np.uint64),
    ('ReleasedTimestamp', np.uint64),
    ('AwaitingPreparation', np.uint64),
    ('AwaitingConsumption', np.uint64),
    ('Active', np.uint64),
    ('AwaitingRelease', np.uint64),
])

MetadataDataType = np.dtype([
    ('NumberOfRecords', np.uint64),
    ('RecordSize', np.uint64),
])

#===============================================================================
# Classes
#===============================================================================
class TraceStore:
    def __init__(self, path):
        self.path = path
        self.metadata_path = '%s:metadata' % path
        self.address_path = '%s:address' % path
        self.eof_path = '%s:eof' % path

        with open(self.metadata_path, 'rb') as f:
            self.metadata_np = np.fromfile(f, dtype=MetadataDataType)
            #self.metadata_data = f.read()

        with open(self.address_path, 'rb') as f:
            self.address_np = np.fromfile(f, dtype=AddressDataType)
            #self.address_data = f.read()

        with open(self.eof_path, 'rb') as f:
            self.eof_data = f.read()

        #self.metadata_np = np.fromstring(
        #    self.metadata_data,
        #    dtype=MetadataDataType,
        #)

        #self.address_np = np.fromstring(
        #    self.address_data,
        #    dtype=AddressDataType,
        #)

        self.metadata_df = pd.DataFrame(self.metadata_np)
        self.address_df = pd.DataFrame(self.address_np)

class PythonTraceEventStore(TraceStore):
    def __init__(self, path):
        TraceStore.__init__(self, path)

        with open(self.path, 'rb') as f:
            self.data_np = np.fromfile(f, dtype=PythonTraceEventDataType)

        self.data_df = pd.DataFrame(self.data_np)

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
