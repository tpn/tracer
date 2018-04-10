/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    Scratch/testing file.

--*/

#include "stdafx.h"

#pragma optimize("", off)

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

#define DELIM ';'

#define DELIMITED_TABLE(String)                              \
    StringTable = Api->CreateStringTableFromDelimitedString( \
        Rtl,                                                 \
        Allocator,                                           \
        Allocator,                                           \
        String,                                              \
        DELIM                                                \
    );                                                       \
    ASSERT(StringTable != NULL)

#if 0
#define END_TIMESTAMP END_TIMESTAMP_RDTSC
#define START_TIMESTAMP START_TIMESTAMP_RDTSC
#endif

#if 1
#define END_TIMESTAMP END_TIMESTAMP_RDTSCP
#define START_TIMESTAMP START_TIMESTAMP_RDTSCP
#endif


RTL GlobalRtl;
ALLOCATOR GlobalAllocator;

PRTL Rtl;
PALLOCATOR Allocator;

HMODULE GlobalModule = 0;

STRING_TABLE_API_EX GlobalApi;
PSTRING_TABLE_API_EX Api;

HMODULE GlobalRtlModule = 0;
HMODULE GlobalStringTableModule = 0;

//
// Slow version with strlen().
//

STRING_TABLE_INDEX
IsPrefixOfStringInTable_SlowC_2(
    PCSZ *StringArray,
    PCSZ String,
    PSTRING_MATCH Match
    )
{
    PCSZ Left;
    PCSZ Right;
    PCSZ *Target;
    ULONG Index = 0;
    ULONG Count;

    SIZE_T LeftLength;
    SIZE_T RightLength;

    for (Target = StringArray; *Target != NULL; Target++, Index++) {
        Count = 0;
        Left = String;
        Right = *Target;

        RightLength = strlen(Right);
        //LeftLength = strnlen(Left, RightLength + 1);
        LeftLength = 0;

        if (LeftLength > RightLength) {
            continue;
        }

        while (*Left && *Right && *Left++ == *Right++) {
            Count++;
        }

        if (Count > 0 && !*Right) {
            if (ARGUMENT_PRESENT(Match)) {
                Match->Index = (BYTE)Index;
                Match->NumberOfMatchedCharacters = (BYTE)Count;
                Match->String = NULL;
            }
            return (STRING_TABLE_INDEX)Index;
        }
    }

    return NO_MATCH_FOUND;
}


VOID
Scratch1(
    PRTL Rtl,
    PALLOCATOR Allocator
    )
{
    STRING_TABLE_INDEX Index;
    const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
    PSTRING_TABLE StringTable;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefixOfStringInTable;
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
    IsPrefixOfStringInTable = Api->IsPrefixOfStringInTable;


    Index = IsPrefixOfStringInTable(StringTable, &lmno, NULL);
    ASSERT(Index == NoMatchFound);

    Index = IsPrefixOfStringInTable(StringTable,
                                    &abcdefghijklmnopqrstuvw,
                                    NULL);
    ASSERT(Index == 0);

    Index = IsPrefixOfStringInTable(StringTable, &abcd, NULL);
    ASSERT(Index == 2);

    Index = IsPrefixOfStringInTable(StringTable, &abcdefghijklm, NULL);
    ASSERT(Index == 1);

    Index = IsPrefixOfStringInTable(StringTable, &fox1, NULL);
    ASSERT(Index == 6);

    Index = IsPrefixOfStringInTable(StringTable,
                                    &abcdefghijklmnopqrstuvwyxz,
                                    NULL);
    ASSERT(Index == 0);

    Index = IsPrefixOfStringInTable(StringTable,
                                    &abcdefghijklmnopqrstu,
                                    NULL);
    ASSERT(Index == 1);

    Index = IsPrefixOfStringInTable(StringTable, &lmnopqrstuv, NULL);
    ASSERT(Index == 8);

    DESTROY_TABLE(StringTable);

}

