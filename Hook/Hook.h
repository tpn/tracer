// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Rtl/Rtl.h"

#pragma pack(push, 1)
typedef struct _LONG_JUMP {
    BYTE Opcode;
    LONGLONG Displacement;
} LONG_JUMP, *PLONG_JUMP, **PPLONG_JUMP;

#pragma pack(pop)

typedef struct _HOOKED_FUNCTION_CALL HOOKED_FUNCTION_CALL;
typedef HOOKED_FUNCTION_CALL *PHOOKED_FUNCTION_CALL;

typedef struct _INVERTED_HOOKED_FUNCTION_CALL INVERTED_HOOKED_FUNCTION_CALL;
typedef INVERTED_HOOKED_FUNCTION_CALL *PINVERTED_HOOKED_FUNCTION_CALL;

typedef VOID (CALLBACK *PHOOK_ENTRY)(
    _In_ PHOOKED_FUNCTION_CALL Call,
    _In_ LARGE_INTEGER Timestamp
    );

typedef VOID (CALLBACK *PHOOK_EXIT)(
    _In_ PHOOKED_FUNCTION_CALL Call,
    _In_ LARGE_INTEGER Timestamp
    );

typedef VOID (CALLBACK *PHOOK_ENTRY_CALLBACK)(
    _In_ PHOOKED_FUNCTION_CALL Call,
    _In_ LARGE_INTEGER Timestamp
    );

typedef VOID (CALLBACK *PHOOK_EXIT_CALLBACK)(
    _In_ PHOOKED_FUNCTION_CALL Call,
    _In_ LARGE_INTEGER Timestamp
    );
/*
typedef VOID (CALLBACK *PHOOK_ENTRY_CALLBACK)(
    _In_ PINVERTED_HOOKED_FUNCTION_CALL Call,
    _In_ LARGE_INTEGER Timestamp
    );

typedef VOID (CALLBACK *PHOOK_EXIT_CALLBACK)(
    _In_ PINVERTED_HOOKED_FUNCTION_CALL Call,
    _In_ LARGE_INTEGER Timestamp
    );
*/

#pragma pack(push, 2)
typedef struct _HOOKED_FUNCTION {
    LARGE_INTEGER Hash;
    PALLOCATION_ROUTINE AllocationRoutine;
    PVOID AllocationContext;
    PRTL Rtl;
    PVOID OriginalAddress;
    PVOID ContinuationAddress;
    PROC HookProlog;
    PHOOK_ENTRY HookEntry;
    PHOOK_ENTRY_CALLBACK EntryCallback;
    PVOID EntryContext;
    PROC HookEpilog;
    PHOOK_EXIT HookExit;
    PHOOK_EXIT_CALLBACK ExitCallback;
    PVOID ExitContext;
    PSTR Name;
    PSTR Module;
    PSTR Signature;
    PSTR NtStyleSignature;
    PVOID Trampoline;
    USHORT NumberOfParameters;
    USHORT SizeOfReturnValueInBytes;
    ULONG Unused2;
} HOOKED_FUNCTION, *PHOOKED_FUNCTION, **PPHOOKED_FUNCTION;
#pragma pack(pop)

typedef struct _HOOKED_FUNCTION_CALL {
    union {
        LARGE_INTEGER ReturnValue;
        LARGE_INTEGER Rax;
    };

    LARGE_INTEGER ExitTimestamp;
    LARGE_INTEGER EntryTimestamp;

    union {
        LARGE_INTEGER RFlags;
        struct {
            ULONG UnusedFlags;
            ULONG EFlags;
        };
    };
    // 8 + 8 + 8 + 8 = 32

    PHOOKED_FUNCTION    HookedFunction;
    PVOID               ReturnAddress;

    union {
        LARGE_INTEGER   HomeRcx;
        LARGE_INTEGER   Param1;
    };

    union {
        LARGE_INTEGER   HomeRdx;
        LARGE_INTEGER   Param2;
    };

    union {
        LARGE_INTEGER   HomeR8;
        LARGE_INTEGER   Param3;
    };

    union {
        LARGE_INTEGER   HomeR9;
        LARGE_INTEGER   Param4;
    };

} HOOKED_FUNCTION_CALL, *PHOOKED_FUNCTION_CALL, **PPHOOKED_FUNCTION_CALL;

