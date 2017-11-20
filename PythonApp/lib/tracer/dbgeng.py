#===============================================================================
# Imports
#===============================================================================

from .logic import (
    Mutex,
)

from .util import (
    defaultdict,

    Dict,
    OrderedDict,
    OrderedDefaultDict,
)

#===============================================================================
# Globals/Aliases
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
    'HResult',
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

class InvalidHResultLine(InvalidLine):
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

def extract_type(line):

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
    m.is_hresult = (first == 'HRESULT')
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

        elif m.is_hresult:
            t = HResultLine(line)

        elif m.is_long:
            t = SignedLongLine(line)

        elif m.is_ulong:
            t = UnsignedLongLine(line)

        elif m.is_longlong:
            t = SignedLongLongLine(line)

        elif m.is_ulonglong:
            t = UnsignedLongLongLine(line)

        elif m.is_float:
            t = FloatLine(line)

        elif m.is_array:
            t = ArrayLine(line)

        elif m.is_cstring:
            t = CStringLine(line)

        elif m.is_wide_cstring:
            t = WideCStringLine(line)

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
# Classes
#===============================================================================

class BaseLine(object):
    name = None
    line = None
    is_numeric = False
    is_integer = False
    is_decimal = False
    is_pointer = False
    is_unnamed = False
    is_bitfield = False
    is_character = False
    is_composite = False

    __keys__ = []
    __default_keys__ = [
        'name',
        'line',
        'size_in_bytes',
        'number_of_elements',
    ]

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

    def __repr__(self):
        keys = self.__keys__ + self.__default_keys__
        keys = [
            key for key in keys if
                hasattr(self, key) and
                getattr(self, key)
        ]

        return '<%s %s>' % (
            self.__class__.__name__,
            ', '.join(
                '%s=%r' % (k, v)
                    for (k, v) in (
                        (k, getattr(self, k))
                            for k in keys
                    )
                )
        )

class BitfieldLine(BaseLine):
    is_bitfield = True
    bit_position = None
    number_of_bits = None

    __keys__ = [
        'bit_position',
        'number_of_bits',
    ]

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

    __keys__ = [
        'type_name',
        'struct_name',
    ]

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

    __keys__ = [
        'type_name',
        'struct_name',
    ]

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

    __keys__ = [
        'type_name',
        'enum_name',
        'number_of_enums',
    ]

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