#define FINISH_TIMESTAMP_2(Id, String, Iterations)           \
    OUTPUT_STRING(&Timestamp##Id##.Name);                    \
    OUTPUT_SEP();                                            \
    OUTPUT_STRING(String);                                   \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Iterations);                                  \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MinimumTsc.QuadPart);         \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MaximumTsc.QuadPart);         \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.TotalTsc.QuadPart);           \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MinimumCycles.QuadPart);      \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MaximumCycles.QuadPart);      \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.TotalCycles.QuadPart);        \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MinimumNanoseconds.QuadPart); \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MaximumNanoseconds.QuadPart); \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.TotalNanoseconds.QuadPart);   \
    OUTPUT_LF()

#define FINISH_TIMESTAMP(Id, String)                         \
    OUTPUT_STRING(&Timestamp##Id##.Name);                    \
    OUTPUT_SEP();                                            \
    OUTPUT_STRING(String);                                   \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MinimumTsc.QuadPart);         \
    OUTPUT_LF()

typedef struct _NAMED_FUNCTION {
        STRING Name;
        PIS_PREFIX_OF_STRING_IN_TABLE Function;
} NAMED_FUNCTION;
typedef NAMED_FUNCTION *PNAMED_FUNCTION;

#define NAMED_FUNC(Name) { RTL_CONSTANT_STRING(#Name), Api->Name }

typedef struct _TEST_INPUT {
    STRING_TABLE_INDEX Expected;
    PSTRING String;
} TEST_INPUT;
typedef TEST_INPUT *PTEST_INPUT;

typedef DECLSPEC_ALIGN(32) union _ALIGNED_BUFFER {
    CHAR Chars[32];
    WCHAR WideChars[16];
} ALIGNED_BUFFER;
typedef ALIGNED_BUFFER *PALIGNED_BUFFER;
C_ASSERT(sizeof(ALIGNED_BUFFER) == 32);

