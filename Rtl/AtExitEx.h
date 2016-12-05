/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    AtExitEx.h

Abstract:

    This module provides the necessary scaffolding to use the AtExitEx()
    function without needing to have any knowledge of the Rtl library.

    To make use of this functionality, add this file and AtExitEx.c to your
    project.  For components loaded via the TracedPythonSession component,
    SetAtExitEx() will be called automatically as soon as the component has
    been dynamically loaded (via LoadLibrary()).

--*/

#ifdef _NO_RTL

typedef struct _Struct_size_bytes_(sizeof(ULONG)) _ATEXITEX_FLAGS {
    ULONG SuppressExceptions:1;
} ATEXITEX_FLAGS, *PATEXITEX_FLAGS;
C_ASSERT(sizeof(ATEXITEX_FLAGS) == sizeof(ULONG));

typedef
VOID
(CALLBACK ATEXITEX_CALLBACK)(
    _In_ BOOL IsProcessTerminating,
    _In_opt_ PVOID Context
    );
typedef ATEXITEX_CALLBACK *PATEXITEX_CALLBACK;

typedef
_Success_(return == 0)
BOOL
(ATEXITEX)(
    _In_ PATEXITEX_CALLBACK Callback,
    _In_opt_ PATEXITEX_FLAGS Flags,
    _In_opt_ PVOID Context,
    _Outptr_opt_result_nullonfailure_ struct _RTL_ATEXIT_ENTRY **EntryPointer
    );
typedef ATEXITEX *PATEXITEX;

typedef
VOID
(SET_ATEXITEX)(
    _In_ PATEXITEX AtExitEx
    );
typedef SET_ATEXITEX *PSET_ATEXITEX;

#endif

extern PATEXITEX AtExitExImpl;
__declspec(dllexport) SET_ATEXITEX SetAtExitEx;

#pragma warning(push)
#pragma warning(disable: 4028 4273)

ATEXITEX AtExitEx;

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
