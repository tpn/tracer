/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Injection.h

Abstract:

    This is the header file for the Rtl component's injection module, which
    facilitates "injection" of objects, such as events and file mappings, into
    a remote process, in conjunction with remote thread creation.

    Callers wire up one or more INJECTION_OBJECT-derived structures and fill
    out a containing INJECTION_OBJECTS container structure.  This container,
    in additional to a DLL name and corresponding exported function name, are
    then passed to an Inject() function, which performs the injection.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Define the set of function pointers available for use during injection.
// The injection protocol relies on the fact that these function pointers will
// always be mapped into memory addresses accessible by every process at the
// same address.  That is, they represent functions residing in modules such
// as kernel32.dll and ntdll.dll, which will always be present in the address
// space of a Win32 process, and always mapped at the same addresses across all
// processes.
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
    PWAIT_FOR_SINGLE_OBJECT WaitForSingleObject;
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
    PGET_MODULE_HANDLE_W GetModuleHandleW;
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

//
// Define function type definitions for initializing an INJECTION_FUNCTIONS
// structure from an instance of RTL (typically only done once, at startup),
// and copying an initialized structure to a new one (which is done as part
// of preparation of an INJECTION_OBJECTS structure prior to calling Inject).
//

typedef
VOID
(INITIALIZE_INJECTION_FUNCTIONS)(
    _In_ PRTL Rtl,
    _Out_ PINJECTION_FUNCTIONS Functions
    );
typedef INITIALIZE_INJECTION_FUNCTIONS *PINITIALIZE_INJECTION_FUNCTIONS;
extern INITIALIZE_INJECTION_FUNCTIONS InitializeInjectionFunctions;

typedef
VOID
(COPY_INJECTION_FUNCTIONS)(
    _Out_ PINJECTION_FUNCTIONS DestFunctions,
    _In_ PCINJECTION_FUNCTIONS SourceFunctions
    );
typedef COPY_INJECTION_FUNCTIONS *PCOPY_INJECTION_FUNCTIONS;

FORCEINLINE
VOID
CopyInjectionFunctionsInline(
    _Out_ PINJECTION_FUNCTIONS DestFunctions,
    _In_ PCINJECTION_FUNCTIONS SourceFunctions
    )
{
    CopyMemory(DestFunctions,
               SourceFunctions,
               sizeof(*DestFunctions));
}

//
// Define the injection thunk structure and supporting flags.  This structure
// is used to control the injection process.  A local version of this structure
// will be prepared and then converted into a version suitable for injection
// into a remote process (that is, addresses will be converted such that they
// are valid in the remote process's address space).  When the remote thread
// is created, its starting address is set to our custom injection routine
// (written in assembly (see ../Asm/Injection.asm) and then copied into the
// remote process's address space and made executable), and the thunk is passed
// as the first parameter to that thread.  The injection routine then performs
// the relevant actions as directed by the thunk, such as opening handles to
// events and shared memory sections, then usually loading a DLL and resolving
// a function, which is called as the final step.
//

typedef union _INJECTION_THUNK_FLAGS {
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
// Define injection object id enum and type structure bitmap.
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

//
// Define the injection object header structure, which contains the set of
// fields common between all injection object types.
//

typedef struct _INJECTION_OBJECT_HEADER {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_FLAGS Flags;
    UNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
} INJECTION_OBJECT_HEADER;
typedef INJECTION_OBJECT_HEADER *PINJECTION_OBJECT_HEADER;
C_ASSERT(FIELD_OFFSET(INJECTION_OBJECT_HEADER, Name) == 8);
C_ASSERT(sizeof(INJECTION_OBJECT_HEADER) == 40);

//
// Define the event injection object event flags and structure.
//

typedef union _INJECTION_OBJECT_EVENT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG ManualReset:1;

        //
        // When set, the injection thunk will perform a wait on this event
        // after successfully obtaining a handle to it via OpenEventW().  This
        // can be useful during debugging.
        //

        ULONG WaitOnEventAfterOpening:1;

        //
        // When set, debug break after the wait above is satisfied.  This has
        // no effect unless WaitOnEventAfterOpening is also set.
        //

        ULONG DebugBreakAfterWaitSatisfied:1;

        //
        // When set, the injection thunk will call SetEvent() after opening the
        // event.  This is checked after (and is mutually exclusive with) the
        // WaitOnEventAfterOpening flag above.
        //

        ULONG SetEventAfterOpening:1;

        //
        // Unused bits.
        //

        ULONG Unused:28;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECT_EVENT_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECT_EVENT_FLAGS) == sizeof(ULONG));
