/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TestInjectThunk.c

Abstract:

    This module provides routines for testing the main public interfaces of the
    Rtl component's injection functionality.

--*/

#include "stdafx.h"


typedef struct _INJECTION_TEST {
    ULONGLONG Count;
    UNICODE_STRING Path;
    WCHAR PathBuffer[_MAX_PATH];
    STRING Function;
    CHAR FunctionBuffer[64];
    PLOAD_LIBRARY_W LoadLibraryW;
    PGET_PROC_ADDRESS GetProcAddress;
    POUTPUT_DEBUG_STRING_A OutputDebugStringA;
    POUTPUT_DEBUG_STRING_W OutputDebugStringW;
} INJECTION_TEST;
typedef INJECTION_TEST *PINJECTION_TEST;

BOOL
AdjustDataBufferPointers(
    PRTL Rtl,
    HANDLE TargetProcessHandle,
    PBYTE OriginalDataBuffer,
    USHORT SizeOfDataBufferInBytes,
    PBYTE TemporaryLocalDataBuffer,
    USHORT BytesRemaining,
    PBYTE RemoteDataBufferAddress
    )
{
    PINJECTION_TEST OriginalTest;
    PINJECTION_TEST Test;

    OriginalTest = (PINJECTION_TEST)OriginalDataBuffer;
    Test = (PINJECTION_TEST)TemporaryLocalDataBuffer;

    Test->Path.Buffer = (PWSTR)(
        RtlOffsetToPointer(
            RemoteDataBufferAddress,
            FIELD_OFFSET(INJECTION_TEST, PathBuffer)
        )
    );

    Test->Function.Buffer = (PCHAR)(
        RtlOffsetToPointer(
            RemoteDataBufferAddress,
            FIELD_OFFSET(INJECTION_TEST, FunctionBuffer)
        )
    );


    return TRUE;
}

DECLSPEC_DLLEXPORT
LONG
TestInjectThunkThreadEntry(
    PINJECTION_TEST Test
    )
{
    Test->OutputDebugStringW(Test->Path.Buffer);
    Test->OutputDebugStringA(Test->Function.Buffer);
    return (LONG)(Test->Count + 1);
}

BOOL
TestInjectThunk(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    BOOL Success;
    LONG ThreadExitCode;
    ULONG WaitResult;
    ULONG RemoteThreadId;
    USHORT Count;
    HANDLE RemoteThreadHandle;
    INJECTION_THUNK_FLAGS Flags;
    PVOID RemoteBaseCodeAddress;
    PVOID RemoteDataBufferAddress;
    STRING FunctionName = RTL_CONSTANT_STRING("TestInjectThunkThreadEntry");
    STRING TestName = RTL_CONSTANT_STRING("TestName");
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    INJECTION_TEST Test;

    if (!CreateThunkExe(&StartupInfo, &ProcessInfo)) {
        return GetLastError();
    }

    ZeroStruct(Test);

    Test.OutputDebugStringA = Rtl->OutputDebugStringA;
    Test.OutputDebugStringW = Rtl->OutputDebugStringW;
    Test.LoadLibraryW = Rtl->LoadLibraryW;
    Test.GetProcAddress = Rtl->GetProcAddress;

    Test.Path.Length = sizeof(Test.PathBuffer) - sizeof(WCHAR);
    Test.Path.MaximumLength = sizeof(Test.PathBuffer);
    Test.Path.Buffer = (PWSTR)&Test.PathBuffer;
    Count = Test.Path.Length >> 1;
    __stosw((PWCHAR)&Test.PathBuffer, L'!', Test.Path.Length >> 1);

    CopyMemory((PBYTE)&Test.FunctionBuffer,
               FunctionName.Buffer,
               FunctionName.Length);

    InitializeStringFromString(&Test.Function, &FunctionName);

    Flags.AsULong = 0;
    Test.Count = 1;

    Success = Rtl->InjectThunk(Rtl,
                               Allocator,
                               Flags,
                               ProcessInfo.hProcess,
                               TestInjectionActiveExePath,
                               &FunctionName,
                               (PBYTE)&Test,
                               sizeof(Test),
                               AdjustDataBufferPointers,
                               &RemoteThreadHandle,
                               &RemoteThreadId,
                               &RemoteBaseCodeAddress,
                               &RemoteDataBufferAddress);

    if (!Success) {
        goto End;
    }

    WaitResult = WaitForSingleObject(RemoteThreadHandle, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
        NOTHING;
    }

    Success = GetExitCodeThread(RemoteThreadHandle, &ThreadExitCode);
    if (ThreadExitCode != 2) {
        __debugbreak();
        Success = FALSE;
        goto End;
    }

    Success = TRUE;

End:

    TerminateProcess(ProcessInfo.hProcess, 0);

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
