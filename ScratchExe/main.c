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

DECLSPEC_NORETURN
VOID
WINAPI
mainCRTStartup()
{
    LONG ExitCode = 0;

#if 1
    BOOL Success;
    ULONG SizeOfBuffer;
    PWSTR WideBuffer;
    PRTL Rtl;
    PTRACER_CONFIG TracerConfig;
    ALLOCATOR Allocator;
    //const UNICODE_STRING Prefix = RTL_CONSTANT_STRING(L"Test");
    UNICODE_STRING ObjectNames[3];
    UNICODE_STRING ObjectPrefixes[] = {
        RTL_CONSTANT_STRING(L"Test1_"),
        RTL_CONSTANT_STRING(L"Test3_")
    };
    PUNICODE_STRING Names[] = {
        &ObjectNames[0],
        &ObjectNames[1],
        &ObjectNames[2]
    };
    PUNICODE_STRING Prefixes[] ={
        &ObjectPrefixes[0],
        NULL,
        &ObjectPrefixes[1]
    };

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

    Success = Rtl->CreateRandomObjectNames(Rtl,
                                           &Allocator,
                                           &Allocator,
                                           ARRAYSIZE(ObjectNames),
                                           64,
                                           57,
                                           NULL,
                                           (PPUNICODE_STRING)&Names,
                                           NULL,
                                           &SizeOfBuffer,
                                           &WideBuffer);

    if (!Success) {
        __debugbreak();
    }

    Allocator.FreePointer(Allocator.Context, &WideBuffer);

    Success = Rtl->CreateRandomObjectNames(Rtl,
                                           &Allocator,
                                           &Allocator,
                                           ARRAYSIZE(ObjectNames),
                                           64,
                                           32,
                                           NULL,
                                           (PPUNICODE_STRING)&Names,
                                           (PPUNICODE_STRING)&Prefixes,
                                           &SizeOfBuffer,
                                           &WideBuffer);

    if (!Success) {
        __debugbreak();
    }

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
