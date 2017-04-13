/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Injection.h

Abstract:

    This is the header file for the Rtl component's remote thread creation
    and DLL injection functionality, referred to collectively as "injection".

    The main routine exposed by this module is RtlInject().  The main public
    data type (created as part of RtlInject()) is the RTL_INJECTION_PACKET
    structure.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// This type is used to communicate the type of injection wanted to RtlInject().
//

typedef union _RTL_INJECTION_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, the caller indicates a module path (e.g. a .dll) and the
        // name of the symbol exported by said module; the module will be loaded
        // in the remote process, the symbol resolved to a function pointer via
        // GetProcAddress(), then invoked by a newly-created remote thread.
        //
        // Invariants:
        //
        //   - If InjectModule == TRUE:
        //          Assert InjectCode == FALSE
        //
        //

        ULONG InjectModule:1;

        //
        // Indicates that the caller has provided arbitrary code bytes to
        // inject into the remote process instead of a module path and symbol
        // name.
        //
        // Invariants:
        //
        //   - If InjectCode == TRUE:
        //          Assert InjectModule == FALSE
        //

        ULONG InjectCode:1;

        //
        // When set, indicates the caller also wishes to inject a payload into
        // the remote process.  Requires a valid Payload pointer be passed to
        // RtlCreateInjectionPacket().
        //

        ULONG InjectPayload:1;

        //
        // When set, indicates the caller wishes to inject the Rtl library into
        // the target process and call InitializeRtl() such that the Rtl field
        // of the injection packet points to an initialized structure by the
        // time the callback routine is invoked.
        //
        // N.B. Not currently supported.
        //

        ULONG InjectRtl:1;

        //
        // When set, the callback routine will be invoked from within a SEH
        // block that catches STATUS_IN_PAGE_ERROR, EXCEPTION_ACCESS_VIOLATION
        // and EXCEPTION_ILLEGAL_INSTRUCTION errors.  If such an exception is
        // caught, the remote thread's exit code will reflect the specific
        // exception code, e.g. InjectedRemoteThreadAccessViolationInCallback.
        //

        ULONG CatchExceptionsInRemoteThreadCallback:1;

        //
        // Unused.
        //

        ULONG Unused:27;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_FLAGS;
typedef RTL_INJECTION_FLAGS *PRTL_INJECTION_FLAGS;
C_ASSERT(sizeof(RTL_INJECTION_FLAGS) == sizeof(ULONG));

typedef union _RTL_INJECTION_PACKET_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // The following two flags are used as part of the injection callback
        // protocol.
        //

        ULONG IsMagicNumberTest:1;
        ULONG IsCodeSizeQuery:1;

        //
        // When set, indicates the function is running in the injected context.
        //

        ULONG IsInjected:1;

        //
        // Unused bits.
        //

        ULONG Unused:29;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_PACKET_FLAGS;
typedef RTL_INJECTION_PACKET_FLAGS *PRTL_INJECTION_PACKET_FLAGS;
C_ASSERT(sizeof(RTL_INJECTION_PACKET_FLAGS) == sizeof(ULONG));

typedef ULARGE_INTEGER RTL_INJECTION_MAGIC_NUMBER;

//
// Injection payload flags.  (Currently unused.)
//

typedef union _RTL_INJECTION_PAYLOAD_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // Unused bits.
        //

        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_PAYLOAD_FLAGS;
typedef RTL_INJECTION_PAYLOAD_FLAGS *PRTL_INJECTION_PAYLOAD_FLAGS;
C_ASSERT(sizeof(RTL_INJECTION_PAYLOAD_FLAGS) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_PAYLOAD {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_INJECTION_PAYLOAD)) ULONG SizeOfStruct;

    //
    // Payload flags.
    //

    RTL_INJECTION_PAYLOAD_FLAGS Flags;

    //
    // Size of the memory pointed to by Buffer.
    //

    ULONGLONG SizeOfBufferInBytes;

    //
    // Pointer to the payload.  In the injecting process, this will be the
    // caller's original buffer address.  In the injected process, this will
    // be the newly-allocated buffer.
    //

    PVOID Buffer;

    //
    // The original address of the payload buffer in the process that requested
    // the injection.  This will be set by the injection machinery prior to
    // completing the injection request.  This is provided such that injected
    // code can potentially relocate addresses contained within the payload
    // buffer relative to an original address.
    //

    ULONG_PTR OriginalBufferAddress;

    //
    // (32 bytes consumed.)
    //

    //
    // Pad out to 64 bytes.
    //

    ULONGLONG Padding[4];
} RTL_INJECTION_PAYLOAD;
typedef RTL_INJECTION_PAYLOAD *PRTL_INJECTION_PAYLOAD;
C_ASSERT(sizeof(RTL_INJECTION_PAYLOAD) == 64);

