/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    Injection.h

Abstract:

    This is the header file for the Rtl component's remote thread creation
    and DLL injection functionality, referred to collectively as "injection".

    Callers request injection by way of creating an RTL_INJECTION_PACKET via
    RtlCreateInjectionPacket().  Arbitrary pages of memory can be added to the
    injection packet via RtlAddInjectionPayload(); sufficient memory will be
    allocated in the target process's address space and the memory will be
    copied over as part of the injection process.  If the caller depends on
    symbols exported by other libraries once injected into the remote process's
    address space, RtlAddInjectionSymbolsRequest() can be used to achieve this
    by passing in module names (e.g. "kernel32.dll") and arrays of symbol names
    (e.g. { "OpenEvent", "CloseHandle" }).  Each injection symbols request will
    create an RTL_INJECTION_SYMBOLS_REQUEST structure and associate it with the
    RTL_INJECTION_PACKET.

    Callers can indicate the code they would like executed by a thread in the
    remote process in one of two ways: by specifying a module path and a name
    to an exported function in the module, or by providing arbitrary code bytes
    to be injected.  This is controlled by the RTL_INJECTION_PACKET_FLAGS param
    provided to RtlCreateInjectionPacket().  In either case, the function must
    match the prototype of RTL_INJECTION_FUNCTION_CALLBACK.

    The behavior of the thread after the completion function has been called
    hasn't been thought about enough yet and is undefined.  (If we're hijacking
    an existing thread, do we need to allow it to return to what it was doing?)

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

typedef union _RTL_CREATE_INJECTION_PACKET_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, the caller indicates a module path (e.g. a .dll) and the
        // name of the symbol exported by said module; the module will be loaded
        // in the remote process, the symbol resolved to a function pointer via
        // GetProcAddress(), then invoked by a newly-created remote thread, or
        // the hijacking of an existing thread, specified by the user at the
        // time the injection packet was created.
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
        // When set, indicates that an existing thread in the target process
        // should be hijacked to run the remote code, instead of creating a
        // new thread in the remote process.  If this flag is set, the caller
        // must provide a valid thread ID as the TargetThreadId parameter to
        // the RtlCreateInjectionPacket() call.
        //

        ULONG HijackExistingThread:1;

        //
        // Unused.
        //

        ULONG Unused:29;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_CREATE_INJECTION_PACKET_FLAGS;
typedef RTL_CREATE_INJECTION_PACKET_FLAGS *PRTL_CREATE_INJECTION_PACKET_FLAGS;
C_ASSERT(sizeof(RTL_CREATE_INJECTION_PACKET_FLAGS) == sizeof(ULONG));

typedef union _RTL_INJECTION_PACKET_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates the function is running in the injected context.
        //

        ULONG IsInjected:1;

        //
        // Unused bits.
        //

        ULONG Unused:31;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_PACKET_FLAGS;
typedef RTL_INJECTION_PACKET_FLAGS *PRTL_INJECTION_PACKET_FLAGS;
C_ASSERT(sizeof(RTL_INJECTION_PACKET_FLAGS) == sizeof(ULONG));

typedef ULARGE_INTEGER RTL_INJECTION_MAGIC_NUMBER;

typedef struct _RTL_INJECTION_PAYLOAD {
    LIST_ENTRY ListEntry;
    ULONGLONG NumberOfPages;
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
    // (48 bytes consumed.)
    //

    //
    // Pad out to 64 bytes.
    //

    ULONGLONG Padding[3];
} RTL_INJECTION_PAYLOAD;
typedef RTL_INJECTION_PAYLOAD *PRTL_INJECTION_PAYLOAD;
C_ASSERT(sizeof(RTL_INJECTION_PAYLOAD) == 64);

