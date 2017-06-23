/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionEx.h

Abstract:

    WIP.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Define injection functions structure.
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
} INJECTION_THUNK_FLAGS;
C_ASSERT(sizeof(INJECTION_THUNK_FLAGS) == sizeof(ULONG));

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

typedef struct _INJECTION_OBJECT_EVENT {
    PUNICODE_STRING EventName;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
} INJECTION_OBJECT_EVENT;

typedef struct _INJECTION_OBJECT_FILE_MAPPING {
    PUNICODE_STRING MappingName;
    HANDLE MappingHandle;
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
} INJECTION_OBJECT_FILE_MAPPING;

typedef union _INJECTION_OBJECT_CONTEXT {
    struct {
        PUNICODE_STRING ObjectName;
        HANDLE ObjectHandle;
    };
    INJECTION_OBJECT_EVENT AsEvent;
    INJECTION_OBJECT_FILE_MAPPING AsFileMapping;
} INJECTION_OBJECT_CONTEXT;
typedef INJECTION_OBJECT_CONTEXT *PINJECTION_OBJECT_CONTEXT;

typedef struct _INJECTION_OBJECT {
    PUNICODE_STRING Name;
    PINJECTION_OBJECT_TYPE Type;
    PINJECTION_OBJECT_CONTEXT Context;
    PVOID Unused;
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

    _Field_range_(==, sizeof(struct _INJECTION_OBJECTS))
        USHORT StructSizeInBytes;

    //
    // Number of objects captured by this structure.  This value governs the
    // number of elements in the arrays for names, contexts etc.
    //

    USHORT NumberOfObjects;

    //
    // Size of an individual INJECTION_OBJECT structure.  This must be filled
    // in with the value of `sizeof(INJECTION_OBJECT)`.  The injection thunk
    // assembly routine compares this size with the size of its similarly
    // defined struct and ensures they match.
    //

    USHORT SizeOfInjectionObjectInBytes;

    //
    // Size of the INJECTION_OBJECT_CONTEXT structure.
    //

    USHORT SizeOfInjectionObjectContextInBytes;

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
    // Base addresses of arrays.
    //

    PINJECTION_OBJECT Objects;
    PUNICODE_STRING Names;
    PINJECTION_OBJECT_TYPE Types;
    PINJECTION_OBJECT_CONTEXT Contexts;
    PULONG Errors;
} INJECTION_OBJECTS;
typedef INJECTION_OBJECTS *PINJECTION_OBJECTS;

//
// Define the main INJECTION_THUNK_CONTEXT structure.
//

typedef struct _INJECTION_THUNK_EX_CONTEXT {
    INJECTION_THUNK_EX_FLAGS Flags;
    USHORT EntryCount;
    USHORT UserDataOffset;
    USHORT UserWritableDataOffset;
    USHORT Unused;

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

} INJECTION_THUNK_CONTEXT;
typedef INJECTION_THUNK_CONTEXT *PINJECTION_THUNK_CONTEXT;

FORCEINLINE
VOID
InitializeInjectionFunctions(
    _In_ PRTL Rtl,
    _In_ PINJECTION_FUNCTIONS Functions
    )
{
    CopyMemory(Functions,
               &Rtl->InjectionFunctions,
               sizeof(Rtl->InjectionFunctions));
}

//
// Define function pointer typedefs for main API functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK ADJUST_THUNK_POINTERS_EX)(
    _In_ PRTL Rtl,
    _In_ HANDLE TargetProcessHandle,
    _In_ PBYTE OriginalThunkBuffer,
    _In_ USHORT SizeOfThunkBufferInBytes,
    _Inout_bytecap_(BytesRemaining) PBYTE TemporaryLocalThunkBuffer,
    _In_ USHORT BytesRemaining,
    _In_ PBYTE RemoteThunkBufferAddress,
    _In_ PBYTE RemoteWritableDataAddress,
    _In_ PRUNTIME_FUNCTION RemoteRuntimeFunction,
    _In_ PVOID RemoteCodeBaseAddress,
    _In_ USHORT EntryCount,
    _Out_ PUSHORT NumberOfBytesWritten,
    _Out_ PBYTE *NewDestUserData,
    _Out_ PBYTE *NewRemoteDestUserData
    );
