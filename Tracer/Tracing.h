// Python Tools for Visual Studio
// Copyright(c) Microsoft Corporation
// All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the License); you may not use
// this file except in compliance with the License. You may obtain a copy of the
// License at http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY
// IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
//
// See the Apache Version 2.0 License for specific language governing
// permissions and limitations under the License.

#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "stdafx.h"

typedef CHAR *PSZ;
typedef const CHAR *PCSZ;

typedef VOID (WINAPI *PGETSYSTEMTIMEPRECISEASFILETIME)(_Out_ LPFILETIME lpSystemTimeAsFileTime);
typedef LONG (WINAPI *PNTQUERYSYSTEMTIME)(_Out_ PLARGE_INTEGER SystemTime);
typedef NTSTATUS (WINAPI *PRTLCHARTOINTEGER)(_In_ PCSZ String, _In_opt_ ULONG Base, _Out_ PULONG Value);

typedef struct _SYSTEM_TIMER_FUNCTION {
    PGETSYSTEMTIMEPRECISEASFILETIME GetSystemTimePreciseAsFileTime;
    PNTQUERYSYSTEMTIME NtQuerySystemTime;
} SYSTEM_TIMER_FUNCTION, *PSYSTEM_TIMER_FUNCTION, **PPSYSTEM_TIMER_FUNCTION;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef struct _TRACE_EVENT1 {
    USHORT          Version;        //  2   2
    USHORT          EventId;        //  2   4
    DWORD           Flags;          //  4   8
    LARGE_INTEGER   SystemTime;     //  8   16
    DWORD           ProcessId;      //  4   20
    DWORD           ThreadId;       //  4   24
    DWORD_PTR       Event;          //  8   30
    USHORT          Unused;         //  2   32
} TRACE_EVENT1, *PTRACE_EVENT1;

typedef struct _TRACE_EVENT {
    USHORT          Version;                //  2   2
    USHORT          EventType;              //  2   4
    DWORD           ProcessId;              //  4   8
    DWORD           ThreadId;               //  4   12
    DWORD           LineNumber;             //  4   16
    DWORD           LineCount;              //  4   20
    DWORD           SequenceId;             //  4   24
    __declspec(align(8))
    union {
        LARGE_INTEGER   liSystemTime;       //  8   32
        FILETIME        ftSystemTime;
    };
    __declspec(align(8))
    union {
        ULARGE_INTEGER  uliFramePointer;    //  8   40
        ULONGLONG       ullFramePointer;
        DWORD_PTR       FramePointer;
    };
    __declspec(align(8))
    union {
        LARGE_INTEGER   uliModulePointer;   //  8   48
        ULONGLONG       ullModulePointer;
        DWORD_PTR       ModulePointer;
    };
    __declspec(align(8))
    union {
        ULARGE_INTEGER  uliFuncPointer;     //  8   56
        ULONGLONG       ullFuncPointer;
        DWORD_PTR       FuncPointer;
    };
    __declspec(align(8))
    union {
        ULARGE_INTEGER  uliObjPointer;      //  8   64
        ULONGLONG       ullObjPointer;
        DWORD_PTR       ObjPointer;
    };
} TRACE_EVENT, *PTRACE_EVENT;

typedef struct _PYTRACE_CALL {
    USHORT Version;
    DWORD LineNumber;
    USHORT Unused1;
    DWORD_PTR FrameToken;
    DWORD_PTR ModuleToken;
    DWORD_PTR FunctionToken;
    DWORD_PTR LineToken;
} PYTRACE_CALL, *PPYTRACE_CALL;

typedef struct _TRACE_STORE_METADATA {
    ULARGE_INTEGER          NumberOfRecords;
    LARGE_INTEGER           RecordSize;
} TRACE_STORE_METADATA, *PTRACE_STORE_METADATA;

typedef struct _TRACE_STORE TRACE_STORE, *PTRACE_STORE;
typedef struct _TRACE_SESSION TRACE_SESSION, *PTRACE_SESSION;
typedef struct _TRACE_CONTEXT TRACE_CONTEXT, *PTRACE_CONTEXT;

typedef _Check_return_ PVOID (*PALLOCATE_RECORDS)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    ULARGE_INTEGER  RecordSize,
    _In_    ULARGE_INTEGER  NumberOfRecords
);

typedef PVOID (*PGET_ALLOCATION_SIZE)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _Inout_ PULARGE_INTEGER TotalSize,
    _Inout_ PULARGE_INTEGER AllocatedSize
);

