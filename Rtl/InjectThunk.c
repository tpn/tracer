/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectThunk.c

Abstract:

    This module implements the InjectThunk routine.

--*/

#include "stdafx.h"

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


_Use_decl_annotations_
BOOL
CopyFunction(
    PRTL Rtl,
    PALLOCATOR Allocator,
    HANDLE TargetProcessHandle,
    PVOID SourceFunction,
    PVOID SourceHandlerFunction,
    USHORT SizeOfDataBufferInBytes,
    PBYTE DataBuffer,
    PADJUST_POINTERS AdjustPointers,
    PPVOID DestBaseCodeAddressPointer,
    PPVOID DestDataBufferAddressPointer,
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
    USHORT NumberOfDataBytesAllocated;
    USHORT NumberOfDataBytesRemaining;
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
    PBYTE DestUserData;
    PBYTE DestUserEndData;
    PBYTE SourceCode;
    PBYTE SourceHandlerCode;
    PBYTE LocalBaseData;
    PBYTE RemoteDestUserData;
    PBYTE RemoteDestBaseData;

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

    *DestBaseCodeAddressPointer = NULL;
    *DestDataBufferAddressPointer = NULL;
    *DestFunctionPointer = NULL;
    *DestRuntimeFunctionPointer = NULL;
    *EntryCountPointer = 0;

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
    LocalBaseData = (PBYTE)LocalBaseDataAddress;

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
    RemoteDestBaseData = (PBYTE)RemoteBaseDataAddress;

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
    DestData += ALIGN_UP(TotalUnwindSize + 16, 16);

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

        DestHandlerUnwindInfo = (PUNWIND_INFO)DestData;

        CopyMemory((PBYTE)DestHandlerUnwindInfo,
                   (PBYTE)HandlerUnwindInfo,
                   TotalHandlerUnwindSize);

        DestData += ALIGN_UP(TotalHandlerUnwindSize + 16, 16);

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

    if (!IsSamePage(LocalBaseDataAddress, DestData)) {
        __debugbreak();
        goto Cleanup;
    }

    if (!SizeOfDataBufferInBytes) {
        goto WriteMemory;
    }

    if (PointerToOffsetCrossesPageBoundary(DestData, SizeOfDataBufferInBytes)) {
        __debugbreak();
        goto Cleanup;
    }

    NumberOfDataBytesAllocated = (USHORT)(DestData - LocalBaseData);
    NumberOfDataBytesRemaining = (USHORT)(
        ALIGN_UP(NumberOfDataBytesAllocated, PAGE_SIZE) -
        NumberOfDataBytesAllocated
    );
    RemoteDestUserData = RemoteDestBaseData + NumberOfDataBytesAllocated;

    DestUserData = DestData;
    DestUserEndData = DestUserData + SizeOfDataBufferInBytes;
    CopyMemory(DestUserData, DataBuffer, SizeOfDataBufferInBytes);

    Success = AdjustPointers(Rtl,
                             DataBuffer,
                             SizeOfDataBufferInBytes,
                             DestUserData,
                             NumberOfDataBytesRemaining,
                             (ULONG_PTR)RemoteDestUserData,
                             (PRUNTIME_FUNCTION)RemoteBaseDataAddress,
                             RemoteBaseCodeAddress,
                             EntryCount);

    if (!Success) {
        goto Cleanup;
    }

WriteMemory:
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

    if (!Success) {
        goto Cleanup;
    }

    *DestRuntimeFunctionPointer = DestRuntimeFunction;
    *DestBaseCodeAddressPointer = RemoteBaseCodeAddress;
    *DestFunctionPointer = ((PBYTE)RemoteBaseCodeAddress) + 16;
    *EntryCountPointer = EntryCount;
    *DestDataBufferAddressPointer = RemoteDestUserData;

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

End:

    return Success;
}

BOOL
InjectThunk(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PCUNICODE_STRING InjectionThunkDllPath,
    PCSTRING InjectionThunkFunctionName,
    PCUNICODE_STRING TargetModuleDllPath,
    PCSTRING TargetFunctionName,
    HANDLE TargetProcessHandle,
    PHANDLE RemoteThreadHandlePointer,
    PULONG RemoteThreadIdPointer
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

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