typedef struct _RTL_INJECTION_SYMBOLS {
    LIST_ENTRY ListEntry;
    ULONG NumberOfSymbols;
    ULONG NumberOfModules;
    PCSZ *SymbolNameArray;
    PCUNICODE_STRING *ModuleNameArray;

    ULONG NumberOfResolvedSymbols;
    PULONG_PTR SymbolAddressesArray;
    PRTL_BITMAP FailedSymbols;
} RTL_INJECTION_SYMBOLS;
typedef RTL_INJECTION_SYMBOLS *PRTL_INJECTION_SYMBOLS;

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
        ULONGLONG InvalidSizeOfCodeInBytes:1;
        ULONGLONG InvalidTargetProcessId:1;
        ULONGLONG InvalidTargetThreadId:1;
        ULONGLONG CodeProbeFailed:1;
        ULONGLONG LoadLibraryFailed:1;
        ULONGLONG GetProcAddressFailed:1;
        ULONGLONG VirtualAllocFailed:1;
        ULONGLONG VirtualProtectFailed:1;
        ULONGLONG FlushInstructionCacheFailed:1;
        ULONGLONG IllegalInstructionInCallbackTest:1;
        ULONGLONG AccessViolationInCallbackTest:1;
        ULONGLONG StatusInPageErrorInCallbackTest:1;
        ULONGLONG IncorrectMagicNumberReturnedInCallbackTest:1;
        ULONGLONG OpenTargetProcessFailed:1;
        ULONGLONG OpenTargetThreadFailed:1;
        ULONGLONG SuspendTargetThreadFailed:1;
        ULONGLONG CreateRemoteThreadFailed:1;
        ULONGLONG InternalAllocationFailure:1;
        ULONGLONG InternalStatusInPageError:1;
        ULONGLONG WriteRemoteMemoryFailed:1;

        ULONGLONG Unused:37;
    };
    LONGLONG ErrorCode;
    ULONGLONG AsULongLong;
} RTL_INJECTION_ERROR;
C_ASSERT(sizeof(RTL_INJECTION_ERROR) == sizeof(ULONGLONG));
typedef RTL_INJECTION_ERROR *PRTL_INJECTION_ERROR;

typedef
_Check_return_
BOOL
(CALLBACK RTL_IS_INJECTION_PROTOCOL_CALLBACK)(
    _In_ struct _RTL_INJECTION_PACKET const *Packet,
    _Outptr_result_maybenull_ PVOID Token
    );
typedef RTL_IS_INJECTION_PROTOCOL_CALLBACK *PRTL_IS_INJECTION_PROTOCOL_CALLBACK;

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

    PRTL_IS_INJECTION_PROTOCOL_CALLBACK IsInjectionProtocolCallback;

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
    // A magic number used to perform simple validation of the injected code.
    //

    RTL_INJECTION_MAGIC_NUMBER MagicNumber;

    //
    // Fully-qualified path name of the target library to load.
    //

    UNICODE_STRING Path;

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
    // Optionally supplies the ID of a thread within the targeted process to
    // use for injection.  If this is 0, a new remote thread will be created.
    //

    ULONG TargetThreadId;

    //
    // Payload information.
    //

    LIST_ENTRY PayloadListHead;
    ULONGLONG NumberOfPayloads;
    PRTL_INJECTION_PAYLOAD Payloads;

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
(RTL_CREATE_INJECTION_PACKET)(
    _In_ struct _RTL *Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PRTL_CREATE_INJECTION_PACKET_FLAGS Flags,

    _When_(Flags->InjectModule == 0, _Pre_null_)
    _When_(Flags->InjectModule != 0, _In_)
        PCUNICODE_STRING ModulePath,

    _When_(Flags->InjectModule == 0, _Pre_null_)
    _When_(Flags->InjectModule != 0, _In_)
        PCSTRING CallbackFunctionName,

    _When_(Flags->InjectCode == 0, _Pre_null_)
    _When_(Flags->InjectCode != 0, _In_)
        PRTL_INJECTION_COMPLETE_CALLBACK Callback,

    _In_ ULONG TargetProcessId,

    _When_(Flags->HijackExistingThread == 0, _Pre_null_)
    _When_(Flags->HijackExistingThread != 0, _In_)
        ULONG TargetThreadId,

    _Outptr_result_nullonfailure_ PRTL_INJECTION_PACKET *InjectionPacketPointer,
    _Outptr_ PRTL_INJECTION_ERROR InjectionError
    );
