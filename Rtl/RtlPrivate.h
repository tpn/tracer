/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    RtlPrivate.h

Abstract:

    This is the private header file for the Rtl component.  It defines function
    typedefs and function declarations for all major (i.e. not local to the
    module) functions available for use by individual modules within this
    component.

--*/

#ifndef _RTL_INTERNAL_BUILD
#error RtlPrivate.h being included but _RTL_INTERNAL_BUILD not set.
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// RtlAtExitRundown-related structures and functions.
//

typedef struct _RTL_ATEXIT_RUNDOWN_FLAGS {
    ULONG IsActive:1;
} RTL_ATEXIT_RUNDOWN_FLAGS, *PRTL_ATEXIT_RUNDOWN_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_ATEXIT_RUNDOWN {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_ATEXIT_RUNDOWN)) USHORT SizeOfStruct;

    //
    // Pad out to 4-bytes.
    //

    USHORT Padding1;

    //
    // Flags.
    //

    RTL_ATEXIT_RUNDOWN_FLAGS Flags;

    //
    // Critical section protecting the rundown list head and heap handle.
    //

    CRITICAL_SECTION CriticalSection;

    //
    // Rundown list head.
    //

    _Guarded_by_(CriticalSection)
    LIST_ENTRY ListHead;

    //
    // Heap handle used for allocating RTL_ATEXIT_ENTRY structures.
    //

    HANDLE HeapHandle;

} RTL_ATEXIT_RUNDOWN, *PRTL_ATEXIT_RUNDOWN;

//
// Define entry bitmap flags.
//
// N.B. This bitmap is intentionally different from (and cannot be used
//      interchangeably with) the public ATEXITEX_FLAGS type.
//

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _RTL_ATEXIT_ENTRY_FLAGS {

    //
    // When set, indicates this is an extended atexit entry.  That is, the
    // entry was registered via AtExitEx() instead of atexit().
    //

    ULONG IsExtended:1;

    //
    // The following flags are only applicable if IsExtended is set.
    //

    //
    // When set, indicates the caller provided a Context to be included in
    // the callback invocation.
    //

    ULONG HasContext:1;

    //
    // When set, indicates that the caller's atexit() function will be wrapped
    // in a __try/__except SEH block that suppresses all exceptions.
    //

    ULONG SuppressExceptions:1;

} RTL_ATEXIT_ENTRY_FLAGS, *PRTL_ATEXIT_ENTRY_FLAGS;
C_ASSERT(sizeof(RTL_ATEXIT_ENTRY_FLAGS) == sizeof(ULONG));

//
// The following structure is used to encapsulate a caller's atexit function
// pointer within a structure that can be added to the rundown list via the
// standard doubly-linked list facilities.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_ATEXIT_ENTRY {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_ATEXIT_RUNDOWN)) USHORT SizeOfStruct;

    //
    // Pad out to 4 bytes.
    //

    USHORT Padding1;

    //
    // Flags for this entry.
    //

    RTL_ATEXIT_ENTRY_FLAGS Flags;

    //
    // Pointer to the caller's function to be called at exit.  If IsExtended
    // flag is set, the function pointer will be treated as an ATEXITEX_CALLBACK
    // function, ATEXITFUNC otherwise.
    //

    union {
        PATEXITFUNC AtExitFunc;
        PATEXITEX_CALLBACK AtExitExCallback;
    };

    //
    // List entry to allow the structure to be registered with the ListHead
    // field of the RTL_ATEXIT_RUNDOWN structure.
    //

    LIST_ENTRY ListEntry;

    //
    // Pointer to the rundown structure we were added to.
    //

    PRTL_ATEXIT_RUNDOWN Rundown;

    //
    // Optional context to be passed back to the extended version of the atexit
    // function.
    //

    PVOID Context;

    //
    // Pad out to 64 bytes.  (Currently at 48 bytes after Context.)
    //

    ULONGLONG Padding2[2];

} RTL_ATEXIT_ENTRY, *PRTL_ATEXIT_ENTRY, **PPRTL_ATEXIT_ENTRY;
C_ASSERT(sizeof(RTL_ATEXIT_ENTRY) == 64);

//
// RtlAtExitRundown-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
_Requires_lock_held_(Rundown->CriticalSection)
BOOL
(CREATE_RTL_ATEXIT_ENTRY)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_ PATEXITFUNC AtExitFunc,
    _Outptr_result_nullonfailure_ PPRTL_ATEXIT_ENTRY AtExitEntryPointer
    );
typedef CREATE_RTL_ATEXIT_ENTRY *PCREATE_RTL_ATEXIT_ENTRY;
extern CREATE_RTL_ATEXIT_ENTRY CreateRtlAtExitEntry;

