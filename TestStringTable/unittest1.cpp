#include "stdafx.h"
#include "CppUnitTest.h"
#include "../StringTable/StringTable.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef struct _STRING_ARRAY2 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[2];
} STRING_ARRAY2, *PSTRING_ARRAY2, **PPSTRING_ARRAY2;

typedef struct _STRING_ARRAY3 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[3];
} STRING_ARRAY3, *PSTRING_ARRAY3, **PPSTRING_ARRAY3;

typedef struct _STRING_ARRAY4 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[4];
} STRING_ARRAY4, *PSTRING_ARRAY4, **PPSTRING_ARRAY4;

typedef struct _STRING_ARRAY5 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[5];
} STRING_ARRAY5, *PSTRING_ARRAY5, **PPSTRING_ARRAY5;

typedef struct _STRING_ARRAY6 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[6];
} STRING_ARRAY6, *PSTRING_ARRAY6, **PPSTRING_ARRAY6;

typedef struct _STRING_ARRAY16 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[16];
} STRING_ARRAY16, *PSTRING_ARRAY16, **PPSTRING_ARRAY16;

typedef struct _STRING_ARRAY17 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[17];
} STRING_ARRAY17, *PSTRING_ARRAY17, **PPSTRING_ARRAY17;

typedef struct _STRING_ARRAY31 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[31];
} STRING_ARRAY31, *PSTRING_ARRAY31, **PPSTRING_ARRAY31;

typedef struct _STRING_ARRAY32 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[32];
} STRING_ARRAY32, *PSTRING_ARRAY32, **PPSTRING_ARRAY32;

typedef struct _STRING_ARRAY33 {
    USHORT SizeInQuadwords;
    USHORT NumberOfElements;
    USHORT MinimumLength;
    USHORT MaximumLength;
    STRING Strings[33];
} STRING_ARRAY33, *PSTRING_ARRAY33, **PPSTRING_ARRAY33;

#include "../TracerHeap/AlignedTracerHeap.h"
#include "../TracerHeap/AlignedTracerHeap.c"

#define INIT_ALLOCATOR(Allocator) AlignedHeapInitializeAllocator(Allocator)
#define DESTROY_ALLOCATOR(Allocator) AlignedHeapDestroyAllocator(Allocator)

#define MAKE_STRING(Name) STRING Name = RTL_CONSTANT_STRING(#Name)

MAKE_STRING(falcon);
MAKE_STRING(tomcat);
MAKE_STRING(warthog);
MAKE_STRING(viper);
MAKE_STRING(fox);
MAKE_STRING(fox1);
MAKE_STRING(fox2);
MAKE_STRING(lightning_storm);
MAKE_STRING(really_bad_lightning_storm);

#define PTR(p) ((ULONG_PTR)(p))
#define LEN(String) ((LONG)((STRING)(String)).Length)

#define CONSTANT_STRING_ARRAY(Strings) { \
    0, 1, 0, 0, { Strings } }

#define CONSTANT_STRING_ARRAY3(S1, S2, S3) { \
    0, 3, 0, 0, { S1, S2, S3 }               \
}

#define CONSTANT_STRING_ARRAY5(S1, S2, S3, S4, S5) { \
    0, 5, 0, 0, { S1, S2, S3, S4, S5 }               \
}

#define CONSTANT_STRING_ARRAY6(S1, S2, S3, S4, S5, S6) { \
    0, 6, 0, 0, { S1, S2, S3, S4, S5, S6 }               \
}

#define MAKE_TABLE(Array) \
    CreateStringTable(&Allocator, reinterpret_cast<PSTRING_ARRAY>(Array))

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

        TEST_METHOD(TestMethod1)
        {
            BOOL Found;
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

            Found = IsPrefixOfStringInTable(StringTable, &tomcat, NULL);

            Assert::AreEqual(Found, FALSE);

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

        TEST_METHOD(TestMethod3)
        {
            BOOL Found;
            USHORT Mask;
            USHORT ExpectedMask;
            USHORT LengthsBitmap;
            USHORT ExpectedLengthsBitmap;
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
            Found = IsPrefixOfStringInTable(StringTable, &tomcat, NULL);
            Assert::AreEqual(Found, FALSE);

            Mask = IsFirstCharacterInStringTable(StringTable, 'F');
            Assert::AreEqual((ULONG)Mask, (ULONG)0);

            Mask = IsFirstCharacterInStringTable(StringTable, 'a');
            Assert::AreEqual((ULONG)Mask, (ULONG)0);

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

            //Assert::AreEqual((ULONG)LengthsBitmap,
            //                 (ULONG)ExpectedLengthsBitmap);

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


    };
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