class HResultLine(SignedLongLine):
    pass

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
    shape = None
    element_type = None
    size_in_bytes = None
    number_of_dimensions = None
    element_size_in_bytes = None
    total_number_of_elements = None

    __keys__ = [
        'shape',
        'element_type',
        'number_of_dimensions',
        'element_size_in_bytes',
        'total_number_of_elements',
    ]

    @classmethod
    def parse(cls, line):
        open_count = line.count('[')
        close_count = line.count(']')
        assert open_count == close_count, (open_count, close_count)
        assert line.startswith('['), line

        last = 0
        elems = 0
        count = 0
        parts = []
        while True:
            ix = line.find(' ', last+1)
            if ix == -1:
                assert count == open_count, (count, open_count)
                break
            prev = line[ix-1]
            if prev != ']':
                assert count == open_count, (count, open_count)
                break
            part = line[last+1:ix-1]
            part = int(part)
            if not elems:
                elems = part
            else:
                elems *= part
            parts.append(part)
            count = count + 1
            last = ix + 1

        prefix = '%s ' % ' '.join('[%d]' % i for i in parts)
        remaining = line.replace(prefix, '')

        shape = tuple(parts)
        number_of_dimensions = len(parts)
        element_type = extract_type(remaining)
        element_size_in_bytes = element_type.size_in_bytes
        total_number_of_elements = elems
        size_in_bytes = (
            element_size_in_bytes *
            total_number_of_elements
        )

        result = {
            'shape': shape,
            'element_type': element_type,
            'size_in_bytes': size_in_bytes,
            'number_of_dimensions': number_of_dimensions,
            'element_size_in_bytes': element_size_in_bytes,
            'total_number_of_elements': total_number_of_elements,
        }

        return result


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
        self.names = []
        self.bitfields = []
        self.finalized = False
        self.last_position = None
        self.name_to_bitfield = {}
        self.last_number_of_bits = None
        self.total_number_of_bits = 0

        self._size_in_bytes = None
        self._implicit_padding_bits = None

    def __repr__(self):
        fmt = (
            "<%s offset=%d"
               " num_bitfields=%d"
               " total_number_of_bits=%d"
        )

        values = [
            self.__class__.__name__,
            self.offset,
            len(self.bitfields),
            self.total_number_of_bits,
        ]

        if not self.finalized:
            fmt += " finalized=False"
        else:
            fmt += (
                " size_in_bytes=%d"
                " implicit_padding_bits=%d"
            )

            values += [
                self.size_in_bytes,
                self.implicit_padding_bits,
            ]

        fmt += " names=%r>"
        values.append(self.names)

        return fmt % tuple(values)

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
        self.names.append(name)
        self.name_to_bitfield[name] = bitfield
        self.bitfields.append(bitfield)

        self.total_number_of_bits += bitfield.number_of_bits

        self.last_position = bitfield.bit_position
        self.last_number_of_bits = bitfield.number_of_bits

    def finalize(self):
        assert not self.finalized
        if self.total_number_of_bits not in (8, 16, 32, 64):

            if self.total_number_of_bits < 8:
                self._size_in_bytes = 1
                self._implicit_padding_bits = 8 - self.total_number_of_bits

            elif self.total_number_of_bits < 16:
                self._size_in_bytes = 2
                self._implicit_padding_bits = 16 - self.total_number_of_bits

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

class ImplicitPadding(object):
    size_to_line = {
        1: 'UChar',
        2: 'Wchar',
        4: 'Uint4B',
        8: 'Uint8B',
    }

    def __init__(self, offset, expected_offset):
        assert offset > expected_offset, (offset, expected_offset)
        self.offset = offset
        self.expected_offset = expected_offset
        size = self.size_in_bytes = offset - expected_offset
        assert size >= 1

        try:
            line = self.size_to_line[size]
        except KeyError:
            line = '[%d] UChar' % size

        self.line = line
        self.line_type = extract_type(line)

    def __repr__(self):
        fmt = "<%s offset=%d expected=%d size_in_bytes=%d line=%s>"
        return fmt % (
            self.__class__.__name__,
            self.offset,
            self.expected_offset,
            self.size_in_bytes,
            self.line,
        )

class TrailingPadding(ImplicitPadding):
    pass

