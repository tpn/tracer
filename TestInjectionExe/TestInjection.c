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

#define ZeroStruct(Name)        __stosq((PDWORD64)&Name, 0, sizeof(Name)  >> 3)
#define ZeroStructPointer(Name) __stosq((PDWORD64)Name,  0, sizeof(*Name) >> 3)

_Use_decl_annotations_
VOID
InitializeRtlInjectionFunctions(
    PRTL Rtl,
    PRTL_INJECTION_FUNCTIONS Functions
    )
{
    PBYTE Code;
    PBYTE NewCode;
    PBYTE *Function;
    PBYTE *Pointers;
    USHORT Index;
    USHORT NumberOfFunctions;

    Functions->LoadLibraryExW = Rtl->LoadLibraryExW;
    Functions->GetProcAddress = Rtl->GetProcAddress;
    Functions->SetEvent = Rtl->SetEvent;
    Functions->OpenEventW = Rtl->OpenEventW;
    Functions->CloseHandle = Rtl->CloseHandle;
    Functions->SignalObjectAndWait = Rtl->SignalObjectAndWait;
    Functions->WaitForSingleObjectEx = Rtl->WaitForSingleObjectEx;
    Functions->OutputDebugStringA = Rtl->OutputDebugStringA;
    Functions->OutputDebugStringW = Rtl->OutputDebugStringW;
    Functions->NtQueueUserApcThread = Rtl->NtQueueApcThread;
    Functions->NtTestAlert = Rtl->NtTestAlert;
    Functions->QueueUserAPC = Rtl->QueueUserAPC;
    Functions->SleepEx = Rtl->SleepEx;
    Functions->ExitThread = Rtl->ExitThread;
    Functions->GetExitCodeThread = Rtl->GetExitCodeThread;
    Functions->CreateFileMappingW = Rtl->CreateFileMappingW;
    Functions->OpenFileMappingW = Rtl->OpenFileMappingW;
    Functions->MapViewOfFileEx = Rtl->MapViewOfFileEx;
    Functions->FlushViewOfFile = Rtl->FlushViewOfFile;
    Functions->UnmapViewOfFileEx = Rtl->UnmapViewOfFileEx;

    Pointers = (PBYTE *)Functions;
    NumberOfFunctions = sizeof(*Functions) / sizeof(ULONG_PTR);

    for (Index = 0; Index < NumberOfFunctions; Index++) {
        Function = Pointers + Index;
        Code = *Function;
        NewCode = SkipJumpsInline(Code);
        *Function = NewCode;
    }
}


ULONG
CALLBACK
InjectionCallback1(
    PRTL_INJECTION_PACKET Packet
    )
{
    ULONGLONG Token;

    if (Packet->IsInjectionProtocolCallback(Packet, &Token)) {
        return (ULONG)Token;
    }

    Packet->Functions->OutputDebugStringA("Entered Injection1 callback.\n");

    return 1;
}

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

