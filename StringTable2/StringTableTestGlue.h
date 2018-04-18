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

#define MAKE_STRING(Name) STRING Name = RTL_CONSTANT_STRING(#Name)
#define DECLARE_STRING(Name) extern STRING Name

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
// Dummy Test Strings
////////////////////////////////////////////////////////////////////////////////

DECLARE_STRING(falcon);
DECLARE_STRING(tomcat);
DECLARE_STRING(warthog);
DECLARE_STRING(viper);
DECLARE_STRING(fox);
DECLARE_STRING(fox1);
DECLARE_STRING(fox2);
DECLARE_STRING(lightning_storm);
DECLARE_STRING(really_bad_lightning_storm);

DECLARE_STRING(a);
DECLARE_STRING(ab);
DECLARE_STRING(abc);
DECLARE_STRING(abcd);
DECLARE_STRING(abcde);
DECLARE_STRING(abcdef);
DECLARE_STRING(abcdefg);
DECLARE_STRING(abcdefgh);
DECLARE_STRING(abcdefghi);
DECLARE_STRING(abcdefghij);
DECLARE_STRING(abcdefghijk);
DECLARE_STRING(abcdefghijkl);
DECLARE_STRING(abcdefghijklm);
DECLARE_STRING(abcdefghijklmn);
DECLARE_STRING(abcdefghijklmno);
DECLARE_STRING(abcdefghijklmnop);
DECLARE_STRING(abcdefghijklmnopq);
DECLARE_STRING(abcdefghijklmnopqr);
DECLARE_STRING(abcdefghijklmnopqrs);
DECLARE_STRING(abcdefghijklmnopqrst);
DECLARE_STRING(abcdefghijklmnopqrstu);
DECLARE_STRING(abcdefghijklmnopqrstuv);
DECLARE_STRING(abcdefghijklmnopqrstuvw);
DECLARE_STRING(abcdefghijklmnopqrstuvwy);
DECLARE_STRING(abcdefghijklmnopqrstuvwyx);
DECLARE_STRING(abcdefghijklmnopqrstuvwyxz);


DECLARE_STRING(abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghij);
DECLARE_STRING(abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklm);
DECLARE_STRING(abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmn);
DECLARE_STRING(bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghij);
DECLARE_STRING(bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmn);
DECLARE_STRING(bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmno);

#define ABC_LONG \
    abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghij

#define ABCD_LONG \
    abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklm

#define ABCD_LONGEST \
    abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmn

#define BCD_LONG \
    bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghij

#define BCDE_LONG \
    bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmn

#define BCDE_LONGEST \
    bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmno

DECLARE_STRING(bcdefgefh);
DECLARE_STRING(efghijklmn);
DECLARE_STRING(ghibdefjf);
DECLARE_STRING(jklmnopqrst);
DECLARE_STRING(klmn);
DECLARE_STRING(klmnopqrstu);
DECLARE_STRING(lmnopqrstuv);
DECLARE_STRING(lmnopqr);
DECLARE_STRING(lmno);
DECLARE_STRING(mnopqrstu);
DECLARE_STRING(mnopqrstuvw);
DECLARE_STRING(nop);
DECLARE_STRING(nopqrstuvwy);

DECLARE_STRING(fourteen141414);
DECLARE_STRING(fifteen15151515);
DECLARE_STRING(sixteen161616161);
DECLARE_STRING(seventeen17171717);
DECLARE_STRING(eighteen1818181818);


////////////////////////////////////////////////////////////////////////////////
// Example Data
////////////////////////////////////////////////////////////////////////////////

#undef DSTR
#define DSTR(String) String ";"

extern const CHAR StringTableDelimiter;
extern const STRING NtfsReservedNames;
extern const PCSZ NtfsReservedNamesCStrings[];

#define DECLARE_NTFS1(Id, Value) extern const STRING Ntfs##Id##Name
#define MAKE_NTFS1(Id, Value) \
    const STRING Ntfs##Id##Name = RTL_CONSTANT_STRING(Value)