typedef struct _TRACE_STORE {
    HANDLE                  FileHandle;
    HANDLE                  MappingHandle;
    LARGE_INTEGER           MappingSize;
    FILE_STANDARD_INFO      FileInfo;
    PCRITICAL_SECTION       CriticalSection;
    PVOID                   BaseAddress;
    PVOID                   NextAddress;
    PTRACE_STORE            MetadataStore;
    PALLOCATE_RECORDS       AllocateRecords;
    union {
        union {
            struct {
                ULARGE_INTEGER  NumberOfRecords;
                LARGE_INTEGER   RecordSize;
            };
            TRACE_STORE_METADATA Metadata;
        };
        PTRACE_STORE_METADATA pMetadata;
    };
} TRACE_STORE, *PTRACE_STORE;

static const LPCWSTR TraceStoreFileNames[] = {
    L"trace_events.dat",
    L"trace_frames.dat",
    L"trace_modules.dat",
    L"trace_functions.dat",
    L"trace_exceptions.dat",
    L"trace_lines.dat",
};

static const PCWSTR TraceStoreMetadataSuffix = L":metadata";
static const DWORD TraceStoreMetadataSuffixLength = (
    sizeof(TraceStoreMetadataSuffix) /
    sizeof(WCHAR)
);

static const DWORD NumberOfTraceStores = (
    sizeof(TraceStoreFileNames) /
    sizeof(LPCWSTR)
);

static const DWORD InitialTraceStoreFileSizes[] = {
    10 << 20,   // events
    10 << 20,   // frames
    10 << 20,   // modules
    10 << 20,   // functions
    10 << 20,   // exceptions
    10 << 20,   // lines
};

typedef struct _TRACE_STORES {
    union {
        TRACE_STORE Stores[sizeof(TraceStoreFileNames)/sizeof(LPCWSTR)];
        struct {
            TRACE_STORE Events;
            TRACE_STORE Frames;
            TRACE_STORE Modules;
            TRACE_STORE Functions;
            TRACE_STORE Exceptions;
            TRACE_STORE Lines;
        };
    };
} TRACE_STORES, *PTRACE_STORES;

typedef struct _PYTRACE_INFO {
    PVOID   FramePyObject;
    LONG    What;
    union {
        PVOID   ArgPyObject;
        PVOID   FunctionPyCodeObject;
    };
    PVOID   ModuleFilenamePyObject;
    PCWSTR  ModuleFilename;
    PCWSTR  ModuleName;
    PCWSTR  FunctionName;
    PCWSTR  Line;
    DWORD   LineNumber;
} PYTRACE_INFO, *PPYTRACE_INFO;

typedef INT (*PPYTRACE_CALLBACK)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PPYTRACE_INFO   TraceInfo
);

typedef VOID (*PREGISTER_NAME_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       NameToken,
    _In_    PCWSTR          Name
);

typedef VOID (*PREGISTER_MODULE_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    PUNICODE_STRING ModuleName
);

// Called for each unique (ModuleToken, FunctionToken, FunctionName).
typedef VOID (*PREGISTER_FUNCTION_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    PCWSTR          FunctionName,
    _In_    LONG            LineNumber
);
//
// Called once for each unique (ModuleToken, ModuleName, ModulePath).
typedef VOID (*PREGISTER_SOURCE_FILE_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    PUNICODE_STRING ModuleName,
    _In_    PUNICODE_STRING ModulePath
);

typedef VOID (*PREGISTER_CALLBACK)(VOID);

typedef struct _REGISTER_TOKEN_CALLBACKS {
    union {
        PREGISTER_CALLBACK Callbacks[3];
        struct {
            PREGISTER_NAME_CALLBACK       RecordName;
            PREGISTER_MODULE_CALLBACK     RecordModule;
            PREGISTER_FUNCTION_CALLBACK   RecordFunction;
        };
    };
} REGISTER_TOKEN_CALLBACKS, *PREGISTER_TOKEN_CALLBACKS;

typedef VOID (*PTRACE_CALL_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    DWORD_PTR       ObjectAddress,
    _In_    LONG            LineNumber
);

typedef VOID (*PTRACE_EXCEPTION_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    LONG            LineNumber,
    _In_    DWORD_PTR       ExceptionAddress
);

typedef VOID (*PTRACE_LINE_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    LONG            LineNumber
);

typedef VOID (*PTRACE_LINE_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    LONG            LineNumber
);

typedef VOID (*PTRACE_RETURN_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    LONG            LineNumber
);

typedef VOID (*PTRACE_C_CALL_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    DWORD_PTR       ObjectAddress,
    _In_    LONG            LineNumber
);

typedef VOID (*PTRACE_C_EXCEPTION_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    LONG            LineNumber,
    _In_    DWORD_PTR       ExceptionAddress
);

