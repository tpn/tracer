/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This is the main file for the StringTable benchmark.

--*/

#include "stdafx.h"

//
// Disable global optimizations, even in release builds.  Without this, the
// compiler does clever things with regards to scheduling the underlying rdtsc
// calls that affect reported times.
//

#pragma optimize("", off)

//
// Define constants.
//

const ULONG Warmup = 100;
const ULONG Batches = 100;
const ULONG Iterations = 1000;

//
// Define globals.
//

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
// Define helper macro glue.
//

#define PTR(p) ((ULONG_PTR)(p))
#define LEN(String) ((LONG)((STRING)(String)).Length)

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
    OUTPUT_RAW(BuildConfigString);                           \
    OUTPUT_SEP();                                            \
    OUTPUT_STRING(String);                                   \
    OUTPUT_SEP();                                            \
    OUTPUT_INT(Timestamp##Id##.MinimumTsc.QuadPart);         \
    OUTPUT_LF()

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

//
// Benchmark functions.
//

VOID
Benchmark1(
    PRTL Rtl,
    PALLOCATOR Allocator
    )
{
    BOOL Success;
    ULONG Count;
    ULONG Index;
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
    ULONG InputIndex;
    ULONG FuncIndex;
    PCSTRING_TABLE_TEST_INPUT Input;
    PCSTRING_TABLE_FUNCTION_OFFSET Func;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix;

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

        for (Index = 0; Index < NumberOfNtfsTestInputs; Index++) {
            Input = &NtfsTestInputs[Index];

            Alignment = GetAddressAlignment(Input->String->Buffer);
            OUTPUT_STRING(Input->String);
            OUTPUT_SEP();
            OUTPUT_INT(Alignment);
            OUTPUT_LF();
        }
        OUTPUT_FLUSH();
        return;
    }

    OUTPUT_RAW("Name,BuildConfig,String,MinimumCycles\n");

#if 0
#define YIELD_EXECUTION() Rtl->NtYieldExecution()
#endif
#if 1
#define YIELD_EXECUTION() Rtl->NtDelayExecution(TRUE, &Delay)
#endif

    for (InputIndex = 0; InputIndex < NumberOfNtfsTestInputs; InputIndex++) {
        Input = &NtfsTestInputs[InputIndex];

        //
        // Copy the input string into our aligned buffer.
        //

        COPY_TEST_INPUT(NtfsTestInputs, InputIndex);

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
        Count = Batches;
        while (Count--) {
            START_TIMESTAMP(1);
            for (Index = 0; Index < Iterations; Index++) {
                Result = Api->IsPrefixOfCStrInArray(
                    (PCSZ *)NtfsReservedNamesCStrings,
                    AlignedInput.Buffer,
                    NULL
                );
            }
            END_TIMESTAMP(1);
        }
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

        for (FuncIndex = 0; FuncIndex < NumberOfIsPrefixFunctions; FuncIndex++) {
            Func = &IsPrefixFunctions[FuncIndex];
            IsPrefix = LOAD_FUNCTION_FROM_OFFSET(Api, Func->Offset);

            Result = IsPrefix(StringTable, &AlignedInput, NULL);

            if (Func->Verify) {
                ASSERT(Result == Input->Expected);
            }

            INIT_TIMESTAMP_FROM_STRING(1, (&Func->Name));

            YIELD_EXECUTION();
            for (Index = 0; Index < Warmup; Index++) {
                Result = IsPrefix(StringTable, &AlignedInput, NULL);
            }

#if 1
            RESET_TIMESTAMP(1);
            Count = Batches;
            while (Count--) {
                START_TIMESTAMP(1);
                for (Index = 0; Index < Iterations; Index++) {
                    Result = IsPrefix(StringTable, &AlignedInput, NULL);
                }
                END_TIMESTAMP(1);
            }
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

volatile ULONG CtrlCPressed;

BOOL
RunSingleFunctionCtrlCHandler(
    ULONG ControlType
    )
{
    if (ControlType == CTRL_C_EVENT) {
        CtrlCPressed = 1;
        return TRUE;
    }
    return FALSE;
}

VOID
RunSingleFunction(
    PRTL Rtl,
    PALLOCATOR Allocator,
    ULONG TargetFunctionId,
    ULONG TargetInputId
    )
{
    BOOL Success;
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
    LARGE_INTEGER Frequency;
    const STRING_TABLE_INDEX NoMatchFound = NO_MATCH_FOUND;
    PSTRING_TABLE StringTable;
    LARGE_INTEGER Delay = { 0, 1 };
    ULONG InputIndex = TargetInputId;
    ULONG FuncIndex = TargetFunctionId;
    PCSTRING_TABLE_TEST_INPUT Input;
    PCSTRING_TABLE_FUNCTION_OFFSET Func;
    PIS_PREFIX_OF_STRING_IN_TABLE IsPrefix;
    CtrlCPressed = 0;

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

    Input = &NtfsTestInputs[InputIndex];
    Func = &IsPrefixFunctions[FuncIndex];
    IsPrefix = LOAD_FUNCTION_FROM_OFFSET(Api, Func->Offset);

    COPY_TEST_INPUT(NtfsTestInputs, InputIndex);

    OUTPUT_STRING(&Func->Name);
    OUTPUT_SEP();
    OUTPUT_STRING(Input->String);
    OUTPUT_LF();
    OUTPUT_FLUSH();

    Result = IsPrefix(StringTable, &AlignedInput, NULL);

    if (Func->Verify) {
        ASSERT(Result == Input->Expected);
    }

    ASSERT(Rtl->SetConsoleCtrlHandler(RunSingleFunctionCtrlCHandler, TRUE));

    while (!CtrlCPressed) {
        IsPrefix(StringTable, &AlignedInput, NULL);
    }

    OUTPUT_RAW("Finished.\n");
    OUTPUT_FLUSH();

    DESTROY_TABLE(StringTable);

    ASSERT(SetConsoleCP(OldCodePage));
}

//
// Main entry point.
//

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
    HMODULE Shell32Module = NULL;
    PCOMMAND_LINE_TO_ARGVW CommandLineToArgvW;
    PSTR CommandLineA;
    PWSTR CommandLineW;
    LONG NumberOfArguments;
    PPSTR ArgvA;
    PPWSTR ArgvW;
    ULONG TargetFunctionId;
    ULONG TargetInputId;
    NTSTATUS Status;

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

    //
    // Extract the command line for the current process.
    //

    LOAD_LIBRARY_A(Shell32Module, Shell32);

    RESOLVE_FUNCTION(CommandLineToArgvW,
                     Shell32Module,
                     PCOMMAND_LINE_TO_ARGVW,
                     CommandLineToArgvW);

    CHECKED_MSG(CommandLineA = GetCommandLineA(), "GetCommandLineA()");
    CHECKED_MSG(CommandLineW = GetCommandLineW(), "GetCommandLineW()");

    ArgvW = CommandLineToArgvW(CommandLineW,
                               &NumberOfArguments);

    CHECKED_MSG(ArgvW, "Shell32!CommandLineToArgvW()");

    CHECKED_MSG(
        Rtl->ArgvWToArgvA(
            ArgvW,
            NumberOfArguments,
            &ArgvA,
            NULL,
            Allocator
        ),
        "Rtl!ArgvWToArgA"
    );

    switch (NumberOfArguments) {
        case 1:
            Benchmark1(Rtl, Allocator);
            break;

        case 3:
            CHECKED_NTSTATUS_MSG(
                Rtl->RtlCharToInteger(
                    ArgvA[1],
                    10,
                    &TargetFunctionId
                ),
                "Rtl->RtlCharToInteger(ArgvA[1])"
            );

            CHECKED_NTSTATUS_MSG(
                Rtl->RtlCharToInteger(
                    ArgvA[2],
                    10,
                    &TargetInputId
                ),
                "Rtl->RtlCharToInteger(ArgvA[2])"
            );


            RunSingleFunction(Rtl,
                              Allocator,
                              TargetFunctionId,
                              TargetInputId);

            break;


        default:
            break;
    }


Error:

    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
