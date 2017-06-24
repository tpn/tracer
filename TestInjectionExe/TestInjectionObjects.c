/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    main.c

Abstract:

    This module is the main entry point for the thunk executable.
    It implements mainCRTStartup().

--*/

#include "stdafx.h"

//
// Begin injection glue.
//

typedef struct _INJECTION_FUNCTIONS {
    PRTL_ADD_FUNCTION_TABLE RtlAddFunctionTable;
    PLOAD_LIBRARY_EX_W LoadLibraryExW;
    PGET_PROC_ADDRESS GetProcAddress;
    PGET_LAST_ERROR GetLastError;
    PSET_LAST_ERROR SetLastError;
    PSET_EVENT SetEvent;
    PRESET_EVENT ResetEvent;
    PGET_THREAD_CONTEXT GetThreadContext;
    PSET_THREAD_CONTEXT SetThreadContext;
    PSUSPEND_THREAD SuspendThread;
    PRESUME_THREAD ResumeThread;
    POPEN_EVENT_W OpenEventW;
    PCLOSE_HANDLE CloseHandle;
    PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;
    PWAIT_FOR_SINGLE_OBJECT_EX WaitForSingleObjectEx;
    POUTPUT_DEBUG_STRING_A OutputDebugStringA;
    POUTPUT_DEBUG_STRING_W OutputDebugStringW;
    PNT_QUEUE_APC_THREAD NtQueueApcThread;
    PNT_TEST_ALERT NtTestAlert;
    PQUEUE_USER_APC QueueUserAPC;
    PSLEEP_EX SleepEx;
    PEXIT_THREAD ExitThread;
    PGET_EXIT_CODE_THREAD GetExitCodeThread;
    PDEVICE_IO_CONTROL DeviceIoControl;
    PCREATE_FILE_W CreateFileW;
    PCREATE_FILE_MAPPING_W CreateFileMappingW;
    POPEN_FILE_MAPPING_W OpenFileMappingW;
    PMAP_VIEW_OF_FILE_EX MapViewOfFileEx;
    PMAP_VIEW_OF_FILE_EX_NUMA MapViewOfFileExNuma;
    PFLUSH_VIEW_OF_FILE FlushViewOfFile;
    PUNMAP_VIEW_OF_FILE_EX UnmapViewOfFileEx;
    PVIRTUAL_ALLOC_EX VirtualAllocEx;
    PVIRTUAL_FREE_EX VirtualFreeEx;
    PVIRTUAL_PROTECT_EX VirtualProtectEx;
    PVIRTUAL_QUERY_EX VirtualQueryEx;
} INJECTION_FUNCTIONS;
typedef INJECTION_FUNCTIONS *PINJECTION_FUNCTIONS;
typedef const INJECTION_FUNCTIONS *PCINJECTION_FUNCTIONS;

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

FORCEINLINE
VOID
CopyInjectionFunctions(
    _Out_ PINJECTION_FUNCTIONS DestFunctions,
    _In_ PCINJECTION_FUNCTIONS SourceFunctions
    )
{
    CopyMemory(DestFunctions,
               SourceFunctions,
               sizeof(*DestFunctions));
}

typedef union _INJECTION_THUNK_EX_FLAGS {
    struct {
        ULONG DebugBreakOnEntry:1;
        ULONG HasInjectionObjects:1;
        ULONG HasModuleAndFunction:1;
        ULONG HasApc:1;
        ULONG Unused:28;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_THUNK_EX_FLAGS;
C_ASSERT(sizeof(INJECTION_THUNK_EX_FLAGS) == sizeof(ULONG));

//
// Define injection objects structure.
//

typedef enum _Enum_is_bitflag_ _INJECTION_OBJECT_ID {
    NullInjectionObjectId               =        0,
    EventInjectionObjectId              =        1,
    FileMappingInjectionObjectId        =  (1 << 1),

    //
    // Make sure the expression within parenthesis below is identical to the
    // last enumeration above.
    //

    InvalidInjectionObjectId            =  (1 << 1) + 1
} INJECTION_OBJECT_ID;
typedef INJECTION_OBJECT_ID *PINJECTION_OBJECT_ID;

typedef union _INJECTION_OBJECT_TYPE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Event:1;
        ULONG FileMapping:1;
        ULONG Unused:30;
    };
    LONG AsLong;
    ULONG AsULong;
    INJECTION_OBJECT_ID AsId;
} INJECTION_OBJECT_TYPE;
C_ASSERT(sizeof(INJECTION_OBJECT_TYPE) == sizeof(ULONG));
typedef INJECTION_OBJECT_TYPE *PINJECTION_OBJECT_TYPE;

