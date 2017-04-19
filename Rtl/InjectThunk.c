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
    ULONG_PTR RemoteThunkBufferAddress,
    PRUNTIME_FUNCTION RemoteRuntimeFunction,
    PVOID RemoteCodeBaseAddress,
    USHORT EntryCount,
    PUSHORT NumberOfBytesWritten
    )
{
    USHORT Bytes;
    USHORT TotalBytes;
    PBYTE Base;
    PBYTE Dest;
    PINJECTION_THUNK_CONTEXT OriginalThunk;
    PINJECTION_THUNK_CONTEXT Thunk;

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

    Bytes = OriginalThunk->ModulePath.Length + sizeof(WCHAR);
    CopyMemory(Dest, (PBYTE)OriginalThunk->ModulePath.Buffer, Bytes);

    Bytes = ALIGN_UP_POINTER(Bytes + 8);
    Dest += Bytes;
    TotalBytes = Bytes;

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
            RemoteThunkBufferAddress,
            SizeOfThunkBufferInBytes + (Dest - Base)
        )
    );

    //
    // Adjust the runtime function entry details for RtlAddFunctionTable().
    //

    Thunk->EntryCount = EntryCount;
    Thunk->FunctionTable = RemoteRuntimeFunction;
    Thunk->BaseCodeAddress = RemoteCodeBaseAddress;

    //
    // Set the offset of the user data from the base thunk.
    //

    Thunk->UserDataOffset = TotalBytes;
    *NumberOfBytesWritten = TotalBytes;

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
    PPVOID RemoteUserDataAddress
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
            DestUserDataAddress,
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
