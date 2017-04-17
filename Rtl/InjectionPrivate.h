/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    InjectionPrivate.h

Abstract:

    This is the private header file for the injection functionality of the Rtl
    component.  It defines data structures, function typedefs and function
    declarations for all major (i.e. not local to the module) functions
    available for use by individual modules within this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// Define the character lengths of the Base64-encoded Unicode strings of random
// data used for event names.
//

#define RTL_INJECTION_CONTEXT_EVENT_NAME_MAXLENGTH 256
#define RTL_INJECTION_CONTEXT_EVENT_NAME_PREFIX L"Local\\"

//
// Define a bitfield struct and enum for representing event types.
//

typedef union _RTL_INJECTION_EVENT_TYPE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Indicates this is the Signal event.
        //

        ULONG Signal:1;

        //
        // Indicates this is the Wait event.
        //

        ULONG Wait:1;

        //
        // Indicates this is the caller's Signal event.
        //

        ULONG CallerSignal:1;

        //
        // Indicates this is the caller's Wait event.
        //

        ULONG CallerWait:1;

        //
        // Unused.
        //

        ULONG Unused:28;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_EVENT_TYPE;
typedef RTL_INJECTION_EVENT_TYPE *PRTL_INJECTION_EVENT_TYPE;
C_ASSERT(sizeof(RTL_INJECTION_EVENT_TYPE) == sizeof(ULONG));

//
// Keep this in sync with the RTL_INJECTION_EVENT_TYPE bitflags struct above.
//

typedef enum _Enum_is_bitflag_ _RTL_INJECTION_EVENT_ID {

    RtlInjectionSignalEventId           =       1,
    RtlInjectionWaitEventId             =  1 << 1,
    RtlInjectionCallerSignalEventId     =  1 << 2,
    RtlInjectionCallerWaitEventId       =  1 << 3,

    RtlInjectionInvalidEventId          = (1 << 3) + 1

} RTL_INJECTION_EVENT_ID;
typedef RTL_INJECTION_EVENT_ID *PRTL_INJECTION_EVENT_ID;

typedef
VOID
(CALLBACK RTL_INJECTION_CALLBACK)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context
    );
typedef RTL_INJECTION_CALLBACK *PRTL_INJECTION_CALLBACK;

typedef union _RTL_INJECTION_CONTEXT_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // The following flag is used as part of the injection context callback
        // protocol.
        //

        ULONG IsCodeSizeQuery:1;

        //
        // When set, indicates the function is running in the injected context.
        //

        ULONG IsInjected:1;

        //
        // When set, indicates the context is backed by stack-allocated memory.
        // (This will initially be set as RtlCreateInjectionPacket() uses a
        //  stack-allocated RTL_INJECTION_CONTEXT structure first before it
        //  commits to a final heap-allocated version.)
        //

        ULONG IsStackAllocated:1;

        //
        // Unused bits.
        //

        ULONG Unused:29;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_CONTEXT_FLAGS;
