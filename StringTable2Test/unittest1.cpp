#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define PTR(p) ((ULONG_PTR)(p))
#define LEN(String) ((LONG)((STRING)(String)).Length)

RTL GlobalRtl;
ALLOCATOR GlobalAllocator;

RTL_BOOTSTRAP GlobalBootstrap;
PRTL_BOOTSTRAP Bootstrap;

PRTL Rtl;
PALLOCATOR Allocator;

STRING_TABLE_API_EX GlobalApi;
PSTRING_TABLE_API_EX Api;

HMODULE GlobalRtlModule = 0;
HMODULE GlobalStringTableModule = 0;

TEST_MODULE_INITIALIZE(UnitTest1Init)
{
    PSTRING_TABLE_ANY_API AnyApi = (PSTRING_TABLE_ANY_API)&GlobalApi;
    ULONG SizeOfRtl = sizeof(GlobalRtl);

    Assert::IsTrue(BootstrapRtl(&GlobalRtlModule, &GlobalBootstrap));

    Bootstrap = &GlobalBootstrap;

    Assert::IsTrue(Bootstrap->InitializeRtl(&GlobalRtl, &SizeOfRtl));
    Assert::IsTrue(Bootstrap->InitializeHeapAllocator(&GlobalAllocator));

    Rtl = &GlobalRtl;
    Allocator = &GlobalAllocator;

    Assert::IsTrue(LoadStringTableApi(Rtl,
                                      &GlobalStringTableModule,
                                      NULL,
                                      sizeof(*AnyApi),
                                      AnyApi));

    Api = &GlobalApi;
}

TEST_MODULE_CLEANUP(UnitTest1Cleanup)
{
    if (GlobalRtlModule != 0) {
        FreeLibrary(GlobalRtlModule);
    }

    if (GlobalStringTableModule != 0) {
        FreeLibrary(GlobalStringTableModule);
    }
}


namespace TestStringTable
{
    TEST_CLASS(UnitTest1)
    {
    public:

        TEST_METHOD(TestMethod1_0)
        {
            BOOL Result;
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            STRING_ARRAY StringArray1 = CONSTANT_STRING_ARRAY(falcon);

            StringTable = MAKE_TABLE(&StringArray1);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreEqual(PTR(StringArray),
                             PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray), PTR(&StringArray1));

            Assert::AreEqual((LONG)StringArray->MinimumLength, LEN(falcon));

            Assert::AreEqual((LONG)StringArray->MaximumLength, LEN(falcon));

            Index = Api->IsPrefixOfStringInTable(StringTable, &tomcat, NULL);

            Result = (Index == NO_MATCH_FOUND);
            Assert::IsTrue(Result);
        }

        TEST_METHOD(TestMethod1_1)
        {
            STRING_TABLE_INDEX Index;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            STRING_ARRAY StringArray1 = CONSTANT_STRING_ARRAY(falcon);

            StringTable = MAKE_TABLE(&StringArray1);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreEqual(PTR(StringArray),
                             PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray), PTR(&StringArray1));

            Assert::AreEqual((LONG)StringArray->MinimumLength, LEN(falcon));

            Assert::AreEqual((LONG)StringArray->MaximumLength, LEN(falcon));

            Index = Api->IsPrefixOfStringInTable(StringTable, &tomcat, NULL);

