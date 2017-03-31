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

typedef union _RTL_INJECTION_PACKET_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:1;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_PACKET_FLAGS;
typedef RTL_INJECTION_PACKET_FLAGS *PRTL_INJECTION_PACKET_FLAGS;
C_ASSERT(sizeof(RTL_INJECTION_PACKET_FLAGS) == sizeof(ULONG));

typedef union _RTL_INJECTION_MAGIC {
    LARGE_INTEGER AsLargeInteger;
    struct {
        ULONG MagicNumber;
        ULONG InjectionPacketCrc32;
    };
} RTL_INJECTION_MAGIC;

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
// Callers request a remote thread + DLL injection by way of the following
// structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_PACKET {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_INJECTION_PACKET)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    RTL_INJECTION_PACKET_FLAGS Flags;

    //
    // Packets are verified via the magic structure before being acted upon.
    //

    RTL_INJECTION_MAGIC Magic;

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

    ULONG OptionalTargetThreadId;

    //
    // Payload information.
    //

    LIST_ENTRY PayloadListHead;
    ULONGLONG NumberOfPayloads;
    PRTL_INJECTION_PAYLOAD Payloads;

} RTL_INJECTION_PACKET;
typedef RTL_INJECTION_PACKET *PRTL_INJECTION_PACKET;
typedef const RTL_INJECTION_PACKET *PCRTL_INJECTION_PACKET;

typedef union _RTL_INJECTION_ERROR {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG InvalidTargetProcessId:1;
        ULONG InvalidOptionalTargetThreadId:1;
        ULONG InvalidPacket:1;
        ULONG InvalidPayload:1;
        ULONG OpenTargetProcessHandleFailed:1;
        ULONG CreateRemoteThreadFailed:1;
        ULONG WriteRemoteMemoryFailed:1;
    };
    LONG AsLong;
    ULONG AsULong;
} RTL_INJECTION_ERROR;
C_ASSERT(sizeof(RTL_INJECTION_ERROR) == sizeof(ULONG));
typedef RTL_INJECTION_ERROR *PRTL_INJECTION_ERROR;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RTL_CREATE_INJECTION_PACKET)(
    _In_ struct _RTL *Rtl,
    _In_opt_ PRTL_INJECTION_PACKET_FLAGS Flags,
    _In_opt_ PCUNICODE_STRING ModulePath,
    _In_opt_ PSTRING CallbackFunctionName,
    _In_opt_ ULONG SizeOfCodeInBytes,
    _In_reads_bytes_opt_(SizeOfCodeInBytes) PBYTE Code,
    _In_ ULONG TargetProcessId,
    _In_opt_ ULONG OptionalTargetThreadId,
    _Outptr_result_maybenull_ PRTL_INJECTION_ERROR InjectionError,
    _Outptr_result_maybenull_ PRTL_INJECTION_PACKET *InjectionPacketPointer
    );
typedef RTL_CREATE_INJECTION_PACKET *PRTL_CREATE_INJECTION_PACKET;

typedef
_Success_(return != 0)
BOOL
(RTL_DESTROY_INJECTION_PACKET)(
    _In_ struct _RTL *Rtl,
    _Pre_ _Notnull_ _Post_ _Notvalid_ PRTL_INJECTION_PACKET *PacketPointer
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

/*
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
*/

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

typedef
VOID
(CALLBACK RTL_INJECTION_COMPLETE_CALLBACK)(
    _In_ PRTL_INJECTION_PACKET Packet,
    _In_opt_ RTL_INJECTION_ERROR InjectionError
    );
typedef RTL_INJECTION_COMPLETE_CALLBACK *PRTL_INJECTION_COMPLETE_CALLBACK;


#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
