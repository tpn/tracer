#===============================================================================
# Imports
#===============================================================================

from collections import (
    defaultdict,
)

from .logic import (
    Mutex,
)

from .util import (
    Constant,
    SlotObject,
)

#===============================================================================
# Globals/Aliases
#===============================================================================

TRACE_STORE_TEXT = """\
struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x000 TraceStoreId     : Enum _TRACE_STORE_ID,  62 total enums

   +0x004 TraceStoreMetadataId : Enum _TRACE_STORE_METADATA_ID,  11 total enums

   +0x008 TraceStoreIndex  : Enum _TRACE_STORE_INDEX,  601 total enums

   +0x00c NoRetire         : Bitfield Pos 0, 1 Bit
   +0x00c NoPrefaulting    : Bitfield Pos 1, 1 Bit
   +0x00c IgnorePreferredAddresses : Bitfield Pos 2, 1 Bit
   +0x00c IsReadonly       : Bitfield Pos 3, 1 Bit
   +0x00c SetEndOfFileOnClose : Bitfield Pos 4, 1 Bit
   +0x00c IsMetadata       : Bitfield Pos 5, 1 Bit
   +0x00c HasRelocations   : Bitfield Pos 6, 1 Bit
   +0x00c NoTruncate       : Bitfield Pos 7, 1 Bit
   +0x00c IsRelocationTarget : Bitfield Pos 8, 1 Bit
   +0x00c HasSelfRelocations : Bitfield Pos 9, 1 Bit
   +0x00c OnlyRelocationIsToSelf : Bitfield Pos 10, 1 Bit
   +0x00c OnlyRelocationIsToNull : Bitfield Pos 11, 1 Bit
   +0x00c HasMultipleRelocationWaits : Bitfield Pos 12, 1 Bit
   +0x00c RequiresSelfRelocation : Bitfield Pos 13, 1 Bit
   +0x00c HasNullStoreRelocations : Bitfield Pos 14, 1 Bit
   +0x00c BeginningRundown : Bitfield Pos 15, 1 Bit
   +0x00c RundownComplete  : Bitfield Pos 16, 1 Bit
   +0x00c HasFlatMapping   : Bitfield Pos 17, 1 Bit
   +0x00c FlatMappingLoaded : Bitfield Pos 18, 1 Bit
   +0x00c UnusedStoreFlagBits : Bitfield Pos 19, 13 Bits
   +0x00c StoreFlags       : struct _TRACE_STORE_FLAGS, 20 elements, 0x4 bytes
   +0x010 TraceFlags       : struct _TRACE_FLAGS, 20 elements, 0x4 bytes
   +0x014 InitialTraits    : struct _TRACE_STORE_TRAITS, 17 elements, 0x4 bytes
   +0x018 TracerConfig     : Ptr64 to struct _TRACER_CONFIG, 15 elements, 0x1c0 bytes
   +0x020 LastError        : Uint4B
   +0x024 TotalNumberOfMemoryMaps : Int4B
   +0x028 NumberOfActiveMemoryMaps : Int4B
   +0x02c NumberOfNonRetiredMemoryMaps : Int4B
   +0x030 CloseMemoryMaps  : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x040 PrepareMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x050 PrepareReadonlyMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x060 NextMemoryMaps   : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x070 FreeMemoryMaps   : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x080 PrefaultMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x090 NonRetiredMemoryMaps : union _SLIST_HEADER, 3 elements, 0x10 bytes
   +0x0a0 SingleMemoryMap  : struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x0e0 StoresListEntry  : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x0f0 MetadataListHead : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x0f0 MetadataListEntry : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x100 ListEntry        : struct _SLIST_ENTRY, 1 elements, 0x10 bytes
   +0x100 Next             : Ptr64 to struct _SLIST_ENTRY, 1 elements, 0x10 bytes
   +0x108 Unused           : Ptr64 to Void
   +0x110 Rtl              : Ptr64 to struct _RTL, 483 elements, 0x1020 bytes
   +0x118 pAllocator       : Ptr64 to struct _ALLOCATOR, 28 elements, 0x90 bytes
   +0x120 TraceContext     : Ptr64 to struct _TRACE_CONTEXT, 73 elements, 0x520 bytes
   +0x128 InitialSize      : union _LARGE_INTEGER, 4 elements, 0x8 bytes
   +0x130 ExtensionSize    : union _LARGE_INTEGER, 4 elements, 0x8 bytes
   +0x138 MappingSize      : union _LARGE_INTEGER, 4 elements, 0x8 bytes
   +0x140 PrefaultFuturePageWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x148 PrepareNextMemoryMapWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x150 PrepareReadonlyNonStreamingMemoryMapWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x158 CloseMemoryMapWork : Ptr64 to struct _TP_WORK, 0 elements, 0x0 bytes
   +0x160 NextMemoryMapAvailableEvent : Ptr64 to Void
   +0x168 AllMemoryMapsAreFreeEvent : Ptr64 to Void
   +0x170 ReadonlyMappingCompleteEvent : Ptr64 to Void
   +0x178 BindCompleteEvent : Ptr64 to Void
   +0x180 ResumeAllocationsEvent : Ptr64 to Void
   +0x188 RelocationCompleteWaitEvent : Ptr64 to Void
   +0x190 RelocationCompleteWaitEvents : Ptr64 to Ptr64 to Void
   +0x198 PrevMemoryMap    : Ptr64 to struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x1a0 MemoryMap        : Ptr64 to struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x1a8 NumberOfRelocationBackReferences : Uint4B
   +0x1ac OutstandingRelocationBinds : Uint4B
   +0x1b0 MappedSequenceId : Int4B
   +0x1b4 MetadataBindsInProgress : Int4B
   +0x1b8 PrepareReadonlyNonStreamingMapsInProgress : Int4B
   +0x1bc ReadonlyNonStreamingBindCompletesInProgress : Int4B
   +0x1c0 ActiveAllocators : Int4B
   +0x1c4 NumberOfRelocationDependencies : Uint4B
   +0x1c8 NumberOfRelocationsRequired : Uint4B
   +0x1cc SequenceId       : Uint4B
   +0x1d0 NumaNode         : Uint4B
   +0x1d4 CreateFileDesiredAccess : Uint4B
   +0x1d8 CreateFileCreationDisposition : Uint4B
   +0x1dc CreateFileMappingProtectionFlags : Uint4B
   +0x1e0 CreateFileFlagsAndAttributes : Uint4B
   +0x1e4 MapViewOfFileDesiredAccess : Uint4B
   +0x1e8 MappingHandle    : Ptr64 to Void
   +0x1f0 FileHandle       : Ptr64 to Void
   +0x1f8 PrevAddress      : Ptr64 to Void
   +0x200 FlatMappingHandle : Ptr64 to Void
   +0x208 FlatAddress      : struct _TRACE_STORE_ADDRESS, 20 elements, 0x80 bytes
   +0x288 FlatAddressRange : struct _TRACE_STORE_ADDRESS_RANGE, 7 elements, 0x40 bytes
   +0x2c8 Padding4         : Uint8B
   +0x2d0 FlatMemoryMap    : struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x310 TraceStore       : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x318 MetadataInfoStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x320 AllocationStore  : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x328 RelocationStore  : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x330 AddressStore     : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x338 AddressRangeStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x340 AllocationTimestampStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x348 AllocationTimestampDeltaStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x350 SynchronizationStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x358 InfoStore        : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x360 RelocationDependencyStores : Ptr64 to Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x360 RelocationDependencyStore : Ptr64 to struct _TRACE_STORE, 148 elements, 0x800 bytes
   +0x368 AllocateRecords  : Ptr64 to     void*
   +0x370 AllocateRecordsWithTimestamp : Ptr64 to     void*
   +0x378 TryAllocateRecords : Ptr64 to     void*
   +0x380 TryAllocateRecordsWithTimestamp : Ptr64 to     void*
   +0x388 SuspendedAllocateRecordsWithTimestamp : Ptr64 to     void*
   +0x390 AllocateRecordsWithTimestampImpl1 : Ptr64 to     void*
   +0x398 AllocateRecordsWithTimestampImpl2 : Ptr64 to     void*
   +0x3a0 BindComplete     : Ptr64 to     int
   +0x3a8 pReloc           : Ptr64 to struct _TRACE_STORE_RELOC, 9 elements, 0x48 bytes
   +0x3b0 pTraits          : Ptr64 to struct _TRACE_STORE_TRAITS, 17 elements, 0x4 bytes
   +0x3b8 Eof              : Ptr64 to struct _TRACE_STORE_EOF, 1 elements, 0x8 bytes
   +0x3c0 Time             : Ptr64 to struct _TRACE_STORE_TIME, 3 elements, 0x58 bytes
   +0x3c8 Stats            : Ptr64 to struct _TRACE_STORE_STATS, 11 elements, 0x40 bytes
   +0x3d0 Totals           : Ptr64 to struct _TRACE_STORE_TOTALS, 4 elements, 0x20 bytes
   +0x3d8 Traits           : Ptr64 to struct _TRACE_STORE_TRAITS, 17 elements, 0x4 bytes
   +0x3e0 Info             : Ptr64 to struct _TRACE_STORE_INFO, 6 elements, 0x100 bytes
   +0x3e8 Reloc            : Ptr64 to struct _TRACE_STORE_RELOC, 9 elements, 0x48 bytes
   +0x3f0 Allocation       : Ptr64 to struct _TRACE_STORE_ALLOCATION, 2 elements, 0x10 bytes
   +0x3f8 AllocationTimestamp : Ptr64 to struct _TRACE_STORE_ALLOCATION_TIMESTAMP, 1 elements, 0x8 bytes
   +0x400 AllocationTimestampDelta : Ptr64 to struct _TRACE_STORE_ALLOCATION_TIMESTAMP_DELTA, 1 elements, 0x4 bytes
   +0x408 Address          : Ptr64 to struct _TRACE_STORE_ADDRESS, 20 elements, 0x80 bytes
   +0x410 AddressRange     : Ptr64 to struct _TRACE_STORE_ADDRESS_RANGE, 7 elements, 0x40 bytes
   +0x418 Sync             : Ptr64 to struct _TRACE_STORE_SYNC, 6 elements, 0x100 bytes
   +0x420 NumberOfAllocations : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x428 NumberOfAddresses : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x430 NumberOfAddressRanges : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x438 NumberOfReadonlyAddressRanges : union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x440 ReadonlyAddresses : Ptr64 to struct _TRACE_STORE_ADDRESS, 20 elements, 0x80 bytes
   +0x448 ReadonlyMemoryMaps : Ptr64 to struct _TRACE_STORE_MEMORY_MAP, 10 elements, 0x40 bytes
   +0x450 ReadonlyAddressRanges : Ptr64 to struct _TRACE_STORE_ADDRESS_RANGE, 7 elements, 0x40 bytes
   +0x458 ReadonlyMappingSizes : Ptr64 to union _ULARGE_INTEGER, 4 elements, 0x8 bytes
   +0x460 ReadonlyPreferredAddressUnavailable : Uint4B
   +0x468 DataType         : struct <unnamed-tag>, 2 elements, 0x10 bytes
   +0x478 Allocator        : struct _ALLOCATOR, 28 elements, 0x90 bytes
   +0x508 IntervalFramesPerSecond : Uint8B
   +0x510 Intervals        : struct _TRACE_STORE_INTERVALS, 16 elements, 0x80 bytes
   +0x590 Db               : Ptr64 to struct _TRACE_STORE_SQLITE3_DB, 40 elements, 0x4b0 bytes
   +0x598 Sqlite3Schema    : Ptr64 to Char
   +0x5a0 Sqlite3VirtualTableName : Ptr64 to Char
   +0x5a8 Sqlite3Column    : Ptr64 to     long
   +0x5b0 Sqlite3Module    : struct _SQLITE3_MODULE, 24 elements, 0xb8 bytes
   +0x668 Sqlite3VirtualTable : struct _TRACE_STORE_SQLITE3_VTAB, 6 elements, 0x18 bytes
   +0x680 Sqlite3IntervalSchema : Ptr64 to Char
   +0x688 Sqlite3IntervalVirtualTableName : Ptr64 to Char
   +0x690 Sqlite3IntervalColumn : Ptr64 to     long
   +0x698 Sqlite3IntervalModule : struct _SQLITE3_MODULE, 24 elements, 0xb8 bytes
   +0x750 Sqlite3IntervalVirtualTable : struct _TRACE_STORE_SQLITE3_VTAB, 6 elements, 0x18 bytes
   +0x768 Padding5         : [18] Uint8B"""