//
// All error state is encapsulated in the following bitfield.  Multiple bits
// may be set, typically with each successive bit providing greater information
// about the type of error (e.g. InvalidParameters and CodeProbeFailed may be
// set).
//

typedef union _RTL_INJECTION_ERROR {
    struct _Struct_size_bytes_(sizeof(ULONGLONG)) {
        ULONGLONG InternalError:1;
        ULONGLONG InvalidParameters:1;
        ULONGLONG InvalidFlags:1;
        ULONGLONG InvalidModuleName:1;
        ULONGLONG InvalidCallbackFunctionName:1;
        ULONGLONG InvalidCodeParameter:1;
        ULONGLONG InvalidCode:1;
        ULONGLONG InvalidPayloadParameter:1;
        ULONGLONG InvalidPayload:1;
        ULONGLONG PayloadReadProbeFailed:1;
        ULONGLONG InvalidTargetProcessId:1;
        ULONGLONG LoadLibraryFailed:1;
        ULONGLONG GetProcAddressFailed:1;
        ULONGLONG VirtualAllocExFailed:1;
        ULONGLONG VirtualProtectExFailed:1;
        ULONGLONG FlushInstructionCacheFailed:1;
        ULONGLONG IllegalInstructionInCallbackTest:1;
        ULONGLONG AccessViolationInCallbackTest:1;
        ULONGLONG StatusInPageErrorInCallbackTest:1;
        ULONGLONG MagicNumberMismatch:1;
        ULONGLONG ExtractCodeSizeFailedDuringCallbackTest:1;
        ULONGLONG CallbackCodeSizeGreaterThanOrEqualToPageSize:1;
        ULONGLONG CallbackCodeCrossesPageBoundary:1;
        ULONGLONG OpenTargetProcessFailed:1;
        ULONGLONG OpenTargetThreadFailed:1;
        ULONGLONG SuspendTargetThreadFailed:1;
        ULONGLONG CreateRemoteThreadFailed:1;
        ULONGLONG InternalAllocationFailure:1;
        ULONGLONG InternalStatusInPageError:1;
        ULONGLONG CreateInjectionContextFailed:1;
        ULONGLONG WriteRemoteMemoryFailed:1;
        ULONGLONG CreateEventWFailed:1;

        //
        // (32 bits consumed.)
        //

        ULONGLONG CreateEventNameFailed:1;
        ULONGLONG InvalidEventId:1;
        ULONGLONG IllegalInstructionInCallback:1;
        ULONGLONG AccessViolationInCallback:1;
        ULONGLONG StatusInPageErrorInCallback:1;
        ULONGLONG InjectionThunkExtractCodeSizeFailed:1;
        ULONGLONG RemoteContextAllocationFailed:1;
        ULONGLONG RemoteInjectionThunkAllocationFailed:1;
        ULONGLONG RemoteCallerCodeAllocationFailed:1;
        ULONGLONG RemotePayloadAllocationFailed:1;
        ULONGLONG CanCallGetLastErrorForMoreInfo:1;
        ULONGLONG VirtualAllocFailed:1;
        ULONGLONG InjectionContextAllocationFailed:1;
        ULONGLONG InjectionContextCodeAllocationFailed:1;
        ULONGLONG InjectionCallerCodeAllocationFailed:1;
        ULONGLONG RemoteMemoryInjectionFailed:1;
        ULONGLONG WriteRemoteContextFailed:1;
        ULONGLONG WriteRemoteContextCodeFailed:1;
        ULONGLONG WriteRemoteCallerCodeFailed:1;
        ULONGLONG WriteRemotePayloadFailed:1;
        ULONGLONG VirtualProtectExContextCodeFailed:1;
        ULONGLONG VirtualProtectExCallerCodeFailed:1;
        ULONGLONG FlushInstructionCacheContextCodeFailed:1;
        ULONGLONG FlushInstructionCacheCallerCodeFailed:1;
        ULONGLONG SignalObjectAndWaitFailed:1;

        //
        // Decrement the remaining bits in the Unused member below when adding
        // new fields.
        //

        ULONGLONG Unused:7;
    };
    LONGLONG ErrorCode;
    ULONGLONG AsULongLong;
} RTL_INJECTION_ERROR;
C_ASSERT(sizeof(RTL_INJECTION_ERROR) == sizeof(ULONGLONG));
typedef RTL_INJECTION_ERROR *PRTL_INJECTION_ERROR;

