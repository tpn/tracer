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

#ifdef __cplusplus
extern "C" {
#endif

#include "StringTableTestGlue.h"

////////////////////////////////////////////////////////////////////////////////
// Dummy Test Strings
////////////////////////////////////////////////////////////////////////////////

MAKE_STRING(falcon);
MAKE_STRING(tomcat);
MAKE_STRING(warthog);
MAKE_STRING(viper);
MAKE_STRING(fox);
MAKE_STRING(fox1);
MAKE_STRING(fox2);
MAKE_STRING(lightning_storm);
MAKE_STRING(really_bad_lightning_storm);

MAKE_STRING(a);
MAKE_STRING(ab);
MAKE_STRING(abc);
MAKE_STRING(abcd);
MAKE_STRING(abcde);
MAKE_STRING(abcdef);
MAKE_STRING(abcdefg);
MAKE_STRING(abcdefgh);
MAKE_STRING(abcdefghi);
MAKE_STRING(abcdefghij);
MAKE_STRING(abcdefghijk);
MAKE_STRING(abcdefghijkl);
MAKE_STRING(abcdefghijklm);
MAKE_STRING(abcdefghijklmn);
MAKE_STRING(abcdefghijklmno);
MAKE_STRING(abcdefghijklmnop);
MAKE_STRING(abcdefghijklmnopq);
MAKE_STRING(abcdefghijklmnopqr);
MAKE_STRING(abcdefghijklmnopqrs);
MAKE_STRING(abcdefghijklmnopqrst);
MAKE_STRING(abcdefghijklmnopqrstu);
MAKE_STRING(abcdefghijklmnopqrstuv);
MAKE_STRING(abcdefghijklmnopqrstuvw);
MAKE_STRING(abcdefghijklmnopqrstuvwy);
MAKE_STRING(abcdefghijklmnopqrstuvwyx);
MAKE_STRING(abcdefghijklmnopqrstuvwyxz);


MAKE_STRING(abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghij);
MAKE_STRING(abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklm);
MAKE_STRING(abcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmn);
MAKE_STRING(bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghij);
MAKE_STRING(bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmn);
MAKE_STRING(bcdefghijklmnopqrstuvwyxzabcdefghijklmnopqrstuvwyxzabcdefghijklmno);

MAKE_STRING(bcdefgefh);
MAKE_STRING(efghijklmn);
MAKE_STRING(ghibdefjf);
MAKE_STRING(jklmnopqrst);
MAKE_STRING(klmn);
MAKE_STRING(klmnopqrstu);
MAKE_STRING(lmnopqrstuv);
MAKE_STRING(lmnopqr);
MAKE_STRING(lmno);
MAKE_STRING(mnopqrstu);
MAKE_STRING(mnopqrstuvw);
MAKE_STRING(nop);
MAKE_STRING(nopqrstuvwy);

MAKE_STRING(fourteen141414);
MAKE_STRING(fifteen15151515);
MAKE_STRING(sixteen161616161);
MAKE_STRING(seventeen17171717);
MAKE_STRING(eighteen1818181818);


////////////////////////////////////////////////////////////////////////////////
// Example Data
////////////////////////////////////////////////////////////////////////////////

const CHAR StringTableDelimiter = ';';

const STRING NtfsReservedNames = RTL_CONSTANT_STRING(
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

const PCSZ NtfsReservedNamesCStrings[] = {
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

MAKE_NTFS1(AttrDef,         "$AttrDef");
MAKE_NTFS1(BadClus,         "$BadClus");
MAKE_NTFS1(Bitmap,          "$Bitmap");
MAKE_NTFS1(Boot,            "$Boot");
MAKE_NTFS1(Extend,          "$Extend");
MAKE_NTFS1(LogFile,         "$LogFile");
MAKE_NTFS1(MftMirr,         "$MftMirr");
MAKE_NTFS1(Mft,             "$Mft");
MAKE_NTFS1(Secure,          "$Secure");
MAKE_NTFS1(UpCase,          "$UpCase");
MAKE_NTFS1(Volume,          "$Volume");
MAKE_NTFS1(Cairo,           "$Cairo");
MAKE_NTFS1(IndexAllocation, "$INDEX_ALLOCATION");
MAKE_NTFS1(Data,            "$DATA");
MAKE_NTFS1(Unknown,         "????");
MAKE_NTFS1(Dot,             ".");
MAKE_NTFS1(WorstCase,       "$Bai123456789012");

//
// Slight twist with lots of $MftMirr variants.
//

const STRING Ntfs2ReservedNames = RTL_CONSTANT_STRING(
    DSTR("$MftMirr1234567890")
    DSTR("$MftMirr123456789")
    DSTR("$MftMirr12345678")
    DSTR("$MftMirr1234567")
    DSTR("$MftMirr123456")
    DSTR("$MftMirr12345")
    DSTR("$MftMirr1234")
    DSTR("$MftMirr123")
    DSTR("$MftMirr12")
    DSTR("$MftMirr1")
    DSTR("$MftMirr")
    DSTR("$Mft")
    DSTR("$INDEX_ALLOCATION")
    DSTR("$DATA")
    DSTR("????")
    DSTR(".")
);

MAKE_NTFS2(MftMirr1234567890, "$MftMirr1234567890");
MAKE_NTFS2(MftMirr123456789, "$MftMirr123456789");
MAKE_NTFS2(MftMirr12345678, "$MftMirr12345678");
MAKE_NTFS2(MftMirr1234567, "$MftMirr1234567");
MAKE_NTFS2(MftMirr123456, "$MftMirr123456");
MAKE_NTFS2(MftMirr12345, "$MftMirr12345");
MAKE_NTFS2(MftMirr1234, "$MftMirr1234");
MAKE_NTFS2(MftMirr123, "$MftMirr123");
MAKE_NTFS2(MftMirr12, "$MftMirr12");
MAKE_NTFS2(MftMirr1, "$MftMirr1");
MAKE_NTFS2(MftMirr, "$MftMirr");
MAKE_NTFS2(Mft, "$Mft");
MAKE_NTFS2(IndexAllocation, "$INDEX_ALLOCATION");
MAKE_NTFS2(Data, "$DATA");
MAKE_NTFS2(Unknown, "????");
MAKE_NTFS2(Dot, ".");

////////////////////////////////////////////////////////////////////////////////
// Test Functions & Input Glue
////////////////////////////////////////////////////////////////////////////////

#define DSTFO DEFINE_STRING_TABLE_FUNCTION_OFFSET

const STRING_TABLE_FUNCTION_OFFSET IsPrefixFunctions[] = {
    DSTFO(IsPrefixOfStringInTable_1,       TRUE),
    DSTFO(IsPrefixOfStringInTable_2,       TRUE),
    DSTFO(IsPrefixOfStringInTable_3,       TRUE),
    DSTFO(IsPrefixOfStringInTable_4,       TRUE),
    DSTFO(IsPrefixOfStringInTable_5,       TRUE),
    DSTFO(IsPrefixOfStringInTable_6,       TRUE),
    DSTFO(IsPrefixOfStringInTable_7,       TRUE),
    DSTFO(IsPrefixOfStringInTable_8,       TRUE),
    DSTFO(IsPrefixOfStringInTable_9,       TRUE),
    DSTFO(IsPrefixOfStringInTable_10,      TRUE),
    DSTFO(IsPrefixOfStringInTable_11,      TRUE),
    DSTFO(IsPrefixOfStringInTable_12,      TRUE),
    DSTFO(IsPrefixOfStringInTable_13,      TRUE),
    DSTFO(IsPrefixOfStringInTable_14,      TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_1,   FALSE),
    DSTFO(IsPrefixOfStringInTable_x64_2,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_3,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_4,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_5,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_6,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_7,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_8,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_9,   TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_10,  TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_11,  TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_12,  TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_13,  TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_14,  TRUE),
    DSTFO(IsPrefixOfStringInTable_x64_15,  TRUE),
    DSTFO(IntegerDivision_x64_1,           FALSE),
};

const ULONG NumberOfIsPrefixFunctions = ARRAYSIZE(IsPrefixFunctions);

//
// Test inputs.
//

STRING_ARRAY16 NtfsStringArray16 = CONSTANT_STRING_ARRAY16(
    RTL_CONSTANT_STRING("$AttrDef"),
    RTL_CONSTANT_STRING("$BadClus"),
    RTL_CONSTANT_STRING("$Bitmap"),
    RTL_CONSTANT_STRING("$Boot"),
    RTL_CONSTANT_STRING("$Extend"),
    RTL_CONSTANT_STRING("$LogFile"),
    RTL_CONSTANT_STRING("$MftMirr"),
    RTL_CONSTANT_STRING("$Mft"),
    RTL_CONSTANT_STRING("$Secure"),
    RTL_CONSTANT_STRING("$UpCase"),
    RTL_CONSTANT_STRING("$Volume"),
    RTL_CONSTANT_STRING("$Cairo"),
    RTL_CONSTANT_STRING("$INDEX_ALLOCATION"),
    RTL_CONSTANT_STRING("$DATA"),
    RTL_CONSTANT_STRING("????"),
    RTL_CONSTANT_STRING(".")
);

//
// Ntfs Test Inputs
//

#define NTFS_TEST_INPUT(N) { Ntfs##N, (PSTRING)&Ntfs##N##Name }

const STRING_TABLE_TEST_INPUT NtfsTestInputs[] = {
    NTFS_TEST_INPUT(AttrDef),
    NTFS_TEST_INPUT(BadClus),
    NTFS_TEST_INPUT(Bitmap),
    NTFS_TEST_INPUT(Boot),
    NTFS_TEST_INPUT(Extend),
    NTFS_TEST_INPUT(LogFile),
    NTFS_TEST_INPUT(MftMirr),
    NTFS_TEST_INPUT(Mft),
    NTFS_TEST_INPUT(Secure),
    NTFS_TEST_INPUT(UpCase),
    NTFS_TEST_INPUT(Volume),
    NTFS_TEST_INPUT(Cairo),
    NTFS_TEST_INPUT(IndexAllocation),
    NTFS_TEST_INPUT(Data),
    NTFS_TEST_INPUT(Unknown),
    NTFS_TEST_INPUT(Dot),
    NTFS_WORST_CASE_TEST_INPUT(),
    { -1, &a },
    { -1, &fox1 },
    { -1, &abcdefghijklmnopqrstuvw },
};

const STRING_TABLE_TEST_INPUT NtfsWorstCaseTestInput = {
    -1,
    (PSTRING)&NtfsWorstCaseName
};

const ULONG NumberOfNtfsTestInputs = ARRAYSIZE(NtfsTestInputs);

//
// Ntfs2 Test Inputs
//

STRING_ARRAY16 Ntfs2StringArray16 = CONSTANT_STRING_ARRAY16(
    RTL_CONSTANT_STRING("$MftMirr1234567890"),
    RTL_CONSTANT_STRING("$MftMirr123456789"),
    RTL_CONSTANT_STRING("$MftMirr12345678"),
    RTL_CONSTANT_STRING("$MftMirr1234567"),
    RTL_CONSTANT_STRING("$MftMirr123456"),
    RTL_CONSTANT_STRING("$MftMirr12345"),
    RTL_CONSTANT_STRING("$MftMirr1234"),
    RTL_CONSTANT_STRING("$MftMirr123"),
    RTL_CONSTANT_STRING("$MftMirr12"),
    RTL_CONSTANT_STRING("$MftMirr1"),
    RTL_CONSTANT_STRING("$MftMirr"),
    RTL_CONSTANT_STRING("$Mft"),
    RTL_CONSTANT_STRING("$INDEX_ALLOCATION"),
    RTL_CONSTANT_STRING("$DATA"),
    RTL_CONSTANT_STRING("????"),
    RTL_CONSTANT_STRING(".")
);

const STRING_TABLE_TEST_INPUT Ntfs2TestInputs[] = {
    NTFS2_TEST_INPUT(MftMirr1234567890),
    NTFS2_TEST_INPUT(MftMirr123456789),
    NTFS2_TEST_INPUT(MftMirr12345678),
    NTFS2_TEST_INPUT(MftMirr1234567),
    NTFS2_TEST_INPUT(MftMirr123456),
    NTFS2_TEST_INPUT(MftMirr12345),
    NTFS2_TEST_INPUT(MftMirr1234),
    NTFS2_TEST_INPUT(MftMirr123),
    NTFS2_TEST_INPUT(MftMirr12),
    NTFS2_TEST_INPUT(MftMirr1),
    NTFS2_TEST_INPUT(MftMirr),
    NTFS2_TEST_INPUT(Mft),
    NTFS2_TEST_INPUT(IndexAllocation),
    NTFS2_TEST_INPUT(Data),
    NTFS2_TEST_INPUT(Unknown),
    NTFS2_TEST_INPUT(Dot),
    { -1, &a },
    { -1, &ab },
    { -1, &abc },
    { -1, &fox1 },
    { -1, &abcd },
    { -1, &abcdefghijkl },
    { -1, &abcdefghijklmnopqr },
    { -1, &abcdefghijklmnopqrstuvw },
};

const ULONG NumberOfNtfs2TestInputs = ARRAYSIZE(Ntfs2TestInputs);

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
