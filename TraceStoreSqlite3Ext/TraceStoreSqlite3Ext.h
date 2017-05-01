/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoreSqlite3Ext.h

Abstract:

    This is the main header file for the TraceStore sqlite3 extension component.
    It defines structures and functions related to the implementation of the
    component.

--*/

#pragma once

#ifdef _TRACE_STORE_SQLITE3_EXT_INTERNAL_BUILD

//
// This is an internal build of the TracerConfig component.
//

#ifdef _TRACE_STORE_SQLITE3_EXT_DLL_BUILD

//
// This is the DLL build.
//

#define TRACE_STORE_SQLITE3_EXT_API __declspec(dllexport)
#define TRACE_STORE_SQLITE3_EXT_DATA extern __declspec(dllexport)

#else

//
// This is the static library build.
//

#define TRACE_STORE_SQLITE3_EXT_API
#define TRACE_STORE_SQLITE3_EXT_DATA extern

#endif

#include "stdafx.h"

#else

//
// We're being included by another project.
//

#ifdef _TRACE_STORE_SQLITE3_EXT_STATIC_LIB

//
// We're being included by another project as a static library.
//

#define TRACE_STORE_SQLITE3_EXT_API
#define TRACE_STORE_SQLITE3_EXT_DATA extern

#else

//
// We're being included by another project that wants to use us as a DLL.
//

#define TRACE_STORE_SQLITE3_EXT_API __declspec(dllimport)
#define TRACE_STORE_SQLITE3_EXT_DATA extern __declspec(dllimport)

#endif

#include "../Rtl/Rtl.h"
#include "../Rtl/Sqlite.h"
#include "../TracerConfig/TracerConfig.h"
#include "../TraceStore/TraceStore.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif


typedef
LONG
(TRACE_STORE_SQLITE3_EXT_INIT)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ PTRACER_CONFIG TracerConfig,
    _In_ PSQLITE3_DB Database,
    _In_ PCSZ *ErrorMessagePointer,
    _In_ PCSQLITE3 Sqlite3
    );
typedef TRACE_STORE_SQLITE3_EXT_INIT *PTRACE_STORE_SQLITE3_EXT_INIT;


#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