typedef struct _INVERTED_HOOKED_FUNCTION_CALL {
    LARGE_INTEGER       HomeR9;
    LARGE_INTEGER       HomeR8;
    LARGE_INTEGER       HomeRdx;
    LARGE_INTEGER       HomeRcx;
    PVOID               ReturnAddress;
    PHOOKED_FUNCTION    HookedFunction;

    union {
        LARGE_INTEGER RFlags;
        struct {
            ULONG UnusedFlags;
            ULONG EFlags;
        };
    };
    // 8 + 8 + 8 + 8 = 32

    LARGE_INTEGER EntryTimestamp;
    LARGE_INTEGER ExitTimestamp;

    union {
        LARGE_INTEGER ReturnValue;
        LARGE_INTEGER Rax;
    };

} INVERTED_HOOKED_FUNCTION_CALL;

typedef INVERTED_HOOKED_FUNCTION_CALL *PINVERTED_HOOKED_FUNCTION_CALL;
typedef INVERTED_HOOKED_FUNCTION_CALL **PPINVERTED_HOOKED_FUNCTION_CALL;


typedef BOOL (*PHOOK)(
    _In_    PRTL  Rtl,
    _Inout_ PVOID *ppSystemFunction,
    _In_    PVOID pHookFunction,
    _In_    PVOID Key
    );

typedef BOOL (*PUNHOOK)(PRTL Rtl, PVOID *ppHookedFunction, PVOID Key);

typedef BOOL (*PHOOK_FUNCTION)(
    _In_    PRTL Rtl,
    _Inout_ PPVOID SystemFunctionPointer,
    _In_    PHOOKED_FUNCTION Function
    );

typedef VOID (*PINITIALIZE_HOOKED_FUNCTION)(
    _In_     PRTL       Rtl,
    _In_     PHOOKED_FUNCTION  Function
    );

RTL_API
BOOL
Hook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key);

RTL_API
BOOL
Mhook_ForceHook(
    _In_    PRTL Rtl,
    _Inout_ PVOID *ppSystemFunction,
    _In_    PVOID pHookFunction,
    _In_    PVOID Key
    );

RTL_API
BOOL
Mhook_SetFunctionHook(
    _In_    PRTL Rtl,
    _Inout_ PPVOID SystemFunctionPointer,
    _In_    PHOOKED_FUNCTION Function
    );

RTL_API
BOOL
Unhook(PRTL Rtl, PVOID *ppHookedFunction, PVOID Key);

RTL_API
BOOL
HookFunction(
    _In_ PRTL Rtl,
    _In_ PPVOID SystemFunctionPointer,
    _In_ PHOOKED_FUNCTION HookedFunction
    );

RTL_API
VOID
HookInit(VOID);

VOID HookProlog(VOID);
VOID HookEpilog(VOID);

ULONGLONG HookOverhead;

typedef VOID (*PVOIDFUNC)(VOID);

/*
typedef struct _FUNCTION {
    PRTL Rtl;
    USHORT NumberOfParameters;
    USHORT Unused1;
    ULONG Key;
    PROC OldAddress;
    PROC NewAddress;
    PVOIDFUNC HookedEntry;
    PVOIDFUNC HookProlog;
    PVOIDFUNC HookEpilog;
    PVOIDFUNC HookedExit;
    ULONG TotalTime;
    ULONG CallCount;
    PSTR Name;
    PSTR Module;
} FUNCTION, *PFUNCTION;

typedef struct _HOOKED_FUNCTION_ENTRY {
    DWORD64 ExitTimestamp;
    DWORD64 EntryTimestamp;

    union {
        DWORD64 RFlags;
        struct {
            DWORD UnusedFlags;
            DWORD EFlags;
        };
    };

    PFUNCTION   Function;
    PVOID       ReturnAddress;

    DWORD64     HomeRcx;
    DWORD64     HomeRdx;
    DWORD64     HomeR8;
    DWORD64     HomeR9;

} HOOKED_FUNCTION_ENTRY, *PHOOKED_FUNCTION_ENTRY;
*/

/*
VOID
WINAPI
HookEntry(PHOOKED_FUNCTION_CALL Entry);

VOID
WINAPI
HookExit(PHOOKED_FUNCTION_CALL Entry);
*/

RTL_API
VOID
WINAPI
HookExit(
    _In_    PHOOKED_FUNCTION_CALL Entry,
    _In_    LARGE_INTEGER Timestamp
    );

RTL_API
VOID
WINAPI
HookExit(
    _In_    PHOOKED_FUNCTION_CALL Entry,
    _In_    LARGE_INTEGER Timestamp
    );


RTL_API
VOID
InitializeHookedFunction(
    _In_     PRTL               Rtl,
    _Inout_  PHOOKED_FUNCTION   HookedFunction
    );


#ifdef __cpplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
