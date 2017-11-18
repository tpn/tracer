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
# Type Helpers
#===============================================================================

def extract_type(self, line):

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
            t = PointerToPointerLine(line)

        elif m.is_void_pointer:
            t = VoidPointerLine(line)

        elif m.is_data_pointer:
            t = DataPointerLine(line)

    return t

#===============================================================================
# Line Classes
#===============================================================================

class BaseLine(object):
    is_numeric = False
    is_integer = False
    is_decimal = False
    is_pointer = False
    is_unnamed = False
    is_bitfield = False
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
    is_bitfield = True
    bit_position = None
    number_of_bits = None

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
        ix = line.find(' ')
        assert ix != -1
        left = line[:ix]
        #import ipdb
        #ipdb.set_trace()

class BaseStringLine(BaseLine):
    is_string = True
    size_in_bytes = 8

class CStringLine(BaseStringLine):
    pass

class WideCStringLine(BaseStringLine):
    pass

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

class Bitmap(object):
    def __init__(self, offset):
        self.offset = offset
        self.bitfields = []
        self.finalized = False
        self.last_position = None
        self.name_to_bitfield = {}
        self.last_number_of_bits = None
        self.total_number_of_bits = 0

        self._size_in_bytes = None
        self._implicit_padding_bits = None

    def add_bitfield(self, offset, name, bitfield):
        assert not self.finalized
        assert isinstance(bitfield, BitfieldLine)
        assert offset == self.offset, (offset, self.offset)

        if self.last_position is None:
            assert bitfield.bit_position == 0, bitfield.bit_position
        else:
            assert bitfield.bit_position == self.expected_next_bit_position

        assert name not in self.name_to_bitfield
        bitfield.name = name
        self.name_to_bitfield[name] = bitfield
        self.bitfields.append(bitfield)

        self.total_number_of_bits += bitfield.number_of_bits

        self.last_position = bitfield.bit_position
        self.last_number_of_bits = bitfield.number_of_bits

    def finalize(self):
        assert not self.finalized
        if self.total_number_of_bits not in (32, 64):
            if self.total_number_of_bits < 32:
                self._size_in_bytes = 4
                self._implicit_padding_bits = 32 - self.total_number_of_bits
            else:
                assert self.total_number_of_bits < 64
                self._size_in_bytes = 8
                self._implicit_padding_bits = 64 - self.total_number_of_bits
        else:
            self._size_in_bytes = self.total_number_of_bits / 8
            self._implicit_padding_bits = 0
        self.finalized = True

    @property
    def number_of_bitfields(self):
        return len(self.bitfields)

    @property
    def expected_next_bit_position(self):
        return self.last_position + self.last_number_of_bits

    @property
    def size_in_bytes(self):
        assert self.finalized
        return self._size_in_bytes

    @property
    def implicit_padding_bits(self):
        assert self.finalized
        return self._implicit_padding_bits

class Struct(StructLine):

    def __init__(self, *args, **kwds):
        StructLine.__init__(self, *args, **kwds)
        self.lines = []
        self.last_offset = None
        self.cumulative_size = 0
        self.bitmaps = []
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
        self.line_types = []
        self.last_line_was_bitfield = False
        self.bitmaps_by_offset = {}
        self.active_bitmap = None
        self.last_bitmap = None
        self.last_bitmap_offset = None
        self.expected_next_offsets = defaultdict(list)
        self.field_sizes_by_offset = defaultdict(list)


    def add_line(self, line):

        if not line.startswith('   +0x'):
            return

        line = line[4:]
        (left, right) = line.split(' : ')
        (offset, field_name) = left.rstrip().split(' ')
        offset = int(offset, 16)
        if self.last_offset:
            assert self.last_offset <= offset, (self.last_offset, offset)

        is_new_offset = offset in self.offsets
        self.offsets[offset] = line

        t = extract_type(right)

        if not t.is_bitfield:
            self.line_types.append(t)

        # Bitmap/bitfield processing.

        m = Mutex()

        m.is_first_bitfield = (
            t.is_bitfield and
            not self.last_line_was_bitfield
        )

        m.is_bitfield_continuation = (
            t.is_bitfield and
            self.last_line_was_bitfield
        )

        m.need_to_finalize_bitmap = (
            not t.is_bitfield and
            self.last_line_was_bitfield
        )

        m.no_bitfield_action_required = (
            not t.is_bitfield and
            not self.last_line_was_bitfield
        )

        try:
            field_size_in_bytes = t.size_in_bytes
        except AttributeError:
            field_size_in_bytes = 0

        with m:
            if m.is_first_bitfield:
                assert not self.active_bitmap
                self.active_bitmap = Bitmap(offset)
                self.active_bitmap.add_bitfield(offset, field_name, t)
                self.last_line_was_bitfield = True

            elif m.is_bitfield_continuation:
                assert offset == self.last_offset, (offset, self.last_offset)
                assert self.last_line_was_bitfield
                self.active_bitmap.add_bitfield(offset, field_name, t)

            elif m.need_to_finalize_bitmap:
                bitmap = self.active_bitmap
                bitmap.finalize()
                self.active_bitmap = None
                self.last_bitmap = bitmap
                self.last_bitmap_offset = offset
                self.bitmaps.append(bitmap)
                self.bitmaps_by_offset[offset] = bitmap

                # Add the bitmap's size to the last offset.
                size = bitmap.size_in_bytes
                self.field_sizes_by_offset[self.last_offset].append(size)

                self.last_line_was_bitfield = False

            elif m.no_bitfield_action_required:
                pass

        if field_size_in_bytes:
            assert not t.is_bitfield
            self.field_sizes_by_offset[offset].append(field_size_in_bytes)
        else:
            assert t.is_bitfield

        self.last_offset = offset

        return t

    @classmethod
    def load(cls, text):
        lines = text.splitlines()

        first_lineno = None

        for (i, line) in enumerate(lines):
            if not line.startswith('struct _'):
                continue
            first_lineno = i
            break

        assert first_lineno is not None

        struct = cls(lines[first_lineno])

        remaining = lines[first_lineno+1:]

        for line in remaining:
            struct.add_line(line)

        return struct


#===============================================================================
# Functions
#===============================================================================


#===============================================================================
# Helpers
#===============================================================================


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