BOOL
InjectRemoteThread(
    PPROCESS_INFORMATION ProcessInfo,
    LPTHREAD_START_ROUTINE StartRoutine,
    PVOID Parameter,
    ULONG CreationFlags,
    PHANDLE RemoteThreadHandlePointer,
    PULONG RemoteThreadIdPointer
    )
{
    ULONG RemoteThreadId;
    HANDLE RemoteThreadHandle;

    *RemoteThreadHandlePointer = NULL;
    *RemoteThreadIdPointer = 0;

    RemoteThreadHandle = (
        CreateRemoteThread(
            ProcessInfo->hProcess,
            NULL,
            0,
            StartRoutine,
            Parameter,
            CreationFlags,
            &RemoteThreadId
        )
    );

    if (!RemoteThreadHandle || RemoteThreadHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    *RemoteThreadHandlePointer = RemoteThreadHandle;
    *RemoteThreadIdPointer = RemoteThreadId;

    return TRUE;
}

ULONG
TestInjection1Old(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    BOOL Success;

    RTL_INJECTION_FLAGS Flags;
    PRTL_INJECTION_PACKET Packet;
    PRTL_INJECTION_COMPLETE_CALLBACK Callback;
    RTL_INJECTION_ERROR Error;
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;

    if (!CreateThunkExe(&StartupInfo, &ProcessInfo)) {
        return GetLastError();
    }

    if (!Rtl->LoadDbgHelp(Rtl)) {
        return (ULONG)-1;
    }

    Flags.AsULong = 0;
    Flags.InjectCode = TRUE;

    Callback = InjectionCallback1;

    Success = Rtl->Inject(Rtl,
                          Allocator,
                          &Flags,
                          NULL,
                          NULL,
                          Callback,
                          NULL,
                          ProcessInfo.dwProcessId,
                          &Packet,
                          &Error);


    return ERROR_SUCCESS;
}


DECLSPEC_DLLEXPORT
LONG
CALLBACK
TestInjection2ThreadEntry(
    PRTL_INJECTION_CONTEXT Context
    )
{
    //Context->Functions.OutputDebugStringA("Test 1 2 3.\n");
    return ++Test2;
}

LONGLONG
CALLBACK
TestJump(
    ULONGLONG Rcx
    )
{
    PVOID ReturnAddress = _ReturnAddress();
    PVOID AddressOfReturnAddress = _AddressOfReturnAddress();
    if (ReturnAddress) {
        OutputDebugStringA("X");
    }
    if (AddressOfReturnAddress) {
        OutputDebugStringA("B");
    }
    return 2;
}

LONGLONG
CALLBACK
TestJump2(
    ULONGLONG Rcx
    )
{
    return (LONGLONG)_ReturnAddress();
}

ADJUST_POINTERS AdjustThunkPointers;

_Use_decl_annotations_
BOOL
AdjustThunkPointers(
    PRTL Rtl,
    PBYTE OriginalDataBuffer,
    USHORT SizeOfDataBufferInBytes,
    PBYTE TemporaryLocalDataBuffer,
    USHORT BytesRemaining,
    ULONG_PTR RemoteDataBufferAddress,
    PRUNTIME_FUNCTION RemoteRuntimeFunction,
    PVOID RemoteCodeBaseAddress,
    ULONG EntryCount
    )
{
    USHORT Bytes;
    USHORT TotalBytes;
    PBYTE Base;
    PBYTE Dest;
    PRTL_INJECTION_THUNK_CONTEXT OriginalThunk;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;

    OriginalThunk = (PRTL_INJECTION_THUNK_CONTEXT)OriginalDataBuffer;
    Thunk = (PRTL_INJECTION_THUNK_CONTEXT)TemporaryLocalDataBuffer;

    Base = Dest = (PBYTE)(
        RtlOffsetToPointer(
            TemporaryLocalDataBuffer,
            SizeOfDataBufferInBytes
        )
    );

    //
    // Adjust the module path's buffer pointer such that it's valid in the
    // remote process's address space.
    //

    Thunk->ModulePath.Buffer = (PWSTR)(
        RtlOffsetToPointer(
            RemoteDataBufferAddress,
            SizeOfDataBufferInBytes
        )
    );

    //
    // Copy the InjectionThunk.dll path.
    //

    Bytes = OriginalThunk->ModulePath.Length + sizeof(WCHAR);
    CopyMemory(Dest, (PBYTE)OriginalThunk->ModulePath.Buffer, Bytes);

    Bytes = ALIGN_UP_POINTER(Bytes + 8);
    TotalBytes = Bytes;
    Dest += TotalBytes;

    //
    // Copy the function name.
    //

    Bytes = OriginalThunk->FunctionName.Length + sizeof(CHAR);
    CopyMemory(Dest, (PBYTE)OriginalThunk->FunctionName.Buffer, Bytes);

    Bytes = ALIGN_UP_POINTER(Bytes + 8);
    TotalBytes += Bytes;

    //
    // Adjust the function name's buffer.
    //

    Thunk->FunctionName.Buffer = (PSTR)(
        RtlOffsetToPointer(
            RemoteDataBufferAddress,
            SizeOfDataBufferInBytes + (Dest - Base)
        )
    );

    //
    // Adjust the runtime function entry details for RtlAddFunctionTable().
    //

    Thunk->EntryCount = EntryCount;
    Thunk->FunctionTable = RemoteRuntimeFunction;
    Thunk->BaseAddress = RemoteCodeBaseAddress;

    return TRUE;
}


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
    //LONG Result;
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
    STRING FunctionName = RTL_CONSTANT_STRING("TestInjection2ThreadEntry");
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    LPTHREAD_START_ROUTINE StartRoutine;
    PCOPY_FUNCTION CopyFunction;

    if (!CreateThunkExe(&StartupInfo, &ProcessInfo)) {
        return GetLastError();
    }

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

    ZeroStruct(Context);

    Thunk = &Context.Thunk;

    Thunk->Flags.DebugBreakOnEntry = TracerConfig->Flags.DebugBreakOnEntry;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;
    Thunk->RtlAddFunctionTable = Rtl->RtlAddFunctionTable;

    InitializeStringFromString(&Thunk->FunctionName, &FunctionName);
    InitializeUnicodeStringFromUnicodeString(&Thunk->ModulePath,
                                             TestInjectionActiveExePath);

    InitializeRtlInjectionFunctions(Rtl, &Context.Functions);

    CopyFunction = Rtl->CopyFunction;

    Success = CopyFunction(Rtl,
                           Allocator,
                           ProcessInfo.hProcess,
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

    Test2 = 0;

    StartRoutine = (LPTHREAD_START_ROUTINE)DestInjectionThunk;
    CreationFlags = 0;

    ModulePathBufferOffset = (USHORT)(
        (PBYTE)&Thunk->ModulePath.Buffer -
        (PBYTE)Thunk
    );

    FunctionNameBufferOffset = (USHORT)(
        (PBYTE)&Thunk->FunctionName.Buffer -
        (PBYTE)Thunk
    );

    RemoteThreadHandle = (
        CreateRemoteThread(
            ProcessInfo.hProcess,
            NULL,
            0,
            StartRoutine,
            DestDataBufferAddress,
            CreationFlags,
            &RemoteThreadId
        )
    );

    if (!RemoteThreadHandle || RemoteThreadHandle == INVALID_HANDLE_VALUE) {
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
