#pragma once

#include "stdafx.h"
#include "Memory.h"

typedef PPWSTR (COMMAND_LINE_TO_ARGVW)(
    _In_  PWSTR  CommandLine,
    _Out_ PLONG  NumberOfArgs
);
typedef COMMAND_LINE_TO_ARGVW *PCOMMAND_LINE_TO_ARGVW;

typedef PWSTR (WINAPI GET_COMMAND_LINE)(VOID);
typedef GET_COMMAND_LINE *PGET_COMMAND_LINE;

typedef PPSTR *PPPSTR;

typedef
_Success_(return != 0)
BOOLEAN
(ARGVW_TO_ARGVA)(
    _In_ PPWSTR ArgvW,
    _In_ ULONG NumberOfArguments,
    _Out_ PPPSTR ArgvA,
    _In_opt_ PSTR Argv0,
    _In_ PALLOCATOR Allocator
    );
typedef ARGVW_TO_ARGVA *PARGVW_TO_ARGVA;

#define MAX_ARGV 100

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
