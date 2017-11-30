#===============================================================================
# Imports
#===============================================================================

from tracer.dbgeng import (
    Struct,
)

#===============================================================================
# Tests
#===============================================================================

struct__LINKED_STRING_C = """typedef struct _LINKED STRING {\
    LIST_ENTRY ListEntry;
    union {
        struct {
            USHORT Length;
            USHORT MaximumLength;
            LONG   Hash;
            union {
                PCHAR Buffer;
                PWCHAR WideBuffer;
            };
        };
        STRING String;
        UNICODE_STRING Unicode;
    };
} LINKED_STRING;"""

struct__LINKED_STRING = """0:000> dt -v Rtl!_LINKED_STRING
struct _LINKED_STRING, 8 elements, 0x20 bytes
   +0x000 ListEntry        : struct _LIST_ENTRY, 2 elements, 0x10 bytes
   +0x010 Length           : Uint2B
   +0x012 MaximumLength    : Uint2B
   +0x014 Hash             : Int4B
   +0x018 Buffer           : Ptr64 to Char
   +0x018 WideBuffer       : Ptr64 to Wchar
   +0x010 String           : struct _STRING, 5 elements, 0x10 bytes
   +0x010 Unicode          : struct _UNICODE_STRING, 5 elements, 0x10 bytes"""


_LINKED_STRING_ctypes_decl = """\
class _LINKED_STRING_INNER_UNION_1(Union):
    pass

class _LINKED_STRING_INNER_STRUCT_1(Structure):
    _anonymous_ = ('u1',)

class _LINKED_STRING_INNER_UNION_2(Union):
    _anonymous_ = ('s1',)

class LINKED_STRING(Structure):
    _anonymous_ = ('u1',)
PLINKED_STRING = POINTER(LINKED_STRING)"""

_LINKED_STRING_ctypes_def = """\
_LINKED_STRING_INNER_UNION_1._fields_ = [
    ('Buffer', PCHAR),
    ('WideBuffer', PWCHAR),
]

_LINKED_STRING_INNER_STRUCT_1._fields_ = [
    ('Length', USHORT),
    ('MaximumLength', USHORT),
    ('Hash', LONG),
    ('u1', _LINKED_STRING_INNER_UNION_1),
]

_LINKED_STRING_INNER_UNION_2._fields_ = [
    ('s1', _LINKED_STRING_INNER_STRUCT_1),
    ('String', STRING),
    ('Unicode', UNICODE_STRING),
]

LINKED_STRING._fields_ = [
    ('ListEntry', LIST_ENTRY),
    ('u1', _LINKED_STRING_INNER_UNION_2),
]"""

def test_linked_string():
    s = Struct.load(struct__LINKED_STRING)
    decl = s.get_ctypes_decl()
    defi = s.get_ctypes_def()

    assert decl == _LINKED_STRING_ctypes_decl
    assert defi == _LINKED_STRING_ctypes_defi

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
