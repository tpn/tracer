/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    StringTableTestGlue.h

Abstract:

    This module provides various structures, constructs and type definitions
    that can be useful when writing unit tests or custom load generators for
    performance profiling.


--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

//
// The following STRING_ARRAY structures are useful for creating static arrays
// of a given size.  They also allow the member strings to be viewed easier
// whilst debugging as the 'STRING Strings[]' array is sized accordingly.
//

typedef struct _STRING_ARRAY2 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[2];
} STRING_ARRAY2, *PSTRING_ARRAY2, **PPSTRING_ARRAY2;

typedef struct _STRING_ARRAY3 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[3];
} STRING_ARRAY3, *PSTRING_ARRAY3, **PPSTRING_ARRAY3;

typedef struct _STRING_ARRAY4 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[4];
} STRING_ARRAY4, *PSTRING_ARRAY4, **PPSTRING_ARRAY4;

typedef struct _STRING_ARRAY5 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[5];
} STRING_ARRAY5, *PSTRING_ARRAY5, **PPSTRING_ARRAY5;

typedef struct _STRING_ARRAY6 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[6];
} STRING_ARRAY6, *PSTRING_ARRAY6, **PPSTRING_ARRAY6;

typedef struct _STRING_ARRAY7 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[7];
} STRING_ARRAY7, *PSTRING_ARRAY7, **PPSTRING_ARRAY7;

typedef struct _STRING_ARRAY8 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[8];
} STRING_ARRAY8, *PSTRING_ARRAY8, **PPSTRING_ARRAY8;

typedef struct _STRING_ARRAY9 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[9];
} STRING_ARRAY9, *PSTRING_ARRAY9, **PPSTRING_ARRAY9;

typedef struct _STRING_ARRAY10 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[10];
} STRING_ARRAY10, *PSTRING_ARRAY10, **PPSTRING_ARRAY10;

typedef struct _STRING_ARRAY11 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[11];
} STRING_ARRAY11, *PSTRING_ARRAY11, **PPSTRING_ARRAY11;

typedef struct _STRING_ARRAY12 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[12];
} STRING_ARRAY12, *PSTRING_ARRAY12, **PPSTRING_ARRAY12;

typedef struct _STRING_ARRAY13 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[13];
} STRING_ARRAY13, *PSTRING_ARRAY13, **PPSTRING_ARRAY13;

typedef struct _STRING_ARRAY14 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[14];
} STRING_ARRAY14, *PSTRING_ARRAY14, **PPSTRING_ARRAY14;

typedef struct _STRING_ARRAY15 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[15];
} STRING_ARRAY15, *PSTRING_ARRAY15, **PPSTRING_ARRAY15;

typedef struct _STRING_ARRAY16 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[16];
} STRING_ARRAY16, *PSTRING_ARRAY16, **PPSTRING_ARRAY16;

typedef struct _STRING_ARRAY17 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[17];
} STRING_ARRAY17, *PSTRING_ARRAY17, **PPSTRING_ARRAY17;

typedef struct _STRING_ARRAY18 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[18];
} STRING_ARRAY18, *PSTRING_ARRAY18, **PPSTRING_ARRAY18;

typedef struct _STRING_ARRAY19 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[19];
} STRING_ARRAY19, *PSTRING_ARRAY19, **PPSTRING_ARRAY19;

typedef struct _STRING_ARRAY20 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[20];
} STRING_ARRAY20, *PSTRING_ARRAY20, **PPSTRING_ARRAY20;

typedef struct _STRING_ARRAY21 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[21];
} STRING_ARRAY21, *PSTRING_ARRAY21, **PPSTRING_ARRAY21;

typedef struct _STRING_ARRAY22 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[22];
} STRING_ARRAY22, *PSTRING_ARRAY22, **PPSTRING_ARRAY22;

typedef struct _STRING_ARRAY23 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[23];
} STRING_ARRAY23, *PSTRING_ARRAY23, **PPSTRING_ARRAY23;