typedef union _INJECTION_OBJECT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECT_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECT_FLAGS) == sizeof(ULONG));
typedef INJECTION_OBJECT_FLAGS *PINJECTION_OBJECT_FLAGS;

typedef struct _INJECTION_OBJECT_HEADER {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_FLAGS Flags;
    PUNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
} INJECTION_OBJECT_HEADER;

typedef union _INJECTION_OBJECT_EVENT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG ManualReset:1;
        ULONG Unused:31;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECT_EVENT_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECT_EVENT_FLAGS) == sizeof(ULONG));
typedef INJECTION_OBJECT_EVENT_FLAGS *PINJECTION_OBJECT_EVENT_FLAGS;

typedef struct _INJECTION_OBJECT_EVENT {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_EVENT_FLAGS Flags;
    PUNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
} INJECTION_OBJECT_EVENT;
typedef INJECTION_OBJECT_EVENT *PINJECTION_OBJECT_EVENT;

typedef struct _INJECTION_OBJECT_FILE_MAPPING {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_FLAGS Flags;
    PUNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
    ULONG ShareMode;
    ULONG CreationDisposition;
    ULONG FlagsAndAttributes;
    ULONG AllocationType;
    ULONG PageProtection;
    ULONG PreferredNumaNode;
    LARGE_INTEGER FileOffset;
    LARGE_INTEGER MappingSize;
    PVOID PreferredBaseAddress;
    PVOID BaseAddress;
    PUNICODE_STRING Path;
    HANDLE FileHandle;
} INJECTION_OBJECT_FILE_MAPPING;
typedef INJECTION_OBJECT_FILE_MAPPING *PINJECTION_OBJECT_FILE_MAPPING;

typedef union _INJECTION_OBJECT {

    //
    // All subtypes currently share the name, handle, desired access and inherit
    // handle values.  Provide direct access to them here.
    //

    struct {
        INJECTION_OBJECT_TYPE Type;
        INJECTION_OBJECT_FLAGS Flags;
        PUNICODE_STRING Name;
        HANDLE Handle;
        ULONG DesiredAccess;
        ULONG InheritHandle;
    };
    INJECTION_OBJECT_EVENT AsEvent;
    INJECTION_OBJECT_FILE_MAPPING AsFileMapping;
} INJECTION_OBJECT;
typedef INJECTION_OBJECT *PINJECTION_OBJECT;

typedef union _INJECTION_OBJECTS_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECTS_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECTS_FLAGS) == sizeof(ULONG));

typedef struct _INJECTION_OBJECTS {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _INJECTION_OBJECTS)) USHORT SizeOfStruct;

    //
    // Size of an individual INJECTION_OBJECT structure.  This must be filled
    // in with the value of `sizeof(INJECTION_OBJECT)`.  The injection thunk
    // assembly routine compares this size with the size of its similarly
    // defined struct and ensures they match.
    //

    USHORT SizeOfInjectionObjectInBytes;

    //
    // Number of objects captured by this structure.  This value governs the
    // number of elements in the arrays for names, contexts etc.
    //

    ULONG NumberOfObjects;

    //
    // Total number of bytes allocated in support of this structure.  This will
    // include all UNICODE_STRING structures and backing wide character buffers,
    // and all handle and error code arrays, plus the size of this structure.
    //

    ULONG TotalAllocSizeInBytes;

    //
    // Flags related to this structure.
    //

    INJECTION_OBJECTS_FLAGS Flags;

    //
    // Base address of first element in objects array.
    //

    PINJECTION_OBJECT Objects;
} INJECTION_OBJECTS;
typedef INJECTION_OBJECTS *PINJECTION_OBJECTS;

typedef struct _INJECTION_THUNK_CONTEXT_EX {
    INJECTION_THUNK_EX_FLAGS Flags;
    USHORT EntryCount;
    USHORT UserDataOffset;
    PRUNTIME_FUNCTION FunctionTable;
    PVOID BaseCodeAddress;

    //
    // Functions.
    //

    INJECTION_FUNCTIONS Functions;

    //
    // Optional injection objects.
    //

    PINJECTION_OBJECTS InjectionObjects;

    //
    // Target module to load and function to resolve and call once injection
    // objects (if any) have been loaded.
    //

    UNICODE_STRING ModulePath;
    STRING FunctionName;

    //
    // User-provided APC-like routine.
    //

    APC UserApc;

} INJECTION_THUNK_CONTEXT_EX;
typedef INJECTION_THUNK_CONTEXT_EX *PINJECTION_THUNK_CONTEXT_EX;