VOID
Scratch4(
    PRTL Rtl,
    PALLOCATOR Allocator
    )
{
    BOOL Success;
    ULONG Index;
    ULONG Warmup;
    ULONG Iterations;
    ULONG OldCodePage;
    ULARGE_INTEGER BytesToWrite;
    HANDLE OutputHandle;
    ULONG BytesWritten;
    ULONG CharsWritten;
    PCHAR Output;
    PCHAR OutputBuffer;
    ULONGLONG Alignment;
    ULONGLONG OutputBufferSize;
    STRING_TABLE_INDEX Result;
    TIMESTAMP Timestamp1;
    LARGE_INTEGER Frequency;
    const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
    PSTRING_TABLE StringTable;
    LARGE_INTEGER Delay = { 0, 1 };
    STRING_ARRAY16 StringArray16 = CONSTANT_STRING_ARRAY16(
        NtfsAttrDefName,
        NtfsBadClusName,
        NtfsBitmapName,
        NtfsBootName,
        NtfsExtendName,
        NtfsLogFileName,
        NtfsMftMirrName,
        NtfsMftName,
        NtfsSecureName,
        NtfsUpCaseName,
        NtfsVolumeName,
        NtfsCairoName,
        NtfsIndexAllocationName,
        NtfsDataName,
        NtfsUnknownName,
        NtfsDotName
    );

#define NTFS_TEST_INPUT(N) { Ntfs##N, (PSTRING)&Ntfs##N##Name }

    TEST_INPUT Inputs[] = {
        NTFS_TEST_INPUT(AttrDef),
        NTFS_TEST_INPUT(BadClus),
        NTFS_TEST_INPUT(Bitmap),
        NTFS_TEST_INPUT(Boot),
        NTFS_TEST_INPUT(Extend),
        NTFS_TEST_INPUT(MftMirr),
        NTFS_TEST_INPUT(LogFile),
        NTFS_TEST_INPUT(Mft),
        NTFS_TEST_INPUT(Secure),
        NTFS_TEST_INPUT(Volume),
        NTFS_TEST_INPUT(UpCase),
        NTFS_TEST_INPUT(Cairo),
        NTFS_TEST_INPUT(IndexAllocation),
        NTFS_TEST_INPUT(Data),
        NTFS_TEST_INPUT(Unknown),
        NTFS_TEST_INPUT(Dot),
        { -1, &a },
        { -1, &ab },
        { -1, &abc },
        { -1, &fox1 },
        { -1, &abcd },
        { -1, &abcdefghijkl },
        { -1, &abcdefghijklmnopqr },
        { -1, &abcdefghijklmnopqrstuvw },
    };

    ULONG NumberOfInputs = ARRAYSIZE(Inputs);

    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix;

    NAMED_FUNCTION NamedFunctions[] = {
        NAMED_FUNC(IsPrefixOfStringInTable_1),
        NAMED_FUNC(IsPrefixOfStringInTable_2),
        NAMED_FUNC(IsPrefixOfStringInTable_3),
        NAMED_FUNC(IsPrefixOfStringInTable_4),
        NAMED_FUNC(IsPrefixOfStringInTable_5),
        NAMED_FUNC(IsPrefixOfStringInTable_6),
        NAMED_FUNC(IsPrefixOfStringInTable_7),
        NAMED_FUNC(IsPrefixOfStringInTable_8),
        NAMED_FUNC(IsPrefixOfStringInTable_9),
        NAMED_FUNC(IsPrefixOfStringInTable_10),
        NAMED_FUNC(IsPrefixOfStringInTable_x64_1),
        NAMED_FUNC(IsPrefixOfStringInTable_x64_2),
        NAMED_FUNC(IsPrefixOfStringInTable_x64_3),
    };
    ULONG NumberOfFuncs = ARRAYSIZE(NamedFunctions);

    ULONG InputIndex;
    ULONG FuncIndex;
    PTEST_INPUT Input;
    PNAMED_FUNCTION Func;

    ALIGNED_BUFFER InputBuffer;
    STRING AlignedInput;

    ZeroStruct(InputBuffer);
    Alignment = GetAddressAlignment(&InputBuffer);
    ASSERT(Alignment >= 32);

    OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    ASSERT(OutputHandle);

    QueryPerformanceFrequency(&Frequency);

    Success = Rtl->CreateBuffer(Rtl,
                                NULL,
                                10,
                                0,
                                &OutputBufferSize,
                                &OutputBuffer);
    ASSERT(Success);

    Output = OutputBuffer;

    OldCodePage = GetConsoleCP();

    ASSERT(SetConsoleCP(20127));

    DELIMITED_TABLE(&NtfsReservedNames);

    if (0) {

        //
        // Alignment output:
        //
        //      $AttrDef,8
        //      $BadClus,8
        //      $Bitmap,8
        //      $Boot,4
        //      $Extend,256
        //      $MftMirr,32
        //      $LogFile,16
        //      $Mft,4
        //      $Secure,8
        //      $Volume,8
        //      $UpCase,8
        //      $Cairo,4
        //      $INDEX_ALLOCATION,16
        //      $DATA,8
        //      ????,16
        //      .,8
        //      a,512
        //      ab,4
        //      abc,8
        //      fox1,8
        //      abcd,4
        //      abcdefghijkl,8
        //      abcdefghijklmnopqr,8
        //      abcdefghijklmnopqrstuvw,8
        //

        for (Index = 0; Index < NumberOfInputs; Index++) {
            Input = &Inputs[Index];

            Alignment = GetAddressAlignment(Input->String->Buffer);
            OUTPUT_STRING(Input->String);
            OUTPUT_SEP();
            OUTPUT_INT(Alignment);
            OUTPUT_LF();
        }
        OUTPUT_FLUSH();
        return;
    }

    Warmup = 100;
    Iterations = 1000;

    OUTPUT_RAW("Name,String,MinimumCycles\n");

    /*
        PSTRING_ARRAY StringArray = StringTable->pStringArray;
        PSTRING BaseString = &StringArray->Strings[0];
        PSTRING TargetString = &StringArray->Strings[12];
        PCHAR TargetBuffer = TargetString->Buffer;
        ULONG Sz1 = sizeof(STRING_ARRAY);
        ULONG Offset = FIELD_OFFSET(STRING_ARRAY, Strings);
    */

    __movsq((PDWORD64)&InputBuffer,
            (PDWORD64)Inputs[12].String->Buffer,
            sizeof(InputBuffer) >> 3);

    AlignedInput.Length = Inputs[12].String->Length;
    AlignedInput.MaximumLength = Inputs[12].String->MaximumLength;
    AlignedInput.Buffer = (PCHAR)&InputBuffer.Chars;

    Api->IsPrefixOfStringInTable_8(StringTable,
                                   Inputs[0].String,
                                   NULL);

    Result = Api->IsPrefixOfStringInTable_x64_3(StringTable,
                                                &AlignedInput,
                                                NULL);

    Result = Api->IsPrefixOfStringInTable_10(StringTable,
                                             &AlignedInput,
                                             NULL);

#if 0
#define YIELD_EXECUTION() Rtl->NtYieldExecution()
#endif
#if 1
#define YIELD_EXECUTION() Rtl->NtDelayExecution(TRUE, &Delay)
#endif

    for (InputIndex = 0; InputIndex < NumberOfInputs; InputIndex++) {
        Input = &Inputs[InputIndex];

        //
        // Copy the input string into our aligned buffer.
        //

        __movsq((PDWORD64)&InputBuffer,
                (PDWORD64)Input->String->Buffer,
                sizeof(InputBuffer) >> 3);

        AlignedInput.Length = Input->String->Length;
        AlignedInput.MaximumLength = Input->String->MaximumLength;
        AlignedInput.Buffer = (PCHAR)&InputBuffer.Chars;

        Alignment = GetAddressAlignment(AlignedInput.Buffer);
        if (Alignment < 32) {
            __debugbreak();
        }


        //
        // Do the c-string based version manually, as it has a different
        // signature to the other methods.
        //

        Result = Api->IsPrefixOfCStrInArray((PCSZ *)NtfsReservedNamesCStrings,
                                            AlignedInput.Buffer,
                                            NULL);

        ASSERT(Result == Input->Expected);

        INIT_TIMESTAMP(1, "IsPrefixOfCStrInArray");

        YIELD_EXECUTION();
        for (Index = 0; Index < Warmup; Index++) {
            Result = Api->IsPrefixOfCStrInArray(
                (PCSZ *)NtfsReservedNamesCStrings,
                AlignedInput.Buffer,
                NULL
            );
        }

#if 1
        RESET_TIMESTAMP(1);
        START_TIMESTAMP(1);
        for (Index = 0; Index < Iterations; Index++) {
            Result = Api->IsPrefixOfCStrInArray(
                (PCSZ *)NtfsReservedNamesCStrings,
                AlignedInput.Buffer,
                NULL
            );
        }
        END_TIMESTAMP(1);
        FINISH_TIMESTAMP(1, Input->String);
#endif

#if 0
        RESET_TIMESTAMP(1);
        for (Index = 0; Index < Iterations; Index++) {
            START_TIMESTAMP(1);
            Result = Api->IsPrefixOfCStrInArray(
                (PCSZ *)NtfsReservedNamesCStrings,
                AlignedInput.Buffer,
                NULL
            );
            END_TIMESTAMP(1);
        }
        FINISH_TIMESTAMP(1, Input->String);
#endif


        //
        // Continue with the remaining functions.
        //

        for (FuncIndex = 0; FuncIndex < NumberOfFuncs; FuncIndex++) {
        //for (FuncIndex = NumberOfFuncs-1; FuncIndex != 0; FuncIndex--) {
            Func = &NamedFunctions[FuncIndex];
            IsPrefix = Func->Function;

            /*
            if (InputIndex == 12 && IsPrefix == Api->IsPrefixOfStringInTable_x64_2) {
                __debugbreak();
            }
            if (IsPrefix == Api->IsPrefixOfStringInTable_8) {
                __debugbreak();
            }
            */

            /*
            if (IsPrefix == Api->IsPrefixOfStringInTable_x64_3) {
                __debugbreak();
            }
            */

            Result = IsPrefix(StringTable, &AlignedInput, NULL);

            if (IsPrefix != Api->IsPrefixOfStringInTable_x64_1 &&
                //IsPrefix != Api->IsPrefixOfStringInTable_x64_2 &&
                //IsPrefix != Api->IsPrefixOfStringInTable_x64_3 &&
                //IsPrefix != Api->IsPrefixOfStringInTable_x64_4 &&
                1) {
                ASSERT(Result == Input->Expected);
            }

            //ASSERT(Result == Input->Expected);

            INIT_TIMESTAMP_FROM_STRING(1, (&Func->Name));

            YIELD_EXECUTION();
            for (Index = 0; Index < Warmup; Index++) {
                Result = IsPrefix(StringTable, &AlignedInput, NULL);
            }

#if 1
            RESET_TIMESTAMP(1);
            START_TIMESTAMP(1);
            for (Index = 0; Index < Iterations; Index++) {
                Result = IsPrefix(StringTable, &AlignedInput, NULL);
            }
            END_TIMESTAMP(1);
            FINISH_TIMESTAMP(1, Input->String);
#endif

#if 0
            RESET_TIMESTAMP(1);
            for (Index = 0; Index < Iterations; Index++) {
                START_TIMESTAMP(1);
                Result = IsPrefix(StringTable, &AlignedInput, NULL);
                END_TIMESTAMP(1);
            }
            FINISH_TIMESTAMP(1, Input->String);
#endif
        }
    }

    OUTPUT_FLUSH();

    DESTROY_TABLE(StringTable);

    ASSERT(SetConsoleCP(OldCodePage));

}

