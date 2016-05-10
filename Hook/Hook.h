// Copyright(c) Trent Nelson <trent@trent.me>
// All rights reserved.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include <Windows.h>
#include "../Rtl/Rtl.h"

typedef struct _FUNCTION FUNCTION, *PFUNCTION;

typedef BOOL (*PHOOK)(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key);
typedef BOOL (*PUNHOOK)(PRTL Rtl, PVOID *ppHookedFunction, PVOID Key);

typedef BOOL (*PHOOK_FUNCTION)(PRTL Rtl, PFUNCTION Function);

typedef VOID (*PINITIALIZE_FUNCTION)(
    _In_     PRTL       Rtl,
    _In_     PFUNCTION  Function
    );

RTL_API
BOOL
Hook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key);

RTL_API
BOOL
Mhook_ForceHook(PRTL Rtl, PVOID *ppSystemFunction, PVOID pHookFunction, PVOID Key);

RTL_API
BOOL
Unhook(PRTL Rtl, PVOID *ppHookedFunction, PVOID Key);

RTL_API
BOOL
HookFunction(PRTL Rtl, PFUNCTION Function);

RTL_API
VOID
HookInit(VOID);

VOID HookProlog(VOID);
VOID HookEpilog(VOID);

ULONGLONG HookOverhead;

typedef VOID (*PVOIDFUNC)(VOID);

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

VOID
WINAPI
HookEntry(PHOOKED_FUNCTION_ENTRY Entry);

VOID
WINAPI
HookExit(PHOOKED_FUNCTION_ENTRY Entry);


typedef struct _HOOKED_FUNCTION {
    PROC    OriginalFunction;
    PVOID   Key;

} HOOKED_FUNCTION, *PHOOKED_FUNCTION;

RTL_API
VOID
InitializeFunction(
    _In_     PRTL       Rtl,
    _Inout_  PFUNCTION  Function
    );


#ifdef __cpplus
} // extern "C"
#endif
