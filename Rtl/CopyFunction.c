/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    CopyFunction.c

Abstract:

    This module implements the CopyFunction routine.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
CopyFunction(
    PRTL Rtl,
    PALLOCATOR Allocator,
    HANDLE TargetProcessHandle,
    PVOID SourceFunction,
    PVOID SourceHandlerFunction,
    PBYTE ThunkBuffer,
    USHORT SizeOfThunkBufferInBytes,
    PBYTE UserData,
    USHORT SizeOfUserDataInBytes,
    PADJUST_THUNK_POINTERS AdjustThunkPointers,
    PADJUST_USER_DATA_POINTERS AdjustUserDataPointers,
    PPVOID DestBaseCodeAddressPointer,
    PPVOID DestThunkBufferAddressPointer,
    PPVOID DestUserDataAddressPointer,
    PPVOID DestFunctionPointer,
    PPVOID LocalBaseCodeAddressPointer,
    PPVOID LocalThunkBufferAddressPointer,
    PPVOID LocalUserDataAddressPointer,
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
    SHORT NumberOfDataBytesRemaining;
    USHORT NumberOfDataBytesWritten;
    ULONG OldProtection;
    USHORT EntryCount;
    SIZE_T NumberOfBytesWritten;
    HANDLE ProcessHandle = GetCurrentProcess();
    const ULONG OnePage = 1 << PAGE_SHIFT;
    const ULONG TwoPages = 2 << PAGE_SHIFT;
    const ULONG ThreePages = 3 << PAGE_SHIFT;
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
    PBYTE SourceCode;
    PBYTE SourceHandlerCode;
    PBYTE LocalBaseData;
    PBYTE RemoteDestUserData;
    PBYTE RemoteDestBaseData;
    PBYTE DestThunkBuffer;
    PBYTE DestThunkBufferEnd;
    PBYTE RemoteDestThunkBuffer;
    PBYTE NewDestUserData;
    PBYTE NewRemoteDestUserData;

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
    *DestThunkBufferAddressPointer = NULL;
    *DestUserDataAddressPointer = NULL;
    *DestFunctionPointer = NULL;
    *DestRuntimeFunctionPointer = NULL;
    *EntryCountPointer = 0;

    //
    // Allocate three local pages; one for code, one for readonly data, and one
    // for read/write data.
    //

    LocalBaseCodeAddress = (
        VirtualAllocEx(
            ProcessHandle,
            NULL,
            ThreePages,
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
            ThreePages,
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

    NumberOfDataBytesAllocated = (USHORT)(DestData - LocalBaseData);
    NumberOfDataBytesRemaining = (USHORT)(
        ALIGN_UP(NumberOfDataBytesAllocated, PAGE_SIZE) -
        NumberOfDataBytesAllocated
    );
    RemoteDestThunkBuffer = RemoteDestBaseData + NumberOfDataBytesAllocated;

    //
    // Copy thunk data and adjust thunk pointers.
    //

    DestThunkBuffer = DestData;
    DestThunkBufferEnd = DestThunkBuffer + SizeOfThunkBufferInBytes;
    CopyMemory(DestThunkBuffer, ThunkBuffer, SizeOfThunkBufferInBytes);
    DestData += SizeOfThunkBufferInBytes;

    Success = AdjustThunkPointers(Rtl,
                                  TargetProcessHandle,
                                  ThunkBuffer,
                                  SizeOfThunkBufferInBytes,
                                  DestThunkBuffer,
                                  NumberOfDataBytesRemaining,
                                  RemoteDestThunkBuffer,
                                  (PRUNTIME_FUNCTION)RemoteBaseDataAddress,
                                  RemoteBaseCodeAddress,
                                  EntryCount,
                                  &NumberOfDataBytesWritten,
                                  &NewDestUserData,
                                  &NewRemoteDestUserData);

    if (NumberOfDataBytesWritten > NumberOfDataBytesRemaining) {
        __debugbreak();
        return FALSE;
    }

    if (NewDestUserData - NumberOfDataBytesWritten !=
        DestThunkBuffer + SizeOfThunkBufferInBytes) {
        __debugbreak();
        return FALSE;
    }

    if (NewRemoteDestUserData - NumberOfDataBytesWritten !=
        RemoteDestThunkBuffer + SizeOfThunkBufferInBytes) {
        __debugbreak();
    }

    DestData += NumberOfDataBytesWritten;
    NumberOfDataBytesRemaining -= NumberOfDataBytesWritten;

    if (DestData != NewDestUserData) {
        __debugbreak();
        return FALSE;
    }

    RemoteDestUserData = NewRemoteDestUserData;

    //
    // Copy user data and adjust pointers.
    //

    if (!SizeOfUserDataInBytes) {
        goto WriteMemory;
    }

    if (SizeOfUserDataInBytes > NumberOfDataBytesRemaining) {
        __debugbreak();
        return FALSE;
    }

    if (PointerToOffsetCrossesPageBoundary(DestData, SizeOfUserDataInBytes)) {
        __debugbreak();
        goto Cleanup;
    }

    DestUserData = DestData;
    CopyMemory(DestUserData, UserData, SizeOfUserDataInBytes);

    NumberOfDataBytesAllocated = (USHORT)(DestData - LocalBaseData);

    if (RemoteDestUserData <=
        (RemoteDestThunkBuffer + SizeOfThunkBufferInBytes)) {
        __debugbreak();
        return FALSE;
    }

    NumberOfDataBytesRemaining = (USHORT)(
        ALIGN_UP(NumberOfDataBytesAllocated, PAGE_SIZE) -
        NumberOfDataBytesAllocated
    );

    if (!AdjustUserDataPointers) {
        goto WriteMemory;
    }

    Success = AdjustUserDataPointers(Rtl,
                                     TargetProcessHandle,
                                     UserData,
                                     SizeOfUserDataInBytes,
                                     DestUserData,
                                     NumberOfDataBytesRemaining,
                                     RemoteDestUserData);

    if (!Success) {
        goto Cleanup;
    }

WriteMemory:

    //
    // Write all three pages with a single call.
    //

    Success = (
        WriteProcessMemory(
            TargetProcessHandle,
            RemoteBaseCodeAddress,
            LocalBaseCodeAddress,
            ThreePages,
            &NumberOfBytesWritten
        )
    );

    if (!Success) {
        goto Cleanup;
    }

    //
    // Make the first page read/executable.
    //

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

    //
    // Make the second page readonly.
    //

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

    //
    // N.B. The third page is already read/write, so we don't have to do
    //      anything for it.
    //

    *EntryCountPointer = EntryCount;
    *DestRuntimeFunctionPointer = DestRuntimeFunction;
    *DestBaseCodeAddressPointer = RemoteBaseCodeAddress;
    *DestFunctionPointer = ((PBYTE)RemoteBaseCodeAddress) + 16;
    *DestUserDataAddressPointer = RemoteDestUserData;
    *DestThunkBufferAddressPointer = RemoteDestThunkBuffer;

    //
    // Intentional follow-on to Cleanup.
    //

Cleanup:

    if (LocalBaseCodeAddress) {

        if (ARGUMENT_PRESENT(LocalBaseCodeAddressPointer)) {
            *LocalBaseCodeAddressPointer = LocalBaseCodeAddress;

            if (ARGUMENT_PRESENT(LocalThunkBufferAddressPointer)) {
                *LocalThunkBufferAddressPointer = DestThunkBuffer;
            }

            if (ARGUMENT_PRESENT(LocalUserDataAddressPointer)) {
                *LocalUserDataAddressPointer = DestUserData;
            }

        } else {

            *LocalBaseCodeAddressPointer = NULL;
            *LocalThunkBufferAddressPointer = NULL;
            *LocalUserDataAddressPointer = NULL;

            VirtualFreeEx(
                ProcessHandle,
                LocalBaseCodeAddress,
                0,
                MEM_RELEASE
            );

            LocalBaseCodeAddress = NULL;
        }
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


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