            Assert::IsTrue(Index == NO_MATCH_FOUND);

        }

        /*
        TEST_METHOD(TestMethod2)
        {
            USHORT Mask;
            PSTRING_TABLE StringTable;
            STRING_ARRAY StringArray1 = CONSTANT_STRING_ARRAY(fox);

            StringTable = MAKE_TABLE(&StringArray1);

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

            Index = Api->IsPrefixOfStringInTable(StringTable, &tomcat, NULL);
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


            Index = Api->IsPrefixOfStringInTable(StringTable, &fox2, &Match);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Assert::AreEqual(Match.Index, (LONG)4);
            Assert::AreEqual((LONG)Match.NumberOfMatchedCharacters, (LONG)3);
            Assert::AreEqual(strncmp(fox.Buffer, Match.String->Buffer, 3), 0);

        }
        */

        /*
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

            Index = Api->IsPrefixOfStringInTable(StringTable, &tomcat, NULL);
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


            Index = Api->IsPrefixOfStringInTable(StringTable, &fox2, &Match);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Assert::AreEqual(Match.Index, (LONG)4);
            Assert::AreEqual((LONG)Match.NumberOfMatchedCharacters, (LONG)3);
            Assert::AreEqual(strncmp(fox.Buffer, Match.String->Buffer, 3), 0);

        }
        */

        /*
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
        */

        TEST_METHOD(TestMethod5_0)
        {
            STRING_TABLE_INDEX Index;
            STRING_TABLE_INDEX Index1;
            STRING_TABLE_INDEX Index2;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PSTRING_ARRAY6 pStringArray6;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix1;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix2;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
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

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;
            IsPrefix1 = Api->IsPrefixOfStringInTable_8;
            IsPrefix2 = Api->IsPrefixOfStringInTable_x64_2;

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            ASSERT(Index == NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);

            Index1 = IsPrefix1(StringTable, &abcdefghijklm, NULL);
            Index2 = IsPrefix2(StringTable, &abcdefghijklm, NULL);

            ASSERT(Index == 4);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            ASSERT(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            ASSERT(Index == 5);

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            ASSERT(Index == NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            ASSERT(Index == 4);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            ASSERT(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            ASSERT(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &nopqrstuvwy,
                                            NULL);
            ASSERT(Index == 1);

            DESTROY_TABLE(StringTable);
        }

        TEST_METHOD(TestMethod5_1)
        {
            STRING_TABLE_INDEX Index;
            const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY9 pStringArray9;
            STRING_ARRAY9 StringArray9 = CONSTANT_STRING_ARRAY9(
                abcdefghijklmnopqrstuvw,
                abcdefghijk,
                abcd,
                abc,
                ab,
                a,
                fox1,
                klmnopqrstu,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray9);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray9 = (PSTRING_ARRAY9)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray9));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(a));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(abcdefghijklmnopqrstuvw));

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::IsTrue(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::IsTrue(Index == 6);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            Assert::IsTrue(Index == 8);

            DESTROY_TABLE(StringTable);

        }

        TEST_METHOD(TestMethod5_1_0)
        {
            STRING_TABLE_INDEX Index;
            const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY9 pStringArray9;
            STRING_ARRAY9 StringArray9 = CONSTANT_STRING_ARRAY9(
                abcdefghijklmnopqrstuvw,
                abcdefghijk,
                abcd,
                abc,
                ab,
                a,
                fox1,
                klmnopqrstu,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray9);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray9 = (PSTRING_ARRAY9)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray9));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(a));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(abcdefghijklmnopqrstuvw));

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::IsTrue(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::IsTrue(Index == 6);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            Assert::IsTrue(Index == 8);

            DESTROY_TABLE(StringTable);

            Assert::IsTrue(1 == 1);
        }

        TEST_METHOD(TestMethod5_1_1)
        {
            STRING_TABLE_INDEX Index;
            const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY9 pStringArray9;
            STRING_ARRAY9 StringArray9 = CONSTANT_STRING_ARRAY9(
                abcdefghijklmnopqrstuvw,
                abcdefghijk,
                abcd,
                abc,
                ab,
                a,
                fox1,
                klmnopqrstu,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray9);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray9 = (PSTRING_ARRAY9)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray9));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(a));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(abcdefghijklmnopqrstuvw));

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::IsTrue(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::IsTrue(Index == 6);

            DESTROY_TABLE(StringTable);
        }

        TEST_METHOD(TestMethod5_2)
        {
            STRING_TABLE_INDEX Index;
            const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY9 pStringArray9;
            STRING_ARRAY9 StringArray9 = CONSTANT_STRING_ARRAY9(
                abcdefghijklmnopqrstuvw,
                abcdefghijk,
                abcd,
                abc,
                ab,
                a,
                fox1,
                klmnopqrstu,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray9);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray9 = (PSTRING_ARRAY9)StringArray;

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            Assert::IsTrue(Index == 8);

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::IsTrue(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::IsTrue(Index == 6);

            DESTROY_TABLE(StringTable);
        }

        TEST_METHOD(TestMethod5_3)
        {
            STRING_TABLE_INDEX Index;
            const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY9 pStringArray9;
            STRING_ARRAY9 StringArray9 = CONSTANT_STRING_ARRAY9(
                abcdefghijklmnopqrstuvw,
                abcdefghijk,
                abcd,
                abc,
                ab,
                a,
                fox1,
                klmnopqrstu,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray9);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray9 = (PSTRING_ARRAY9)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray9));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(a));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(abcdefghijklmnopqrstuvw));

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::IsTrue(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::IsTrue(Index == 6);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            Assert::IsTrue(Index == 8);

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::IsTrue(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::IsTrue(Index == 6);


            DESTROY_TABLE(StringTable);
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

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONG, NULL);
            Assert::AreEqual((LONG)Index, (LONG)0);

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONGEST, NULL);
            Assert::AreEqual((LONG)Index, (LONG)0);

            Index = IsPrefixOfStringInTable(StringTable, &ABC_LONG, NULL);
            Assert::IsTrue(Index == NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONG, NULL);
            Assert::IsTrue(Index == NO_MATCH_FOUND);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONGEST, NULL);
            Assert::AreEqual((LONG)Index, (LONG)1);

            DESTROY_TABLE(StringTable);
        }

        TEST_METHOD(TestMethod6_1)
        {
            STRING_TABLE_INDEX Index;
            const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
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

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONG, NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &ABCD_LONGEST, NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &ABC_LONG, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONG, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable, &BCDE_LONGEST, NULL);
            Assert::IsTrue(Index == 1);

            DESTROY_TABLE(StringTable);
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

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;
            Index = IsPrefixOfStringInTable(StringTable, &fox2, &Match);
            Assert::AreEqual((LONG)Index, (LONG)4);

            Assert::AreEqual((LONG)Match.NumberOfMatchedCharacters, (LONG)3);
            Assert::AreEqual(strncmp(fox.Buffer, Match.String->Buffer, 3), 0);

            DESTROY_TABLE(StringTable);
        }

        TEST_METHOD(TestMethod8_1)
        {
            STRING_TABLE_INDEX Index;
            const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
            PSTRING_TABLE StringTable;
            PSTRING_ARRAY StringArray;
            PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
            PSTRING_ARRAY9 pStringArray9;
            STRING_ARRAY9 StringArray9 = CONSTANT_STRING_ARRAY9(
                abcdefghijklmnopqrstuvw,
                abcdefghijk,
                abcd,
                abc,
                ab,
                a,
                fox1,
                klmnopqrstu,
                lmnopqr
            );

            StringTable = MAKE_TABLE(&StringArray9);

            Assert::IsNotNull(StringTable);

            StringArray = StringTable->pStringArray;
            pStringArray9 = (PSTRING_ARRAY9)StringArray;

            Assert::IsNotNull(StringArray);

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringTable->StringArray));

            Assert::AreNotEqual(PTR(StringArray),
                                PTR(&StringArray9));

            Assert::AreEqual((LONG)StringArray->MinimumLength,
                             LEN(a));

            Assert::AreEqual((LONG)StringArray->MaximumLength,
                             LEN(abcdefghijklmnopqrstuvw));

            IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;

            Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
            Assert::IsTrue(Index == NoMatchFound);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvw,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
            Assert::IsTrue(Index == 2);

            Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
            Assert::IsTrue(Index == 6);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstuvwyxz,
                                            NULL);
            Assert::IsTrue(Index == 0);

            Index = IsPrefixOfStringInTable(StringTable,
                                            &abcdefghijklmnopqrstu,
                                            NULL);
            Assert::IsTrue(Index == 1);

            Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
            Assert::IsTrue(Index == 8);

            DESTROY_TABLE(StringTable);
        }

    };
}


