/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Injection.c

Abstract:

    This module implements the Rtl component's remote thread, code and object
    injection functionality, collectively referred to as "injection".  The
    main function used to interface to this functionality is the Inject()
    routine.  An internal routine is also implemented for adjusting a thunk
    buffer as part of the CopyFunction() interface, upon which this injection
    logic depends.

--*/

#include "stdafx.h"

//
// Forward declaration.
//

ADJUST_THUNK_POINTERS AdjustThunkPointers;

//
// Main injection function.
//

_Use_decl_annotations_
BOOL
Inject(
    PRTL Rtl,
    PALLOCATOR Allocator,
    INJECTION_THUNK_FLAGS Flags,
    HANDLE TargetProcessHandle,
    PCUNICODE_STRING TargetModuleDllPath,
    PCSTRING TargetFunctionName,
    PAPC UserApc,
    PBYTE UserData,
    USHORT SizeOfUserDataInBytes,
    PINJECTION_OBJECTS InjectionObjects,
    PADJUST_USER_DATA_POINTERS AdjustUserDataPointers,
    PHANDLE RemoteThreadHandlePointer,
    PULONG RemoteThreadIdPointer,
    PPVOID RemoteBaseCodeAddress,
    PPVOID RemoteUserDataAddress,
    PPVOID LocalBaseCodeAddressPointer,
    PPVOID LocalThunkBufferAddressPointer,
    PPVOID LocalUserDataAddressPointer
    )
/*++

Routine Description:

    This routine creates a remote thread in the target process, creates handles
    for any injection objects requested (e.g. named events, named file mappings,
    etc.), optionally calls an APC-conforming routine (which must already exist
    in the remote process), and then loads a given DLL module, looks up the
    address of the exported function name, then calls the resulting function
    pointer (which should point to a routine that conforms with the function
    signature INJECTION_COMPLETE) with relevant addresses of the user's data
    (which will be copied and pointer-adjusted in the remote address space),
    injection objects, and injection functions as parameters.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure to use for all
        memory allocations for this injection.

    Flags - Supplies injection thunk flags, such as whether or not the remote
        thread thunk should immediately debug break at the earliest opportunity,
        whether or not injection objects are being used, etc.

    TargetProcessHandle - Supplies a handle to the target process for which the
        injection is to take place.  The caller must have sufficient privilege
        to call CreateRemoteThread() against this handle in order for injection
        to work.

    TargetModuleDllPath - Supplies a pointer to a UNICODE_STRING structure that
        represents a fully-qualified path of a DLL to be loaded after the thunk
        has initialized any injection objects.

    TargetFunctionName - Supplies a pointer to a STRING structure that reflects
        the exported name of the function conforming to the INJECTION_COMPLETE
        signature in the module identified by TargetModuleDllPath.  This will
        be passed to GetProcAddress() and then called with the relevant params
        (as per the signature) as the final step by the injection thunk thread.

    UserApc - Optionally supplies a pointer to an APC structure representing
        an APC routine to call in the remote process.  All addresses within the
        APC structure must already be in the remote process address space.

    UserData - Supplies the address of opaque user data that will be injected
        into the remote process in a readonly memory segment.  Tnis is typically
        used for user-specific context structures related to the injection.

    SizeOfUserDataInBytes - Supplies the size of the data represented by the
        UserData parameter, in bytes.  The maximum size of user data is about
        ~3500 bytes -- essentially, it must fit within the single readonly page
        being used for data (by both the thunk and the user).  Any attempt to
        write to memory backed by this address range in the remote process will
        fault due to readonly protection.

    AdjustUserDataPointers - Optionally supplies a pointer to a function that
        will be invoked as part of injection preparation such that any addresses
        (i.e. pointer fields) can be converted into their remote equivalents.

    RemoteThreadHandlePointer - Supplies the address of a variable that will
        receive a handle to the remote thread in the target process.  This
        handle can be subsequently waited on, for example, if the caller wants
        to wait for the injection process to complete (including execution of
        their function).

    RemoteThreadIdPointer - Supplies the address of a variable that will receive
        the thread ID of the remote thread in the target process.

    RemoteBaseCodeAddress - Supplies the address of a variable that will receive
        the base address of the page of memory in the remote process where the
        injection thunk routine will be copied.

    RemoteUserDataAddress - Supplies the address of a variable that will receive
        the address of the user's data in the remote process's address space.

    LocalBaseCodeAddressPointer - Undocumented.

    LocalThunkBufferAddressPointer - Undocumented.

    LocalUserDataAddressPointer - Undocumented.

Return Value:

    TRUE on success, FALSE on error.

--*/
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
    LPTHREAD_START_ROUTINE StartRoutine;
    PCOPY_FUNCTION CopyFunction;
    STRING NullString;
    UNICODE_STRING NullUnicodeString;

    if (!Rtl->InitializeInjection(Rtl)) {
        return FALSE;
    }

    Thunk.Flags.AsULong = Flags.AsULong;

    ZeroStruct(NullString);
    ZeroStruct(NullUnicodeString);

    CopyInjectionFunctionsInline(&Thunk.Functions,
                                 &Rtl->InjectionFunctions);

    if (!ARGUMENT_PRESENT(TargetModuleDllPath)) {

        if (ARGUMENT_PRESENT(TargetFunctionName)) {
            __debugbreak();
            return FALSE;
        }

        InitializeStringFromString(&Thunk.FunctionName, &NullString);
        InitializeUnicodeStringFromUnicodeString(&Thunk.ModulePath,
                                                 &NullUnicodeString);

    } else {

        InitializeStringFromString(&Thunk.FunctionName, TargetFunctionName);
        InitializeUnicodeStringFromUnicodeString(&Thunk.ModulePath,
                                                 TargetModuleDllPath);
    }

    if (ARGUMENT_PRESENT(UserApc)) {
        CopyMemory(&Thunk.UserApc, UserApc, sizeof(Thunk.UserApc));
    }

    Thunk.InjectionObjects = InjectionObjects;

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