typedef
_Check_return_
_Success_(return != 0)
_Requires_lock_held_(Rundown->CriticalSection)
BOOL
(CREATE_RTL_ATEXITEX_ENTRY)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_ PATEXITEX_CALLBACK Callback,
    _In_opt_ PATEXITEX_FLAGS Flags,
    _In_opt_ PVOID Context,
    _Outptr_result_nullonfailure_ PPRTL_ATEXIT_ENTRY AtExitEntryPointer
    );
typedef CREATE_RTL_ATEXITEX_ENTRY *PCREATE_RTL_ATEXITEX_ENTRY;
extern CREATE_RTL_ATEXITEX_ENTRY CreateRtlAtExitExEntry;

typedef
_Requires_lock_not_held_(Rundown->CriticalSection)
VOID
(RUNDOWN_ATEXIT_FUNCTIONS)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_opt_ BOOL IsProcessTerminating
    );
typedef RUNDOWN_ATEXIT_FUNCTIONS *PRUNDOWN_ATEXIT_FUNCTIONS;
extern RUNDOWN_ATEXIT_FUNCTIONS RundownAtExitFunctions;

typedef
BOOL
(IS_RTL_ATEXIT_RUNDOWN_ACTIVE)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown
    );
typedef IS_RTL_ATEXIT_RUNDOWN_ACTIVE *PIS_RTL_ATEXIT_RUNDOWN_ACTIVE;
extern IS_RTL_ATEXIT_RUNDOWN_ACTIVE IsRtlAtExitRundownActive;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_RTL_ATEXIT_RUNDOWN)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown
    );
typedef INITIALIZE_RTL_ATEXIT_RUNDOWN \
      *PINITIALIZE_RTL_ATEXIT_RUNDOWN;
extern INITIALIZE_RTL_ATEXIT_RUNDOWN InitializeRtlAtExitRundown;

typedef
VOID
(DESTROY_RTL_ATEXIT_RUNDOWN)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown
    );
typedef DESTROY_RTL_ATEXIT_RUNDOWN \
      *PDESTROY_RTL_ATEXIT_RUNDOWN;
extern DESTROY_RTL_ATEXIT_RUNDOWN DestroyRtlAtExitRundown;

typedef
_Requires_lock_held_(Rundown->CriticalSection)
VOID
(ADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_ PRTL_ATEXIT_ENTRY AtExitEntry
    );
typedef ADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN *PADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN;
extern ADD_RTL_ATEXIT_ENTRY_TO_RUNDOWN AddRtlAtExitEntryToRundown;

typedef
_Requires_lock_held_(AtExitEntry->Rundown->CriticalSection)
VOID
(REMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN)(
    _In_ PRTL_ATEXIT_ENTRY AtExitEntry
    );
typedef REMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN \
      *PREMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN;
extern REMOVE_RTL_ATEXIT_ENTRY_FROM_RUNDOWN RemoveRtlAtExitEntryFromRundown;

typedef
_Success_(return != 0)
_Check_return_
_Requires_lock_not_held_(Rundown->CriticalSection)
BOOL
(REGISTER_ATEXITFUNC)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_ PATEXITFUNC AtExitFunc
    );
typedef REGISTER_ATEXITFUNC *PREGISTER_ATEXITFUNC;
extern REGISTER_ATEXITFUNC RegisterAtExitFunc;

typedef
_Success_(return != 0)
_Check_return_
_Requires_lock_not_held_(Rundown->CriticalSection)
BOOL
(REGISTER_ATEXITEX_CALLBACK)(
    _In_ PRTL_ATEXIT_RUNDOWN Rundown,
    _In_ PATEXITEX_CALLBACK Callback,
    _In_opt_ PATEXITEX_FLAGS Flags,
    _In_opt_ PVOID Context,
    _Outptr_opt_result_nullonfailure_ PPRTL_ATEXIT_ENTRY EntryPointer
    );
typedef REGISTER_ATEXITEX_CALLBACK *PREGISTER_ATEXITEX_CALLBACK;
extern REGISTER_ATEXITEX_CALLBACK RegisterAtExitExCallback;

//
// RtlGlobalAtExitRundown-related functions.
//

typedef
PRTL_ATEXIT_RUNDOWN
(GET_GLOBAL_RTL_ATEXIT_RUNDOWN)(
    VOID
    );
typedef GET_GLOBAL_RTL_ATEXIT_RUNDOWN *PGET_GLOBAL_RTL_ATEXIT_RUNDOWN;
extern GET_GLOBAL_RTL_ATEXIT_RUNDOWN GetGlobalRtlAtExitRundown;

typedef
BOOL
(IS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE)(
    VOID
    );
typedef IS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE \
      *PIS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE;
extern IS_GLOBAL_RTL_ATEXIT_RUNDOWN_ACTIVE IsGlobalRtlAtExitRundownActive;

