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

ULONG
TestInjection1(
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


BOOL
TestInjection3(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    PWSTR Path;
    LONG Result;
    RTL_INJECTION_CONTEXT Context;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
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
            "InjectionThunk"
        )
    );

    if (!InjectionThunk) {
        return FALSE;
    }

    ZeroStruct(Context);

    Thunk = &Context.Thunk;

    Thunk->ExitThread = Rtl->ExitThread;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;
    Thunk->ModulePath = TestInjectionActiveExePath->Buffer;
    Thunk->FunctionName = TestInjectionFunctionName;

    InitializeRtlInjectionFunctions(Rtl, &Context.Functions);

    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

    return TRUE;
}

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK COPY_FUNCTION)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ HANDLE TargetProcessHandle,
    _In_ PVOID SourceFunction,
    _In_ PPVOID DestBaseAddressPointer,
    _In_ PPVOID DestFunctionPointer
    );
typedef COPY_FUNCTION *PCOPY_FUNCTION;

COPY_FUNCTION CopyFunction;

_Use_decl_annotations_
BOOL
CopyFunction(
    PRTL Rtl,
    PALLOCATOR Allocator,
    HANDLE TargetProcessHandle,
    PVOID SourceFunction,
    PVOID *DestBaseAddressPointer,
    PVOID *DestFunctionPointer
    )
{
    BOOL Success = FALSE;
    USHORT CodeSize;
    SHORT UnwindCodeSize;
    SHORT UnwindInfoSize;
    SHORT TotalUnwindSize;
    ULONG OldProtection;
    SIZE_T NumberOfBytesWritten;
    HANDLE ProcessHandle = GetCurrentProcess();
    ULONG OnePage = 1 << PAGE_SHIFT;
    ULONG TwoPages = 2 << PAGE_SHIFT;
    PVOID SourceFunc;
    PVOID LocalBaseCodeAddress = NULL;
    PVOID LocalBaseDataAddress = NULL;
    PVOID RemoteBaseCodeAddress = NULL;
    PVOID RemoteBaseDataAddress = NULL;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRUNTIME_FUNCTION SourceRuntimeFunction;
    RUNTIME_FUNCTION_EX SourceRuntimeFunctionEx;
    PUNWIND_INFO UnwindInfo;
    PUNWIND_INFO DestUnwindInfo;
    ULONGLONG SourceBaseAddress;
    PBYTE DestCode;
    PBYTE EndCode;
    PBYTE DestData;
    PBYTE SourceCode;

    struct {
        LARGE_INTEGER Frequency;
        struct {
            LARGE_INTEGER Enter;
            LARGE_INTEGER Exit;
            LARGE_INTEGER Elapsed;
        } FillCodePages;
        struct {
            LARGE_INTEGER Enter;
            LARGE_INTEGER Exit;
            LARGE_INTEGER Elapsed;
        } FillDataPages;
    } Timestamps;

    *DestBaseAddressPointer = NULL;
    *DestFunctionPointer = NULL;

    //
    // Allocate local pages for code and data.
    //

    LocalBaseCodeAddress = (
        VirtualAllocEx(
            ProcessHandle,
            NULL,
            TwoPages,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!LocalBaseCodeAddress) {
        return FALSE;
    }

    LocalBaseDataAddress = (PVOID)((PCHAR)LocalBaseCodeAddress + OnePage);

    //
    // Allocate remote code and data.
    //

    RemoteBaseCodeAddress = (
        VirtualAllocEx(
            TargetProcessHandle,
            NULL,
            TwoPages,
            MEM_COMMIT,
            PAGE_READWRITE
        )
    );

    if (!RemoteBaseCodeAddress) {
        goto Cleanup;
    }

    RemoteBaseDataAddress = (PVOID)((PCHAR)RemoteBaseCodeAddress + OnePage);

    //
    // Fill local code page with 0xCC (int 3), and local data page with zeros.
    //

    QueryPerformanceCounter(&Timestamps.FillCodePages.Enter);
    Rtl->FillPages((PCHAR)LocalBaseCodeAddress, 0xCC, 1);
    QueryPerformanceCounter(&Timestamps.FillCodePages.Exit);

    QueryPerformanceCounter(&Timestamps.FillDataPages.Enter);
    Rtl->FillPages((PCHAR)LocalBaseDataAddress, 0x00, 1);
    QueryPerformanceCounter(&Timestamps.FillDataPages.Exit);

    Timestamps.FillCodePages.Elapsed.QuadPart = (
        Timestamps.FillCodePages.Exit.QuadPart -
        Timestamps.FillCodePages.Enter.QuadPart
    );

    Timestamps.FillDataPages.Elapsed.QuadPart = (
        Timestamps.FillDataPages.Exit.QuadPart -
        Timestamps.FillDataPages.Enter.QuadPart
    );

    QueryPerformanceFrequency(&Timestamps.Frequency);

    SourceFunc = SkipJumpsInline(SourceFunction);

    SourceRuntimeFunction = (
        Rtl->RtlLookupFunctionEntry(
            (ULONGLONG)SourceFunc,
            &SourceBaseAddress,
            NULL
        )
    );

    if (!SourceRuntimeFunction) {
        goto Cleanup;
    }

    SourceRuntimeFunctionEx.BeginAddress = (PVOID)(
        SourceBaseAddress + SourceRuntimeFunction->BeginAddress
    );

    SourceRuntimeFunctionEx.EndAddress = (PVOID)(
        SourceBaseAddress + SourceRuntimeFunction->EndAddress
    );

    SourceRuntimeFunctionEx.UnwindInfo = (PUNWIND_INFO)(
        SourceBaseAddress + SourceRuntimeFunction->UnwindInfoAddress
    );

    UnwindInfo = SourceRuntimeFunctionEx.UnwindInfo;

    CodeSize = (USHORT)(
        (ULONGLONG)SourceRuntimeFunction->EndAddress -
        (ULONGLONG)SourceRuntimeFunction->BeginAddress
    );

    SourceCode = (PBYTE)SourceRuntimeFunctionEx.BeginAddress;

    UnwindInfoSize = sizeof(UNWIND_INFO);
    UnwindCodeSize = (UnwindInfo->CountOfCodes - 1) * sizeof(UNWIND_CODE);
    TotalUnwindSize = UnwindInfoSize + UnwindCodeSize;

    DestCode = (PBYTE)LocalBaseCodeAddress;

    DestCode += 16;
    CopyMemory(DestCode, SourceCode, CodeSize);
    EndCode = DestCode + CodeSize;

    DestData = (PBYTE)LocalBaseDataAddress;
    DestRuntimeFunction = (PRUNTIME_FUNCTION)DestData;
    DestData += ALIGN_UP(sizeof(RUNTIME_FUNCTION), 16);

    DestUnwindInfo = (PUNWIND_INFO)DestData;
    CopyMemory((PBYTE)DestUnwindInfo, (PBYTE)UnwindInfo, TotalUnwindSize);

    DestRuntimeFunction->BeginAddress = (ULONG)(
        (ULONG_PTR)DestCode - (ULONG_PTR)LocalBaseCodeAddress
    );

    DestRuntimeFunction->EndAddress = (ULONG)(
        (ULONG_PTR)EndCode - (ULONG_PTR)LocalBaseCodeAddress
    );

    if (DestRuntimeFunction->BeginAddress != 16) {
        __debugbreak();
    }

    if (DestRuntimeFunction->EndAddress != (16 + CodeSize)) {
        __debugbreak();
    }

    DestRuntimeFunction->UnwindInfoAddress = (ULONG)(
        (ULONG_PTR)DestUnwindInfo - (ULONG_PTR)LocalBaseCodeAddress
    );

    Success = (
        WriteProcessMemory(
            TargetProcessHandle,
            RemoteBaseCodeAddress,
            LocalBaseCodeAddress,
            TwoPages,
            &NumberOfBytesWritten
        )
    );

    if (!Success) {
        goto Cleanup;
    }

    Success = (
        VirtualProtectEx(
            TargetProcessHandle,
            RemoteBaseCodeAddress,
            OnePage,
            PAGE_EXECUTE_READ,
            &OldProtection
        )
    );

    if (!Success) {
        goto Cleanup;
    }

    Success = (
        FlushInstructionCache(
            TargetProcessHandle,
            RemoteBaseCodeAddress,
            OnePage
        )
    );

    if (!Success) {
        __debugbreak();
    }

    Success = (
        VirtualProtectEx(
            TargetProcessHandle,
            RemoteBaseDataAddress,
            OnePage,
            PAGE_READONLY,
            &OldProtection
        )
    );

    if (ProcessHandle == TargetProcessHandle) {
        Success = (
            Rtl->RtlAddFunctionTable(
                DestRuntimeFunction,
                1,
                (ULONGLONG)RemoteBaseCodeAddress
            )
        );

        if (!Success) {
            goto Cleanup;
        }

    }

    if (Success) {
        *DestBaseAddressPointer = RemoteBaseCodeAddress;
        *DestFunctionPointer = (
            (PBYTE)RemoteBaseCodeAddress + 16
        );
        goto End;
    }

    //
    // Intentional follow-on to Cleanup.
    //

Cleanup:

    if (LocalBaseCodeAddress) {

        VirtualFreeEx(
            ProcessHandle,
            LocalBaseCodeAddress,
            0,
            MEM_RELEASE
        );

        LocalBaseCodeAddress = NULL;
    }

    /*
    if (LocalBaseDataAddress) {

        VirtualFreeEx(
            ProcessHandle,
            LocalBaseDataAddress,
            0,
            MEM_RELEASE
        );

        LocalBaseDataAddress = NULL;
    }
    */

    if (Success) {
        goto End;
    }

    //
    // Deallocate remote memory as well.
    //

    if (RemoteBaseCodeAddress) {

        VirtualFreeEx(
            ProcessHandle,
            RemoteBaseCodeAddress,
            0,
            MEM_RELEASE
        );

        RemoteBaseCodeAddress = NULL;
    }

    /*
    if (RemoteBaseDataAddress) {

        VirtualFreeEx(
            ProcessHandle,
            RemoteBaseDataAddress,
            0,
            MEM_RELEASE
        );

        RemoteBaseDataAddress = NULL;
    }
    */

End:

    return Success;
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
    LONG Result;
    HANDLE ProcessHandle;
    RTL_INJECTION_CONTEXT Context;
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

    ProcessHandle = GetCurrentProcess();

    Success = CopyFunction(Rtl,
                           Allocator,
                           ProcessHandle,
                           InjectionThunk,
                           &DestBaseAddress,
                           (PVOID *)&DestInjectionThunk);

    if (!Success) {
        return FALSE;
    }

    ZeroStruct(Context);

    Thunk = &Context.Thunk;

    Thunk->Flags.AddFunctionTable = FALSE;
    Thunk->ExitThread = Rtl->ExitThread;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;
    Thunk->ModulePath = TestInjectionActiveExePath->Buffer;
    Thunk->FunctionName = TestInjectionFunctionName;

    InitializeRtlInjectionFunctions(Rtl, &Context.Functions);

    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);
    Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

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
    USHORT ModulePathOffset;
    USHORT ModuleHandleOffset;
    USHORT FunctionNameOffset;
    HANDLE ProcessHandle;
    //RTL_INJECTION_CONTEXT Context;
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
                           &DestBaseAddress,
                           (PVOID *)&DestInjectionThunk);

    if (!Success) {
        return FALSE;
    }

InitThunk:

    //ZeroStruct(Context);
    ZeroStruct(LocalThunk);

    //Thunk = &Context.Thunk;
    Thunk = &LocalThunk;

    Thunk->Flags.AddFunctionTable = FALSE;
    Thunk->ExitThread = Rtl->ExitThread;
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

    TEST(5);
    TEST(4);

    return ERROR_SUCCESS;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