typedef RTL_CREATE_INJECTION_PACKET *PRTL_CREATE_INJECTION_PACKET;

typedef
_Success_(return != 0)
_When_(return != 0, _Post_satisfies_(InjectionError->ErrorCode == 0))
BOOL
(RTL_DESTROY_INJECTION_PACKET)(
    _In_ struct _RTL *Rtl,
    _Pre_ _Notnull_ _Post_ptr_invalid_ PRTL_INJECTION_PACKET *PacketPointer,
    _Outptr_ PRTL_INJECTION_ERROR InjectionError
    );
typedef RTL_DESTROY_INJECTION_PACKET *PRTL_DESTROY_INJECTION_PACKET;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTL_ADD_INJECTION_PAYLOAD)(
    _In_ struct _RTL *Rtl,
    _In_ PRTL_INJECTION_PACKET Packet,
    _In_ PRTL_INJECTION_PAYLOAD Payload,
    _Out_ PRTL_INJECTION_ERROR InjectionError
    );
typedef RTL_ADD_INJECTION_PAYLOAD *PRTL_ADD_INJECTION_PAYLOAD;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTL_CREATE_INJECTION_SYMBOLS)(
    _In_count_(NumberOfSymbolNames) CONST PCSZ *SymbolNameArray,
    _In_ ULONG NumberOfSymbolNames,
    _In_count_(NumberOfSymbolAddresses) PULONG_PTR SymbolAddressArray,
    _In_ ULONG NumberOfSymbolAddresses,
    _In_ HMODULE Module,
    _In_ PRTL_BITMAP FailedSymbols,
    _Out_ PULONG NumberOfResolvedSymbolsPointer
    );

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTL_ADD_INJECTION_SYMBOLS)(
    _In_ struct _RTL *Rtl,
    _In_ PRTL_INJECTION_PACKET Packet,
    _In_ PRTL_INJECTION_SYMBOLS Symbols,
    _Out_ PRTL_INJECTION_ERROR InjectionError
    );
typedef RTL_ADD_INJECTION_SYMBOLS *PRTL_ADD_INJECTION_SYMBOLS;

typedef
_Check_return_
_Success_(return == 0)
BOOL
(RTL_INJECT)(
    _In_ struct _RTL *Rtl,
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_ PRTL_INJECTION_ERROR InjectionError
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

RTL_API RTL_CREATE_INJECTION_PACKET RtlCreateInjectionPacket;
RTL_API RTL_DESTROY_INJECTION_PACKET RtlDestroyInjectionPacket;
RTL_API RTL_ADD_INJECTION_PAYLOAD RtlAddInjectionPayload;
RTL_API RTL_ADD_INJECTION_SYMBOLS RtlAddInjectionSymbols;
RTL_API RTL_INJECT RtlInject;

//
// Other injection related methods supporting the primary API.
//

//RTL_API RTL_INITIALIZE_INJECTION RtlInitializeInjection;
RTL_API RTL_IS_INJECTION_PROTOCOL_CALLBACK RtlIsInjectionProtocolCallback;

//
// Additional helper methods that can be used independently to injection.
//

RTL_API GET_INSTRUCTION_POINTER GetInstructionPointer;
RTL_API GET_APPROXIMATE_FUNCTION_BOUNDARIES GetApproximateFunctionBoundaries;
RTL_API SKIP_JUMPS SkipJumps;
RTL_API IS_JUMP IsJump;

//
// The RIP-relative code isn't implemented yet.
//

/*
RTL_API IS_RIP_RELATIVE_INSTRUCTION IsRipRelativeInstruction;
RTL_API EXTRACT_RIP_RELATIVE_OFFSET ExtractRipRelativeOffset;
RTL_API GET_EFFECTIVE_ADDRESS_FROM_RIP_RELATIVE_INSTRUCTION GetEffectiveAddressFromRipRelativeInstruction;
RTL_API RELOCATE_RIP_RELATIVE_CODE RelocateRipRelativeCode;
*/

#pragma component(browser, on)


#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