//
// Define an enum for remote thread exit codes.
//

typedef enum _RTL_INJECTION_REMOTE_THREAD_EXIT_CODE {

    InjectedRemoteThreadNoError = 0,

    //
    // Start the remaining error codes off at an arbitrarily high number, such
    // that it's easy to distinguish between an error code and the size of the
    // function if a context protocol callback is being dispatched.
    //

    InjectedRemoteThreadNullContext = 1 << 20,
    InjectedRemoteThreadLoadLibraryRtlFailed,
    InjectedRemoteThreadResolveInitializeRtlFailed,
    InjectedRemoteThreadInitializeRtlFailed,
    InjectedRemoteThreadNullResultParameter,
    InjectedRemoteThreadEncounteredExceptionInCompletionCallback,

    InjectedRemoteThreadOpenSignalEventFailed,
    InjectedRemoteThreadOpenWaitEventFailed,
    InjectedRemoteThreadOpenCallerSignalEventFailed,
    InjectedRemoteThreadOpenCallerWaitEventFailed,

    InjectedRemoteThreadStatusInPageErrorInCallback,
    InjectedRemoteThreadAccessViolationInCallback,
    InjectedRemoteThreadIllegalInstructionInCallback,

    InjectedRemoteThreadQueuedAPCExitThread,

} RTL_INJECTION_REMOTE_THREAD_EXIT_CODE;

typedef
_Check_return_
BOOL
(CALLBACK RTL_IS_INJECTION_PROTOCOL_CALLBACK)(
    _In_ struct _RTL_INJECTION_PACKET const *Packet,
    _Outptr_result_maybenull_ PVOID Token
    );
typedef RTL_IS_INJECTION_PROTOCOL_CALLBACK *PRTL_IS_INJECTION_PROTOCOL_CALLBACK;

//
// A minimal set of functions used to bootstrap injected code.
//

typedef struct _RTL_INJECTION_FUNCTIONS {
    PSET_EVENT SetEvent;
    POPEN_EVENT_W OpenEventW;
    PCLOSE_HANDLE CloseHandle;
    PGET_PROC_ADDRESS GetProcAddress;
    PLOAD_LIBRARY_EX_W LoadLibraryExW;
    PSIGNAL_OBJECT_AND_WAIT SignalObjectAndWait;
    PWAIT_FOR_SINGLE_OBJECT_EX WaitForSingleObjectEx;
    POUTPUT_DEBUG_STRING_A OutputDebugStringA;
    POUTPUT_DEBUG_STRING_W OutputDebugStringW;
    PNT_QUEUE_APC_THREAD NtQueueUserApcThread;
    PNT_TEST_ALERT NtTestAlert;
    PQUEUE_USER_APC QueueUserAPC;
    PSLEEP_EX SleepEx;
    PEXIT_THREAD ExitThread;
    PGET_EXIT_CODE_THREAD GetExitCodeThread;
    PCREATE_FILE_MAPPING_W CreateFileMappingW;
    POPEN_FILE_MAPPING_W OpenFileMappingW;
    PMAP_VIEW_OF_FILE_EX MapViewOfFileEx;
    PFLUSH_VIEW_OF_FILE FlushViewOfFile;
    PUNMAP_VIEW_OF_FILE_EX UnmapViewOfFileEx;
} RTL_INJECTION_FUNCTIONS;
typedef RTL_INJECTION_FUNCTIONS *PRTL_INJECTION_FUNCTIONS;
typedef const RTL_INJECTION_FUNCTIONS *PCRTL_INJECTION_FUNCTIONS;