//
// N.B. RegisterGlobalAtExitFunc will be the atexit() endpoint at runtime.
//

typedef
_Success_(return != 0)
_Check_return_
BOOL
(REGISTER_GLOBAL_ATEXITFUNC)(
    _In_ PATEXITFUNC AtExitFunc
    );
typedef REGISTER_GLOBAL_ATEXITFUNC *PREGISTER_GLOBAL_ATEXITFUNC;
extern REGISTER_GLOBAL_ATEXITFUNC RegisterGlobalAtExitFunc;


//
// N.B. RegisterGlobalAtExitExCallback function will be the AtExitEx() endpoint
//      at runtime.
//

typedef
_Success_(return != 0)
_Check_return_
BOOL
(REGISTER_GLOBAL_ATEXITEX_CALLBACK)(
    _In_ PATEXITEX_CALLBACK Callback,
    _In_opt_ PATEXITEX_FLAGS Flags,
    _In_opt_ PVOID Context,
    _Outptr_opt_result_nullonfailure_ PPRTL_ATEXIT_ENTRY EntryPointer
    );
typedef REGISTER_GLOBAL_ATEXITEX_CALLBACK *PREGISTER_GLOBAL_ATEXIT_CALLBACK;
extern REGISTER_GLOBAL_ATEXITEX_CALLBACK RegisterGlobalAtExitExCallback;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN)(
    VOID
    );
typedef INITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN \
      *PINITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN;
extern INITIALIZE_GLOBAL_RTL_ATEXIT_RUNDOWN InitializeGlobalRtlAtExitRundown;

typedef
VOID
(DESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN)(
    VOID
    );
typedef DESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN \
      *PDESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN;
extern DESTROY_GLOBAL_RTL_ATEXIT_RUNDOWN DestroyGlobalRtlAtExitRundown;

//
// Loader-related structures and functions.
//

typedef struct _RTL_LDR_NOTIFICATION_TABLE_FLAGS {
    ULONG IsRegistered:1;
} RTL_LDR_NOTIFICATION_TABLE_FLAGS, *PRTL_LDR_NOTIFICATION_TABLE_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_LDR_NOTIFICATION_TABLE {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_LDR_NOTIFICATION_TABLE))
        USHORT SizeOfStruct;

    //
    // Number of loader notifications registered.  When this hits 0, our thunk
    // loader notification callback will be unregistered.
    //

    _Guarded_by_(CriticalSection)
    USHORT NumberOfEntries;

    //
    // Flags.
    //

    RTL_LDR_NOTIFICATION_TABLE_FLAGS Flags;

    //
    // Critical section protecting the rundown list head and heap handle.
    //

    CRITICAL_SECTION CriticalSection;

    //
    // Rundown list head.
    //

    _Guarded_by_(CriticalSection)
    LIST_ENTRY ListHead;

    //
    // Heap handle used for allocating RTL_LDR_NOTIFICATION_ENTRY structures.
    //

    _Guarded_by_(CriticalSection)
    HANDLE HeapHandle;

    //
    // Pointer to the RTL structure that owns us.
    //

    PRTL Rtl;

    //
    // Pointer to our thunk loader notification callback that is used to drive
    // registered callbacks.
    //

    PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction;

    //
    // Cookie provided to us by LdrRegisterDllNotification().
    //

    PVOID Cookie;

} RTL_LDR_NOTIFICATION_TABLE, *PRTL_LDR_NOTIFICATION_TABLE;

typedef
struct
_Struct_size_bytes_(sizeof(ULONG))
_RTL_LDR_NOTIFICATION_ENTRY_FLAGS {
    ULONG Unused:1;
} RTL_LDR_NOTIFICATION_ENTRY_FLAGS, *PRTL_LDR_NOTIFICATION_ENTRY_FLAGS;
C_ASSERT(sizeof(RTL_LDR_NOTIFICATION_ENTRY_FLAGS) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(SizeOfStruct) _RTL_LDR_NOTIFICATION_ENTRY {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _RTL_LDR_NOTIFICATION_ENTRY))
        USHORT SizeOfStruct;

    //
    // Pad out to 4 bytes.
    //

    USHORT Padding1;

    //
    // Flags for this entry.
    //

    RTL_LDR_NOTIFICATION_ENTRY_FLAGS Flags;

    //
    // Pointer to the caller's loader DLL notification callback routine.
    //

    PDLL_NOTIFICATION_CALLBACK NotificationCallback;

    //
    // List entry to allow the structure to be registered with the ListHead
    // field of the RTL_LDR_NOTIFICATION_ENTRY structure.
    //

    LIST_ENTRY ListEntry;

    //
    // Pointer to the rundown structure we were added to.
    //

    PRTL_LDR_NOTIFICATION_TABLE NotificationTable;

    //
    // Optional context to be passed back to the caller when invoking their
    // callback.
    //

    PVOID Context;

    //
    // Pad out to 64 bytes.  (Currently at 48 bytes after Context.)
    //

    ULONGLONG Padding2[2];

} RTL_LDR_NOTIFICATION_ENTRY, *PRTL_LDR_NOTIFICATION_ENTRY;
typedef RTL_LDR_NOTIFICATION_ENTRY **PPRTL_LDR_NOTIFICATION_ENTRY;
C_ASSERT(sizeof(RTL_LDR_NOTIFICATION_ENTRY) == 64);

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_RTL_LDR_NOTIFICATION_TABLE)(
    _In_ PRTL Rtl,
    _In_ PRTL_LDR_NOTIFICATION_TABLE NotificationTable
    );
