/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectThunkEx.c

Abstract:

    This module implements the AdjustThunkPointersEx and InjectThunkEx routines.

--*/

#include "stdafx.h"

ADJUST_THUNK_POINTERS_EX AdjustThunkPointersEx;

_Use_decl_annotations_
BOOL
AdjustThunkPointersEx(
    PRTL Rtl,
    HANDLE TargetProcessHandle,
    PBYTE OriginalThunkBuffer,
    USHORT SizeOfThunkBufferInBytes,
    PBYTE TemporaryLocalThunkBuffer,
    USHORT BytesRemaining,
    PBYTE RemoteThunkBufferAddress,
    PRUNTIME_FUNCTION RemoteRuntimeFunction,
    PVOID RemoteCodeBaseAddress,
    USHORT EntryCount,
    PUSHORT NumberOfBytesWritten,
    PBYTE *NewDestUserData,
    PBYTE *NewRemoteDestUserData
    )
{
    USHORT TotalBytes;
    USHORT UserDataOffset;
    USHORT ModulePathAllocSizeInBytes;
    USHORT FunctionNameAllocSizeInBytes;
    PBYTE Base;
    PBYTE Dest;
    PBYTE NewUserData;
    PBYTE NewRemoteUserData;
    PBYTE UserPage;
    PBYTE RemotePage;
    PINJECTION_THUNK_EX_CONTEXT Thunk;
    PINJECTION_THUNK_EX_CONTEXT OriginalThunk;

    if (SizeOfThunkBufferInBytes != sizeof(INJECTION_THUNK_EX_CONTEXT)) {
        __debugbreak();
        return FALSE;
    }

    OriginalThunk = (PINJECTION_THUNK_EX_CONTEXT)OriginalThunkBuffer;
    Thunk = (PINJECTION_THUNK_EX_CONTEXT)TemporaryLocalThunkBuffer;

    Base = Dest = (PBYTE)(
        RtlOffsetToPointer(
            TemporaryLocalThunkBuffer,
            SizeOfThunkBufferInBytes
        )
    );

    //
    // Adjust the module path's buffer pointer such that it's valid in the
    // remote process's address space.
    //

    Thunk->ModulePath.Buffer = (PWSTR)(
        RtlOffsetToPointer(
            RemoteThunkBufferAddress,
            SizeOfThunkBufferInBytes
        )
    );

    //
    // Copy the InjectionThunk.dll path.
    //

    CopyMemory(Dest,
               OriginalThunk->ModulePath.Buffer,
               OriginalThunk->ModulePath.Length);

    ModulePathAllocSizeInBytes = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            OriginalThunk->ModulePath.Length +
            sizeof(WCHAR)
        )
    );

    Dest += ModulePathAllocSizeInBytes;
    TotalBytes = ModulePathAllocSizeInBytes;

    //
    // Copy the function name.
    //

    CopyMemory(Dest,
               OriginalThunk->FunctionName.Buffer,
               OriginalThunk->FunctionName.Length);

    FunctionNameAllocSizeInBytes = (
        ALIGN_UP_USHORT_TO_POINTER_SIZE(
            OriginalThunk->FunctionName.Length +
            sizeof(CHAR)
        )
    );

    TotalBytes += FunctionNameAllocSizeInBytes;

    //
    // Adjust the function name's buffer.
    //

    Thunk->FunctionName.Buffer = (PSTR)(
        RtlOffsetToPointer(
            RemoteThunkBufferAddress,
            SizeOfThunkBufferInBytes + (Dest - Base)
        )
    );

    Dest += FunctionNameAllocSizeInBytes;

    //
    // Adjust the runtime function entry details for RtlAddFunctionTable().
    //

    Thunk->EntryCount = EntryCount;
    Thunk->FunctionTable = RemoteRuntimeFunction;
    Thunk->BaseCodeAddress = RemoteCodeBaseAddress;

    //
    // Invariant checks.
    //

    NewUserData = Base + TotalBytes;
    if (NewUserData != Dest) {
        __debugbreak();
        return FALSE;
    }

    UserDataOffset = (USHORT)(Dest - TemporaryLocalThunkBuffer);

    NewRemoteUserData = (PBYTE)(
        RtlOffsetToPointer(
            RemoteThunkBufferAddress,
            UserDataOffset
        )
    );

    //
    // Isolate the PAGE_SIZE bits of each address and ensure they match.
    //

    UserPage = (PBYTE)((ULONG_PTR)NewUserData & PAGE_SIZE);
    RemotePage = (PBYTE)((ULONG_PTR)NewRemoteUserData & PAGE_SIZE);

    if (UserPage != RemotePage) {
        __debugbreak();
    }

    //
    // Set the offset of the user data from the base thunk.
    //

    Thunk->UserDataOffset = UserDataOffset;
    *NumberOfBytesWritten = TotalBytes;
    *NewDestUserData = NewUserData;
    *NewRemoteDestUserData = NewRemoteUserData;

    return TRUE;
}