typedef RTL_INJECTION_CONTEXT_FLAGS *PRTL_INJECTION_CONTEXT_FLAGS;
C_ASSERT(sizeof(RTL_INJECTION_CONTEXT_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
BOOL
(CALLBACK RTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context,
    _Outptr_result_maybenull_ PVOID Token
    );
typedef RTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK
      *PRTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK;

typedef
LONG
(CALLBACK RTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context
    );
typedef RTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK
      *PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK;

typedef
LONG
(CALLBACK RTLP_INJECTION_REMOTE_THREAD_ENTRY)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context
    );
typedef RTLP_INJECTION_REMOTE_THREAD_ENTRY *PRTLP_INJECTION_REMOTE_THREAD_ENTRY;

//
// This structure captures the bare minimum information required by our
// injection thunk code.
//

typedef struct _RTL_INJECTION_THUNK_CONTEXT {
    union {
        struct {
            ULONG AddFunctionTable:1;
            ULONG TestExceptionHandler:1;
            ULONG TestAccessViolation:1;
            ULONG TestIllegalInstruction:1;
            ULONG Unused:28;
        };
        LONG AsLong;
        ULONG AsULong;
    } Flags;
    ULONG EntryCount;
    PRUNTIME_FUNCTION FunctionTable;
    PVOID BaseAddress;
    PRTL_ADD_FUNCTION_TABLE RtlAddFunctionTable;
    PLOAD_LIBRARY_W LoadLibraryW;
    PGET_PROC_ADDRESS GetProcAddress;
    PWSTR ModulePath;
    HANDLE ModuleHandle;
    PSTR FunctionName;
    union {
        PRTLP_INJECTION_REMOTE_THREAD_ENTRY ThreadEntry;
        PVOID FunctionAddress;
    };
} RTL_INJECTION_THUNK_CONTEXT;
typedef RTL_INJECTION_THUNK_CONTEXT *PRTL_INJECTION_THUNK_CONTEXT;

//
// An RTL_INJECTION_CONTEXT is created for each RTL_INJECTION_PACKET requested
// by a caller.  This structure encapsulates internal state needed in order to
// complete the injection request.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_CONTEXT {

    RTL_INJECTION_THUNK_CONTEXT Thunk;

    union {
        PRTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK IsInjectionContextProtocolCallback;
        PAPC_ROUTINE ContextProtocolThunk;
    };

    //
    // An APC routine we can queue to the remote thread for shutdown purposes.
    //

    PAPC_ROUTINE ExitThreadThunk;

    //
    // Fully-qualified path to the InjectionThunk.dll to load.
    //

    UNICODE_STRING InjectionThunkDllPath;

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_INJECTION_CONTEXT)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    RTL_INJECTION_CONTEXT_FLAGS Flags;

    //
    // Injection flags passed to RtlInject().
    //

    RTL_INJECTION_FLAGS InjectionFlags;

    //
    // Pad out to 8 bytes.
    //

    ULONG Padding;

    //
    // Fully-qualified path of Rtl.dll.
    //

    UNICODE_STRING RtlDllPath;

    //
    // A handle to the Rtl.dll module.  Only valid in the target process.
    //

    HMODULE RtlModuleHandle;

    //
    // An instance of the RTL structure initialized in the target process.
    //

    PRTL Rtl;

    //
    // The name of the event that will be signaled by the injection callback
    // prior to waiting on the wait event name.
    //

    UNICODE_STRING SignalEventName;

    //
    // The name of the event that will be waited on by the initial injection
    // callback prior to dispatching the caller's injection complete routine.
    //

    UNICODE_STRING WaitEventName;

    //
    // The caller also gets signal and wait events created for it.
    //

    UNICODE_STRING CallerSignalEventName;
    UNICODE_STRING CallerWaitEventName;

    //
    // Handles to the events.
    //

    HANDLE SignalEventHandle;
    HANDLE WaitEventHandle;
    HANDLE CallerSignalEventHandle;
    HANDLE CallerWaitEventHandle;

    //
    // Handles to the target process and thread.
    //

    HANDLE TargetProcessHandle;
    HANDLE TargetThreadHandle;

    //
    // If we're injecting a named function in a module, a handle to that module.
    //

    HMODULE ModuleHandle;

    //
    // The original pointer value provided by the caller as the Code argument,
    // or the initial result of GetProcAddress().
    //

    union {
        PBYTE OriginalCode;
        PRTL_INJECTION_COMPLETE_CALLBACK OriginalCallback;
    };

    //
    // The result of SkipJumps(OriginalCode).  May be identical to OriginalCode
    // above, may not be.
    //

    union {
        PAPC_ROUTINE CallerCode;
        PRTL_INJECTION_COMPLETE_CALLBACK Callback;
    };

    //
    // Approximate start and end addresses of the caller's code, as determined
    // by the GetApproximateFunctionBoundaries() routine.
    //

    ULONG_PTR CallerCodeStartAddress;
    ULONG_PTR CallerCodeEndAddress;

    //
    // The start address of our initial remote thread thunk, allocated in the
    // target process's memory space.
    //

    struct {
        union {
            LPTHREAD_START_ROUTINE StartRoutine;
            PAPC_ROUTINE InjectionRoutine;
        };
        PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
    } RemoteThread;

    //
    // Approximate start and end addresses of our remote thread's injection
    // thunk, as determined by the GetApproximateFunctionBoundaries() routine.
    //

    ULONG_PTR InjectionThunkStartAddress;
    ULONG_PTR InjectionThunkEndAddress;

    //
    // Approximate start and end addresses of our remote thread's exit thread
    // thunk, as determined by the GetApproximateFunctionBoundaries() routine.
    //

    ULONG_PTR ExitThreadThunkStartAddress;
    ULONG_PTR ExitThreadThunkEndAddress;

    //
    // Approximate start and end addresses of our remote thread's context
    // protocol thunk, as determined by the GetApproximateFunctionBoundaries()
    // routine.
    //

    ULONG_PTR ContextProtocolThunkStartAddress;
    ULONG_PTR ContextProtocolThunkEndAddress;

    //
    // Approximate start and end addresses of our remote thread's callback
    // protocol thunk for the caller.
    //

    ULONG_PTR CallbackProtocolThunkStartAddress;
    ULONG_PTR CallbackProtocolThunkEndAddress;

    //
    // Offsets from the base of the respective code allocation where each
    // routine starts.
    //

    USHORT ExitThreadThunkOffset;
    USHORT InjectionThunkOffset;
    USHORT ContextProtocolThunkOffset;
    USHORT CallbackProtocolThunkOffset;

    //
    // Caller's code resides in a separate page.
    //

    USHORT CallerCodeOffset;

    //
    // Pad out to 4 bytes.
    //

    USHORT Padding2;

    //
    // Approximate size of our thread start routine.
    //

    LONG SizeOfInjectionThunkInBytes;

    //
    // Approximate size of our exit thread thunk.
    //

    LONG SizeOfExitThreadThunkInBytes;

    //
    // Approximate size of our injection context protocol thunk.
    //

    LONG SizeOfContextProtocolThunkInBytes;

    //
    // Approximate size of our injection callback protocol thunk for the caller.
    //

    LONG SizeOfCallerProtocolThunkInBytes;

    //
    // An approximate size of the callback function code.
    //

    LONG SizeOfCallerCodeInBytes;

    //
    // Allocation size used for the context plus all supporting string buffers.
    // This size will be identical in both the original and remote process.
    //

    LONG_INTEGER TotalContextAllocSize;

    //
    // Total size of the data bytes that were allocated for the payload, if one
    // was requested.
    //

    LONG_INTEGER TotalPayloadAllocSize;

    //
    // Total size of the code bytes that were allocated in the remote process
    // for the caller's code.  This includes the callback function size plus
    // at least six leading 0xCC padding bytes, plus a dummy post-injection
    // callback protocol implementation (and necessary padding bytes).
    //

    LONG_INTEGER TotalCallerCodeAllocSize;

    //
    // Total size of the code (executable) memory allocation for our internal
    // thread start routine, protocol thunk, and exit thread thunk, plus any
    // requisite padding.
    //

    LONG_INTEGER TotalContextCodeAllocSize;

    //
    // Base address of the context allocation, the size of which is indicated by
    // the TotalContextAllocSize struct member above.
    //

    LPVOID BaseContextAddress;
    LPVOID RemoteBaseContextAddress;

    //
    // Base address of our context code.
    //

    LPVOID BaseContextCodeAddress;
    LPVOID RemoteBaseContextCodeAddress;

    //
    // Base address of the code allocation, the size of which is indicated by
    // the TotalCallerCodeAllocSize struct member above.
    //

    LPVOID BaseCallerCodeAddress;
    LPVOID RemoteBaseCallerCodeAddress;

    //
    // Base address of the caller's payload, if one was requested.
    //

    LPVOID BasePayloadAddress;
    LPVOID RemoteBasePayloadAddress;

    //
    // A pointer to the remote context prepared on the injection side prior to
    // being dispatched to the target process via WriteProcessMemory().
    //

    struct _RTL_INJECTION_CONTEXT *TemporaryRemoteContext;

    //
    // A pointer to the remote context code prepared on the injection side
    // prior to injection.
    //

    PBYTE TemporaryRemoteBaseContextCodeAddress;

    //
    // A pointer to the remote caller code prepared on the injection side
    // prior to injection.
    //

    PBYTE TemporaryRemoteBaseCallerCodeAddress;

    //
    // The number of pages consumed by each allocation.  (They should all be 1.)
    //

    USHORT NumberOfPagesForContext;
    USHORT NumberOfPagesForContextCode;
    USHORT NumberOfPagesForCallerCode;
    USHORT NumberOfPagesForPayload;

    //
    // Function pointers required by the initial injection callback.
    //

    RTL_INJECTION_FUNCTIONS Functions;

    //
    // For linking to AtExitEx().
    //

    struct _RTL_ATEXIT_ENTRY *AtExitEntry;

    //
    // The injection packet.
    //

    RTL_INJECTION_PACKET Packet;

} RTL_INJECTION_CONTEXT;
typedef RTL_INJECTION_CONTEXT *PRTL_INJECTION_CONTEXT;
typedef RTL_INJECTION_CONTEXT **PPRTL_INJECTION_CONTEXT;
typedef const RTL_INJECTION_CONTEXT *PCRTL_INJECTION_CONTEXT;
typedef const RTL_INJECTION_CONTEXT **PPCRTL_INJECTION_CONTEXT;

