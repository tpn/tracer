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
        // When set, indicates the injection machinery is performing a test of
        // the caller's requested code injection prior to actually doing the
        // injection.  The injected code must detect the presence of this flag,
        // read the contents of Packet->MagicNumber field, then return the XOR'd
        // contents of the High and Low parts.
        //

        ULONG IsInjectionCompleteCallbackTest:1;

        //
        // When set, indicates that a calling routine wishes to obtain the
        // code size of the given function as the return value.  The function
        // pointer Packet->GetReturnAddress will be provided in order to assist
        // with determining the callback code size.
        //

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
ULONG_PTR
(CALLBACK GET_INSTRUCTION_POINTER)(VOID);
typedef GET_INSTRUCTION_POINTER *PGET_INSTRUCTION_POINTER;

typedef
BOOL
(CALLBACK GET_APPROXIMATE_FUNCTION_BOUNDARIES)(
    _In_ ULONG_PTR Address,
    _Out_ PULONG_PTR StartAddress,
    _Out_ PULONG_PTR EndAddress
    );
typedef GET_APPROXIMATE_FUNCTION_BOUNDARIES
      *PGET_APPROXIMATE_FUNCTION_BOUNDARIES;

typedef
BOOL
(CALLBACK IS_JUMP)(
    _In_ PBYTE Code
    );
typedef IS_JUMP *PIS_JUMP;

typedef
PBYTE
(FOLLOW_JUMPS_TO_FIRST_CODE_BYTE)(
    _In_ PBYTE Code
    );
typedef FOLLOW_JUMPS_TO_FIRST_CODE_BYTE *PFOLLOW_JUMPS_TO_FIRST_CODE_BYTE;

//
// Callers request a remote thread + DLL injection by way of the following
// structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_INJECTION_PACKET {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_INJECTION_PACKET)) ULONG SizeOfStruct;

    RTL_INJECTION_PACKET_FLAGS Flags;

    //
    // All errors will be communicated through this error field.
    //

    RTL_INJECTION_ERROR Error;

    //
    // Packets are verified via the magic structure before being acted upon.
    //

    RTL_INJECTION_MAGIC_NUMBER MagicNumber;

    //
    // The following two function pointers will always be provided when an
    // injection complete callback routine is being invoked, regardless of
    // whether or not the code is in the original process (e.g. a test) or
    // the injected process.
    //

    PGET_INSTRUCTION_POINTER GetInstructionPointer;
    PGET_APPROXIMATE_FUNCTION_BOUNDARIES GetApproximateFunctionBoundaries;

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
        PBYTE Code,

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
ULONG
(CALLBACK RTL_INJECTION_COMPLETE_CALLBACK)(
    _In_ PRTL_INJECTION_PACKET Packet
    );
typedef RTL_INJECTION_COMPLETE_CALLBACK *PRTL_INJECTION_COMPLETE_CALLBACK;

typedef
BOOL
(CALLBACK RTL_IS_INJECTION_CODE_SIZE_QUERY)(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_opt_ PULONG CodeSize
    );
typedef RTL_IS_INJECTION_CODE_SIZE_QUERY
      *PRTL_IS_INJECTION_CODE_SIZE_QUERY;

FORCEINLINE
BOOL
RtlIsInjectionCodeSizeQueryInline(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_opt_ PULONG CodeSize
    )
/*++

Routine Description:

    This routine implements the code size query protocol required by injected
    routines.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    CodeSize - Supplies the address of a variable that will receive the size of
        the injection callback code, in bytes.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the CodeSize parameter.

--*/
{
    ULONG_PTR Address;
    ULONG_PTR Start;
    ULONG_PTR End;

    if (!Packet->Flags.IsCodeSizeQuery) {
        return FALSE;
    }

    Address = Packet->GetInstructionPointer();
    if (!Packet->GetApproximateFunctionBoundaries(Address, &Start, &End)) {
        return FALSE;
    }

    *CodeSize = (ULONG)(End - Start);

    return TRUE;
}

typedef
BOOL
(CALLBACK RTL_IS_INJECTION_COMPLETE_CALLBACK_TEST)(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_opt_ PULONG MagicNumber
    );