#===============================================================================
# Enums
#===============================================================================

#===============================================================================
# Classes
#===============================================================================

Names = [
    'Bitfield',
    'Struct',
    'Union',
    'Enum',
    'Char',
    'Byte',
    'WideChar',
    'SignedShort',
    'UnsignedShort',
    'SignedLong',
    'UnsignedLong',
    'SignedLongLong',
    'UnsignedLongLong',
    'Float',
    'Array',
    'CString',
    'WideCString',
    'FunctionPointer',
    'PointerToPointer',
    'VoidPointer',
    'DataPointer',
]

#===============================================================================
# Invalid Line Exceptions
#===============================================================================

class InvalidLine(Exception):
    def __init__(self, line=None, part=None):
        self.line = line
        self.part = part

class InvalidBitfieldLine(InvalidLine):
    pass

class InvalidStructLine(InvalidLine):
    pass

class InvalidUnionLine(InvalidLine):
    pass

class InvalidEnumLine(InvalidLine):
    pass

class InvalidCharLine(InvalidLine):
    pass

class InvalidByteLine(InvalidLine):
    pass

class InvalidWideCharLine(InvalidLine):
    pass

class InvalidSignedShortLine(InvalidLine):
    pass

class InvalidUnsignedShortLine(InvalidLine):
    pass