typedef INITIALIZE_RTL_LDR_NOTIFICATION_TABLE \
      *PINITIALIZE_RTL_LDR_NOTIFICATION_TABLE;
INITIALIZE_RTL_LDR_NOTIFICATION_TABLE InitializeRtlLdrNotificationTable;

typedef
VOID
(DESTROY_RTL_LDR_NOTIFICATION_TABLE)(
    _In_ PRTL_LDR_NOTIFICATION_TABLE NotificationTable
    );
typedef DESTROY_RTL_LDR_NOTIFICATION_TABLE \
      *PDESTROY_RTL_LDR_NOTIFICATION_TABLE;
DESTROY_RTL_LDR_NOTIFICATION_TABLE DestroyRtlLdrNotificationTable;

typedef
_Check_return_
_Success_(return != 0)
_Requires_lock_held_(NotificationTable->CriticalSection)
BOOL
(CREATE_RTL_LDR_NOTIFICATION_ENTRY)(
    _In_ PRTL_LDR_NOTIFICATION_TABLE NotificationTable,
    _In_ PDLL_NOTIFICATION_CALLBACK NotificationCallback,
    _In_opt_ PDLL_NOTIFICATION_FLAGS Flags,
    _In_opt_ PVOID Context,
    _Outptr_result_nullonfailure_
        PPRTL_LDR_NOTIFICATION_ENTRY NotificationEntryPointer
    );
typedef CREATE_RTL_LDR_NOTIFICATION_ENTRY \
      *PCREATE_RTL_LDR_NOTIFICATION_ENTRY;
CREATE_RTL_LDR_NOTIFICATION_ENTRY CreateRtlLdrNotificationEntry;

typedef
_Requires_lock_held_(NotificationTable->CriticalSection)
BOOL
(REMOVE_RTL_LDR_NOTIFICATION_ENTRY)(
    _In_ PRTL_LDR_NOTIFICATION_ENTRY NotificationEntry
    );
typedef REMOVE_RTL_LDR_NOTIFICATION_ENTRY \
      *PREMOVE_RTL_LDR_NOTIFICATION_ENTRY;
REMOVE_RTL_LDR_NOTIFICATION_ENTRY RemoveRtlLdrNotificationEntry;

//
// This is the function we register with LdrRegisterDllNotification().
//

LDR_DLL_NOTIFICATION_FUNCTION LdrDllNotificationFunction;

//
// Symbol loading related typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RESOLVE_RTL_FUNCTIONS)(
    _Inout_ PRTL Rtl
    );
typedef RESOLVE_RTL_FUNCTIONS *PRESOLVE_RTL_FUNCTIONS;
RESOLVE_RTL_FUNCTIONS ResolveRtlFunctions;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RESOLVE_RTL_EX_FUNCTIONS)(
    _In_     PRTL Rtl,
    _In_     HMODULE RtlExModule,
    _Inout_  PRTLEXFUNCTIONS RtlExFunctions
    );
typedef RESOLVE_RTL_EX_FUNCTIONS *PRESOLVE_RTL_EX_FUNCTIONS;
RESOLVE_RTL_EX_FUNCTIONS ResolveRtlExFunctions;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(RESOLVE_DBGHELP_FUNCTIONS)(
    _In_     PRTL Rtl,
    _In_     HMODULE DbgHelpModule,
    _Inout_  PDBG Dbg
    );
typedef RESOLVE_DBGHELP_FUNCTIONS *PRESOLVE_DBGHELP_FUNCTIONS;
RESOLVE_DBGHELP_FUNCTIONS ResolveDbgHelpFunctions;

//
// Test-related glue.
//

#ifdef _RTL_TEST
RTL_API TEST_LOAD_SYMBOLS TestLoadSymbols;
RTL_API TEST_LOAD_SYMBOLS_FROM_MULTIPLE_MODULES TestLoadSymbolsFromMultipleModules;
#endif

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