//
// Callers request a remote thread + DLL injection by way of the following
// structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_PACKET {

    //
    // The injection protocol requires that callback code must perform the
    // following:
    //
    //      PVOID Token;
    //
    //      if (Packet->IsInjectionProtocolCallback(Packet, &Token)) {
    //          return Token;
    //      }
    //
    // The following field contains the function pointer to facilitate such
    // a check.  It is guaranteed to always be the first element in the
    // injection packet structure.
    //

    union {
        PRTL_IS_INJECTION_PROTOCOL_CALLBACK IsInjectionProtocolCallback;
        PAPC_ROUTINE CallbackProtocolThunk;
    };

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_INJECTION_PACKET)) ULONG SizeOfStruct;

    //
    // Flags used for communicating various states of the packet.
    //

    RTL_INJECTION_PACKET_FLAGS Flags;

    //
    // All errors will be communicated through this error field.
    //

    RTL_INJECTION_ERROR Error;

    //
    // Injection flags passed to RtlInject().
    //

    RTL_INJECTION_FLAGS InjectionFlags;

    //
    // A magic number used to perform simple validation of the injected code.
    //

    RTL_INJECTION_MAGIC_NUMBER MagicNumber;

    //
    // An instance of the RTL structure initialized in the target process.
    //

    PRTL Rtl;

    //
    // Fully-qualified path name of the target library to load if InjectModule
    // was requested.
    //

    UNICODE_STRING ModulePath;

    //
    // Name of the procedure to resolve via GetProcAddress(). The function
    // should conform to the RTL_INJECTION_COMPLETE_CALLBACK signature, and
    // will be invoked by a remote thread once injection has completed.
    //

    STRING CallbackFunctionName;

    //
    // Supplies the process ID to use as the target for injection.
    //

    ULONG TargetProcessId;

    //
    // The ID of the remote thread created as part of injection.
    //

    ULONG TargetThreadId;

    //
    // Signal and Wait event handles and names for general purpose use.
    //

    HANDLE SignalEventHandle;
    HANDLE WaitEventHandle;

    PCUNICODE_STRING SignalEventName;
    PCUNICODE_STRING WaitEventName;

    //
    // A pointer to the structure of bootstrap functions,.
    //

    PCRTL_INJECTION_FUNCTIONS Functions;

    //
    // Payload information.
    //

    RTL_INJECTION_PAYLOAD Payload;

} RTL_INJECTION_PACKET;
typedef RTL_INJECTION_PACKET *PRTL_INJECTION_PACKET;
typedef const RTL_INJECTION_PACKET *PCRTL_INJECTION_PACKET;

typedef
ULONGLONG
(CALLBACK RTL_INJECTION_COMPLETE_CALLBACK)(
    _In_ PRTL_INJECTION_PACKET Packet
    );
typedef RTL_INJECTION_COMPLETE_CALLBACK *PRTL_INJECTION_COMPLETE_CALLBACK;


typedef
_Check_return_
_Success_(return != 0)
_Pre_satisfies_(_Notnull_(Flags))
_Pre_satisfies_(Flags->InjectModule || Flags->InjectCode)
_Pre_satisfies_(Flags->Unused == 0)
_When_(return != 0, _Post_satisfies_(InjectionError->ErrorCode == 0))
_When_(return == 0, _Post_satisfies_(InjectionError->ErrorCode != 0))
BOOL
(RTL_INJECT)(
    _In_ struct _RTL *Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PRTL_INJECTION_FLAGS Flags,

    _When_(Flags->InjectModule == 0, _Pre_null_)
    _When_(Flags->InjectModule != 0, _In_)
        PCUNICODE_STRING ModulePath,

    _When_(Flags->InjectModule == 0, _Pre_null_)
    _When_(Flags->InjectModule != 0, _In_)
        PCSTRING CallbackFunctionName,

    _When_(Flags->InjectCode == 0, _Pre_null_)
    _When_(Flags->InjectCode != 0, _In_)
        PRTL_INJECTION_COMPLETE_CALLBACK Callback,

    _When_(Flags->InjectPayload == 0, _Pre_null_)
    _When_(Flags->InjectPayload != 0, _In_)
        PRTL_INJECTION_PAYLOAD Payload,

    _In_ ULONG TargetProcessId,

    _Outptr_result_nullonfailure_ PRTL_INJECTION_PACKET *InjectionPacketPointer,
    _Outptr_ PRTL_INJECTION_ERROR InjectionError
    );
typedef RTL_INJECT *PRTL_INJECT;

//
// Include inline functions.
//

#include "InjectionInline.h"

//
// Public symbol declarations.
//

#pragma component(browser, off)

//
// Primary Injection API.
//

RTL_API RTL_INJECT RtlInject;

//
// Other injection related methods supporting the primary API.
//

RTL_API RTL_IS_INJECTION_PROTOCOL_CALLBACK RtlIsInjectionProtocolCallback;

//
// Additional helper methods that can be used independently to injection.
//

RTL_API GET_INSTRUCTION_POINTER GetInstructionPointer;
RTL_API GET_APPROXIMATE_FUNCTION_BOUNDARIES GetApproximateFunctionBoundaries;
RTL_API SKIP_JUMPS SkipJumps;
RTL_API IS_JUMP IsJump;

#pragma component(browser, on)

#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