typedef VOID (*PTRACE_C_RETURN_CALLBACK)(
    _Inout_ PTRACE_CONTEXT  TraceContext,
    _In_    DWORD_PTR       ModuleToken,
    _In_    DWORD_PTR       FunctionToken,
    _In_    DWORD_PTR       FrameAddress,
    _In_    LONG            LineNumber
);

//typedef VOID (*PPYTRACE_CALLBACK)(VOID);

typedef struct _PYTRACE_CALLBACKS {
    union {
        PPYTRACE_CALLBACK Callbacks[7];
        struct {
            PTRACE_CALL_CALLBACK        TraceCall;
            PTRACE_EXCEPTION_CALLBACK   TraceException;
            PTRACE_LINE_CALLBACK        TraceLine;
            PTRACE_RETURN_CALLBACK      TraceReturn;
            PTRACE_C_CALL_CALLBACK      TraceCCall;
            PTRACE_C_EXCEPTION_CALLBACK TraceCException;
            PTRACE_C_RETURN_CALLBACK    TraceCReturn;
        };
    };
} PYTRACE_CALLBACKS, *PPYTRACE_CALLBACKS;

typedef BOOL (*PPYTRACE)(LPVOID Frame, INT What, LPVOID Arg);

typedef struct _TRACE_SESSION {
    LARGE_INTEGER       SessionId;
    GUID                MachineGuid;
    PISID               Sid;
    PCWSTR              UserName;
    PCWSTR              ComputerName;
    PCWSTR              DomainName;
    FILETIME            SystemTime;
} TRACE_SESSION, *PTRACE_SESSION;

typedef struct _TRACE_CONTEXT {
    DWORD                       Size;
    DWORD                       SequenceId;
    PTRACE_SESSION              TraceSession;
    PTRACE_STORES               TraceStores;
    PPYTRACE_CALLBACK           TraceCallback;
    PSYSTEM_TIMER_FUNCTION      SystemTimerFunction;
} TRACE_CONTEXT, *PTRACE_CONTEXT;

_Check_return_
TRACER_API
BOOL
CallSystemTimer(
    _Out_       PFILETIME               SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
);

TRACER_API
BOOL
InitializeTraceStores(
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PDWORD          SizeOfTraceStores,
    _In_opt_    PDWORD          InitialFileSizes
);

TRACER_API
BOOL
InitializeTraceContext(
    _Inout_bytecap_(*SizeOfTraceContext) PTRACE_CONTEXT TraceContext,
    _In_    PDWORD              SizeOfTraceContext,
    _In_    PTRACE_SESSION      TraceSession,
    _In_    PTRACE_STORES       TraceStores,
    _In_    PPYTRACE_CALLBACK   TraceCallback
);

TRACER_API
VOID
CloseTraceStore(PTRACE_STORE TraceStore);

TRACER_API
VOID
CloseTraceStores(PTRACE_STORES TraceStores);

TRACER_API
DWORD
GetTraceStoresAllocationSize();

TRACER_API
BOOL
GetTraceStoreBytesWritten(
    PTRACE_STORE TraceStore,
    PULARGE_INTEGER BytesWritten
);

TRACER_API
BOOL
GetTraceStoreNumberOfRecords(
    PTRACE_STORE TraceStore,
    PULARGE_INTEGER NumberOfRecords
);

TRACER_API
LPVOID
GetNextRecord(
    PTRACE_CONTEXT TraceContext,
    PTRACE_STORE TraceStore,
    ULARGE_INTEGER RecordSize
);

TRACER_API
LPVOID
GetNextRecords(
    PTRACE_STORE TraceStore,
    ULARGE_INTEGER RecordSize,
    ULARGE_INTEGER RecordCount
);

TRACER_API
VOID
RegisterName(
    _Inout_     PTRACE_CONTEXT  TraceContext,
    _In_        DWORD_PTR       NameToken,
    _In_        PCWSTR          Name
);

TRACER_API
VOID
RegisterFunction(
    _Inout_     PTRACE_CONTEXT  TraceContext,
    _In_        DWORD_PTR       FunctionToken,
    _In_        PCWSTR          FunctionName,
    _In_        DWORD           LineNumber,
    _In_opt_    DWORD_PTR       ModuleToken,
    _In_opt_    PCWSTR          ModuleName,
    _In_opt_    PCWSTR          ModuleFilename
);

TRACER_API
VOID
RegisterModule(
    _Inout_     PTRACE_CONTEXT  TraceContext,
    _In_        DWORD_PTR       ModuleToken,
    _In_        PCWSTR          ModuleName,
    _In_        PCWSTR          ModuleFilename
);

#ifdef __cpp
} // extern "C"
#endif
