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
    _In_opt_ PVOID SourceHandlerFunction,
    _Out_ PPVOID DestBaseAddressPointer,
    _Out_ PPVOID DestFunctionPointer,
    _Out_ PRUNTIME_FUNCTION *DestRuntimeFunctionPointer,
    _Out_ PRUNTIME_FUNCTION *DestHandlerRuntimeFunctionPointer,
    _Out_ PULONG EntryCountPointer
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
    PVOID SourceHandlerFunction,
    PPVOID DestBaseAddressPointer,
    PPVOID DestFunctionPointer,
    PRUNTIME_FUNCTION *DestRuntimeFunctionPointer,
    PRUNTIME_FUNCTION *DestHandlerRuntimeFunctionPointer,
    PULONG EntryCountPointer
    )
{
    BOOL Success = FALSE;
    BOOL HasHandler = FALSE;
    USHORT CodeSize;
    USHORT HandlerCodeSize;
    SHORT UnwindCodeSize;
    SHORT UnwindInfoSize;
    SHORT TotalUnwindSize;
    SHORT UnwindInfoHeaderSize;
    SHORT NumberOfUnwindCodes;
    SHORT HandlerUnwindCodeSize;
    SHORT TotalHandlerUnwindSize;
    ULONG OldProtection;
    ULONG EntryCount;
    SIZE_T NumberOfBytesWritten;
    HANDLE ProcessHandle = GetCurrentProcess();
    const ULONG OnePage = 1 << PAGE_SHIFT;
    const ULONG TwoPages = 2 << PAGE_SHIFT;
    PVOID SourceFunc;
    PVOID HandlerFunc;
    PVOID HandlerFunction;
    PVOID LocalBaseCodeAddress = NULL;
    PVOID LocalBaseDataAddress = NULL;
    PVOID RemoteBaseCodeAddress = NULL;
    PVOID RemoteBaseDataAddress = NULL;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRUNTIME_FUNCTION DestHandlerRuntimeFunction;
    PRUNTIME_FUNCTION SourceRuntimeFunction;
    PRUNTIME_FUNCTION SourceHandlerRuntimeFunction;
    RUNTIME_FUNCTION_EX SourceRuntimeFunctionEx;
    RUNTIME_FUNCTION_EX SourceHandlerRuntimeFunctionEx;
    PUNWIND_INFO UnwindInfo;
    PUNWIND_INFO DestUnwindInfo;
    PUNWIND_INFO HandlerUnwindInfo;
    PUNWIND_INFO DestHandlerUnwindInfo;
    ULONG SourceHandlerFieldOffset;
    ULONG SourceHandlerOffset;
    ULONG SourceScopeTableFieldOffset;
    ULONG SourceScopeTableOffset;
    ULONGLONG SourceBaseAddress;
    ULONGLONG HandlerBaseAddress;
    PBYTE DestCode;
    PBYTE DestHandlerCode;
    PBYTE EndCode;
    PBYTE EndHandlerCode;
    PBYTE DestData;
    PBYTE SourceCode;
    PBYTE SourceHandlerCode;

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
    *DestRuntimeFunctionPointer = NULL;

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

    if (UnwindInfo->Flags & UNW_FLAG_CHAININFO) {

        //
        // We don't support copying functions with chained unwind info at the
        // moment.
        //

        __debugbreak();
        goto Cleanup;
    } else if ((UnwindInfo->Flags & (UNW_FLAG_EHANDLER | UNW_FLAG_UHANDLER))) {
        HasHandler = TRUE;
        EntryCount = 2;
    } else {
        EntryCount = 1;
    }

    if (UnwindInfo->Version != 1) {
        __debugbreak();
        goto Cleanup;
    }

    CodeSize = (USHORT)(
        (ULONGLONG)SourceRuntimeFunction->EndAddress -
        (ULONGLONG)SourceRuntimeFunction->BeginAddress
    );

    SourceCode = (PBYTE)SourceRuntimeFunctionEx.BeginAddress;

    UnwindInfoSize = sizeof(UNWIND_INFO);
    UnwindInfoHeaderSize = FIELD_OFFSET(UNWIND_INFO, UnwindCode);

    if (UnwindInfo->CountOfCodes > 0) {
        NumberOfUnwindCodes = UnwindInfo->CountOfCodes;
        if (NumberOfUnwindCodes % 2 != 0) {
            NumberOfUnwindCodes += 1;
        }
        UnwindCodeSize = NumberOfUnwindCodes * sizeof(UNWIND_CODE);
    }

    TotalUnwindSize = UnwindInfoHeaderSize + UnwindCodeSize;

    SourceHandlerFieldOffset = UnwindInfoHeaderSize + UnwindCodeSize;
    SourceScopeTableFieldOffset = SourceHandlerFieldOffset + sizeof(ULONG);

    SourceHandlerOffset = *((PULONG)(
        (PBYTE)UnwindInfo + SourceHandlerFieldOffset
    ));

    SourceScopeTableOffset = *((PULONG)(
        (PBYTE)UnwindInfo + SourceScopeTableFieldOffset
    ));

    SourceRuntimeFunctionEx.HandlerFunction = (PVOID)(
        SourceBaseAddress + SourceHandlerOffset
    );

    if (HasHandler && SourceScopeTableOffset) {
        ULONG ScopeCount;

        SourceRuntimeFunctionEx.ScopeTable = (PVOID)(
            SourceBaseAddress + SourceScopeTableOffset
        );

        ScopeCount = SourceRuntimeFunctionEx.ScopeTable->Count;

        if (ScopeCount > 0) {
            TotalUnwindSize += (
                ((USHORT)sizeof(ULONG)) +
                ((USHORT)ScopeCount * (USHORT)sizeof(UNWIND_SCOPE_RECORD))
            );
        }
    }

    DestCode = (PBYTE)LocalBaseCodeAddress;

    DestCode += 16;
    CopyMemory(DestCode, SourceCode, CodeSize);
    EndCode = DestCode + CodeSize;

    DestData = (PBYTE)LocalBaseDataAddress;
    DestRuntimeFunction = (PRUNTIME_FUNCTION)DestData;
    DestHandlerRuntimeFunction = DestRuntimeFunction + 1;

    DestData = (PBYTE)ALIGN_UP(DestHandlerRuntimeFunction + 16, 16);

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

    if (HasHandler) {

        HandlerFunction = SourceRuntimeFunctionEx.HandlerFunction;

        HandlerFunc = SkipJumpsInline(HandlerFunction);

        SourceHandlerRuntimeFunction = (
            Rtl->RtlLookupFunctionEntry(
                (ULONGLONG)HandlerFunc,
                &HandlerBaseAddress,
                NULL
            )
        );

        if (!SourceHandlerRuntimeFunction) {
            goto Cleanup;
        }

        SourceHandlerRuntimeFunctionEx.BeginAddress = (PVOID)(
            HandlerBaseAddress + SourceHandlerRuntimeFunction->BeginAddress
        );

        SourceHandlerRuntimeFunctionEx.EndAddress = (PVOID)(
            HandlerBaseAddress + SourceHandlerRuntimeFunction->EndAddress
        );

        SourceHandlerRuntimeFunctionEx.UnwindInfo = (PUNWIND_INFO)(
            HandlerBaseAddress +
            SourceHandlerRuntimeFunction->UnwindInfoAddress
        );

        HandlerUnwindInfo = SourceHandlerRuntimeFunctionEx.UnwindInfo;

        HandlerCodeSize = (USHORT)(
            (ULONGLONG)SourceHandlerRuntimeFunction->EndAddress -
            (ULONGLONG)SourceHandlerRuntimeFunction->BeginAddress
        );

        SourceHandlerCode = (PBYTE)SourceHandlerRuntimeFunctionEx.BeginAddress;

        if (HandlerUnwindInfo->CountOfCodes > 0) {
            NumberOfUnwindCodes = HandlerUnwindInfo->CountOfCodes;
            if (NumberOfUnwindCodes % 2 != 0) {
                NumberOfUnwindCodes += 1;
            }
            HandlerUnwindCodeSize = NumberOfUnwindCodes * sizeof(UNWIND_CODE);
        } else {
            HandlerUnwindCodeSize = 0;
        }

        TotalHandlerUnwindSize = UnwindInfoHeaderSize + HandlerUnwindCodeSize;

        if (HandlerUnwindInfo->Flags != 0) {
            __debugbreak();
            goto Cleanup;
        }

        if (HandlerUnwindInfo->Version != 1) {
            __debugbreak();
            goto Cleanup;
        }

        DestHandlerCode = (PBYTE)ALIGN_UP(EndCode + 16, 16);
        CopyMemory(DestHandlerCode, SourceHandlerCode, HandlerCodeSize);
        EndHandlerCode = DestHandlerCode + HandlerCodeSize;

        DestHandlerUnwindInfo = (PUNWIND_INFO)(
            (PBYTE)DestUnwindInfo + ALIGN_UP(TotalUnwindSize, 16)
        );

        CopyMemory((PBYTE)DestHandlerUnwindInfo,
                   (PBYTE)HandlerUnwindInfo,
                   TotalHandlerUnwindSize);

        DestHandlerRuntimeFunction->BeginAddress = (ULONG)(
            (ULONG_PTR)DestHandlerCode - (ULONG_PTR)LocalBaseCodeAddress
        );

        DestHandlerRuntimeFunction->EndAddress = (ULONG)(
            (ULONG_PTR)EndHandlerCode - (ULONG_PTR)LocalBaseCodeAddress
        );

        DestHandlerRuntimeFunction->UnwindInfoAddress = (ULONG)(
            (ULONG_PTR)DestHandlerUnwindInfo - (ULONG_PTR)LocalBaseCodeAddress
        );

    }

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

    if (0 && ProcessHandle == TargetProcessHandle) {
        Success = (
            Rtl->RtlAddFunctionTable(
                DestRuntimeFunction,
                EntryCount,
                (ULONGLONG)RemoteBaseCodeAddress
            )
        );

        if (!Success) {
            goto Cleanup;
        }

    }

    if (Success) {
        *DestRuntimeFunctionPointer = DestRuntimeFunction;
        *DestBaseAddressPointer = RemoteBaseCodeAddress;
        *DestFunctionPointer = ((PBYTE)RemoteBaseCodeAddress) + 16;
        *EntryCountPointer = EntryCount;
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
TestInjection1(
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
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRUNTIME_FUNCTION DestHandlerRuntimeFunction;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK DestInjectionThunk;
    PRTL_EXCEPTION_HANDLER InjectionThunkHandler;
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
            "InjectionThunk1"
        )
    );

    if (!InjectionThunk) {
        return FALSE;
    }

    InjectionThunkHandler = (PRTL_EXCEPTION_HANDLER)(
        GetProcAddress(
            InjectionThunkModule,
            "InjectionThunk1Handler"
        )
    );

    if (!InjectionThunkHandler) {
        return FALSE;
    }

    InjectionThunkHandler = (PRTL_EXCEPTION_HANDLER)(
        SkipJumpsInline((PBYTE)InjectionThunkHandler)
    );

    ProcessHandle = GetCurrentProcess();

    Success = CopyFunction(Rtl,
                           Allocator,
                           ProcessHandle,
                           InjectionThunk,
                           InjectionThunkHandler,
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

    Thunk->EntryCount = 1;
    Thunk->BaseAddress = DestBaseAddress;
    Thunk->FunctionTable = DestRuntimeFunction;
    Thunk->RtlAddFunctionTable = Rtl->RtlAddFunctionTable;
    Thunk->ExitThread = Rtl->ExitThread;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;
    Thunk->ModulePath = TestInjectionActiveExePath->Buffer;
    Thunk->FunctionName = TestInjectionFunctionName;

    InitializeRtlInjectionFunctions(Rtl, &Context.Functions);

    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

    Thunk->Flags.AddFunctionTable = FALSE;
    Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

    Thunk->Flags.TestExceptionHandler = TRUE;
    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);
    Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

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
    LONG Result;
    ULONG EntryCount;
    ULONG RipOffset;
    HANDLE ProcessHandle;
    RTL_INJECTION_CONTEXT Context;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    PRUNTIME_FUNCTION DestHandlerRuntimeFunction;
    PRTL_INJECTION_THUNK_CONTEXT Thunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
    PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK DestInjectionThunk;
    PRTL_EXCEPTION_HANDLER InjectionThunkHandler;
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

    InjectionThunkHandler = (PRTL_EXCEPTION_HANDLER)(
        GetProcAddress(
            InjectionThunkModule,
            "InjectionThunkHandler"
        )
    );

    if (!InjectionThunkHandler) {
        return FALSE;
    }

    InjectionThunkHandler = (PRTL_EXCEPTION_HANDLER)(
        SkipJumpsInline((PBYTE)InjectionThunkHandler)
    );

    ProcessHandle = GetCurrentProcess();

    Success = CopyFunction(Rtl,
                           Allocator,
                           ProcessHandle,
                           InjectionThunk,
                           InjectionThunkHandler,
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

    Thunk->Flags.AddFunctionTable = TRUE;
    Thunk->EntryCount = EntryCount;
    Thunk->BaseAddress = DestBaseAddress;
    Thunk->FunctionTable = DestRuntimeFunction;
    Thunk->RtlAddFunctionTable = Rtl->RtlAddFunctionTable;
    Thunk->ExitThread = Rtl->ExitThread;
    Thunk->LoadLibraryW = Rtl->LoadLibraryW;
    Thunk->GetProcAddress = Rtl->GetProcAddress;
    Thunk->ModulePath = TestInjectionActiveExePath->Buffer;
    Thunk->FunctionName = TestInjectionFunctionName;

    InitializeRtlInjectionFunctions(Rtl, &Context.Functions);

    //goto TestExceptionHandler;

    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);
    Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

//TestExceptionHandler:

    RipOffset = FIELD_OFFSET(CONTEXT, Rip);

    Thunk->Flags.TestExceptionHandler = TRUE;
    Thunk->Flags.TestAccessViolation = TRUE;
    Result = InjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);
    Result = DestInjectionThunk((PRTL_INJECTION_CONTEXT)Thunk);

    Thunk->Flags.TestAccessViolation = FALSE;
    Thunk->Flags.TestIllegalInstruction = TRUE;
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
