/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This is the main file for the PerfectHashTableSelfTest component..

--*/

#include "stdafx.h"

//
// Define globals.
//

RTL GlobalRtl;
ALLOCATOR GlobalAllocator;

PRTL Rtl;
PALLOCATOR Allocator;

HMODULE GlobalModule = 0;

PERFECT_HASH_TABLE_API_EX GlobalApi;
PPERFECT_HASH_TABLE_API_EX Api;

HMODULE GlobalRtlModule = 0;
HMODULE GlobalPerfectHashTableModule = 0;

//
// Main entry point.
//

DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    BOOL Success;
    LONG ExitCode;
    LONG SizeOfRtl = sizeof(GlobalRtl);
    ULONG OldCodePage;
    HMODULE RtlModule;
    HANDLE StdErrorHandle;
    RTL_BOOTSTRAP Bootstrap;
    HMODULE Shell32Module = NULL;
    PCOMMAND_LINE_TO_ARGVW CommandLineToArgvW;
    PWSTR CommandLineW;
    LONG NumberOfArguments;
    PPWSTR ArgvW;
    const STRING Usage = RTL_CONSTANT_STRING(
        "Usage: PerfectHashTableSelfTest.exe <Test Data Directory>\n"
    );
    UNICODE_STRING Path;
    PPERFECT_HASH_TABLE_ANY_API AnyApi;

    if (!BootstrapRtl(&RtlModule, &Bootstrap)) {
        ExitCode = 1;
        goto Error;
    }

    Success = Bootstrap.InitializeHeapAllocatorEx(&GlobalAllocator,
                                                  HEAP_GENERATE_EXCEPTIONS,
                                                  0,
                                                  0);

    if (!Success) {
        ExitCode = 1;
        goto Error;
    }

    CHECKED_MSG(
        Bootstrap.InitializeRtl(&GlobalRtl, &SizeOfRtl),
        "InitializeRtl()"
    );

    Rtl = &GlobalRtl;
    Allocator = &GlobalAllocator;
    Api = &GlobalApi;
    AnyApi = (PPERFECT_HASH_TABLE_ANY_API)&GlobalApi;

    SetCSpecificHandler(Rtl->__C_specific_handler);

    ASSERT(LoadPerfectHashTableApi(Rtl,
                                   &GlobalPerfectHashTableModule,
                                   NULL,
                                   sizeof(GlobalApi),
                                   AnyApi));

    //
    // Extract the command line for the current process.
    //

    LOAD_LIBRARY_A(Shell32Module, Shell32);

    RESOLVE_FUNCTION(CommandLineToArgvW,
                     Shell32Module,
                     PCOMMAND_LINE_TO_ARGVW,
                     CommandLineToArgvW);

    CHECKED_MSG(CommandLineW = GetCommandLineW(), "GetCommandLineW()");

    ArgvW = CommandLineToArgvW(CommandLineW, &NumberOfArguments);

    CHECKED_MSG(ArgvW, "Shell32!CommandLineToArgvW()");

    switch (NumberOfArguments) {
        case 2:

            Path.Buffer = ArgvW[1];
            Path.Length = (USHORT)wcslen(Path.Buffer) << 1;
            Path.MaximumLength = Path.Length + sizeof(Path.Buffer[0]);

            Success = Api->SelfTestPerfectHashTable(Rtl,
                                                    Allocator,
                                                    AnyApi,
                                                    &PerfectHashTableTestData,
                                                    &Path);
            ExitCode = (Success ? 0 : 1);
            break;

        default:
            StdErrorHandle = GetStdHandle(STD_ERROR_HANDLE);
            ASSERT(StdErrorHandle);
            OldCodePage = GetConsoleCP();
            ASSERT(SetConsoleCP(20127));
            Success = WriteFile(StdErrorHandle,
                                Usage.Buffer,
                                Usage.Length,
                                NULL,
                                NULL);
            ASSERT(Success);
            SetConsoleCP(OldCodePage);
            ExitCode = 1;
            break;
    }

Error:

    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
