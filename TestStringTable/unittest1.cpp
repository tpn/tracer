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

#include "../TracerHeap/DefaultTracerHeap.h"
#include "../TracerHeap/DefaultTracerHeap.c"

#define INIT_ALLOCATOR(Allocator) DefaultHeapInitializeAllocator(Allocator)
#define DESTROY_ALLOCATOR(Allocator) DefaultHeapDestroyAllocator(Allocator)

#define MAKE_STRING(Name) STRING Name = RTL_CONSTANT_STRING(#Name)

MAKE_STRING(falcon);
MAKE_STRING(tomcat);
MAKE_STRING(viper);
MAKE_STRING(fox2);

#define CONSTANT_STRING_ARRAY(Strings) { 0, 1, 0, 0, { Strings } }
#define LEN(String) ((LONG)((STRING)(String)).Length)

#define PTR(p) ((ULONG_PTR)(p))

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

            Assert::IsFalse(Found);

        }

    };
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