typedef ADJUST_THUNK_POINTERS_EX *PADJUST_THUNK_POINTERS_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK ADJUST_USER_DATA_POINTERS_EX)(
    _In_ PRTL Rtl,
    _In_ HANDLE TargetProcessHandle,
    _In_ PBYTE OriginalDataBuffer,
    _In_ USHORT SizeOfDataBufferInBytes,
    _In_ PBYTE TemporaryLocalDataBuffer,
    _In_ USHORT BytesRemaining,
    _In_ PBYTE RemoteDataBufferAddress
    );
typedef ADJUST_USER_DATA_POINTERS_EX *PADJUST_USER_DATA_POINTERS_EX;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK COPY_FUNCTION_EX)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ HANDLE TargetProcessHandle,
    _In_ PVOID SourceFunction,
    _In_opt_ PVOID SourceHandlerFunction,
    _In_reads_bytes_(SizeOfThunkBufferInBytes) PBYTE ThunkBuffer,
    _In_ USHORT SizeOfThunkBufferInBytes,
    _In_reads_bytes_(SizeOfUserDataInBytes) PBYTE UserData,
    _In_ USHORT SizeOfUserDataInBytes,
    _In_ USHORT DesiredNumberOfWritablePages,
    _In_ PADJUST_THUNK_POINTERS_EX AdjustThunkPointersEx,
    _In_ PADJUST_USER_DATA_POINTERS_EX AdjustUserDataPointersEx,
    _Out_ PPVOID DestBaseCodeAddressPointer,
    _Out_ PPVOID DestThunkBufferAddressPointer,
    _Out_ PPVOID DestUserDataAddressPointer,
    _Out_ PPVOID DestWritableUserDataAddressPointer,
    _Out_ PPVOID DestFunctionPointer,
    _Out_ PRUNTIME_FUNCTION *DestRuntimeFunctionPointer,
    _When_(SourceHandlerFunction != NULL, _Outptr_result_nullonfailure_)
    _When_(SourceHandlerFunction == NULL, _Out_opt_)
        PRUNTIME_FUNCTION *DestHandlerRuntimeFunctionPointer,
    _Out_ PULONG EntryCountPointer,
    _Out_ PULONG ActualNumberOfWritablePages
    );
typedef COPY_FUNCTION *PCOPY_FUNCTION;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INJECT_THUNK_EX)(
    _In_ struct _RTL *Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ INJECTION_THUNK_FLAGS Flags,
    _In_ HANDLE TargetProcessHandle,
    _In_opt_ PINJECTION_OBJECTS InjectionObjects,
    _In_opt_ PCUNICODE_STRING TargetModuleDllPath,
    _In_opt_ PCSTRING TargetFunctionName,
    _In_opt_ PAPC Apc,
    _In_ PBYTE UserData,
    _In_ USHORT SizeOfUserDataInBytes,
    _In_ USHORT DesiredNumberOfWritablePages,
    _Out_ PHANDLE RemoteThreadHandlePointer,
    _Out_ PULONG RemoteThreadIdPointer,
    _Out_ PPVOID RemoteBaseCodeAddress,
    _Out_ PPVOID RemoteUserDataAddress,
    _Out_ PPVOID RemoteWritableDataAddress,
    _Out_ PPVOID RemoteUserWritableDataAddress,
    _Out_ PULONG ActualNumberOfWritablePages
    );
typedef INJECT_THUNK_EX *PINJECT_THUNK_EX;
extern INJECT_THUNK_EX InjectThunkEx;




#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