class InvalidSignedLongLine(InvalidLine):
    pass

class InvalidUnsignedLongLine(InvalidLine):
    pass

class InvalidSignedLongLongLine(InvalidLine):
    pass

class InvalidUnsignedLongLongLine(InvalidLine):
    pass

class InvalidFloatLine(InvalidLine):
    pass

class InvalidArrayLine(InvalidLine):
    pass

class InvalidCStringLine(InvalidLine):
    pass

class InvalidWideCStringLine(InvalidLine):
    pass

class InvalidFunctionPointerLine(InvalidLine):
    pass

class InvalidPointerToPointerLine(InvalidLine):
    pass

class InvalidVoidPointerLine(InvalidLine):
    pass

class InvalidDataPointerLine(InvalidLine):
    pass

#===============================================================================
# Line Classes
#===============================================================================

class BaseLine(object):
    is_recursive = False
    is_numeric = False
    is_integer = False
    is_decimal = False
    is_pointer = False
    is_unnamed = False
    is_character = False
    is_composite = False

    name = None

    def __init__(self, line):
        self.line = line
        try:
            parsed = self.parse(line)
        except AttributeError:
            parsed = None
        if not parsed:
            return
        for (key, value) in parsed.items():
            setattr(self, key, value)

class BitfieldLine(BaseLine):
    bit_position = None
    number_of_bits = None

    def __init__(self, line):
        BaseLine.__init__(self, line)

    @classmethod
    def parse(cls, line):
        parts = line.split(', ')
        (left, right) = parts
        prefix = 'Bitfield Pos '
        if not left.startswith(prefix):
            raise InvalidBitfieldLine(line=line)

        try:
            bit_position = int(left.replace(prefix, ''))
        except ValueError:
            raise InvalidBitfieldLine(part=left)

        if not right.endswith(' Bit') and not right.endswith(' Bits'):
            raise InvalidBitfieldLine(part=right)

        bit_part = right.split(' ')[0]

        try:
            number_of_bits = int(bit_part)
        except ValueError:
            raise InvalidBitfieldLine(part=bit_part)

        return {
            'bit_position': bit_position,
            'number_of_bits': number_of_bits,
        }


