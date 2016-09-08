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

STRING struct
    Length_ USHORT ?
    MaximumLength USHORT ?
    Hash ULONG ?
    Buffer PCHAR ?
STRING ends

PSTRING typedef ptr STRING

;
; WideChar structure definition.  Maps to WIDE_CHARACTER in C.
;

WIDE_CHAR union
    struct
        LowPart CHAR ?
        HighPart CHAR ?
    ends
    WidePart WCHAR ?
WIDE_CHAR ends

PWIDE_CHAR typedef ptr WIDE_CHAR

;
; String slot structure.
;

;STRING_SLOT typedef CHAR 16 dup ?

;
; StringTable structure definition.  This maps to the STRING_TABLE structure
; defined in C in the StringTable.h module.
;

STRING_TABLE struct
    OccupiedBitmap USHORT ?
    ContinuationBitmap USHORT ?
    HorizontalDepth BYTE ?
    VerticalDepth BYTE ?
    NextHorizontalOffsetInQuadwords BYTE ?
    NextVerticalOffsetInQuadwords BYTE ?
    StringSizes ULONGLONG ?
    FirstAndSecondCharacter CHAR 32 dup (?)
    Padding CHAR 16 dup (?)
    ;StringArray STRING_SLOT 16 dup ?
    StringArray CHAR 256 dup (?)
STRING_TABLE ends

echo Size of STRING_TABLE: %(sizeof STRING_TABLE)

PSTRING_TABLE typedef ptr STRING_TABLE

;
; StringMatch structure.  This maps to the STRING_MATCH structure defined in
; C in the StringTable.h module.
;

STRING_MATCH struct
    Index LONG ?
    NumberOfMatchedCharacters USHORT ?
    Padding USHORT ?
    String PSTRING ?
STRING_MATCH ends

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