typedef
VOID
(CALLBACK RTLP_INITIALIZE_RTL_INJECTION_FUNCTIONS)(
    _In_ struct _RTL *Rtl,
    _Inout_ PRTL_INJECTION_FUNCTIONS Functions
    );
typedef RTLP_INITIALIZE_RTL_INJECTION_FUNCTIONS
      *PRTLP_INITIALIZE_RTL_INJECTION_FUNCTIONS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK RTLP_VERIFY_INJECTION_CALLBACK)(
    _In_ PRTL Rtl,
    _Inout_ PRTL_INJECTION_CONTEXT Context,
    _Out_ PRTL_INJECTION_ERROR Error
    );
typedef RTLP_VERIFY_INJECTION_CALLBACK *PRTLP_VERIFY_INJECTION_CALLBACK;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK RTLP_INJECT_CONTEXT)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _Inout_ PPRTL_INJECTION_CONTEXT DestContext,
    _In_ PCRTL_INJECTION_CONTEXT SourceContext,
    _Out_ PRTL_INJECTION_ERROR InjectionError
    );
typedef RTLP_INJECT_CONTEXT *PRTLP_INJECT_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK RTLP_INJECTION_CREATE_EVENT_NAME)(
    _In_ PRTL Rtl,
    _Inout_ PRTL_INJECTION_CONTEXT Context,
    _In_ RTL_INJECTION_EVENT_ID EventId,
    _Out_ PRTL_INJECTION_ERROR InjectionError
    );

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK RTLP_INJECTION_CREATE_EVENTS)(
    _In_ PRTL Rtl,
    _Inout_ PRTL_INJECTION_CONTEXT Context,
    _Out_ PRTL_INJECTION_ERROR Error
    );