class StructLine(BaseLine):
    type_name = None
    struct_name = None
    is_composite = True
    size_in_bytes = None
    number_of_elements = None

    @classmethod
    def parse(cls, line):
        parts = line.split(', ')
        (left, center, right) = parts

        if not left.startswith('struct '):
            raise InvalidStructLine(part=left)

        if not center.endswith(' element') and not center.endswith(' elements'):
            raise InvalidStructLine(part=center)

        if not right.endswith(' byte') and not right.endswith(' bytes'):
            raise InvalidStructLine(part=right)

        type_name = None
        struct_name = left[len('struct '):]
        if struct_name[0] == '_':
            type_name = struct_name[1:]

        name = (type_name if type_name else struct_name)

        element_part = center.split(' ')[0]
        try:
            number_of_elements = int(element_part)
        except ValueError:
            raise InvalidStructLine(part=element_part)

        size_part = right.split(' ')[0]
        try:
            size_in_bytes = int(size_part, 16)
        except ValueError:
            raise InvalidStructLine(part=size_part)

        return {
            'name': name,
            'type_name': type_name,
            'struct_name': struct_name,
            'size_in_bytes': size_in_bytes,
            'number_of_elements': number_of_elements,
        }


class UnionLine(BaseLine):
    type_name = None
    union_name = None
    is_composite = True
    size_in_bytes = None
    number_of_elements = None

    @classmethod
    def parse(cls, line):
        parts = line.split(', ')
        (left, center, right) = parts

        if not left.startswith('union '):
            raise InvalidUnionLine(part=left)

        if not center.endswith(' element') and not center.endswith(' elements'):
            raise InvalidUnionLine(part=center)

        if not right.endswith(' byte') and not right.endswith(' bytes'):
            raise InvalidUnionLine(part=right)

        type_name = None
        union_name = left[len('union '):]
        if union_name[0] == '_':
            type_name = union_name[1:]

        name = (type_name if type_name else union_name)

        element_part = center.split(' ')[0]
        try:
            number_of_elements = int(element_part)
        except ValueError:
            raise InvalidStructLine(part=element_part)

        size_part = right.split(' ')[0]
        try:
            size_in_bytes = int(size_part, 16)
        except ValueError:
            raise InvalidStructLine(part=size_part)

        return {
            'name': name,
            'type_name': type_name,
            'union_name': union_name,
            'size_in_bytes': size_in_bytes,
            'number_of_elements': number_of_elements,
        }

