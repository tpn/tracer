#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#include "../TracerHeap/AlignedTracerHeap.c"

#define PTR(p) ((ULONG_PTR)(p))
#define LEN(String) ((LONG)((STRING)(String)).Length)

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

namespace TestStringTable
{
    TEST_CLASS(UnitTest1)
    {
    public:

        ALLOCATOR Allocator;

        TEST_METHOD_INITIALIZE(Initialize)
        {
            INIT_ALLOCATOR(&Allocator);
        }

        TEST_METHOD_CLEANUP(Cleanup)
        {
            DESTROY_ALLOCATOR(&Allocator);
        }

        TEST_METHOD(TestMethod1_0)
        {
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            STRING_ARRAY StringArray1 = CONSTANT_STRING_ARRAY(falcon);

            StringTable = CreateStringTable(&Allocator, &StringArray1);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreEqual(PTR(StringArray),
                             PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray), PTR(&StringArray1));

            Assert::AreEqual((LONG)StringArray->MinimumLength, LEN(falcon));

            Assert::AreEqual((LONG)StringArray->MaximumLength, LEN(falcon));

            IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &tomcat, NULL);

            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

        }

        TEST_METHOD(TestMethod1_1)
        {
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            STRING_ARRAY StringArray1 = CONSTANT_STRING_ARRAY(falcon);

            StringTable = CreateStringTable(&Allocator, &StringArray1);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreEqual(PTR(StringArray),
                             PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray), PTR(&StringArray1));

            Assert::AreEqual((LONG)StringArray->MinimumLength, LEN(falcon));

            Assert::AreEqual((LONG)StringArray->MaximumLength, LEN(falcon));

            IsPrefixOfStringInTable = IsPrefixOfStringInSingleTableInline;

            Index = IsPrefixOfStringInTable(StringTable, &tomcat, NULL);

            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

        }

        TEST_METHOD(TestMethod2)
        {
            USHORT Mask;
            PSTRING_TABLE StringTable;
            STRING_ARRAY StringArray1 = CONSTANT_STRING_ARRAY(fox);

            StringTable = CreateStringTable(&Allocator, &StringArray1);

            Mask = IsFirstCharacterInStringTable(StringTable, 'f');
            Assert::AreEqual((ULONG)Mask, (ULONG)1);

            Mask = IsFirstCharacterInStringTable(StringTable, 'F');
            Assert::AreEqual((ULONG)Mask, (ULONG)0);

            Mask = IsFirstCharacterInStringTable(StringTable, 'a');
            Assert::AreEqual((ULONG)Mask, (ULONG)0);

        }

        TEST_METHOD(TestMethod3_0)
        {
            STRING_TABLE_INDEX Index;
            USHORT Mask;
            USHORT ExpectedMask;
            USHORT LengthsBitmap;
            USHORT ExpectedLengthsBitmap;
            STRING_MATCH Match;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY6 pStringArray6;
            STRING_ARRAY6 StringArray6 = CONSTANT_STRING_ARRAY6(
                falcon,
                lightning_storm,
                fox1,
                warthog,
                fox,
                really_bad_lightning_storm
            );

            StringTable = MAKE_TABLE(&StringArray6);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray6 = (PSTRING_ARRAY6)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray6));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(fox));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(really_bad_lightning_storm));

            IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;
            Index = IsPrefixOfStringInTable(StringTable, &tomcat, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Mask = IsFirstCharacterInStringTable(StringTable, 'F');
            Assert::AreEqual((LONG)Mask, (LONG)0);

            Mask = IsFirstCharacterInStringTable(StringTable, 'a');
            Assert::AreEqual((LONG)Mask, (LONG)0);

            //
            // Slots that start with f are in bitmap positions 1, 3 and 5.
            //

            ExpectedMask = (1 | (1 << (3-1)) | (1 << (5-1)));
            Mask = IsFirstCharacterInStringTable(StringTable, 'f');
            Assert::AreEqual((ULONG)Mask, (ULONG)ExpectedMask);

            //
            // Length-appropriate indexes should be 3 and 5 (fox1 and fox).
            //

            ExpectedLengthsBitmap = (1 << (3-1)) | (1 << (5-1));
            LengthsBitmap = GetBitmapForViablePrefixSlotsByLengths(
                StringTable,
                &fox2
            );

            Assert::AreEqual((ULONG)LengthsBitmap,
                             (ULONG)ExpectedLengthsBitmap);


            Index = IsPrefixOfStringInTable(StringTable, &fox2, &Match);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Assert::AreEqual(Match.Index, (LONG)4);
            Assert::AreEqual((LONG)Match.NumberOfMatchedCharacters, (LONG)3);
            Assert::AreEqual(strncmp(fox.Buffer, Match.String->Buffer, 3), 0);

        }

        TEST_METHOD(TestMethod3_1)
        {
            STRING_TABLE_INDEX Index;
            USHORT Mask;
            USHORT ExpectedMask;
            USHORT LengthsBitmap;
            USHORT ExpectedLengthsBitmap;
            STRING_MATCH Match;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY6 pStringArray6;
            STRING_ARRAY6 StringArray6 = CONSTANT_STRING_ARRAY6(
                falcon,
                lightning_storm,
                fox1,
                warthog,
                fox,
                really_bad_lightning_storm
            );

            StringTable = MAKE_TABLE(&StringArray6);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray6 = (PSTRING_ARRAY6)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray6));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(fox));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(really_bad_lightning_storm));

            IsPrefixOfStringInTable = IsPrefixOfStringInSingleTableInline;
            Index = IsPrefixOfStringInTable(StringTable, &tomcat, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Mask = IsFirstCharacterInStringTable(StringTable, 'F');
            Assert::AreEqual((LONG)Mask, (LONG)0);

            Mask = IsFirstCharacterInStringTable(StringTable, 'a');
            Assert::AreEqual((LONG)Mask, (LONG)0);

            //
            // Slots that start with f are in bitmap positions 1, 3 and 5.
            //

            ExpectedMask = (1 | (1 << (3-1)) | (1 << (5-1)));
            Mask = IsFirstCharacterInStringTable(StringTable, 'f');
            Assert::AreEqual((ULONG)Mask, (ULONG)ExpectedMask);

            //
            // Length-appropriate indexes should be 3 and 5 (fox1 and fox).
            //

            ExpectedLengthsBitmap = (1 << (3-1)) | (1 << (5-1));
            LengthsBitmap = GetBitmapForViablePrefixSlotsByLengths(
                StringTable,
                &fox2
            );

            Assert::AreEqual((ULONG)LengthsBitmap,
                             (ULONG)ExpectedLengthsBitmap);


            Index = IsPrefixOfStringInTable(StringTable, &fox2, &Match);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Assert::AreEqual(Match.Index, (LONG)4);
            Assert::AreEqual((LONG)Match.NumberOfMatchedCharacters, (LONG)3);
            Assert::AreEqual(strncmp(fox.Buffer, Match.String->Buffer, 3), 0);

        }

        TEST_METHOD(TestMethod4)
        {
            ULONG NumberOfElements = 5;
            ULONG Input = 3279;
            ULONG Original = 0xaaaaaaaa;
            ULONG Mask1 = Original;
            ULONG Mask2 = Original;
            ULONG Mask3 = Original;
            ULONG Result1;
            ULONG Result2;
            ULONG Result3;
            ULONG Result4;
            ULONG Bitmap1;
            //ULONG Bitmap2;
            ULONG Bitcount;
            ULONG Temp1;
            ULONG Temp2;
            ULONG Temp3;
            ULONG ExpectedLengthsBitmap1;
            ULONG ExpectedLengthsBitmap2;
            ULONG ExpectedLengthsBitmap3;
            PARALLEL_SUFFIX_MOVE_MASK32 Suffix;

            Bitcount = __popcnt(Original);

            Result1 = CompressUlongNaive(Input, Mask1);
            Result2 = CompressUlongParallelSuffixDynamicMask(Input, Mask2);

            CreateParallelSuffixMoveMask(Mask3, &Suffix);
            Result3 = CompressUlongParallelSuffix(3279, &Suffix);
            Result4 = CompressUlongParallelSuffix(
                3279,
                &ParallelSuffix32HighBitFromEveryOtherByte
            );

            Bitmap1 = Result3 ^ ~((1 << NumberOfElements) - 1);

            Temp1 = ~((1 << NumberOfElements) - 1);
            Temp2 = Result1 & Temp1;
            Temp3 = Temp2 ^ Result1;
            //Temp2 = ~Temp1;

            Bitmap1 = Temp2; // ~Result3 ^ Temp;
            //Bitmap2 = ~(Result3 ^ Temp);

            ExpectedLengthsBitmap1 = (1 << (3-1)) | (1 << (5-1));
            ExpectedLengthsBitmap2 = (1 << 3) | (1 << 5);
            ExpectedLengthsBitmap3 = 0x20;
            Assert::AreEqual(Bitmap1, ExpectedLengthsBitmap3);
        }

        TEST_METHOD(TestMethod5_0)
        {
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY6 pStringArray6;
            STRING_ARRAY6 StringArray6 = CONSTANT_STRING_ARRAY6(
                abcdefghijklmnopqrstuvw,
                nop,
                fox1,
                klmnopqrstu,
                abcdefghijk,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray6);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray6 = (PSTRING_ARRAY6)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray6));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(nop));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(abcdefghijklmnopqrstuvw));

            IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;
            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::AreEqual((LONG)Index, (LONG)2);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            Assert::AreEqual((LONG)Index, (LONG)5);

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            Assert::AreEqual((ULONG)Index, (ULONG)0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::AreEqual((ULONG)Index, (ULONG)0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &nopqrstuvwy,
                                            NULL);
            Assert::AreEqual((ULONG)Index, (ULONG)1);

        }

        TEST_METHOD(TestMethod5_1)
        {
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY6 pStringArray6;
            STRING_ARRAY6 StringArray6 = CONSTANT_STRING_ARRAY6(
                abcdefghijklmnopqrstuvw,
                nop,
                fox1,
                klmnopqrstu,
                abcdefghijk,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray6);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray6 = (PSTRING_ARRAY6)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray6));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(nop));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(abcdefghijklmnopqrstuvw));

            IsPrefixOfStringInTable = IsPrefixOfStringInSingleTableInline;
            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::AreEqual((LONG)Index, (LONG)2);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            Assert::AreEqual((LONG)Index, (LONG)5);

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            Assert::AreEqual((ULONG)Index, (ULONG)0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::AreEqual((ULONG)Index, (ULONG)0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &nopqrstuvwy,
                                            NULL);
            Assert::AreEqual((ULONG)Index, (ULONG)1);

        }

        TEST_METHOD(TestMethod6_0)
        {
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY2 pStringArray2;
            STRING_ARRAY2 StringArray2 = CONSTANT_STRING_ARRAY2(
                ABCD_LONG,
                BCDE_LONGEST
            );

            StringTable = MAKE_TABLE(&StringArray2);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray2 = (PSTRING_ARRAY2)StringArray;

            IsPrefixOfStringInTable = StringTable->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONG, NULL);
            Assert::AreEqual((LONG)Index, (LONG)0);

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONGEST, NULL);
            Assert::AreEqual((LONG)Index, (LONG)0);

            Index = IsPrefixOfStringInTable(StringTable, &ABC_LONG, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONG, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONGEST, NULL);
            Assert::AreEqual((LONG)Index, (LONG)1);
        }

        TEST_METHOD(TestMethod6_1)
        {
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY2 pStringArray2;
            STRING_ARRAY2 StringArray2 = CONSTANT_STRING_ARRAY2(
                ABCD_LONG,
                BCDE_LONGEST
            );

            StringTable = MAKE_TABLE(&StringArray2);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray2 = (PSTRING_ARRAY2)StringArray;

            IsPrefixOfStringInTable = IsPrefixOfStringInSingleTableInline;

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONG, NULL);
            Assert::AreEqual((LONG)Index, (LONG)0);

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONGEST, NULL);
            Assert::AreEqual((LONG)Index, (LONG)0);

            Index = IsPrefixOfStringInTable(StringTable, &ABC_LONG, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONG, NULL);
            Assert::AreEqual((LONG)Index, (LONG)NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONGEST, NULL);
            Assert::AreEqual((LONG)Index, (LONG)1);
        }

        TEST_METHOD(TestMethod7)
        {
            STRING_TABLE_INDEX Index;
            STRING_MATCH Match;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY6 pStringArray6;
            STRING_ARRAY6 StringArray6 = CONSTANT_STRING_ARRAY6(
                falcon,
                lightning_storm,
                fox1,
                warthog,
                fox,
                really_bad_lightning_storm
            );

            StringTable = MAKE_TABLE(&StringArray6);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray6 = (PSTRING_ARRAY6)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                PTR(&StringArray6));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                LEN(fox));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                LEN(really_bad_lightning_storm));

            IsPrefixOfStringInTable = IsPrefixOfStringInSingleTableInline;
            Index = IsPrefixOfStringInTable(StringTable, &fox2, &Match);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Assert::AreEqual((LONG)Match.NumberOfMatchedCharacters, (LONG)3);
            Assert::AreEqual(strncmp(fox.Buffer, Match.String->Buffer, 3), 0);
        }


    };
}


