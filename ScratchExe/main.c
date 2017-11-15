/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This module is the main entry point for the thunk executable.
    It implements mainCRTStartup().

--*/

#include "stdafx.h"

LONG
FindStreams(
    PCUNICODE_STRING Path
    )
{
    HANDLE Handle;
    ULONG LastError;
    WIN32_FIND_STREAM_DATA StreamData;

    Handle = FindFirstStreamW(Path->Buffer,
                              FindStreamInfoStandard,
                              &StreamData,
                              0);

    if (Handle == INVALID_HANDLE_VALUE) {
        LastError = GetLastError();
        return LastError;
    }

    OutputDebugStringW(StreamData.cStreamName);

    while (FindNextStreamW(Handle, &StreamData)) {
        OutputDebugStringW(StreamData.cStreamName);
    }

    FindClose(Handle);

    return 0;
}

#if 0

typedef
VOID
(TEST_PARAMS1)(
    _In_ LONG Param1,
    _In_ LONG Param2,
    _In_ LONG Param3,
    _In_ LONG Param4,
    _In_ LONG Param5,
    _In_ LONG Param6,
    _In_ LONG Param7,
    _In_ LONG Param8
    );
typedef TEST_PARAMS1 *PTEST_PARAMS1;

typedef
VOID
(TEST_PARAMS2)(
    _In_ LONG Param1,
    _In_ LONG Param2,
    _In_ LONG Param3,
    _In_ LONG Param4,
    _In_ LONG Param5,
    _In_ LONG Param6,
    _In_ LONG Param7,
    _In_ LONG Param8
    );
typedef TEST_PARAMS2 *PTEST_PARAMS2;

extern TEST_PARAMS1 TestParams1;
extern TEST_PARAMS2 TestParams2;

#endif

DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    LONG ExitCode = 0;

#if 0

    TestParams1(1, 2, 3, 4, 5, 6, 7, 8);
    TestParams2(1, 2, 3, 4, 5, 6, 7, 8);

#endif

#if 0
    HMODULE Module;
    PROC Proc;
    PTEST_PARAMS1 TestParams1;
    PTEST_PARAMS2 TestParams2;

    Module = LoadLibraryA("T:\\Users\\Trent\\src\\tracer\\x64\\Release\\Asm.dll");
    Proc = GetProcAddress(Module, "TestParams1");
    TestParams1 = (PTEST_PARAMS1)Proc;

    TestParams1(1, 2, 3, 4, 5, 6, 7, 8);

    Proc = GetProcAddress(Module, "TestParams2");
    TestParams2 = (PTEST_PARAMS2)Proc;

    TestParams2(1, 2, 3, 4, 5, 6, 7, 8);

#endif

#if 0
    BOOL Success;
    PRTL Rtl;
    PTRACER_CONFIG TracerConfig;
    HANDLE EventHandle;
    ALLOCATOR Allocator;
    const UNICODE_STRING Prefix = RTL_CONSTANT_STRING(L"Test");
    UNICODE_STRING EventName;
    WCHAR EventNameBuffer[256];

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        ExitCode = 1;
        goto Error;
    }

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            &Allocator,
            (PUNICODE_STRING)&TracerRegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    EventName.Length = 0;
    EventName.MaximumLength = sizeof(EventNameBuffer);
    EventName.Buffer = (PWSTR)&EventNameBuffer;

    Success = Rtl->CreateNamedEvent(Rtl,
                                    &Allocator,
                                    &EventHandle,
                                    NULL,
                                    FALSE,
                                    FALSE,
                                    (PCUNICODE_STRING)&Prefix,
                                    NULL,
                                    &EventName);

    if (!Success) {
        __debugbreak();
    }


#endif

#if 1
    BOOL Success;
    PRTL Rtl;
    PTRACER_CONFIG TracerConfig;
    //HANDLE EventHandle;
    ALLOCATOR Allocator;
    const UNICODE_STRING Prefix = RTL_CONSTANT_STRING(L"Test");
    UNICODE_STRING Name;

    if (!DefaultHeapInitializeAllocator(&Allocator)) {
        ExitCode = 1;
        goto Error;
    }

    CHECKED_MSG(
        CreateAndInitializeTracerConfigAndRtl(
            &Allocator,
            (PUNICODE_STRING)&TracerRegistryPath,
            &TracerConfig,
            &Rtl
        ),
        "CreateAndInitializeTracerConfigAndRtl()"
    );

    Success = Rtl->CreateSingleRandomObjectName(Rtl,
                                                &Allocator,
                                                &Allocator,
                                                &Prefix,
                                                &Name);

    if (!Success) {
        __debugbreak();
    }


#endif


#if 0
    UNICODE_STRING Path = RTL_CONSTANT_STRING(L"\\\\?\\S:\\trace\\2017-04-29-194742.551\\TraceStore_ImageFile.dat");

    ExitCode = FindStreams(&Path);

#endif

#if 0
    RTL Rtl;
    ULONG SizeOfRtl = sizeof(Rtl);

    if (!InitializeRtl(&Rtl, &SizeOfRtl)) {
        ExitCode = 1;
        goto Error;
    }

    if (Rtl.TsxAvailable) {
        NOTHING;
    }

#endif

    //SleepEx(INFINITE, TRUE);

    goto Error;

Error:

    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