VOID
Scratch5(
    PRTL Rtl,
    PALLOCATOR Allocator
    )
{
    BOOL Success;
    ULONG Index;
    ULONG Warmup;
    ULONG Iterations;
    ULONG OldCodePage;
    ULARGE_INTEGER BytesToWrite;
    HANDLE OutputHandle;
    ULONG BytesWritten;
    ULONG CharsWritten;
    PCHAR Output;
    PCHAR OutputBuffer;
    ULONGLONG Alignment;
    ULONGLONG OutputBufferSize;
    STRING_TABLE_INDEX Result;
    TIMESTAMP Timestamp1;
    LARGE_INTEGER Frequency;
    const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
    PSTRING_TABLE StringTable;
    STRING_MATCH StringMatch;
    LARGE_INTEGER Delay = { 0, 1 };
    STRING_ARRAY16 StringArray16 = CONSTANT_STRING_ARRAY16(
        NtfsAttrDefName,
        NtfsBadClusName,
        NtfsBitmapName,
        NtfsBootName,
        NtfsExtendName,
        NtfsLogFileName,
        NtfsMftMirrName,
        NtfsMftName,
        NtfsSecureName,
        NtfsUpCaseName,
        NtfsVolumeName,
        NtfsCairoName,
        NtfsIndexAllocationName,
        NtfsDataName,
        NtfsUnknownName,
        NtfsDotName
    );

#define NTFS_TEST_INPUT(N) { Ntfs##N, (PSTRING)&Ntfs##N##Name }

    TEST_INPUT Inputs[] = {
        NTFS_TEST_INPUT(AttrDef),
        NTFS_TEST_INPUT(BadClus),
        NTFS_TEST_INPUT(Bitmap),
        NTFS_TEST_INPUT(Boot),
        NTFS_TEST_INPUT(Extend),
        NTFS_TEST_INPUT(LogFile),
        NTFS_TEST_INPUT(MftMirr),
        NTFS_TEST_INPUT(Mft),
        NTFS_TEST_INPUT(Secure),
        NTFS_TEST_INPUT(Volume),
        NTFS_TEST_INPUT(UpCase),
        NTFS_TEST_INPUT(Cairo),
        NTFS_TEST_INPUT(IndexAllocation),
        NTFS_TEST_INPUT(Data),
        NTFS_TEST_INPUT(Unknown),
        NTFS_TEST_INPUT(Dot),
        { -1, &a },
        { -1, &ab },
        { -1, &abc },
        { -1, &fox1 },
        { -1, &abcd },
        { -1, &abcdefghijkl },
        { -1, &abcdefghijklmnopqr },
        { -1, &abcdefghijklmnopqrstuvw },
    };

    ULONG NumberOfInputs = ARRAYSIZE(Inputs);

    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix;

    NAMED_FUNCTION NamedFunctions[] = {
        NAMED_FUNC(IsPrefixOfStringInTable),
        NAMED_FUNC(IsPrefixOfStringInTable_2),
        NAMED_FUNC(IsPrefixOfStringInTable_3),
        NAMED_FUNC(IsPrefixOfStringInTable_4),
        NAMED_FUNC(IsPrefixOfStringInTable_5),
        NAMED_FUNC(IsPrefixOfStringInTable_6),
        NAMED_FUNC(IsPrefixOfStringInTable_7),
        NAMED_FUNC(IsPrefixOfStringInTable_8),
        NAMED_FUNC(IsPrefixOfStringInTable_x64_1),
        NAMED_FUNC(IsPrefixOfStringInTable_x64_2),
    };
    ULONG NumberOfFuncs = ARRAYSIZE(NamedFunctions);

    ULONG InputIndex;
    ULONG FuncIndex;
    PTEST_INPUT Input;
    PNAMED_FUNCTION Func;

    ALIGNED_BUFFER InputBuffer;
    STRING AlignedInput;

    ZeroStruct(StringMatch);
    ZeroStruct(InputBuffer);
    Alignment = GetAddressAlignment(&InputBuffer);
    ASSERT(Alignment >= 32);

    OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    ASSERT(OutputHandle);

    QueryPerformanceFrequency(&Frequency);

    Success = Rtl->CreateBuffer(Rtl,
                                NULL,
                                10,
                                0,
                                &OutputBufferSize,
                                &OutputBuffer);
    ASSERT(Success);

    Output = OutputBuffer;

    OldCodePage = GetConsoleCP();

    ASSERT(SetConsoleCP(20127));

    DELIMITED_TABLE(&NtfsReservedNames);

    if (0) {

        //
        // Alignment output:
        //
        //      $AttrDef,8
        //      $BadClus,8
        //      $Bitmap,8
        //      $Boot,4
        //      $Extend,256
        //      $MftMirr,32
        //      $LogFile,16
        //      $Mft,4
        //      $Secure,8
        //      $Volume,8
        //      $UpCase,8
        //      $Cairo,4
        //      $INDEX_ALLOCATION,16
        //      $DATA,8
        //      ????,16
        //      .,8
        //      a,512
        //      ab,4
        //      abc,8
        //      fox1,8
        //      abcd,4
        //      abcdefghijkl,8
        //      abcdefghijklmnopqr,8
        //      abcdefghijklmnopqrstuvw,8
        //

        for (Index = 0; Index < NumberOfInputs; Index++) {
            Input = &Inputs[Index];

            Alignment = GetAddressAlignment(Input->String->Buffer);
            OUTPUT_STRING(Input->String);
            OUTPUT_SEP();
            OUTPUT_INT(Alignment);
            OUTPUT_LF();
        }
        OUTPUT_FLUSH();
        return;
    }

    Warmup = 500;
    Iterations = 5000;

    OUTPUT_RAW("Name,String,MinimumCycles\n");

    /*
        PSTRING_ARRAY StringArray = StringTable->pStringArray;
        PSTRING BaseString = &StringArray->Strings[0];
        PSTRING TargetString = &StringArray->Strings[12];
        PCHAR TargetBuffer = TargetString->Buffer;
        ULONG Sz1 = sizeof(STRING_ARRAY);
        ULONG Offset = FIELD_OFFSET(STRING_ARRAY, Strings);
    */

#define COPY_TEST_INPUT(Ix)                                              \
    __movsq((PDWORD64)&InputBuffer,                                      \
            (PDWORD64)Inputs[Ix].String->Buffer,                         \
            sizeof(InputBuffer) >> 3);                                   \
    AlignedInput.Length = Inputs[Ix].String->Length;                     \
    AlignedInput.MaximumLength = Inputs[Ix].String->MaximumLength;       \
    AlignedInput.Buffer = (PCHAR)&InputBuffer.Chars;

#define COPY_STRING_MATCH(Ix)                                            \
    StringMatch.Index = Ix;                                              \
    StringMatch.NumberOfMatchedCharacters = Inputs[Ix].String->Length;   \
    StringMatch.String = &StringTable->pStringArray->Strings[Ix];

    COPY_TEST_INPUT(12);

    Result = Api->IsPrefixOfStringInTable_x64_2(StringTable,
                                                &AlignedInput,
                                                &StringMatch);

    ASSERT(StringMatch.Index == 12);
    ASSERT(StringMatch.NumberOfMatchedCharacters == 17);
    ASSERT(StringMatch.String == &StringTable->pStringArray->Strings[12]);

    ZeroStruct(StringMatch);

    COPY_TEST_INPUT(6);

    Result = Api->IsPrefixOfStringInTable_x64_2(StringTable,
                                                &AlignedInput,
                                                &StringMatch);

    ASSERT(StringMatch.Index == 6);
    ASSERT(StringMatch.NumberOfMatchedCharacters == 8);
    ASSERT(StringMatch.String == &StringTable->pStringArray->Strings[6]);

#if 0
#define YIELD_EXECUTION() Rtl->NtYieldExecution()
#endif
#if 1
#define YIELD_EXECUTION() Rtl->NtDelayExecution(TRUE, &Delay)
#endif

    for (InputIndex = 0; InputIndex < NumberOfInputs; InputIndex++) {
        Input = &Inputs[InputIndex];

        //
        // Copy the input string into our aligned buffer.
        //

        COPY_TEST_INPUT(InputIndex);

        //
        // Do the c-string based version manually, as it has a different
        // signature to the other methods.
        //

        Result = Api->IsPrefixOfCStrInArray((PCSZ *)NtfsReservedNamesCStrings,
                                            AlignedInput.Buffer,
                                            NULL);

        ASSERT(Result == Input->Expected);

        INIT_TIMESTAMP(1, "IsPrefixOfCStrInArray");

        YIELD_EXECUTION();
        for (Index = 0; Index < Warmup; Index++) {
            Result = Api->IsPrefixOfCStrInArray(
                (PCSZ *)NtfsReservedNamesCStrings,
                AlignedInput.Buffer,
                NULL
            );
        }

        RESET_TIMESTAMP(1);
        START_TIMESTAMP(1);
        for (Index = 0; Index < Iterations; Index++) {
            Result = Api->IsPrefixOfCStrInArray(
                (PCSZ *)NtfsReservedNamesCStrings,
                AlignedInput.Buffer,
                NULL
            );
        }
        END_TIMESTAMP(1);
        FINISH_TIMESTAMP(1, Input->String);

        //
        // Continue with the remaining functions.
        //

        //for (FuncIndex = 0; FuncIndex < NumberOfFuncs; FuncIndex++) {
        for (FuncIndex = NumberOfFuncs-1; FuncIndex != 0; FuncIndex--) {
            Func = &NamedFunctions[FuncIndex];
            IsPrefix = Func->Function;

            /*
            if (InputIndex == 12 && IsPrefix == Api->IsPrefixOfStringInTable_x64_2) {
                __debugbreak();
            }
            */

            Result = IsPrefix(StringTable, &AlignedInput, NULL);

            if (IsPrefix != Api->IsPrefixOfStringInTable_x64_1) {
                ASSERT(Result == Input->Expected);
            }

            //ASSERT(Result == Input->Expected);

            INIT_TIMESTAMP_FROM_STRING(1, (&Func->Name));

            YIELD_EXECUTION();
            for (Index = 0; Index < Warmup; Index++) {
                Result = IsPrefix(StringTable, &AlignedInput, NULL);
            }

            RESET_TIMESTAMP(1);
            START_TIMESTAMP(1);
            for (Index = 0; Index < Iterations; Index++) {
                Result = IsPrefix(StringTable, &AlignedInput, NULL);
            }
            END_TIMESTAMP(1);
            FINISH_TIMESTAMP(1, Input->String);
        }
    }

    OUTPUT_FLUSH();

    DESTROY_TABLE(StringTable);

    ASSERT(SetConsoleCP(OldCodePage));

}

