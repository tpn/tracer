/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Commandline.h

Abstract:

    This is the header file for the Rtl commandline component.

--*/

#pragma once

#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef
PPWSTR
(COMMAND_LINE_TO_ARGVW)(
    _In_  PWSTR  CommandLine,
    _Out_ PLONG  NumberOfArgs
    );
typedef COMMAND_LINE_TO_ARGVW *PCOMMAND_LINE_TO_ARGVW;

typedef PWSTR (WINAPI GET_COMMAND_LINE)(VOID);
typedef GET_COMMAND_LINE *PGET_COMMAND_LINE;

typedef PPSTR *PPPSTR;

typedef
_Success_(return != 0)
BOOL
(ARGVW_TO_ARGVA)(
    _In_ PPWSTR ArgvW,
    _In_ ULONG NumberOfArguments,
    _Out_ PPPSTR ArgvA,
    _In_opt_ PSTR Argv0,
    _In_ PALLOCATOR Allocator
    );
typedef ARGVW_TO_ARGVA *PARGVW_TO_ARGVA;
RTL_API ARGVW_TO_ARGVA ArgvWToArgvA;

#define MAX_ARGV 100

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