DECLARE_NTFS1(AttrDef,         "$AttrDef");
DECLARE_NTFS1(BadClus,         "$BadClus");
DECLARE_NTFS1(Bitmap,          "$Bitmap");
DECLARE_NTFS1(Boot,            "$Boot");
DECLARE_NTFS1(Extend,          "$Extend");
DECLARE_NTFS1(LogFile,         "$LogFile");
DECLARE_NTFS1(MftMirr,         "$MftMirr");
DECLARE_NTFS1(Mft,             "$Mft");
DECLARE_NTFS1(Secure,          "$Secure");
DECLARE_NTFS1(UpCase,          "$UpCase");
DECLARE_NTFS1(Volume,          "$Volume");
DECLARE_NTFS1(Cairo,           "$Cairo");
DECLARE_NTFS1(IndexAllocation, "$INDEX_ALLOCATION");
DECLARE_NTFS1(Data,            "$DATA");
DECLARE_NTFS1(Unknown,         "????");
DECLARE_NTFS1(Dot,             ".");
DECLARE_NTFS1(WorstCase,       "$Bai123456789012");

#define NTFS_WORST_CASE_TEST_INPUT() { -1, (PSTRING)&NtfsWorstCaseName }

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

//
// Slight twist with lots of $MftMirr variants.
//

extern const STRING Ntfs2ReservedNames;

#define DECLARE_NTFS2(Id, Value) extern const STRING Ntfs2##Id##Name
#define MAKE_NTFS2(Id, Value) \
    const STRING Ntfs2##Id##Name = RTL_CONSTANT_STRING(Value)

DECLARE_NTFS2(MftMirr1234567890, "$MftMirr1234567890");
DECLARE_NTFS2(MftMirr123456789, "$MftMirr123456789");
DECLARE_NTFS2(MftMirr12345678, "$MftMirr12345678");
DECLARE_NTFS2(MftMirr1234567, "$MftMirr1234567");
DECLARE_NTFS2(MftMirr123456, "$MftMirr123456");
DECLARE_NTFS2(MftMirr12345, "$MftMirr12345");
DECLARE_NTFS2(MftMirr1234, "$MftMirr1234");
DECLARE_NTFS2(MftMirr123, "$MftMirr123");
DECLARE_NTFS2(MftMirr12, "$MftMirr12");
DECLARE_NTFS2(MftMirr1, "$MftMirr1");
DECLARE_NTFS2(MftMirr, "$MftMirr");
DECLARE_NTFS2(Mft, "$Mft");
DECLARE_NTFS2(IndexAllocation, "$INDEX_ALLOCATION");
DECLARE_NTFS2(Data, "$DATA");
DECLARE_NTFS2(Unknown, "????");
DECLARE_NTFS2(Dot, ".");

typedef enum _NTFS2_RESERVED_NAME_ID {
    NotNtfs2ReservedName = -1,
    Ntfs2MftMirr1234567890 = 0,
    Ntfs2MftMirr123456789,
    Ntfs2MftMirr12345678,
    Ntfs2MftMirr1234567,
    Ntfs2MftMirr123456,
    Ntfs2MftMirr12345,
    Ntfs2MftMirr1234,
    Ntfs2MftMirr123,
    Ntfs2MftMirr12,
    Ntfs2MftMirr1,
    Ntfs2MftMirr,
    Ntfs2Mft,
    Ntfs2IndexAllocation,
    Ntfs2Data,
    Ntfs2Unknown,
    Ntfs2Dot,
} NTFS2_RESERVED_NAME_ID;

//
// Declare various constants defined in StringTableTestGlue.c.
//

extern const STRING_TABLE_FUNCTION_OFFSET IsPrefixFunctions[];
extern const ULONG NumberOfIsPrefixFunctions;

//
// Ntfs Test Inputs
//

#define NTFS_TEST_INPUT(N) { Ntfs##N, (PSTRING)&Ntfs##N##Name }

extern const STRING_TABLE_TEST_INPUT NtfsTestInputs[];
extern const ULONG NumberOfNtfsTestInputs;
extern STRING_ARRAY16 NtfsStringArray16;

//
// Ntfs2 Test Inputs
//

#define NTFS2_TEST_INPUT(N) { Ntfs2##N, (PSTRING)&Ntfs2##N##Name }

extern const STRING_TABLE_TEST_INPUT Ntfs2TestInputs[];
extern const ULONG NumberOfNtfs2TestInputs;
extern STRING_ARRAY16 Ntfs2StringArray16;

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
