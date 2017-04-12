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
ULONGLONG
(CALLBACK RTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context
    );
typedef RTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK
      *PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK;

typedef
_Success_(return != 0)
BOOL
(CALLBACK RTLP_INJECTION_REMOTE_THREAD_ENTRY)(
    _In_ struct _RTL_INJECTION_CONTEXT *Context,
    _Inout_ PULONGLONG ResultPointer
    );
typedef RTLP_INJECTION_REMOTE_THREAD_ENTRY *PRTLP_INJECTION_REMOTE_THREAD_ENTRY;

//
// An RTL_INJECTION_CONTEXT is created for each RTL_INJECTION_PACKET requested
// by a caller.  This structure encapsulates internal state needed in order to
// complete the injection request.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_CONTEXT {

    //
    // As with RTL_INJECTION_PACKET, the first member of this structure will
    // always be the function pointer that a remote thread is required to call
    // before executing any other code.
    //

    PRTLP_IS_INJECTION_CONTEXT_PROTOCOL_CALLBACK IsInjectionContextProtocolCallback;

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
    // Fully-qualified path of the Rtl.dll module to load.
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
        PBYTE Code;
        PRTL_INJECTION_COMPLETE_CALLBACK Callback;
    };

    //
    // Approximate start and end addresses of the caller's code, as determined
    // by the GetApproximateFunctionBoundaries() routine.
    //

    ULONG_PTR CodeStartAddress;
    ULONG_PTR CodeEndAddress;

    //
    // The start address of our initial remote thread thunk, allocated in the
    // target process's memory space.
    //

    struct {
        union {
            LPTHREAD_START_ROUTINE StartRoutine;
            PRTLP_INJECTION_REMOTE_THREAD_ENTRY_THUNK InjectionThunk;
        };
        PRTLP_INJECTION_REMOTE_THREAD_ENTRY InjectionThreadEntry;
    } RemoteThread;

    //
    // Approximate start and end addresses of our remote thread's injection
    // thunk, as determined by the GetApproximateFunctionBoundaries() routine.
    //

    ULONG_PTR InjectionThunkStartAddress;
    ULONG_PTR InjectionThunkEndAddress;

    //
    // An approximate size of the callback function code.
    //

    LONG SizeOfCallbackCodeInBytes;

    //
    // Approximate size of our thread start routine.
    //

    LONG SizeOfInjectionThunkInBytes;

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
    // for the caller's code.
    //

    LONG_INTEGER TotalCodeAllocSize;

    //
    // Total size of the code (executable) memory allocation for our internal
    // thread start routine.
    //

    LONG_INTEGER TotalInjectionThunkAllocSize;

    //
    // Base address of the context allocation, the size of which is indicated by
    // the TotalContextAllocSize struct member above.
    //

    LPVOID BaseContextAddress;

    //
    // Base address of the code allocation, the size of which is indicated by
    // the TotalCallerCodeAllocSize struct member above.  The caller's code will
    // be copied into this space from the 7th byte onward -- the first six are
    // padded with `int 3` (0xCC).
    //

    LPVOID BaseCodeAddress;

    //
    // Base address of our internal thread entry code allocation, the size of
    // which is indicated by the TotalCallerCodeAllocSize struct member above.  The caller's code will
    // The ThreadStartRoutine will be offset 7 bytes into this space, as per
    // the `int 3` (0xCC) padding requirements.
    //

    LPVOID BaseInjectionThunkAddress;

    //
    // Function pointers required by the initial injection callback.
    //

    //
    // Inline RTL_INJECTION_FUNCTIONS.
    //

    union {
        struct {
            PSET_EVENT SetEvent;
            POPEN_EVENT_W OpenEventW;
            PCLOSE_HANDLE CloseHandle;
            PGET_PROC_ADDRESS GetProcAddress;
            PLOAD_LIBRARY_EX_W LoadLibraryExW;
            PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;
            PWAIT_FOR_SINGLE_OBJECT WaitForSingleObject;
            POUTPUT_DEBUG_STRING_A OutputDebugStringA;
            POUTPUT_DEBUG_STRING_W OutputDebugStringW;
        };

        RTL_INJECTION_FUNCTIONS Functions;
    };

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
