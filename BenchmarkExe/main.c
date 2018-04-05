/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    Scratch/testing file.

--*/

#include "stdafx.h"

RTL GlobalRtl;
ALLOCATOR GlobalAllocator;

PRTL Rtl;
PALLOCATOR Allocator;

HMODULE GlobalModule = 0;

VOID
Scratch(
    PRTL Rtl,
    PALLOCATOR Allocator
    )
{

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

    Scratch(Rtl, Allocator);

Error:

    ExitProcess(ExitCode);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