typedef struct _STRING_ARRAY24 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[24];
} STRING_ARRAY24, *PSTRING_ARRAY24, **PPSTRING_ARRAY24;

typedef struct _STRING_ARRAY25 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[25];
} STRING_ARRAY25, *PSTRING_ARRAY25, **PPSTRING_ARRAY25;

typedef struct _STRING_ARRAY26 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[26];
} STRING_ARRAY26, *PSTRING_ARRAY26, **PPSTRING_ARRAY26;

typedef struct _STRING_ARRAY27 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[27];
} STRING_ARRAY27, *PSTRING_ARRAY27, **PPSTRING_ARRAY27;

typedef struct _STRING_ARRAY28 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[28];
} STRING_ARRAY28, *PSTRING_ARRAY28, **PPSTRING_ARRAY28;

typedef struct _STRING_ARRAY29 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[29];
} STRING_ARRAY29, *PSTRING_ARRAY29, **PPSTRING_ARRAY29;

typedef struct _STRING_ARRAY30 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[30];
} STRING_ARRAY30, *PSTRING_ARRAY30, **PPSTRING_ARRAY30;

typedef struct _STRING_ARRAY31 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[31];
} STRING_ARRAY31, *PSTRING_ARRAY31, **PPSTRING_ARRAY31;

typedef struct _STRING_ARRAY32 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    struct _STRING_TABLE *StringTable;
    STRING Strings[32];
} STRING_ARRAY32, *PSTRING_ARRAY32, **PPSTRING_ARRAY32;

////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////

#define INIT_ALLOCATOR(Allocator) RtlInitializeHeapAllocator(Allocator)
#define DESTROY_ALLOCATOR(Allocator) RtlDestroyHeapAllocator(Allocator)

#define MAKE_STRING(Name) static STRING Name = RTL_CONSTANT_STRING(#Name)

#define MAKE_TABLE(Array)                          \
    Api->CreateStringTable(Rtl,                    \
                           (PALLOCATOR)Allocator,  \
                           (PALLOCATOR)Allocator,  \
                           (PSTRING_ARRAY)(Array), \
                           TRUE)

#define DESTROY_TABLE(StringTable) \
    Api->DestroyStringTable(       \
        Allocator,                 \
        Allocator,                 \
        StringTable                \
    );                             \
    StringTable = NULL


#define CONSTANT_STRING_ARRAY(S1) { \
    0, 1, 0, 0, NULL, { S1 }        \
}

#define CONSTANT_STRING_ARRAY2(S1, S2) { \
    0, 2, 0, 0, NULL, { S1, S2 }         \
}

#define CONSTANT_STRING_ARRAY3(S1, S2, S3) { \
    0, 3, 0, 0, NULL, { S1, S2, S3 }         \
}

#define CONSTANT_STRING_ARRAY5(S1, S2, S3, S4, S5) { \
    0, 5, 0, 0, NULL, { S1, S2, S3, S4, S5 }         \
}

#define CONSTANT_STRING_ARRAY6(S1, S2, S3, S4, S5, S6) { \
    0, 6, 0, 0, NULL, { S1, S2, S3, S4, S5, S6 }         \
}

#define CONSTANT_STRING_ARRAY7(S1, S2, S3, S4, S5, S6, S7) { \
    0, 7, 0, 0, NULL, { S1, S2, S3, S4, S5, S6, S7 }         \
}

#define CONSTANT_STRING_ARRAY8(S1, S2, S3, S4, S5, S6, S7, S8) { \
    0, 8, 0, 0, NULL, { S1, S2, S3, S4, S5, S6, S7, S8 }         \
}

#define CONSTANT_STRING_ARRAY9(S1, S2, S3, S4, S5, S6, S7, S8, S9) { \
    0, 9, 0, 0, NULL, { S1, S2, S3, S4, S5, S6, S7, S8, S9 }         \
}

#define CONSTANT_STRING_ARRAY16(S1,  S2,  S3,  S4,  S5,  S6,  S7,  S8,  S9, \
                                S10, S11, S12, S13, S14, S15, S16) {        \
    0, 16, 0, 0, NULL, {                                                    \
        S1,  S2,  S3,  S4,  S5,  S6,  S7,  S8,  S9,                         \
        S10, S11, S12, S13, S14, S15, S16                                   \
    }                                                                       \
}