BOOL
AdjustThunkPointers(
    PRTL Rtl,
    HANDLE TargetProcessHandle,
    PBYTE OriginalThunkBuffer,
    USHORT SizeOfThunkBufferInBytes,
    PBYTE LocalThunkBuffer,
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
    USHORT Count;
    ULONG Index;
    ULONG NumberOfObjects;
    ULONG InjectionObjectArraySizeInBytes;
    ULONG TotalInjectionObjectsAllocSizeInBytes;
    PBYTE Base;
    PBYTE Dest;
    PBYTE NewUserData;
    PBYTE NewRemoteUserData;
    PBYTE UserPage;
    PBYTE RemotePage;
    PBYTE LocalThunkPage;
    PBYTE LocalInjectionObjectsPage;
    PBYTE RemoteThunkPage;
    PBYTE RemoteInjectionObjectsPage;
    PWCHAR LocalNameBuffer;
    PWCHAR RemoteNameBuffer;
    PUNICODE_STRING Name;
    PUNICODE_STRING OriginalName;
    PINJECTION_THUNK_CONTEXT Thunk;
    PINJECTION_THUNK_CONTEXT OriginalThunk;
    PINJECTION_OBJECT InjectionObject;
    PINJECTION_OBJECT LocalInjectionObject;
    PINJECTION_OBJECT RemoteInjectionObject;
    PINJECTION_OBJECT OriginalInjectionObject;
    PINJECTION_OBJECTS LocalInjectionObjects;
    PINJECTION_OBJECTS RemoteInjectionObjects;
    PINJECTION_OBJECTS OriginalInjectionObjects;

    if (SizeOfThunkBufferInBytes != sizeof(INJECTION_THUNK_CONTEXT)) {
        __debugbreak();
        return FALSE;
    }

    OriginalThunk = (PINJECTION_THUNK_CONTEXT)OriginalThunkBuffer;
    Thunk = (PINJECTION_THUNK_CONTEXT)LocalThunkBuffer;

    Base = Dest = (PBYTE)(
        RtlOffsetToPointer(
            LocalThunkBuffer,
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
    // Copy the downstream injection module path.
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

    UserDataOffset = (USHORT)(Dest - LocalThunkBuffer);

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

    //
    // Process injection objects if applicable.  If there were none, we're done,
    // return success.
    //

    OriginalInjectionObjects = OriginalThunk->InjectionObjects;
    if (!OriginalInjectionObjects) {
        return TRUE;
    }

    //
    // The writable injection objects structure will live one page after the
    // local thunk buffer address.  Resolve the relevant page address for both
    // local and remote thunks and injection objects structures.
    //

    LocalThunkPage = (PBYTE)ALIGN_DOWN_PAGE(LocalThunkBuffer);
    LocalInjectionObjectsPage = LocalThunkPage + PAGE_SIZE;
    LocalInjectionObjects = (PINJECTION_OBJECTS)LocalInjectionObjectsPage;

    RemoteThunkPage = (PBYTE)ALIGN_DOWN_PAGE(RemoteThunkBufferAddress);
    RemoteInjectionObjectsPage = RemoteThunkPage + PAGE_SIZE;
    RemoteInjectionObjects = (PINJECTION_OBJECTS)RemoteInjectionObjectsPage;

    //
    // The first injection object in the array will live after the injection
    // object structure in the remote memory layout.
    //

    LocalInjectionObject = (PINJECTION_OBJECT)(
        RtlOffsetToPointer(
            LocalInjectionObjects,
            sizeof(INJECTION_OBJECTS)
        )
    );

    RemoteInjectionObject = (PINJECTION_OBJECT)(
        RtlOffsetToPointer(
            RemoteInjectionObjects,
            sizeof(INJECTION_OBJECTS)
        )
    );

    //
    // Invariant check: struct sizes indicated on the incoming objects should
    // match sizeof() against the corresponding object.
    //

    if (OriginalInjectionObjects->SizeOfStruct !=
        (USHORT)sizeof(*LocalInjectionObjects)) {

        __debugbreak();
        return FALSE;
    }

    if (OriginalInjectionObjects->SizeOfInjectionObjectInBytes !=
        (USHORT)sizeof(*LocalInjectionObject)) {

        __debugbreak();
        return FALSE;
    }

    //
    // Copy the original injection objects structure and adjust the objects
    // pointer to the base of the remote injection object array.
    //

    CopyMemory((PBYTE)LocalInjectionObjects,
               (PBYTE)OriginalInjectionObjects,
               sizeof(*LocalInjectionObjects));

    LocalInjectionObjects->Objects = RemoteInjectionObject;

    //
    // Copy the array of original injection object structures into the local
    // injection object structure.
    //

    InjectionObjectArraySizeInBytes = (
        (ULONG)OriginalInjectionObjects->NumberOfObjects *
        (ULONG)sizeof(INJECTION_OBJECT)
    );

    CopyMemory((PBYTE)LocalInjectionObject,
               (PBYTE)OriginalInjectionObjects->Objects,
               InjectionObjectArraySizeInBytes);

    //
    // Initialize the base addresses of the local and remote wide character
    // buffers that will receive a copy of the injection object's Name.Buffer
    // contents.
    //

    LocalNameBuffer = (PWCHAR)(
        RtlOffsetToPointer(
            LocalInjectionObject,
            InjectionObjectArraySizeInBytes
        )
    );

    RemoteNameBuffer = (PWCHAR)(
        RtlOffsetToPointer(
            RemoteInjectionObject,
            InjectionObjectArraySizeInBytes
        )
    );


    //
    // Loop through all of the original injection object structures and carve
    // out UNICODE_STRING structure + buffer space for each object name.
    //

    NumberOfObjects = OriginalInjectionObjects->NumberOfObjects;

    TotalInjectionObjectsAllocSizeInBytes = (
        (ULONG)sizeof(INJECTION_OBJECTS) +
        InjectionObjectArraySizeInBytes
    );


    for (Index = 0; Index < NumberOfObjects; Index++) {

        //
        // Resolve the original injection object and pointer to its name field.
        //

        OriginalInjectionObject = OriginalInjectionObjects->Objects + Index;
        OriginalName = &OriginalInjectionObject->Name;

        //
        // Resolve the local injection object and a pointer to its name field.
        // Initialize the Length and MaximumLength fields to reflect the values
        // indicated by the original name.
        //

        InjectionObject = LocalInjectionObject + Index;
        Name = &InjectionObject->Name;
        Name->Length = OriginalName->Length;
        Name->MaximumLength = OriginalName->MaximumLength;

        //
        // Capture the number of characters for this name by shifting the name's
        // maximum length (size in bytes) right by 1.  Copy the original name's
        // buffer over, update the local name's buffer to the relevant remote
        // wide character buffer offset, then update both wide character buffer
        // pointers to account for the count (number of chars) we just copied.
        //

        Count = Name->MaximumLength >> 1;
        __movsw(LocalNameBuffer, OriginalName->Buffer, Count);
        Name->Buffer = RemoteNameBuffer;

        LocalNameBuffer += Count;
        RemoteNameBuffer += Count;

        TotalInjectionObjectsAllocSizeInBytes += Name->MaximumLength;
    }

    //
    // Finally, point the local thunk's injection objects pointer at the remote
    // address.
    //

    Thunk->InjectionObjects = RemoteInjectionObjects;

    return TRUE;
}

//
// The following routines are simple wrappers around their inline counterparts.
//

_Use_decl_annotations_
BOOL
IsJump(
    PBYTE Code
    )
{
    return IsJumpInline(Code);
}

_Use_decl_annotations_
PBYTE
SkipJumps(
    PBYTE Code
    )
{
    return SkipJumpsInline(Code);
}

_Use_decl_annotations_
BOOL
GetApproximateFunctionBoundaries(
    ULONG_PTR Address,
    PULONG_PTR StartAddress,
    PULONG_PTR EndAddress
    )
{
    return GetApproximateFunctionBoundariesInline(Address,
                                                  StartAddress,
                                                  EndAddress);
}

//
// End of inline function wrappers.
//

_Use_decl_annotations_
DECLSPEC_NOINLINE
ULONG_PTR
GetInstructionPointer(
    VOID
    )
/*++

Routine Description:

    Returns the return address of a routine.  This effectively provides an
    address of the instruction pointer within the routine, which is used to
    derive an approximate size of the function when passed to the routine
    GetApproximateFunctionBoundaries().

Arguments:

    None.

Return Value:

    The address of the instruction pointer prior to entering the current call.

--*/
{
    return (ULONG_PTR)_ReturnAddress();
}

//
// Functions for initializing and copying injection functions.
//

VOID
InitializeInjectionFunctions(
    _In_ PRTL Rtl,
    _Out_ PINJECTION_FUNCTIONS Functions
    )
{
    Functions->RtlAddFunctionTable = Rtl->RtlAddFunctionTable;
    Functions->LoadLibraryExW = Rtl->LoadLibraryExW;
    Functions->GetProcAddress = Rtl->GetProcAddress;
    Functions->GetLastError = Rtl->GetLastError;
    Functions->SetLastError = Rtl->SetLastError;
    Functions->SetEvent = Rtl->SetEvent;
    Functions->ResetEvent = Rtl->ResetEvent;
    Functions->GetThreadContext = Rtl->GetThreadContext;
    Functions->SetThreadContext = Rtl->SetThreadContext;
    Functions->SuspendThread = Rtl->SuspendThread;
    Functions->ResumeThread = Rtl->ResumeThread;
    Functions->OpenEventW = Rtl->OpenEventW;
    Functions->CloseHandle = Rtl->CloseHandle;
    Functions->SignalObjectAndWait = Rtl->SignalObjectAndWait;
    Functions->WaitForSingleObject = Rtl->WaitForSingleObject;
    Functions->WaitForSingleObjectEx = Rtl->WaitForSingleObjectEx;
    Functions->OutputDebugStringA = Rtl->OutputDebugStringA;
    Functions->OutputDebugStringW = Rtl->OutputDebugStringW;
    Functions->NtQueueApcThread = Rtl->NtQueueApcThread;
    Functions->NtTestAlert = Rtl->NtTestAlert;
    Functions->QueueUserAPC = Rtl->QueueUserAPC;
    Functions->SleepEx = Rtl->SleepEx;
    Functions->ExitThread = Rtl->ExitThread;
    Functions->GetExitCodeThread = Rtl->GetExitCodeThread;
    Functions->DeviceIoControl = Rtl->DeviceIoControl;
    Functions->GetModuleHandleW = Rtl->GetModuleHandleW;
    Functions->CreateFileW = Rtl->CreateFileW;
    Functions->CreateFileMappingW = Rtl->CreateFileMappingW;
    Functions->OpenFileMappingW = Rtl->OpenFileMappingW;
    Functions->MapViewOfFileEx = Rtl->MapViewOfFileEx;
    Functions->MapViewOfFileExNuma = Rtl->MapViewOfFileExNuma;
    Functions->FlushViewOfFile = Rtl->FlushViewOfFile;
    Functions->UnmapViewOfFileEx = Rtl->UnmapViewOfFileEx;
    Functions->VirtualAllocEx = Rtl->VirtualAllocEx;
    Functions->VirtualFreeEx = Rtl->VirtualFreeEx;
    Functions->VirtualProtectEx = Rtl->VirtualProtectEx;
    Functions->VirtualQueryEx = Rtl->VirtualQueryEx;
}

VOID
CopyInjectionFunctions(
    _Out_ PINJECTION_FUNCTIONS DestFunctions,
    _In_ PCINJECTION_FUNCTIONS SourceFunctions
    )
{
    CopyInjectionFunctionsInline(DestFunctions, SourceFunctions);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