class EnumLine(BaseLine):
    is_integer = True
    is_numeric = True
    type_name = None
    enum_name = None
    size_in_bytes = 4
    number_of_enums = None

    @classmethod
    def parse(cls, line):
        parts = line.split(',  ')
        (left, right) = parts

        if not left.startswith('Enum '):
            raise InvalidEnumLine(part=left)

        suffixes = (
            ' total enum',
            ' total enums',
        )

        if not right.endswith(suffixes):
            raise InvalidEnumLine(part=right)

        type_name = None
        enum_name = left[len('Enum '):]
        if enum_name[0] == '_':
            type_name = enum_name[1:]

        name = (type_name if type_name else enum_name)

        enum_part = right.split(' ')[0]
        try:
            number_of_enums = int(enum_part)
        except ValueError:
            raise InvalidEnumLine(part=enum_part)

        return {
            'name': name,
            'type_name': type_name,
            'enum_name': enum_name,
            'number_of_enums': number_of_enums,
        }

class CharLine(BaseLine):
    is_character = True
    size_in_bytes = 1

class ByteLine(BaseLine):
    size_in_bytes = 1

class WideCharLine(BaseLine):
    size_in_bytes = 2

class BaseIntegerLine(BaseLine):
    is_signed = None
    is_integer = True
    is_numeric = True

    def __hash__(self):
        return (
            self.is_signed ^
            self.is_numeric ^
            self.is_integer ^
            self.size_in_bytes
        )