typedef RTL_IS_INJECTION_COMPLETE_CALLBACK_TEST *PRTL_IS_INJECTION_COMPLETE_CALLBACK_TEST;

FORCEINLINE
BOOL
RtlIsInjectionCompleteCallbackTestInline(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_opt_ PULONG MagicNumber
    )
/*++

Routine Description:

    This routine implements the simple callback test protocol required as part
    of the initial injection complete callback test.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    MagicNumber - Supplies the address of a variable that will receive the
        magic number if this is a callback test.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the MagicNumber parameter.

--*/
{
    if (Packet->Flags.IsInjectionCompleteCallbackTest) {
        *MagicNumber = (
            Packet->MagicNumber.LowPart ^
            Packet->MagicNumber.HighPart
        );
        return TRUE;
    }
    return FALSE;
}

typedef
BOOL
(CALLBACK RTL_IS_INJECTION_PROTOCOL_CALLBACK)(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_opt_ PULONG ReturnValue
    );
typedef RTL_IS_INJECTION_PROTOCOL_CALLBACK
      *PRTL_IS_INJECTION_PROTOCOL_CALLBACK;

//
// Inline functions.
//

FORCEINLINE
BOOL
IsJumpInline(
    PBYTE Code
    )
/*++

Routine Description:

    Given an address of AMD64 byte code, returns TRUE if the underlying
    instruction is a jump.

Arguments:

    Code - Supplies a pointer to the first byte of the code.

Return Value:

    TRUE if this address represents a jump, FALSE otherwise.

--*/
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Code)) {
        return FALSE;
    }

    return ((
        (Code[0] == 0xFF && Code[1] == 0x25)                    |
        (Code[0] == 0x48 && Code[1] == 0xFF && Code[2] == 0x25) |
        (Code[0] == 0xE9)                                       |
        (Code[0] == 0xEB)
    ) ? TRUE : FALSE);
}

FORCEINLINE
PBYTE
FollowJumpsToFirstCodeByteInline(
    PBYTE Code
    )
/*++

Routine Description:

    Given an address of AMD64 byte code, follows any jumps until the first non-
    jump byte code is found, and returns that byte.  Alternatively, if the first
    bytes passed in do not indicate a jump, the original byte code address is
    returned.

    This is used to traverse jump tables and get to the actual underlying
    function.

Arguments:

    Code - Supplies a pointer to the first byte of the code for which any jumps
        should be followed.

Return Value:

    The address of the first non-jump byte code encountered.

--*/
{
    LONG Offset;
    CHAR ShortOffset;
    PBYTE OriginalCode = Code;
    PBYTE Target;

    if (!ARGUMENT_PRESENT(Code)) {
        return NULL;
    }

    while (TRUE) {

        Target = NULL;

        if (Code[0] == 0xFF && Code[1] == 0x25) {

            //
            // Offset is 32-bit.
            //

            Offset = *((PLONG)&Code[2]);

            Target = *((PBYTE *)(Code + 6 + Offset));

        } else if (Code[0] == 0x48 && Code[1] == 0xFF && Code[2] == 0x25) {

            //
            // REX-encoded 32-bit offset.
            //

            Offset = *((PLONG)&Code[3]);

            Target = *((PBYTE *)(Code + 7 + Offset));

        } else if (Code[0] == 0xE9) {

            //
            // Offset is 32-bit.
            //

            Offset = *((PLONG)&Code[1]);

            Target = *((PBYTE *)(Code + 7 + Offset));

        } else if (Code[0] == 0xEB) {

            //
            // Short jump; 8-bit offset.
            //

            ShortOffset = *((CHAR *)&Code[1]);

            Target = *((PBYTE *)(Code + 2 + ShortOffset));

        }

        if (!Target) {

            //
            // No more jumps, break out of the loop.
            //

            break;
        }
    }

    return Target;
}

FORCEINLINE
BOOL
GetApproximateFunctionBoundariesInline(
    ULONG_PTR Address,
    PULONG_PTR StartAddress,
    PULONG_PTR EndAddress
    )