INJECT_THUNK_EX InjectThunkExImpl

_Use_decl_annotations_
BOOL
InjectThunkEx(
    PRTL Rtl,
    PALLOCATOR Allocator,
    INJECTION_THUNK_EX_FLAGS Flags,
    HANDLE TargetProcessHandle,
    PINJECTION_OBJECTS InjectionObjects,
    PCUNICODE_STRING TargetModuleDllPath,
    PCSTRING TargetFunctionName,
    PAPC Apc,
    PBYTE UserData,
    USHORT SizeOfUserDataInBytes,
    USHORT DesiredNumberOfWritablePages,
    PADJUST_USER_DATA_POINTERS AdjustUserDataPointers,
    PHANDLE RemoteThreadHandlePointer,
    PULONG RemoteThreadIdPointer,
    PPVOID RemoteBaseCodeAddress,
    PPVOID RemoteUserDataAddress,
    PPVOID RemoteWritableDataAddress,
    PPVOID RemoteUserWritableDataAddress,
    PUSHORT ActualNumberOfWritablePages
    )
{
    BOOL Success;
    ULONG EntryCount;
    ULONG CreationFlags;
    ULONG RemoteThreadId;
    HANDLE RemoteThreadHandle;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    INJECTION_THUNK_EX_CONTEXT Thunk;
    PVOID DestInjectionThunk;
    PVOID DestBaseCodeAddress;
    PVOID DestUserDataAddress;
    PVOID DestThunkBufferAddress;
    PVOID DestWritableThunkBufferAddress;
    LPTHREAD_START_ROUTINE StartRoutine;
    PCOPY_FUNCTION_EX CopyFunctionEx;

    if (!Rtl->InitializeInjection(Rtl)) {
        return FALSE;
    }

    Thunk.Flags.AsULong = Flags.AsULong;
    Thunk.LoadLibraryW = Rtl->LoadLibraryW;
    Thunk.GetProcAddress = Rtl->GetProcAddress;
    Thunk.RtlAddFunctionTable = Rtl->RtlAddFunctionTable;

    InitializeStringFromString(&Thunk.FunctionName, TargetFunctionName);
    InitializeUnicodeStringFromUnicodeString(&Thunk.ModulePath,
                                             TargetModuleDllPath);

    CopyFunctionEx = Rtl->CopyFunctionEx;

    Success = CopyFunctionEx(Rtl,
                             Allocator,
                             TargetProcessHandle,
                             Rtl->InjectionThunkRoutine,
                             NULL,
                             (PBYTE)&Thunk,
                             sizeof(Thunk),
                             (PBYTE)UserData,
                             SizeOfUserDataInBytes,
                             DesiredNumberOfWritablePages,
                             AdjustThunkPointers,
                             AdjustUserDataPointers,
                             &DestBaseCodeAddress,
                             &DestThunkBufferAddress,
                             &DestUserDataAddress,
                             &DestInjectionThunk,
                             &DestRuntimeFunction,
                             NULL,
                             &EntryCount);

    if (!Success) {
        return FALSE;
    }

    StartRoutine = (LPTHREAD_START_ROUTINE)DestInjectionThunk;
    CreationFlags = 0;

    RemoteThreadHandle = (
        CreateRemoteThread(
            TargetProcessHandle,
            NULL,
            0,
            StartRoutine,
            DestThunkBufferAddress,
            CreationFlags,
            &RemoteThreadId
        )
    );

    if (!RemoteThreadHandle || RemoteThreadHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    *RemoteThreadHandlePointer = RemoteThreadHandle;
    *RemoteThreadIdPointer = RemoteThreadId;
    *RemoteBaseCodeAddress = DestBaseCodeAddress;
    *RemoteUserDataAddress = DestUserDataAddress;

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