typedef INJECTION_OBJECT_EVENT_FLAGS *PINJECTION_OBJECT_EVENT_FLAGS;

typedef struct _INJECTION_OBJECT_EVENT {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_EVENT_FLAGS Flags;
    UNICODE_STRING Name;
    HANDLE Handle;
    ULONG DesiredAccess;
    ULONG InheritHandle;
} INJECTION_OBJECT_EVENT;
C_ASSERT(sizeof(INJECTION_OBJECT_EVENT) == 40);
typedef INJECTION_OBJECT_EVENT *PINJECTION_OBJECT_EVENT;

//
// Define the file mapping injection object flags and structure.  File mappings
// can be backed by actual file system files or anonymous system memory.
//

typedef union _INJECTION_OBJECT_FILE_MAPPING_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} INJECTION_OBJECT_FILE_MAPPING_FLAGS;
C_ASSERT(sizeof(INJECTION_OBJECT_FILE_MAPPING_FLAGS) == sizeof(ULONG));
typedef INJECTION_OBJECT_FILE_MAPPING_FLAGS
       *PINJECTION_OBJECT_FILE_MAPPING_FLAGS;

typedef struct _INJECTION_OBJECT_FILE_MAPPING {
    INJECTION_OBJECT_TYPE Type;
    INJECTION_OBJECT_FILE_MAPPING_FLAGS Flags;
    UNICODE_STRING Name;
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
    UNICODE_STRING Path;
    HANDLE FileHandle;
    PVOID Padding;
} INJECTION_OBJECT_FILE_MAPPING;
C_ASSERT(sizeof(INJECTION_OBJECT_FILE_MAPPING) == 128);
typedef INJECTION_OBJECT_FILE_MAPPING *PINJECTION_OBJECT_FILE_MAPPING;

//
// Define the injection object "base" structure, which inlines the common
// injection object header structure, and then pads itself out to the maximum
// size used by all injection object subtypes.  This typically isn't interacted
// with in code; its main purpose is to easily assert structure sizes are as
// expected.
//

typedef struct _INJECTION_OBJECT_BASE {

    //
    // Inline INJECTION_OBJECT_HEADER.
    //

    struct {
        INJECTION_OBJECT_TYPE Type;
        INJECTION_OBJECT_FLAGS Flags;
        UNICODE_STRING Name;
        HANDLE Handle;
        ULONG DesiredAccess;
        ULONG InheritHandle;
    };

    //
    // Pad out to 128 bytes (40 bytes currently consumed by the header).
    //

    ULONGLONG Padding[11];
} INJECTION_OBJECT_BASE;
C_ASSERT(sizeof(INJECTION_OBJECT_BASE) == 128);
typedef INJECTION_OBJECT_BASE *PINJECTION_OBJECT_BASE;

//
// Define the primary injection object union that is worked with directly by
// code.  A union of all possible representations is used as it provides the
// most flexibility in code when working with objects.
//

typedef union _INJECTION_OBJECT {

    //
    // Inline INJECTION_OBJECT_HEADER.
    //

    struct {
        INJECTION_OBJECT_TYPE Type;
        INJECTION_OBJECT_FLAGS Flags;
        UNICODE_STRING Name;
        HANDLE Handle;
        ULONG DesiredAccess;
        ULONG InheritHandle;
    };

    //
    // N.B. The base structure is intended to control the total structure size.
    //      It must always have sufficient padding such that its size meets or
    //      exceeds the size of the largest injection object subtype.
    //

    INJECTION_OBJECT_BASE AsBase;

    //
    // Define injection object subtype representations.  These enable easy
    // casting based on the injection object's type.
    //

    INJECTION_OBJECT_EVENT AsEvent;
    INJECTION_OBJECT_FILE_MAPPING AsFileMapping;
} INJECTION_OBJECT;
C_ASSERT(sizeof(INJECTION_OBJECT) == sizeof(INJECTION_OBJECT_BASE));
typedef INJECTION_OBJECT *PINJECTION_OBJECT;