class SignedShortLine(BaseIntegerLine):
    is_signed = True
    size_in_bytes = 2

class UnsignedShortLine(BaseIntegerLine):
    is_signed = False
    size_in_bytes = 2

class SignedLongLine(BaseIntegerLine):
    is_signed = True
    size_in_bytes = 4

class UnsignedLongLine(BaseIntegerLine):
    is_signed = False
    size_in_bytes = 4

class SignedLongLongLine(BaseIntegerLine):
    is_signed = True
    size_in_bytes = 8

class UnsignedLongLongLine(BaseIntegerLine):
    is_signed = False
    size_in_bytes = 8

class FloatLine(BaseLine):
    is_numeric = True
    size_in_bytes = 8

class ArrayLine(BaseLine):

    @classmethod
    def parse(cls, line):
        import ipdb
        ipdb.set_trace()

class BaseStringLine(BaseLine):
    is_string = True

class CStringLine(BaseStringLine):

    @classmethod
    def parse(cls, line):
        import ipdb; ipdb.set_trace()

class WideCStringLine(BaseLine):

    @classmethod
    def parse(cls, line):
        import ipdb; ipdb.set_trace()

class BasePointerLine(BaseLine):
    size_in_bytes = 8

class FunctionPointerLine(BasePointerLine):
    pass

class PointerToPointerLine(BasePointerLine):
    pass

class VoidPointerLine(BasePointerLine):
    pass

class DataPointerLine(BasePointerLine):
    pass

class StructLine2(SlotObject):
    __slots__ = [
        'size_in_bytes',
        'num_elements',
        'struct_name',
        'type_name',
    ]

    @classmethod
    def parse(cls, line):

        words = line.split(' ')

        struct_index = None

        for (i, word) in enumerate(words):
            if word == 'Ptr64':
                break
            if word != 'struct':
                continue
            struct_index = i
            break

        if struct_index is None:
            raise InvalidStructLine()

        line = ' '.join(words[struct_index+1:])
        words = line.split(', ')

        obj = cls()
        obj.struct_name = words.pop(0)
        assert obj.struct_name[0] == '_'
        obj.type_name = obj.struct_name[1:]

        obj.num_elements = int(words.pop(0).replace(' elements', ''))
        obj.size_in_bytes = int(words.pop(0).replace(' bytes', ''), 16)

        return obj

    @classmethod
    def try_create(cls, line):
        try:
            return cls.create(line)
        except InvalidStructLine:
            pass

    def __hash__(self):
        return (
            self.struct_name ^
            self.num_elements ^
            self.size_in_bytes
        )

