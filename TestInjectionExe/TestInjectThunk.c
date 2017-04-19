/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TestInjectionExe.c

Abstract:

    This module provides routines for testing the main public interfaces of the
    Rtl component's injection functionality.

--*/

#include "stdafx.h"

typedef struct _TEST_DATA {
    ULONGLONG   Long64;
    CHAR        Buffer[24];
} TEST_DATA;

#pragma data_seg(".testshared")

LONG Test2 = 0;
TEST_DATA TestData = { 0, };

PUNICODE_STRING ThunkExePath = NULL;

PUNICODE_STRING InjectionThunkActiveDllPath = NULL;
PUNICODE_STRING InjectionThunkDebugDllPath = NULL;
PUNICODE_STRING InjectionThunkReleaseDllPath = NULL;

PUNICODE_STRING TestInjectionActiveExePath = NULL;
PUNICODE_STRING TestInjectionDebugExePath = NULL;
PUNICODE_STRING TestInjectionReleaseExePath = NULL;

HMODULE InjectionThunkModule = NULL;
PRTL_INJECTION_REMOTE_THREAD_ENTRY InjectionEntry = NULL;

CHAR TestInjectionFunctionName[64] = { 0 };

PVOID DataPage1;
PVOID CodePage1;

#pragma data_seg()
#pragma comment(linker, "/section:.testshared,rws")


typedef STARTUPINFOW *PSTARTUPINFOW;

UNICODE_STRING ThunkExeFilename =
    RTL_CONSTANT_STRING(L"thunk.exe");

UNICODE_STRING InjectionThunkFilename =
    RTL_CONSTANT_STRING(L"InjectionThunk.dll");

UNICODE_STRING TestInjectionFilename =
    RTL_CONSTANT_STRING(L"TestInjection.exe");


BOOL
CreateThunkExe(
    PSTARTUPINFOW StartupInfo,
    PPROCESS_INFORMATION ProcessInfo
    )
{
    BOOL Success;
    DWORD LastError;

    ZeroStructPointer(StartupInfo);
    ZeroStructPointer(ProcessInfo);

    StartupInfo->cb = sizeof(*StartupInfo);

    Success = CreateProcessW(ThunkExePath->Buffer,
                             NULL,
                             NULL,
                             NULL,
                             FALSE,
                             0,
                             NULL,
                             NULL,
                             StartupInfo,
                             ProcessInfo);

    if (!Success) {
        LastError = GetLastError();
    }

    return Success;
}


typedef struct _INJECTION_TEST {
    ULONGLONG Count;
    UNICODE_STRING Path;
    WCHAR PathBuffer[_MAX_PATH];
    STRING Function;
    CHAR FunctionBuffer[64];
    LOAD_LIBRARY_W LoadLibraryW;
    LOAD_LIBRARY_W GetProcAddress;
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
    SHORT ThunkOffset,
    USHORT BytesRemaining,
    ULONG_PTR RemoteDataBufferAddress
    )
{
    USHORT Bytes;
    USHORT TotalBytes;
    PBYTE Base;
    PBYTE Dest;
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
    Test->Count++;
    Test->OutputDebugStringW(Test->Path.Buffer);
    return 0;
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
    PWSTR Path;
    LONG ThreadExitCode;
    ULONG EntryCount;
    ULONG WaitResult;
    ULONG CreationFlags;
    ULONG RemoteThreadId;
    USHORT ModulePathBufferOffset;
    USHORT FunctionNameBufferOffset;
    HANDLE RemoteThreadHandle;
    RTL_INJECTION_CONTEXT Context;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK DestInjectionThunk;
    PVOID DestBaseCodeAddress;
    PVOID DestDataBufferAddress;
    STRING FunctionName = RTL_CONSTANT_STRING("TestInjectThunkThreadEntry");
    STRING TestName = RTL_CONSTANT_STRING("TestName");
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    LPTHREAD_START_ROUTINE StartRoutine;
    INJECTION_TEST Test;
    INJECTION_THUNK_FLAGS Flags;

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
    Test.Path.Buffer = &Test.PathBuffer;
    __stosw(&Test.PathBuffer, L'!', Test.Path.Length >> 1);

    CopyMemory((PBYTE)&Test.FunctionBuffer,
               FunctionName.Buffer,
               FunctionName.Length);

    InitializeStringFromString(&Test.FunctionName, &FunctionName);

    Flags.AsULong = 0;

    Success = Rtl->InjectThunk(Rtl,
                               Allocator
                               Flags,
                               ProcessInfo.hProcess,
                               TestInjectionActiveExePath,
                               FunctionName,
                               (PBYTE)Test,
                               sizeof(Test),
                               AdjustDataBufferPointers,
                               &RemoteThreadHandle,
                               &RemoteThreadId,
                               &RemoteBaseCodeAddress,
                               &RemoteDataBufferAddress);

    if (!Success) {
        return FALSE;
    }

    WaitResult = WaitForSingleObject(RemoteThreadHandle, INFINITE);
    if (WaitResult != WAIT_OBJECT_0) {
        __debugbreak();
        return FALSE;
    }

    if (Test2 != 1) {
        __debugbreak();
        return FALSE;
    }

    Success = GetExitCodeThread(RemoteThreadHandle, &ThreadExitCode);
    if (ThreadExitCode != 1) {
        __debugbreak();
        return FALSE;
   }

    return TRUE;
}

