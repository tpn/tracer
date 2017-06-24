/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectThunk.c

Abstract:

    This module implements the AdjustThunkPointers and InjectThunk routine.

--*/

#include "stdafx.h"

ADJUST_THUNK_POINTERS AdjustThunkPointers;

_Use_decl_annotations_
BOOL
AdjustThunkPointers(
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
    PINJECTION_THUNK_CONTEXT Thunk;
    PINJECTION_THUNK_CONTEXT OriginalThunk;

    if (SizeOfThunkBufferInBytes != sizeof(INJECTION_THUNK_CONTEXT)) {
        __debugbreak();
        return FALSE;
    }

    OriginalThunk = (PINJECTION_THUNK_CONTEXT)OriginalThunkBuffer;
    Thunk = (PINJECTION_THUNK_CONTEXT)TemporaryLocalThunkBuffer;

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

_Use_decl_annotations_
BOOL
InjectThunk(
    PRTL Rtl,
    PALLOCATOR Allocator,
    INJECTION_THUNK_FLAGS Flags,
    HANDLE TargetProcessHandle,
    PCUNICODE_STRING TargetModuleDllPath,
    PCSTRING TargetFunctionName,
    PBYTE UserData,
    USHORT SizeOfUserDataInBytes,
    PADJUST_USER_DATA_POINTERS AdjustUserDataPointers,
    PHANDLE RemoteThreadHandlePointer,
    PULONG RemoteThreadIdPointer,
    PPVOID RemoteBaseCodeAddress,
    PPVOID RemoteUserDataAddress,
    PPVOID LocalBaseCodeAddressPointer,
    PPVOID LocalThunkBufferAddressPointer,
    PPVOID LocalUserDataAddressPointer
    )
{
    BOOL Success;
    ULONG EntryCount;
    ULONG CreationFlags;
    ULONG RemoteThreadId;
    HANDLE RemoteThreadHandle;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    INJECTION_THUNK_CONTEXT Thunk;
    PVOID DestInjectionThunk;
    PVOID DestBaseCodeAddress;
    PVOID DestUserDataAddress;
    PVOID DestThunkBufferAddress;
    PVOID LocalBaseCodeAddress;
    PVOID LocalThunkBufferAddress;
    PVOID LocalUserDataAddress;
    //PVOID InjectionThunkRoutine;
    LPTHREAD_START_ROUTINE StartRoutine;
    PCOPY_FUNCTION CopyFunction;

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

    CopyFunction = Rtl->CopyFunction;

    Success = CopyFunction(Rtl,
                           Allocator,
                           TargetProcessHandle,
                           Rtl->InjectionThunkRoutine,
                           NULL,
                           (PBYTE)&Thunk,
                           sizeof(Thunk),
                           (PBYTE)UserData,
                           SizeOfUserDataInBytes,
                           AdjustThunkPointers,
                           AdjustUserDataPointers,
                           &DestBaseCodeAddress,
                           &DestThunkBufferAddress,
                           &DestUserDataAddress,
                           &DestInjectionThunk,
                           &LocalBaseCodeAddress,
                           &LocalThunkBufferAddress,
                           &LocalUserDataAddress,
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

    if (ARGUMENT_PRESENT(LocalBaseCodeAddressPointer)) {
        *LocalBaseCodeAddressPointer = LocalBaseCodeAddress;

        if (ARGUMENT_PRESENT(LocalThunkBufferAddressPointer)) {
            *LocalThunkBufferAddressPointer = LocalThunkBufferAddress;
        }

        if (ARGUMENT_PRESENT(LocalUserDataAddressPointer)) {
            *LocalUserDataAddressPointer = LocalUserDataAddress;
        }
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
