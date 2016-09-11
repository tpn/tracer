        title "StringTable"
        option casemap:none
;++
;
; Copyright (c) Trent Nelson, 2016.
;
; Module Name:
;
;   StringTable.inc
;
; Abstract:
;
;   This is an include module for the StringTable component.
;
;--

include ksamd64.inc

CHAR typedef byte
PCHAR typedef ptr byte
USHORT typedef word
WCHAR typedef word
PVOID typedef dq
LONG typedef dw
ULONG typedef dw
ULONG64 typedef dq
ULONGLONG typedef dq
PIS_PREFIX_OF_STRING_IN_TABLE typedef ptr dq

STRING struct
    Length_ USHORT ?
    MaximumLength USHORT ?
    Hash ULONG ?
    Buffer PCHAR ?
STRING ends

PSTRING typedef ptr STRING

;
; Structure definitions.  These generally map 1:1 with their StringTable.h
; counterparts.
;

WIDE_CHARACTER union
    struct
        LowPart CHAR ?
        HighPart CHAR ?
    ends
    WidePart WCHAR ?
WIDE_CHARACTER ends
PWIDE_CHARACTER typedef ptr WIDE_CHARACTER

STRING_ARRAY struct
    SizeInQuadwords USHORT ?
    NumberOfElements USHORT ?
    MinimumLength USHORT ?
    MaximumLength USHORT ?
    Strings STRING 1 dup ({?})
STRING_ARRAY ends
PSTRING_ARRAY typedef ptr STRING_ARRAY

STRING_SLOT union 16
    Char CHAR 16 dup (?)
    WideChar WIDE_CHARACTER 8 dup ({?})
STRING_SLOT ends
PSTRING_SLOT typedef ptr STRING_SLOT

SLOT_LENGTHS union 16
    struct
        Lengths USHORT 16 dup (?)
    ends
    struct
        union
            Slot0  USHORT ?
            Slot1  USHORT ?
            Slot2  USHORT ?
            Slot3  USHORT ?
            Slot4  USHORT ?
            Slot5  USHORT ?
            Slot6  USHORT ?
            Slot7  USHORT ?
        ends
        union
            Slot8  USHORT ?
            Slot9  USHORT ?
            Slot10 USHORT ?
            Slot11 USHORT ?
            Slot12 USHORT ?
            Slot13 USHORT ?
            Slot14 USHORT ?
            Slot15 USHORT ?
        ends
    ends
SLOT_LENGTHS ends

STRING_MATCH struct
    Index LONG ?
    NumberOfMatchedCharacters USHORT ?
    Padding USHORT ?
    String PSTRING ?
STRING_MATCH ends

STRING_TABLE struct
    OccupiedBitmap USHORT ?
    ContinuationBitmap USHORT ?
    HorizontalDepth BYTE ?
    VerticalDepth BYTE ?
    NextHorizontalOffsetInQuadwords BYTE ?
    NextVerticalOffsetInQuadwords BYTE ?
    pStringArray PSTRING_ARRAY ?
    FirstChars STRING_SLOT { }
    Lengths SLOT_LENGTHS { }
    Slots STRING_SLOT 16 dup ({ })
    IsPrefixOfStringInTable PIS_PREFIX_OF_STRING_IN_TABLE ?
    Flags ULONGLONG ?
    union
        StringArray STRING_ARRAY {?}
        Padding CHAR 172 dup (?)
    ends
STRING_TABLE ends

echo Size of STRING_TABLE: %(sizeof STRING_TABLE)

PSTRING_TABLE typedef ptr STRING_TABLE

PARAMS struct
    ReturnAddress dq ?
    union
        HomeRcx dq ?
        Param1  dq ?
    ends

    union
        HomeRdx dq ?
        Param2  dq ?
    ends

    union
        HomeR8  dq ?
        Param3  dq ?
    ends

    union
        HomeR9  dq ?
        Param4  dq ?
    ends
PARAMS ends

HOME_PARAMS struct
    ReturnAddress           dq      ?       ; 8     32      40      (28h)
    HomeRcx                 dq      ?       ; 8     24      32      (20h)
    HomeRdx                 dq      ?       ; 8     16      24      (18h)
    HomeR8                  dq      ?       ; 8     8       16      (10h)
    HomeR9                  dq      ?       ; 8     0       8       (08h)
HOME_PARAMS ends

; vim:set tw=80 ts=8 sw=4 sts=4 expandtab syntax=masm formatoptions=croql      :

end