//
// Define the injection objects composite structure (and corresponding flags).
// This is used to encapsulate a set of injection objects, plus some additional
// utility fields that can be written to by the injected thread (such as the
// DLL handle of the requested module and corresponding function pointer of the
// requested function, supplied by LoadLibraryW() and GetProcAddress(),
// respectively).
//
// The underlying memory backing this structure and all individual objects is
// the only memory that is given read/write permission in the target process.
// The code and user data pages are set to read/execute and readonly,
// respectively.  (We need the injection object memory to be writable as the
// initial injection thunk needs to save handle information once the relevant
// subtype has been initialized.)
//

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
    // number of elements in the array of pointers to injection objects whose
    // first element is supplied by the Objects field below.
    //

    ULONG NumberOfObjects;

    //
    // Total number of bytes allocated in support of this structure.  This is
    // filled out automatically as a convenience -- it is not currently used for
    // anything and should not be relied upon.
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

    //
    // (24 bytes consumed.)
    //

    //
    // If a module and function name was requested as part of injection, the
    // module handle resolved by LoadLibraryW() and the resulting proc address
    // resolved by GetProcAddress() will be stored here.
    //

    HANDLE ModuleHandle;
    PROC FunctionPointer;

    //
    // Pad out to 128 bytes such that our size matches that of the injection
    // object structure.  This eases size calculations and boundary conditions
    // given that we keep everything as powers of two.
    //

    ULONGLONG Padding[11];

} INJECTION_OBJECTS;
C_ASSERT(sizeof(INJECTION_OBJECTS) == sizeof(INJECTION_OBJECT));
typedef INJECTION_OBJECTS *PINJECTION_OBJECTS;

//
// Define the injection thunk context structure.  This structure is passed as
// the first argument to our injected thread, and is the primary means for
// directing the behavior of the injected thread's thunk routine, as described
// earlier.
//

typedef struct _INJECTION_THUNK_CONTEXT {

    //
    // Define flags related to the thunk.
    //

    INJECTION_THUNK_FLAGS Flags;

    //
    // Supplies the number of RUNTIME_FUNCTION entries captured by the pointer
    // FunctionTable below.  This is passed directly to RtlAddFunctionTable().
    //

    USHORT EntryCount;

    //
    // Supplies the offset from the readonly data section of injected memory
    // where the user's custom structure starts.  The final address is provided
    // as the first parameter to the user's DLL+function, if such functionality
    // has been requested.
    //

    USHORT UserDataOffset;

    //
    // Supplies a pointer to the runtime function table entry registered for
    // our injection routine.  This is automatically wired up for us as part
    // copying our injection routine into a separate memory address space
    // (via CopyFunction()), and is passed to RtlAddFunctionTable().
    //

    PRUNTIME_FUNCTION FunctionTable;

    //
    // Supplies the base address of the injection routine's code.  This value
    // is passed to RtlAddFunctionTable().
    //

    PVOID BaseCodeAddress;

    //
    // Supplies a pointer to an optional injection objects container structure,
    // which, in conjunction with Thunk.Flags.HasInjectionObjects being set to
    // TRUE, will be initialized by the injection routine.
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

    //
    // We embed the injection functions structure directly into our thunk to
    // facilitate easy access by our injection routine.
    //

    INJECTION_FUNCTIONS Functions;

} INJECTION_THUNK_CONTEXT;
typedef INJECTION_THUNK_CONTEXT *PINJECTION_THUNK_CONTEXT;

//
// Define the function typedef for the main injection function.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INJECT)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ INJECTION_THUNK_FLAGS Flags,
    _In_ HANDLE TargetProcessHandle,
    _In_opt_ PCUNICODE_STRING TargetModuleDllPath,
    _In_opt_ PCSTRING TargetFunctionName,
    _In_opt_ PAPC UserApc,
    _In_opt_ PBYTE UserData,
    _In_opt_ USHORT SizeOfUserDataInBytes,
    _In_opt_ PINJECTION_OBJECTS InjectionObjects,
    _In_opt_ PADJUST_USER_DATA_POINTERS AdjustUserDataPointers,
    _Out_ PHANDLE RemoteThreadHandlePointer,
    _Out_ PULONG RemoteThreadIdPointer,
    _Out_ PPVOID RemoteBaseCodeAddress,
    _Out_ PPVOID RemoteUserDataAddress,
    _Out_opt_ PPVOID LocalBaseCodeAddressPointer,
    _Out_opt_ PPVOID LocalThunkBufferAddressPointer,
    _Out_opt_ PPVOID LocalUserDataAddressPointer
    );
typedef INJECT *PINJECT;
extern INJECT Inject;

//
// Define the function prototype for the injection complete function, which is
// the function that is resolved dynamically in the remote process based on the
// DLL module path and function name provided to Inject().
//
// That is, if providing a DLL and function name, the target function must
// conform to this function signature.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INJECTION_COMPLETE)(
    _In_ PCBYTE UserData,
    _In_opt_ PINJECTION_OBJECTS InjectionObjects,
    _In_ PINJECTION_FUNCTIONS InjectionFunctions
    );
typedef INJECTION_COMPLETE *PINJECTION_COMPLETE;

//
// Include inline functions.
//

#include "InjectionInline.h"

#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
