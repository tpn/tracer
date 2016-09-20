#include "stdafx.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define PTR(p) ((ULONG_PTR)(p))
#define LEN(String) ((LONG)((STRING)(String)).Length)

MAKE_STRING(Foo);
MAKE_STRING(Bar);

#define DELIM ';'

#define DELIMITED_TABLE(String)                         \
    StringTable = CreateStringTableFromDelimitedString( \
        Rtl,                                            \
        &Allocator,                                     \
        String,                                         \
        DELIM                                           \
    );                                                  \
    Assert::IsNotNull(StringTable)

#define ASSERT_FAILED_DELIMITED_TABLE(String)           \
    StringTable = CreateStringTableFromDelimitedString( \
        Rtl,                                            \
        &Allocator,                                     \
        String,                                         \
        DELIM                                           \
    );                                                  \
    Assert::IsNull(StringTable)

#define DESTROY_TABLE()                                 \
    StringTable->DestroyStringTable(                    \
        &Allocator,                                     \
        StringTable                                     \
    );                                                  \
    StringTable = NULL

#define ASSERT_SIZE(Size)                                \
    Assert::AreEqual(                                    \
        (ULONG)(Size),                                   \
        (ULONG)(__popcnt16(StringTable->OccupiedBitmap)) \
    )

namespace TestStringTable
{
    TEST_CLASS(UnitTest2)
    {
    public:

        PRTL Rtl;
        HMODULE RtlModule;
        PINITIALIZE_RTL InitializeRtl;
        ALLOCATOR Allocator;

        TEST_METHOD_INITIALIZE(Initialize)
        {
            BOOL Success;
            ULONG RequiredSize;
            INIT_ALLOCATOR(&Allocator);

            RtlModule = LoadLibraryA("Rtl.dll");
            Assert::IsNotNull(RtlModule);

            InitializeRtl = (PINITIALIZE_RTL)(
                GetProcAddress(RtlModule, "InitializeRtl")
            );

            Assert::IsNotNull((PVOID)InitializeRtl);

            InitializeRtl(NULL, &RequiredSize);

            Rtl = (PRTL)Allocator.Calloc(&Allocator.Context, 1, RequiredSize);

            Assert::IsNotNull((PVOID)Rtl);

            Success = InitializeRtl(Rtl, &RequiredSize);
            Assert::AreEqual(Success, TRUE);

        }

        TEST_METHOD_CLEANUP(Cleanup)
        {
            DESTROY_ALLOCATOR(&Allocator);
            Rtl->DestroyRtl(&Rtl);
            FreeLibrary(RtlModule);
        }

        TEST_METHOD(TestCreateStringTableFromDelimitedString1)
        {
            MAKE_STRING(Foo);
            PSTRING_TABLE StringTable;

            StringTable = CreateStringTableFromDelimitedString(
                Rtl,
                &Allocator,
                &Foo,
                ';'
            );

            Assert::IsNotNull(StringTable);

            StringTable->DestroyStringTable(&Allocator, StringTable);

        }

        TEST_METHOD(TestCreateStringTableFromDelimitedString2)
        {
            PSTRING_TABLE StringTable;

            STRING Test1 = RTL_CONSTANT_STRING(";Foo;Bar");
            STRING Test2 = RTL_CONSTANT_STRING(";Foo;;Bar");
            STRING Test3 = RTL_CONSTANT_STRING(";;Foo;Bar");
            STRING Test4 = RTL_CONSTANT_STRING("Foo;Bar;;;");
            STRING Test5 = RTL_CONSTANT_STRING(";;;;;Foo;B");
            STRING Test6 = RTL_CONSTANT_STRING("Foo;;;;;;;B;");

            DELIMITED_TABLE(&Test1);
            ASSERT_SIZE(2);
            DESTROY_TABLE();

            DELIMITED_TABLE(&Test2);
            ASSERT_SIZE(2);
            DESTROY_TABLE();

            DELIMITED_TABLE(&Test3);
            ASSERT_SIZE(2);
            DESTROY_TABLE();

            DELIMITED_TABLE(&Test4);
            ASSERT_SIZE(2);
            DESTROY_TABLE();

            DELIMITED_TABLE(&Test5);
            ASSERT_SIZE(2);
            DESTROY_TABLE();

            DELIMITED_TABLE(&Test6);
            ASSERT_SIZE(2);
            DESTROY_TABLE();

        }

        TEST_METHOD(TestCreateStringTableFromDelimitedString3)
        {
            PSTRING_TABLE StringTable;

            STRING Test7 = RTL_CONSTANT_STRING(";;;");

            ASSERT_FAILED_DELIMITED_TABLE(&Test7);
        }

    };
}


