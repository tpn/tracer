        title "StringTable"
        option nokeyword:<Length>

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

IACA_VC_START macro Name

        mov     byte ptr gs:[06fh], 06fh

        endm

IACA_VC_END macro Name

        mov     byte ptr gs:[0deh], 0deh

        endm

CHAR typedef byte
PCHAR typedef ptr byte
USHORT typedef word
WCHAR typedef word
PVOID typedef qword
LONG typedef dword
ULONG typedef dword
ULONG64 typedef qword
ULONGLONG typedef qword
PIS_PREFIX_OF_STRING_IN_TABLE typedef ptr qword

STRING struct 1
    Length USHORT ?
    MaximumLength USHORT ?
    Hash ULONG ?
    Buffer PCHAR ?
STRING ends
.erre (size STRING eq 16), @CatStr(<IncorrectStructSize: STRING: >, %(size STRING))

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
    StringTable PVOID ?
    Strings STRING 1 dup ({?})
STRING_ARRAY ends
PSTRING_ARRAY typedef ptr STRING_ARRAY

STRING_SLOT union 16
    Chars CHAR 16 dup (?)
    WideChars WIDE_CHARACTER 8 dup ({?})
STRING_SLOT ends
PSTRING_SLOT typedef ptr STRING_SLOT

SLOT_LENGTHS struct 16
    Lengths BYTE 16 dup (?)
SLOT_LENGTHS ends

SLOT_INDEX struct 16
    Index BYTE 16 dup (?)
SLOT_INDEX ends

STRING_MATCH struct
    Index BYTE ?
    NumberOfMatchedCharacters BYTE ?
    Padding USHORT 3 dup (?)
    String PSTRING ?
STRING_MATCH ends

STRING_TABLE struct
    UniqueChars         CHAR 16 dup  (?)
    UniqueIndex         BYTE 16 dup  (?)
    Lengths             BYTE 16 dup  (?)
    pStringArray        PSTRING_ARRAY ?
    Flags               ULONG         ?
    OccupiedBitmap      USHORT        ?
    ContinuationBitmap  USHORT        ?
    Slots               STRING_SLOT 16 dup ({ })
    union
        StringArray STRING_ARRAY {?}
        Padding CHAR 192 dup (?)
    ends
STRING_TABLE ends

;
; Assert our critical field offsets and structure size as per the same approach
; taken in StringTable.h.
;

.erre (STRING_TABLE.UniqueIndex  eq  16), @CatStr(<UnexpectedOffset STRING_TABLE.UniqueIndex: >, %(STRING_TABLE.UniqueIndex))
.erre (STRING_TABLE.Lengths      eq  32), @CatStr(<UnexpectedOffset STRING_TABLE.Lengths: >, %(STRING_TABLE.Lengths))
.erre (STRING_TABLE.pStringArray eq  48), @CatStr(<UnexpectedOffset STRING_TABLE.pStringArray: >, %(STRING_TABLE.pStringArray))
.erre (STRING_TABLE.Slots        eq  64), @CatStr(<UnexpectedOffset STRING_TABLE.Slots: >, %(STRING_TABLE.Slots))
.erre (STRING_TABLE.Padding      eq 320), @CatStr(<UnexpectedOffset STRING_TABLE.Padding: >, %(STRING_TABLE.Padding))
.erre (size STRING_TABLE eq 512), @CatStr(<IncorrectStructSize: STRING_TABLE: >, %(size STRING_TABLE))

PSTRING_TABLE typedef ptr STRING_TABLE

;
; Helper typedefs.
;

StringTable typedef STRING_TABLE
StringArray typedef STRING_ARRAY
StringMatch typedef STRING_MATCH

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

Home typedef HOME_PARAMS

; vim:set tw=80 ts=8 sw=4 sts=4 expandtab syntax=masm                          :