class Struct(StructLine):

    __keys__ = [
        'type_name',
        'struct_name',
        'module_name',
    ]

    def __init__(self, *args, **kwds):
        StructLine.__init__(self, *args, **kwds)
        self.lines = []
        self.dt_line = None
        self.module_name = None
        self.last_offset = None
        self.cumulative_size = 0
        self.bitmaps = []
        self.offsets = OrderedDefaultDict(list)
        self.offset_to_line_type = OrderedDefaultDict(list)
        self.offset_to_field_name = OrderedDefaultDict(list)
        self.inline_union_offsets = []
        self.inline_bitfields = {}
        self.inline_bitfields_by_offset = OrderedDefaultDict(list)
        self.enums = {}
        self.enums_by_offset = OrderedDefaultDict(list)
        self.expected_next_offset = 0
        self.line_types = []
        self.field_names = set()
        self.field_name_to_line_type = OrderedDict()
        self.last_line_was_bitfield = False
        self.bitmaps_by_offset = {}
        self.active_bitmap = None
        self.last_bitmap = None
        self.last_bitmap_offset = None
        self.expected_next_offsets = OrderedDefaultDict(list)
        self.field_sizes_by_offset = OrderedDefaultDict(list)
        self.offset_to_max_size_in_bytes = OrderedDict()
        self.implicit_paddings = OrderedDict()
        self.trailing_padding = None
        self.finalized = False

    def add_line(self, line):
        assert not self.finalized

        if not line.startswith('   +0x'):
            return

        self.lines.append(line)

        line = line[4:]
        (left, right) = line.split(' : ')
        (offset, field_name) = left.rstrip().split(' ')
        offset = int(offset, 16)

        if self.last_offset:
            #assert self.last_offset <= offset, (self.last_offset, offset)
            pass

        assert field_name not in self.field_names
        self.field_names.add(field_name)

        self.offsets[offset].append(line)

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

                size = bitmap.size_in_bytes
                self.field_sizes_by_offset[self.last_offset].append(size)
                self.offset_to_line_type[offset].append(bitmap)
                self.line_types.append(bitmap)

                self.last_line_was_bitfield = False

            elif m.no_bitfield_action_required:
                pass

        if field_size_in_bytes:
            assert not t.is_bitfield
            self.field_sizes_by_offset[offset].append(field_size_in_bytes)

            self.offset_to_line_type[offset].append(t)
            self.offset_to_field_name[offset].append(field_name)
        else:
            assert t.is_bitfield

        self.line_types.append(t)
        self.field_name_to_line_type[field_name] = t

        self.last_offset = offset

        return t

    def finalize(self):
        assert not self.finalized

        i = -1
        first = True
        last_sizes = None
        total_size = 0
        last_offset = 0
        expected_offset = 0
        is_union = False
        last_offset_was_union = False
        alternate_expected_offset = None
        offsets = self.field_sizes_by_offset.keys()
        offset_sizes = self.offset_to_max_size_in_bytes

        for offset, sizes in self.field_sizes_by_offset.items():

            i = i + 1

            if len(sizes) == 1:
                is_union = False
                size = sizes[0]
            else:
                is_union = True
                self.inline_union_offsets.append(offset)
                max_size = max(sizes)
                min_size = min(sizes)

                try:
                    next_offset = offsets[i+1]
                except IndexError:
                    next_offset = self.size_in_bytes
                size = next_offset - offset

            offset_sizes[offset] = size

            total_size += size

            if first:
                first = False
                expected_offset = last_offset + size
                continue

            if offset != expected_offset:
                assert offset > expected_offset, (offset, expected_offset)
                padding = ImplicitPadding(offset, expected_offset)
                new_expected_offset = expected_offset + padding.size_in_bytes
                assert offset == new_expected_offset
                self.implicit_paddings[expected_offset] = padding

            last_offset = offset
            expected_offset = last_offset + size

        if total_size != self.size_in_bytes:
            size_in_bytes = self.size_in_bytes
            assert size_in_bytes > total_size, (size_in_bytes, total_size)
            padding = TrailingPadding(size_in_bytes, total_size)
            new_total_size = total_size + padding.size_in_bytes
            assert new_total_size == self.size_in_bytes
            self.trailing_padding = padding

        self.finalized = True

    @property
    def has_implicit_padding(self):
        return bool(self.implicit_paddings)

    @property
    def has_trailing_padding(self):
        return bool(self.trailing_padding)

    @property
    def has_padding(self):
        return self.has_implicit_padding or self.has_trailing_padding

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

        struct.finalize()

        return struct

    @classmethod
    def load_all_from_text(cls, text):
        lines = text.splitlines()

        dt_line = None
        dt_prefix = '0:000> dt -v '
        active_struct = False
        start = 0
        end = 0

        indexes = []

        for (i, line) in enumerate(lines):

            if not active_struct:
                if line.startswith(dt_prefix):
                    dt_line = line
                elif not line.startswith('struct _'):
                    continue
                else:
                    active_struct = True
                    start_line = i
                    continue
            else:
                if not line or line.startswith('   +0x'):
                    continue
                else:
                    end_line = i
                    assert dt_line
                    assert start_line
                    indexes.append((dt_line, start_line, end_line))
                    active_struct = False
                    start_line = None
                    if line.startswith(dt_prefix):
                        dt_line = line
                    else:
                        dt_line = None


        structs = []
        module_names = set()
        by_module = defaultdict(dict)
        has_padding = list()
        has_implicit_padding = list()
        has_trailing_padding = list()

        for (dt_line, start, end) in indexes:
            (left, right) = dt_line.replace('0:000> dt -v ', '').split('!')
            struct_line = lines[start]
            struct = cls(struct_line)
            struct.module_name = left
            struct.dt_line = dt_line
            for i in range(start+1, end):
                line = lines[i]
                struct.add_line(line)
            struct.finalize()
            module_names.add(struct.name)
            by_module[struct.module_name][struct.name] = struct
            if struct.has_padding:
                has_padding.append(struct)
            structs.append(struct)

        return Dict({
            'all': structs,
            'by_module': { k: v for (k, v) in by_module.items() },
            'has_padding': has_padding,
        })

    def offset_diff(self, ctypes_struct):
        left_offsets = self.offsets
        right_offsets = OrderedDict(
            (getattr(ctypes_struct, name[0]).offset, name)
                for name in ctypes_struct._fields_
        )

        left_keys = left_offsets.keys()
        right_keys = right_offsets.keys()

        left = [ str(k) for k in left_keys ]
        right = [ str(k) for k in right_keys ]

        lines = []
        added = {}
        missing = {}

        from difflib import unified_diff
        diff = list(unified_diff(left, right, fromfile='ours', tofile='theirs'))
        for line in diff:
            if not line:
                continue

            first = line[0]

            try:
                second = line[1]
            except IndexError:
                second = None

            if not second:
                lines.append(line)
                continue

            if first == '-':
                if second == '-':
                    lines.append(line)
                    continue
                else:
                    offset = int(line[1:])
                    field_name = self.offsets[offset][0]
                    missing[offset] = field_name
                    lines.append('%s (%s)' % (line, field_name))
                    continue
            elif first == '+':
                if second == '+':
                    lines.append(line)
                    continue
                else:
                    offset = int(line[1:])
                    field_name = right_offsets[offset]
                    added[offset] = field_name
                    lines.append('%s (%s)' % (line, field_name))
                    continue
            else:
                lines.append(line)
                continue

        return {
            'diff': diff,
            'lines': lines,
            'added': added,
            'missing': missing,
        }

    def offset_and_fields_diff(self, ctypes_struct):
        left_offsets = self.offset_to_field_name
        right_offsets = OrderedDict(
            (getattr(ctypes_struct, name[0]).offset, name)
                for name in ctypes_struct._fields_
        )

        left = [
            "%s\t%s" % (offset, names[0])
                for (offset, names) in left_offsets.items()
        ]

        right = [
            "%s\t%s" % (offset, name[0])
                for (offset, name) in right_offsets.items()
        ]

        lines = []
        added = {}
        missing = {}

        from difflib import unified_diff
        diff = list(unified_diff(left, right, fromfile='ours', tofile='theirs'))
        for line in diff:
            if not line:
                continue

            first = line[0]

            try:
                second = line[1]
            except IndexError:
                second = None

            if not second:
                lines.append(line)
                continue

            if first == '-':
                if second == '-':
                    lines.append(line)
                    continue
                else:
                    offset = int(line[1:line.find('\t', 1)])
                    field_name = self.offsets[offset][0]
                    missing[offset] = field_name
                    lines.append(line)
                    continue
            elif first == '+':
                if second == '+':
                    lines.append(line)
                    continue
                else:
                    offset = int(line[1:line.find('\t', 1)])
                    field_name = right_offsets[offset]
                    added[offset] = field_name
                    lines.append(line)
                    continue
            else:
                lines.append(line)
                continue

        return {
            'diff': diff,
            'lines': lines,
            'added': added,
            'missing': missing,
        }


    def as_ctypes_struct(self):
        buf = StringIO.StringIO()

        w = lambda chunk: buf.write(chunk)
        wl = lambda line: buf.write(line + '\n')



# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