/*++

Routine Description:

    Given an address which resides within a function, return the approximate
    start and end addresses of a function by searching forward and backward
    for repeat occurrences of the `int 3` (breakpoint) instruction, represented
    by 0xCC in byte code.  Such occurrences are usually good indicators of
    function boundaries -- in fact, the Microsoft AMD64 calling convention
    requires that function entry points should be padded with 6 bytes (to allow
    for hot-patching), and this padding is always the 0xCC byte.

    The term "approximate" is used to qualify both the start and end addresses,
    because although scanning for repeat 0xCC occurrences is quite reliable, it
    is not as reliable as a more authoritative source of function size, such as
    debug symbols.

Arguments:

    Address - Supplies an address that resides somewhere within the function
        for which the boundaries are to be obtained.

    StartAddress - Supplies a pointer to a variable that will receive the
        address of the approximate starting point of the function.

    EndAddress - Supplies a pointer to a variable that will receive the
        address of the approximate ending point of the function (this will
        typically be the `ret` (0xC3) instruction).

Return Value:

    TRUE if the method was successful, FALSE otherwise.  FALSE will only be
    returned if parameter validation fails.  StartAddress and EndAddress will
    not be updated in this case.

--*/
{
    PWORD Start;
    PWORD End;
    const WORD Int3x2 = 0xCCCC;

    //
    // Verify arguments.
    //

    if (!ARGUMENT_PRESENT(Address)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StartAddress)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(EndAddress)) {
        return FALSE;
    }

    //
    // Skip through any jump instructions of the initial function address,
    // using this instruction as both the Start and End address;
    //

    Start = End = (PWORD)FollowJumpsToFirstCodeByteInline((PBYTE)Address);

    //
    // Search backward through memory until we find two `int 3` instructions.
    //

    for (; *Start != Int3x2; --((PBYTE)Start));

    //
    // Search forward through memory until we find two `int 3` instructions.
    //

    for (; *End != Int3x2; ++((PBYTE)End));

    //
    // Update the caller's address pointers.
    //

    *StartAddress = (ULONG_PTR)(Start+1);
    *EndAddress = (ULONG_PTR)End;

    return TRUE;
}

FORCEINLINE
BOOL
RtlIsInjectionProtocolCallbackInline(
    _In_ PCRTL_INJECTION_PACKET Packet,
    _Out_opt_ PULONG ReturnValue
    )
/*++

Routine Description:

    This routine implements the injection callback protocol required by injected
    code routines.  It should be called as the first step in the injected code,
    and must return the ReturnValue if it returns TRUE.

Arguments:

    Packet - Supplies a pointer to an injection packet.

    ReturnValue - Supplies the address of a variable that will receive the
        return value if this is a protocol callback.

Return Value:

    TRUE if this was a callback test, FALSE otherwise.  If TRUE, the caller
    should return the value of the ReturnValue parameter.

--*/
{
    if (RtlIsInjectionCodeSizeQueryInline(Packet, ReturnValue)) {
        return TRUE;
    }

    if (RtlIsInjectionCompleteCallbackTestInline(Packet, ReturnValue)) {
        return TRUE;
    }

    return FALSE;
}

//
// Public symbol declarations.
//

#pragma component(browser, off)
RTL_API RTL_CREATE_INJECTION_PACKET RtlCreateInjectionPacket;
RTL_API RTL_IS_INJECTION_COMPLETE_CALLBACK_TEST RtlIsInjectionCompleteCallbackTest;
RTL_API RTL_DESTROY_INJECTION_PACKET RtlDestroyInjectionPacket;
RTL_API RTL_ADD_INJECTION_PAYLOAD RtlAddInjectionPayload;
RTL_API RTL_ADD_INJECTION_SYMBOLS RtlAddInjectionSymbols;
RTL_API RTL_INJECT RtlInject;
RTL_API GET_INSTRUCTION_POINTER GetInstructionPointer;
RTL_API GET_APPROXIMATE_FUNCTION_BOUNDARIES GetApproximateFunctionBoundaries;
RTL_API FOLLOW_JUMPS_TO_FIRST_CODE_BYTE FollowJumpsToFirstCodeByte;
RTL_API IS_JUMP IsJump;
#pragma component(browser, on)


#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