/*
BOOL
TestInjection4(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    BOOL Success;
    PWSTR Path;
    LONG Result;
    ULONG EntryCount;
    USHORT SizeOfDataBufferInBytes;
    HANDLE ProcessHandle;
    RTL_INJECTION_CONTEXT Context;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK DestInjectionThunk;
    PVOID DestBaseCodeAddress;
    PVOID DestDataBufferAddress;
    STRING FunctionName = RTL_CONSTANT_STRING("TestInjection2ThreadEntry");

    Path = InjectionThunkActiveDllPath->Buffer;
    InjectionThunkModule = Rtl->LoadLibraryW(Path);
    if (!InjectionThunkModule) {
        return FALSE;
    }

    InjectionThunk = (PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK)(
        GetProcAddress(
            InjectionThunkModule,
            "InjectionThunk"
        )
    );

    if (!InjectionThunk) {
        return FALSE;
    }

    Thunk = &Context.Thunk;

    Thunk->EntryCount = EntryCount;
    Thunk->BaseAddress = DestBaseCodeAddress;
    Thunk->FunctionTable = DestRuntimeFunction;
    Thunk->RtlAddFunctionTable = Rtl->RtlAddFunctionTable;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;

    InitializeStringFromString(Thunk->FunctionName, &FunctionName);
    InitializeUnicodeStringFromUnicodeString(&Thunk->ModulePath,
                                             TestInjectionActiveExePath);

    InitializeRtlInjectionFunctions(Rtl, &Context.Functions);


    ProcessHandle = GetCurrentProcess();

    Success = CopyFunction(Rtl,
                           Allocator,
                           ProcessHandle,
                           InjectionThunk,
                           NULL,
                           sizeof(Context),
                           (PBYTE)Thunk,
                           AdjustThunkPointers,
                           &DestBaseCodeAddress,
                           &DestDataBufferAddress,
                           (PVOID *)&DestInjectionThunk,
                           &DestRuntimeFunction,
                           NULL,
                           &EntryCount);

    if (!Success) {
        return FALSE;
    }

    ZeroStruct(Context);

    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);
    Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

    if (Test2 != 2) {
        __debugbreak();
    }

    return TRUE;
}


BOOL
TestInjection5(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    BOOL Success;
    PWSTR Path;
    LONG Result;
    ULONG EntryCount;
    USHORT ModulePathOffset;
    USHORT ModuleHandleOffset;
    USHORT FunctionNameOffset;
    HANDLE ProcessHandle;
    //RTL_INJECTION_CONTEXT Context;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRUNTIME_FUNCTION DestHandlerRuntimeFunction;
    RTL_INJECTION_THUNK_CONTEXT LocalThunk;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK DestInjectionThunk;
    PVOID DestBaseAddress;
    CHAR FunctionName[] = "TestInjection2ThreadEntry";

    ZeroStruct(TestInjectionFunctionName);

    __movsb((PBYTE)TestInjectionFunctionName,
            (PBYTE)FunctionName,
            sizeof(FunctionName));

    Path = InjectionThunkActiveDllPath->Buffer;
    InjectionThunkModule = Rtl->LoadLibraryW(Path);
    if (!InjectionThunkModule) {
        return FALSE;
    }

    InjectionThunk = (PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK)(
        GetProcAddress(
            InjectionThunkModule,
            "InjectionThunk4"
        )
    );

    if (!InjectionThunk) {
        return FALSE;
    }

    goto InitThunk;

    ProcessHandle = GetCurrentProcess();

    Success = CopyFunction(Rtl,
                           Allocator,
                           ProcessHandle,
                           InjectionThunk,
                           NULL,
                           &DestBaseAddress,
                           (PVOID *)&DestInjectionThunk,
                           &DestRuntimeFunction,
                           &DestHandlerRuntimeFunction,
                           &EntryCount);

    if (!Success) {
        return FALSE;
    }

InitThunk:

    //ZeroStruct(Context);
    ZeroStruct(LocalThunk);

    //Thunk = &Context.Thunk;
    Thunk = &LocalThunk;

    Thunk->Flags.AddFunctionTable = FALSE;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;
    Thunk->ModulePath = TestInjectionActiveExePath->Buffer;
    Thunk->FunctionName = TestInjectionFunctionName;

    //InitializeRtlInjectionFunctions(Rtl, &Context.Functions);

    ModulePathOffset = FIELD_OFFSET(RTL_INJECTION_THUNK_CONTEXT, ModulePath);
    ModuleHandleOffset = FIELD_OFFSET(RTL_INJECTION_THUNK_CONTEXT, ModuleHandle);
    FunctionNameOffset = FIELD_OFFSET(RTL_INJECTION_THUNK_CONTEXT, FunctionName);

    //Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);
    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

    return TRUE;
}

BOOL
TestInjection6(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    BOOL Success;
    PWSTR Path;
    LONG Result;
    ULONG EntryCount;
    HANDLE ProcessHandle;
    RTL_INJECTION_CONTEXT Context;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRUNTIME_FUNCTION DestHandlerRuntimeFunction;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK DestInjectionThunk;
    PVOID DestBaseAddress;
    CHAR FunctionName[] = "TestInjection2ThreadEntry";

    ZeroStruct(TestInjectionFunctionName);

    __movsb((PBYTE)TestInjectionFunctionName,
            (PBYTE)FunctionName,
            sizeof(FunctionName));

    Path = InjectionThunkActiveDllPath->Buffer;
    InjectionThunkModule = Rtl->LoadLibraryW(Path);
    if (!InjectionThunkModule) {
        return FALSE;
    }

    InjectionThunk = (PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK)(
        GetProcAddress(
            InjectionThunkModule,
            "InjectionThunk4"
        )
    );

    if (!InjectionThunk) {
        return FALSE;
    }

    ProcessHandle = GetCurrentProcess();

    Success = CopyFunction(Rtl,
                           Allocator,
                           ProcessHandle,
                           InjectionThunk,
                           NULL,
                           &DestBaseAddress,
                           (PVOID *)&DestInjectionThunk,
                           &DestRuntimeFunction,
                           &DestHandlerRuntimeFunction,
                           &EntryCount);

    if (!Success) {
        return FALSE;
    }

    ZeroStruct(Context);

    Thunk = &Context.Thunk;

    Thunk->Flags.AddFunctionTable = FALSE;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;
    Thunk->ModulePath = TestInjectionActiveExePath->Buffer;
    Thunk->FunctionName = TestInjectionFunctionName;

    InitializeRtlInjectionFunctions(Rtl, &Context.Functions);

    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);
    Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

    return TRUE;
}
*/