typedef RTLP_INJECTION_CREATE_EVENTS *PRTLP_INJECTION_CREATE_EVENTS;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK RTLP_INJECTION_ALLOCATE_REMOTE_MEMORY)(
    _In_ PRTL Rtl,
    _Inout_ PRTL_INJECTION_CONTEXT Context,
    _Out_ PRTL_INJECTION_ERROR Error
    );
typedef RTLP_INJECTION_ALLOCATE_REMOTE_MEMORY
      *PRTLP_INJECTION_ALLOCATE_REMOTE_MEMORY;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK RTLP_INJECTION_CREATE_REMOTE_THREAD)(
    _In_ PRTL Rtl,
    _Inout_ PRTL_INJECTION_CONTEXT Context,
    _Out_ PRTL_INJECTION_ERROR Error
    );
typedef RTLP_INJECTION_CREATE_REMOTE_THREAD
      *PRTLP_INJECTION_CREATE_REMOTE_THREAD;

typedef
VOID
(CALLBACK RTLP_DESTROY_INJECTION_CONTEXT)(
    _Inout_ _Pre_notnull_ _Post_invalid_ PPRTL_INJECTION_CONTEXT ContextPointer
    );
typedef RTLP_DESTROY_INJECTION_CONTEXT *PRTLP_DESTROY_INJECTION_CONTEXT;

#pragma component(browser, off)
RTL_IS_INJECTION_PROTOCOL_CALLBACK RtlpPreInjectionProtocolCallbackImpl;
RTL_IS_INJECTION_PROTOCOL_CALLBACK RtlpPostInjectionProtocolCallbackImpl;
RTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK RtlpPreInjectionContextProtocolCallbackImpl;
RTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK RtlpPostInjectionContextProtocolCallbackImpl;
RTLP_INITIALIZE_RTL_INJECTION_FUNCTIONS RtlpInitializeRtlInjectionFunctions;
RTLP_INJECTION_REMOTE_THREAD_ENTRY RtlpInjectionRemoteThreadEntry;
RTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK RtlpInjectionRemoteThreadEntryThunk;
RTLP_VERIFY_INJECTION_CALLBACK RtlpInjectionCallbackVerifyMagicNumber;
RTLP_VERIFY_INJECTION_CALLBACK RtlpInjectionCallbackExtractCodeSize;
RTLP_VERIFY_INJECTION_CALLBACK RtlpVerifyInjectionContext;
RTLP_VERIFY_INJECTION_CALLBACK RtlpVerifyInjectionCallback;
RTLP_INJECTION_CREATE_EVENT_NAME RtlpInjectionCreateEventName;
RTLP_INJECTION_CREATE_EVENTS RtlpInjectionCreateEvents;
RTLP_INJECTION_ALLOCATE_REMOTE_MEMORY RtlpInjectionAllocateRemoteMemory;
RTLP_INJECTION_CREATE_REMOTE_THREAD RtlpInjectionCreateRemoteThread;
RTLP_INJECT_CONTEXT RtlpInjectContext;
RTLP_DESTROY_INJECTION_CONTEXT RtlpDestroyInjectionContext;
#pragma component(browser, on)


#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab nowrap                              :