#define CONSTANT_STRING_ARRAY18(S1,  S2,  S3,  S4,  S5,  S6,  S7,  S8,  S9,    \
                                S10, S11, S12, S13, S14, S15, S16, S17, S18) { \
    0, 18, 0, 0, NULL, {                                                       \
        S1,  S2,  S3,  S4,  S5,  S6,  S7,  S8,  S9,                            \
        S10, S11, S12, S13, S14, S15, S16, S17, S18                            \
    }                                                                          \
}

////////////////////////////////////////////////////////////////////////////////
// Example Data
////////////////////////////////////////////////////////////////////////////////

#undef DSTR
#define DSTR(String) String ";"

static const CHAR StringTableDelimiter = ';';

static const STRING NtfsReservedNames = RTL_CONSTANT_STRING(
    DSTR("$AttrDef")
    DSTR("$BadClus")
    DSTR("$Bitmap")
    DSTR("$Boot")
    DSTR("$Extend")
    DSTR("$LogFile")
    DSTR("$MftMirr")
    DSTR("$Mft")
    DSTR("$Secure")
    DSTR("$UpCase")
    DSTR("$Volume")
    DSTR("$Cairo")
    DSTR("$INDEX_ALLOCATION")
    DSTR("$DATA")
    DSTR("????")
    DSTR(".")
);

static const PCSZ NtfsReservedNamesCStrings[] = {
    "$AttrDef",
    "$BadClus",
    "$Bitmap",
    "$Boot",
    "$Extend",
    "$LogFile",
    "$MftMirr",
    "$Mft",
    "$Secure",
    "$UpCase",
    "$Volume",
    "$Cairo",
    "$INDEX_ALLOCATION",
    "$DATA",
    "????",
    ".",
    NULL
};

static const STRING NtfsAttrDefName = RTL_CONSTANT_STRING("$AttrDef");
static const STRING NtfsBadClusName = RTL_CONSTANT_STRING("$BadClus");
static const STRING NtfsBitmapName = RTL_CONSTANT_STRING("$Bitmap");
static const STRING NtfsBootName = RTL_CONSTANT_STRING("$Boot");
static const STRING NtfsExtendName = RTL_CONSTANT_STRING("$Extend");
static const STRING NtfsLogFileName = RTL_CONSTANT_STRING("$LogFile");
static const STRING NtfsMftMirrName = RTL_CONSTANT_STRING("$MftMirr");
static const STRING NtfsMftName = RTL_CONSTANT_STRING("$Mft");
static const STRING NtfsSecureName = RTL_CONSTANT_STRING("$Secure");
static const STRING NtfsUpCaseName = RTL_CONSTANT_STRING("$UpCase");
static const STRING NtfsVolumeName = RTL_CONSTANT_STRING("$Volume");
static const STRING NtfsCairoName = RTL_CONSTANT_STRING("$Cairo");
static const STRING NtfsIndexAllocationName = RTL_CONSTANT_STRING("$INDEX_ALLOCATION");
static const STRING NtfsDataName = RTL_CONSTANT_STRING("$DATA");
static const STRING NtfsUnknownName = RTL_CONSTANT_STRING("????");
static const STRING NtfsDotName = RTL_CONSTANT_STRING(".");

typedef enum _NTFS_RESERVED_NAME_ID {
    NotNtfsReservedName = -1,
    NtfsAttrDef = 0,
    NtfsBadClus,
    NtfsBitmap,
    NtfsBoot,
    NtfsExtend,
    NtfsLogFile,
    NtfsMftMirr,
    NtfsMft,
    NtfsSecure,
    NtfsUpCase,
    NtfsVolume,
    NtfsCairo,
    NtfsIndexAllocation,
    NtfsData,
    NtfsUnknown,
    NtfsDot,
} NTFS_RESERVED_NAME_ID;


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