#define TEST(n) TestInjection##n##(Rtl, Allocator, TracerConfig, Session)

_Use_decl_annotations_
LONG
TestInjection(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    TRACER_BINARY_TYPE_INDEX ReleaseBinaries = TracerReleaseBinaries;
    TRACER_BINARY_TYPE_INDEX DebugBinaries = TracerDebugBinaries;

    if (!MakeTracerPath(TracerConfig, &ThunkExeFilename, NULL, &ThunkExePath)) {
        return -1;
    }


    //
    // Active binaries.
    //

    if (!MakeTracerPath(TracerConfig,
                        &InjectionThunkFilename,
                        NULL,
                        &InjectionThunkActiveDllPath)) {
        return -2;
    }

    if (!MakeTracerPath(TracerConfig,
                        &TestInjectionFilename,
                        NULL,
                        &TestInjectionActiveExePath)) {
        return -3;
    }

    //
    // Release binaries.
    //

    if (!MakeTracerPath(TracerConfig,
                        &InjectionThunkFilename,
                        &ReleaseBinaries,
                        &InjectionThunkReleaseDllPath)) {
        return -2;
    }

    if (!MakeTracerPath(TracerConfig,
                        &TestInjectionFilename,
                        &ReleaseBinaries,
                        &TestInjectionReleaseExePath)) {
        return -3;
    }

    //
    // Debug binaries.
    //

    if (!MakeTracerPath(TracerConfig,
                        &InjectionThunkFilename,
                        &DebugBinaries,
                        &InjectionThunkDebugDllPath)) {
        return -4;
    }

    if (!MakeTracerPath(TracerConfig,
                        &TestInjectionFilename,
                        &DebugBinaries,
                        &TestInjectionDebugExePath)) {
        return -5;
    }

    TEST(4);
    //TEST(5);
    //TEST(1);

    return ERROR_SUCCESS;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