class Struct(StructLine):

    def __init__(self, *args, **kwds):
        StructLine.__init__(self, *args, **kwds)
        self.lines = []
        self.last_offset = None
        self.cumulative_size = 0
        self.offsets = defaultdict(list)
        self.inline_unions = {}
        self.inline_unions_by_offset = defaultdict(list)
        self.inline_bitfields = {}
        self.inline_bitfields_by_offset = defaultdict(list)
        self.inline_structs = {}
        self.inline_structs_by_offset = defaultdict(list)
        self.enums = {}
        self.enums_by_offset = defaultdict(list)
        self.expected_next_offset = 0

    @classmethod
    def extract_type(self, line, chained=None):

        parts = line.split(' ')
        first = parts[0]
        last = parts[-1]

        m = Mutex()
        m.is_bitfield = (first == 'Bitfield')
        m.is_union = (first == 'union')
        m.is_struct = (first == 'struct')
        m.is_enum = (first == 'Enum')
        m.is_char = (first == 'Char')
        m.is_byte = (first == 'UChar')
        m.is_wide_char = (first == 'Wchar')
        m.is_short = (first == 'Int2B')
        m.is_ushort = (first == 'Uint2B')
        m.is_long = (first == 'Int4B')
        m.is_ulong = (first == 'Uint4B')
        m.is_longlong = (first == 'Int8B')
        m.is_ulonglong = (first == 'Uint8B')
        m.is_float = (first == 'Float')
        m.is_array = (first[0] == '[')

        m.is_function_pointer = (line.startswith('Ptr64 to     '))
        m.is_pointer_to_pointer = (line.startswith('Ptr64 to Ptr64'))
        m.is_void_pointer = (line.startswith('Ptr64 to Void'))
        m.is_data_pointer = (
            line.startswith('Ptr64 to ') and
            not line.startswith('Ptr64 to     ') and
            not line.startswith('Ptr64 to Ptr64') and
            not line.startswith('Ptr64 to Char') and
            not line.startswith('Ptr64 to Wchar') and
            not line.startswith('Ptr64 to Void')
        )
        m.is_cstring = (line.startswith('Ptr64 to Char'))
        m.is_wide_cstring = (line.startswith('Ptr64 to Wchar'))

        with m as m:

            if m.is_bitfield:
                t = BitfieldLine(line)

            elif m.is_union:
                t = UnionLine(line)

            elif m.is_struct:
                t = StructLine(line)

            elif m.is_enum:
                t = EnumLine(line)

            elif m.is_char:
                t = CharLine(line)

            elif m.is_byte:
                t = ByteLine(line)

            elif m.is_wide_char:
                t = WideCharLine(line)

            elif m.is_short:
                t = SignedShortLine(line)

            elif m.is_ushort:
                t = UnsignedShortLine(line)

            elif m.is_long:
                t = SignedLongLine(line)

            elif m.is_ulong:
                t = UnsignedLongLine(line)

            elif m.is_longlong:
                t = SignedLongLong(line)

            elif m.is_ulonglong:
                t = UnsignedLongLine(line)

            elif m.is_float:
                t = FloatLine(line)

            elif m.is_array:
                t = ArrayLine(line)

            elif m.is_cstring:
                t = CStringLine(line)

            elif m.is_wide_cstring:
                t = CWideStringLine(line)

            elif m.is_function_pointer:
                t = FunctionPointerLine(line)

            elif m.is_pointer_to_pointer:
                recurse = True
                t = PointerToPointerLine(line)

            elif m.is_void_pointer:
                t = VoidPointerLine(line)

            elif m.is_data_pointer:
                t = DataPointerLine(line)

        if t.is_recursive:
            import ipdb
            ipdb.set_trace()

        return t


    def add_line(self, line):

        if not line.startswith('   +0x'):
            return

        line = line[4:]
        (left, right) = line.split(' : ')
        (offset, field_name) = left.split(' ')
        offset = int(offset, 16)
        if self.last_offset:
            assert self.last_offset <= offset, (self.last_offset, offset)

        if offset != self.expected_next_offset:
            import ipdb
            ipdb.set_trace()

        is_new_offset = offset in self.offsets
        self.offsets[offset] = line

        t = self.extract_type(right)
        return t


#===============================================================================
# Functions
#===============================================================================

def parse_struct(text):
    lines = text.splitlines()

    first_lineno = None

    for (i, line) in enumerate(lines):
        if not line.startswith('struct _'):
            continue
        first_lineno = i
        break

    assert first_lineno is not None

    struct = Struct.create(lines[first_lineno])
    assert struct

    remaining = lines[first_lineno+1:]

    return struct


#===============================================================================
# Helpers
#===============================================================================


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