//
// Start of InjectThunkEx.c.
//

BOOL
AdjustThunkPointersEx(
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
    PBYTE Base;
    PBYTE Dest;
    PBYTE NewUserData;
    PBYTE NewRemoteUserData;
    PBYTE UserPage;
    PBYTE RemotePage;
    PINJECTION_THUNK_CONTEXT_EX Thunk;
    PINJECTION_THUNK_CONTEXT_EX OriginalThunk;
    PINJECTION_OBJECTS InjectionObjects;
    PINJECTION_OBJECTS OriginalInjectionObjects;

    if (SizeOfThunkBufferInBytes != sizeof(INJECTION_THUNK_CONTEXT_EX)) {
        __debugbreak();
        return FALSE;
    }

    OriginalThunk = (PINJECTION_THUNK_CONTEXT_EX)OriginalThunkBuffer;
    Thunk = (PINJECTION_THUNK_CONTEXT_EX)LocalThunkBuffer;

    OriginalInjectionObjects = OriginalThunk->InjectionObjects;

    //
    // xxx todo: need to point the new injection objects at the writable mem.
    //

    InjectionObjects = (PINJECTION_OBJECTS)NULL;

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
    // xxx todo: process injection objects
    //


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

    return TRUE;
}

BOOL
InjectThunkEx(
    PRTL Rtl,
    PALLOCATOR Allocator,
    INJECTION_THUNK_EX_FLAGS Flags,
    HANDLE TargetProcessHandle,
    PCUNICODE_STRING TargetModuleDllPath,
    PCSTRING TargetFunctionName,
    PAPC Apc,
    PBYTE UserData,
    USHORT SizeOfUserDataInBytes,
    PINJECTION_OBJECTS InjectionObjects,
    //PADJUST_USER_DATA_POINTERS_EX AdjustUserDataPointersEx,
    PHANDLE RemoteThreadHandlePointer,
    PULONG RemoteThreadIdPointer,
    PPVOID RemoteBaseCodeAddress,
    PPVOID RemoteUserDataAddress//,
    //PPINJECTION_THUNK_CONTEXT_EX ThunkPointer,
    //PPINJECTION_OBJECTS InjectionObjectsPointer
    )
{
    BOOL Success;
    ULONG EntryCount;
    ULONG CreationFlags;
    ULONG RemoteThreadId;
    //ULONG ThunkSize;
    HANDLE RemoteThreadHandle;
    PRUNTIME_FUNCTION DestRuntimeFunction;
    INJECTION_THUNK_CONTEXT_EX Thunk;
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

    //InitializeInjectionFunctions(&Thunk.Functions, Rtl);


    InitializeStringFromString(&Thunk.FunctionName, TargetFunctionName);
    InitializeUnicodeStringFromUnicodeString(&Thunk.ModulePath,
                                             TargetModuleDllPath);

    CopyFunction = Rtl->CopyFunction;

    Success = CopyFunction(Rtl,
                           Allocator,
                           TargetProcessHandle,
                           Rtl->InjectionThunkExRoutine,
                           NULL,
                           (PBYTE)&Thunk,
                           sizeof(Thunk),
                           (PBYTE)UserData,
                           SizeOfUserDataInBytes,
                           AdjustThunkPointersEx,
                           NULL, //AdjustUserDataPointers,
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

    /*
    if (ARGUMENT_PRESENT(LocalBaseCodeAddressPointer)) {
        *LocalBaseCodeAddressPointer = LocalBaseCodeAddress;

        if (ARGUMENT_PRESENT(LocalThunkBufferAddressPointer)) {
            *LocalThunkBufferAddressPointer = LocalThunkBufferAddress;
        }

        if (ARGUMENT_PRESENT(LocalUserDataAddressPointer)) {
            *LocalUserDataAddressPointer = LocalUserDataAddress;
        }
    }
    */

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
    //ULONG EntryCount;
    ULONG CreationFlags;
    ULONG RemoteThreadId;
    HANDLE RemoteThreadHandle;
    //PRUNTIME_FUNCTION DestRuntimeFunction;
    INJECTION_THUNK_CONTEXT_EX Thunk;
    PVOID DestInjectionThunk = NULL;
    PVOID DestBaseCodeAddress = NULL;
    PVOID DestUserDataAddress = NULL;
    PVOID DestThunkBufferAddress = NULL;
    LPTHREAD_START_ROUTINE StartRoutine;
    PCOPY_FUNCTION CopyFunction;

    if (!Rtl->InitializeInjection(Rtl)) {
        return FALSE;
    }

    Thunk.Flags.AsULong = Flags.AsULong;

    InitializeStringFromString(&Thunk.FunctionName, TargetFunctionName);
    InitializeUnicodeStringFromUnicodeString(&Thunk.ModulePath,
                                             TargetModuleDllPath);

    CopyFunction = Rtl->CopyFunction;

    /*
    Success = CopyFunction(Rtl,
                           Allocator,
                           TargetProcessHandle,
                           Rtl->InjectionThunkRoutine,
                           NULL,
                           (PBYTE)&Thunk,
                           sizeof(Thunk),
                           (PBYTE)UserData,
                           SizeOfUserDataInBytes,
                           NULL, //AdjustThunkPointers,
                           AdjustUserDataPointers,
                           &DestBaseCodeAddress,
                           &DestThunkBufferAddress,
                           &DestUserDataAddress,
                           &DestInjectionThunk,
                           &DestRuntimeFunction,
                           NULL,
                           &EntryCount);
    */

    Success = FALSE;

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

//
// End injection glue.
//

BOOL
TestInjectionObjects(
    PRTL Rtl,
    PALLOCATOR Allocator,
    PTRACER_CONFIG TracerConfig,
    PDEBUG_ENGINE_SESSION Session
    )
{
    LONG ExitCode = 1;

    BOOL Success;
    ULONG SizeOfBuffer;
    ULONG LastError;
    USHORT Index;
    USHORT MappingIndex;
    USHORT NumberOfObjects;
    USHORT NumberOfEvents;
    USHORT NumberOfFileMappings;
    HANDLE Handle;
    PWSTR WideBuffer;
    PVOID BaseAddress;
    PUNICODE_STRING Name;
    STARTUPINFOW StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    UNICODE_STRING LogFile = RTL_CONSTANT_STRING(L"\\\\?\\S:\\ScratchLog.txt");
    INJECTION_OBJECT Objects[7];
    UNICODE_STRING ObjectNames[7];
    UNICODE_STRING ObjectPrefixes[] = {
        RTL_CONSTANT_STRING(L"Event1_"),
        RTL_CONSTANT_STRING(L"Event2_"),
        RTL_CONSTANT_STRING(L"Event3_"),
        RTL_CONSTANT_STRING(L"Event4_"),
        RTL_CONSTANT_STRING(L"SharedMem1_"),
        RTL_CONSTANT_STRING(L"SharedMem2_"),
        RTL_CONSTANT_STRING(L"Log_"),
    };
    PUNICODE_STRING Names[] = {
        &ObjectNames[0],
        &ObjectNames[1],
        &ObjectNames[2],
        &ObjectNames[3],
        &ObjectNames[4],
        &ObjectNames[5],
        &ObjectNames[6],
    };
    PUNICODE_STRING Prefixes[] ={
        &ObjectPrefixes[0],
        &ObjectPrefixes[1],
        &ObjectPrefixes[2],
        &ObjectPrefixes[3],
        &ObjectPrefixes[4],
        &ObjectPrefixes[5],
        &ObjectPrefixes[6],
    };
    ULONG FileMappingDesiredAccess[] = {
        FILE_MAP_READ,
        FILE_MAP_WRITE,
        FILE_MAP_WRITE,
    };
    ULONG FileMappingPageProtection[] = {
        PAGE_READONLY,
        PAGE_READWRITE,
        PAGE_READWRITE,
    };
    LARGE_INTEGER MappingSizes[] = {
        {
            1 << 16,    // 64KB
            0,
        },
        {
            1 << 24,    // 24MB
            0,
        },
        {
            1 << 16,    // 64KB
            0,
        },
    };
    LARGE_INTEGER MappingSize;
    INJECTION_OBJECTS InjectionObjects;
    INJECTION_FUNCTIONS InjectionFunctions;
    PINJECTION_OBJECT Object;
    PINJECTION_OBJECT_EVENT Event;
    PINJECTION_OBJECT_FILE_MAPPING FileMapping;

    ZeroStruct(Objects);
    ZeroStruct(InjectionObjects);
    ZeroStruct(InjectionFunctions);

    NumberOfObjects = ARRAYSIZE(ObjectNames);
    NumberOfEvents = 4;
    NumberOfFileMappings = 3;

    if (!Rtl->InitializeInjection(Rtl)) {
        goto Error;
    }

    InitializeInjectionFunctions(Rtl, &InjectionFunctions);

    if (!CreateThunkExe(&StartupInfo, &ProcessInfo)) {
        __debugbreak();
        LastError = GetLastError();
        goto Error;
    }

    Success = Rtl->CreateRandomObjectNames(Rtl,
                                           Allocator,
                                           Allocator,
                                           ARRAYSIZE(ObjectNames),
                                           64,
                                           NULL,
                                           (PPUNICODE_STRING)&Names,
                                           (PPUNICODE_STRING)&Prefixes,
                                           &SizeOfBuffer,
                                           &WideBuffer);

    if (!Success) {
        __debugbreak();
    }

    for (Index = 0; Index < NumberOfEvents; Index++) {
        Name = Names[Index];
        Object = &Objects[Index];
        Event = &Object->AsEvent;
        Event->Name = Name;
        Event->Type.AsId = EventInjectionObjectId;
        Event->Flags.ManualReset = FALSE;
        Event->DesiredAccess = EVENT_MODIFY_STATE;

        Event->Handle = Rtl->CreateEventW(NULL,
                                          Event->Flags.ManualReset,
                                          FALSE,
                                          Name->Buffer);

        LastError = GetLastError();

        if (!Event->Handle || LastError == ERROR_ALREADY_EXISTS) {
            __debugbreak();
            goto Error;
        }
    }

    Object = &Objects[ARRAYSIZE(Objects)-1];
    FileMapping = &Object->AsFileMapping;
    FileMapping->Path = &LogFile;

    for (Index = NumberOfEvents, MappingIndex = 0;
         Index < NumberOfObjects;
         Index++, MappingIndex++) {

        Name = Names[Index];
        Object = &Objects[Index];
        FileMapping = &Object->AsFileMapping;
        FileMapping->Name = Name;
        FileMapping->Type.AsId = FileMappingInjectionObjectId;
        FileMapping->DesiredAccess = FileMappingDesiredAccess[MappingIndex];
        FileMapping->PageProtection = FileMappingPageProtection[MappingIndex];
        MappingSize.QuadPart = MappingSizes[MappingIndex].QuadPart;
        FileMapping->MappingSize.QuadPart = MappingSize.QuadPart;

        if (FileMapping->Path) {

            Handle = Rtl->CreateFileW(FileMapping->Path->Buffer,
                                      GENERIC_READ | GENERIC_WRITE,
                                      FILE_SHARE_DELETE,
                                      NULL,
                                      TRUNCATE_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

            if (!Handle || Handle == INVALID_HANDLE_VALUE) {
                __debugbreak();
                goto Error;
            }

            FileMapping->FileHandle = Handle;
            LastError = GetLastError();

            if (FALSE && LastError == ERROR_ALREADY_EXISTS) {
                FILE_STANDARD_INFO FileInfo;
                FILE_INFO_BY_HANDLE_CLASS Class;

                Class = (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo;

                Success = GetFileInformationByHandleEx(Handle,
                                                       Class,
                                                       &FileInfo,
                                                       sizeof(FileInfo));
                if (!Success) {
                    __debugbreak();
                    goto Error;
                }

                FileMapping->FileOffset.QuadPart = FileInfo.EndOfFile.QuadPart;
            }
        }

        Handle = Rtl->CreateFileMappingW(FileMapping->FileHandle,
                                         NULL,
                                         PAGE_READWRITE,
                                         FileMapping->MappingSize.HighPart,
                                         FileMapping->MappingSize.LowPart,
                                         FileMapping->Name->Buffer);

        LastError = GetLastError();

        if (!Handle || LastError == ERROR_ALREADY_EXISTS) {
            __debugbreak();
            goto Error;
        }

        FileMapping->Handle = Handle;

        BaseAddress = Rtl->MapViewOfFileEx(Handle,
                                           FILE_MAP_ALL_ACCESS,
                                           FileMapping->FileOffset.HighPart,
                                           FileMapping->FileOffset.LowPart,
                                           FileMapping->MappingSize.QuadPart,
                                           FileMapping->BaseAddress);

        if (!BaseAddress) {
            LastError = GetLastError();
            __debugbreak();
            goto Error;
        }

        FileMapping->PreferredBaseAddress = BaseAddress;
    }

    //
    // Initialize the INJECTION_OBJECTS container.
    //

    InjectionObjects.SizeOfStruct = sizeof(InjectionObjects);
    InjectionObjects.SizeOfInjectionObjectInBytes = sizeof(INJECTION_OBJECT);
    InjectionObjects.NumberOfObjects = NumberOfObjects;
    InjectionObjects.TotalAllocSizeInBytes = 0;
    InjectionObjects.Objects = Objects;

    Success = TRUE;

    goto End;

Error:

    Success = FALSE;

End:

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