VOID
Scratch6(
    PRTL Rtl,
    PALLOCATOR Allocator
    )
{
    BOOL Success;
    PCHAR Buffer;
    PCHAR End;
    ULONGLONG BufferSize;
    STRING String;
    HANDLE ProcessHandle = NULL;
    PSTRING_TABLE StringTable;
    STRING_TABLE_INDEX Index;
    STRING Filler = RTL_CONSTANT_STRING("Filler12");
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix;

    DELIMITED_TABLE(&NtfsReservedNames);

    Success = Rtl->CreateBuffer(Rtl,
                                &ProcessHandle,
                                1,
                                0,
                                &BufferSize,
                                (PPVOID)&Buffer);

    ASSERT(Success);

    End = (Buffer + BufferSize) - 8;
    CopyMemory(End, Filler.Buffer, Filler.Length);

    String.Length = 9;
    String.MaximumLength = 9;
    String.Buffer = End;

    IsPrefix = Api->IsPrefixOfStringInTable_x64_2;

    Index = IsPrefix(StringTable, &String, NULL);
    ASSERT(Index == NO_MATCH_FOUND);

    Rtl->VirtualFreeEx(ProcessHandle, Buffer, 0, MEM_RELEASE);

}


DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    LONG ExitCode = 0;
    LONG SizeOfRtl = sizeof(GlobalRtl);
    HMODULE RtlModule;
    RTL_BOOTSTRAP Bootstrap;
    HANDLE ProcessHandle;
    HANDLE ThreadHandle;
    SYSTEM_INFO SystemInfo;
    DWORD_PTR ProcessAffinityMask;
    DWORD_PTR SystemAffinityMask;
    DWORD_PTR OldThreadAffinityMask;
    DWORD_PTR AffinityMask;
    DWORD IdealProcessor;
    DWORD Result;

    if (!BootstrapRtl(&RtlModule, &Bootstrap)) {
        ExitCode = 1;
        goto Error;
    }

    if (!Bootstrap.InitializeHeapAllocator(&GlobalAllocator)) {
        ExitCode = 1;
        goto Error;
    }

    CHECKED_MSG(
        Bootstrap.InitializeRtl(&GlobalRtl, &SizeOfRtl),
        "InitializeRtl()"
    );

    Rtl = &GlobalRtl;
    Allocator = &GlobalAllocator;

    SetCSpecificHandler(Rtl->__C_specific_handler);

    ASSERT(LoadStringTableApi(Rtl,
                              &GlobalStringTableModule,
                              NULL,
                              sizeof(GlobalApi),
                              (PSTRING_TABLE_ANY_API)&GlobalApi));
    Api = &GlobalApi;

    GetSystemInfo(&SystemInfo);
    IdealProcessor = SystemInfo.dwNumberOfProcessors - 1;

    ProcessHandle = GetCurrentProcess();
    ThreadHandle = GetCurrentThread();

    Result = SetThreadIdealProcessor(ThreadHandle, IdealProcessor);
    ASSERT(Result != (DWORD)-1);

    ASSERT(GetProcessAffinityMask(ProcessHandle,
                                  &ProcessAffinityMask,
                                  &SystemAffinityMask));

    AffinityMask = ((DWORD_PTR)1 << (DWORD_PTR)IdealProcessor);

    OldThreadAffinityMask = SetThreadAffinityMask(ThreadHandle, AffinityMask);
    ASSERT(OldThreadAffinityMask);

    ASSERT(SetThreadPriority(ThreadHandle, THREAD_PRIORITY_HIGHEST));

    Scratch4(Rtl, Allocator);

Error:

    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
