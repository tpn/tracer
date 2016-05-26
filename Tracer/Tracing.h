#pragma once

#ifdef __cpplus
extern "C" {
#endif

#include "TraceStoreIndex.h"

#include "stdafx.h"

#define TIMESTAMP_TO_SECONDS    1000000
#define SECONDS_TO_MICROSECONDS 1000000

#define MAX_UNICODE_STRING 255
#define _OUR_MAX_PATH MAX_UNICODE_STRING

typedef struct _TRACE_STORE_ALLOCATION {
    union {
        ULARGE_INTEGER  NumberOfRecords;
        ULARGE_INTEGER  NumberOfAllocations;
    };
    union {
        LARGE_INTEGER   RecordSize;
        ULARGE_INTEGER  AllocationSize;
    };
} TRACE_STORE_ALLOCATION, *PTRACE_STORE_ALLOCATION;

typedef struct _TRACE_STORE_ADDRESS {
    PVOID         PreferredBaseAddress;                     // 8    0   8
    PVOID         BaseAddress;                              // 8    8   16
    LARGE_INTEGER FileOffset;                               // 8    16  24
    LARGE_INTEGER MappedSize;                               // 8    24  32
    DWORD         ProcessId;                                // 4    32  36
    DWORD         RequestingThreadId;                       // 4    36  40

    //
    // Timestamps are kept at each stage of the memory map's lifecycle.  They
    // are used to calculate the elapsed times (in microseconds) below.  The
    // LARGE_INTEGER structs are passed to QueryPerformanceCounter(&Counter).
    //

    struct {

        //
        // The time where the new memory map was pushed onto the "prepare
        // memory map" threadpool work queue.
        //

        LARGE_INTEGER Requested;                            // 8    40  48

        //
        // The time the memory map was pushed onto the "next memory map
        // available" list.
        //

        LARGE_INTEGER Prepared;                             // 8    48  56

        //
        // The time the memory map was popped off the available list and put
        // into active use.
        //

        LARGE_INTEGER Consumed;                             // 8    56  64

        //
        // The time the memory map was pushed onto the "release memory map"
        // queue.
        //

        LARGE_INTEGER Retired;                              // 8    64  72

        //
        // The time the memory map had been released; this will be after the
        // view has been flushed and the memory map handle has been closed.
        //

        LARGE_INTEGER Released;                             // 8    72  80

    } Timestamp;

    //
    // Elapsed timestamps that capture the time the memory map spent in various
    // states.
    //

    struct {

        //
        // Time between Requested and Prepared.
        //

        LARGE_INTEGER AwaitingPreparation;                  // 8    80  88


        //
        // Time between Prepared and Consumed.
        //

        LARGE_INTEGER AwaitingConsumption;                  // 8    88  96

        //
        // Time between Consumed and Retired.
        //

        LARGE_INTEGER Active;                               // 8    96  104

        //
        // Time between Retired and Released.
        //

        LARGE_INTEGER AwaitingRelease;                      // 8    104 112

    } Elapsed;

    LONG MappedSequenceId;                                  // 4    112 116

    union {
        PROCESSOR_NUMBER RequestingProcessor;               // 4    116 120
        struct {
            WORD RequestingProcGroup;
            BYTE RequestingProcNumber;
            union {
                BYTE RequestingProcReserved;
                UCHAR RequestingNumaNode;
            };
        };
    };

    union {
        PROCESSOR_NUMBER FulfillingProcessor;               // 4    120 124
        struct {
            WORD FulfillingProcGroup;
            BYTE FulfillingProcNumber;
            union {
                BYTE FulfillingProcReserved;
                UCHAR FulfillingNumaNode;
            };
        };
    };

    DWORD FulfillingThreadId;                               // 4    124 128

} TRACE_STORE_ADDRESS, *PTRACE_STORE_ADDRESS, **PPTRACE_STORE_ADDRESS;

C_ASSERT(sizeof(TRACE_STORE_ADDRESS) == 128);

typedef struct _TRACE_STORE_EOF {
    LARGE_INTEGER EndOfFile;
} TRACE_STORE_EOF, *PTRACE_STORE_EOF, **PPTRACE_STORE_EOF;

typedef struct _TRACE_STORE_TIME {

    //
    // The value of QueryPerformanceFrequency().
    //

    LARGE_INTEGER   Frequency;

    //
    // The multiplicand used when calculating elapsed time from the performance
    // counter and frequency using the MSDN-recommended approach:
    //
    //      Elapsed.QuadPart *= Multiplicand.QuadPart;
    //      Elapsed.QuadPart /= Frequency.QuadPart;
    //
    //

    LARGE_INTEGER   Multiplicand;

    //
    // The time the store had its first record allocated, plus a corresponding
    // performance counter taken just after the time is recorded.
    //

    struct {
        FILETIME        FileTimeUtc;
        FILETIME        FileTimeLocal;
        SYSTEMTIME      SystemTimeUtc;
        SYSTEMTIME      SystemTimeLocal;
        ULARGE_INTEGER  SecondsSince1970;
        ULARGE_INTEGER  MicrosecondsSince1970;
    } StartTime;

    LARGE_INTEGER   StartCounter;

} TRACE_STORE_TIME, *PTRACE_STORE_TIME, **PPTRACE_STORE_TIME;

typedef struct _TRACE_STORE_STATS {
    ULONG DroppedRecords;
    ULONG ExhaustedFreeMemoryMaps;
    ULONG AllocationsOutpacingNextMemoryMapPreparation;
    ULONG PreferredAddressUnavailable;
} TRACE_STORE_STATS, *PTRACE_STORE_STATS, **PPTRACE_STORE_STATS;

typedef struct _TRACE_STORE_INFO {

    //
    // Inline TRACE_STORE_EOF.
    //

    union {
        TRACE_STORE_EOF Eof;
        struct {
            LARGE_INTEGER EndOfFile;
        };
    };

    //
    // Inline TRACE_STORE_TIME.
    //

    union {
        TRACE_STORE_TIME Time;
        struct {
            FILETIME        FileTimeUtc;
            FILETIME        FileTimeLocal;
            SYSTEMTIME      SystemTimeUtc;
            SYSTEMTIME      SystemTimeLocal;
            ULARGE_INTEGER  SecondsSince1970;
            ULARGE_INTEGER  MicrosecondsSince1970;
        };
    };

    //
    // Inline TRACE_STORE_STATS.
    //

    union {
        TRACE_STORE_STATS Stats;
        struct {
            ULONG DroppedRecords;
            ULONG ExhaustedFreeMemoryMaps;
            ULONG AllocationsOutpacingNextMemoryMapPreparation;
            ULONG PreferredAddressUnavailable;
        };
    };

} TRACE_STORE_INFO, *PTRACE_STORE_INFO, **PPTRACE_STORE_INFO;

typedef struct _TRACE_STORE TRACE_STORE, *PTRACE_STORE;
typedef struct _TRACE_SESSION TRACE_SESSION, *PTRACE_SESSION;
typedef struct _TRACE_CONTEXT TRACE_CONTEXT, *PTRACE_CONTEXT;

typedef _Check_return_ PVOID (*PALLOCATE_RECORDS)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
);

typedef _Check_return_ VOID (*PFREE_RECORDS)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PVOID           Buffer
);

typedef BOOL (*PGET_ALLOCATION_SIZE)(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _Inout_ PULARGE_INTEGER TotalSize,
    _Inout_ PULARGE_INTEGER AllocatedSize
);

typedef struct _TRACE_STORE_THREADPOOL {
    PTP_POOL Threadpool;
    TP_CALLBACK_ENVIRON CallbackEnvironment;
    PTP_WORK ExtendTraceStoreCallback;
    HANDLE ExtendTraceStoreEvent;
} TRACE_STORE_THREADPOOL, *PTRACE_STORE_THREADPOOL;

typedef struct _TRACE_STORES TRACE_STORES, *PTRACE_STORES;

typedef struct DECLSPEC_ALIGN(16) _TRACE_STORE_MEMORY_MAP {
    union {
        DECLSPEC_ALIGN(16) SLIST_ENTRY              ListEntry;   // 8       8
        struct {
            DECLSPEC_ALIGN(8) PVOID                 PrevAddress; // 8       8
            DECLSPEC_ALIGN(8) PTRACE_STORE_ADDRESS  pAddress;    // 8       16
        };
    };
    DECLSPEC_ALIGN(8)  HANDLE        FileHandle;                // 8        24
    DECLSPEC_ALIGN(8)  HANDLE        MappingHandle;             // 8        32
    DECLSPEC_ALIGN(8)  LARGE_INTEGER FileOffset;                // 8        40
    DECLSPEC_ALIGN(8)  LARGE_INTEGER MappingSize;               // 8        48
    DECLSPEC_ALIGN(8)  PVOID         BaseAddress;               // 8        56
    DECLSPEC_ALIGN(8)
    union {
        PVOID PreferredBaseAddress;                             // 8        64
        PVOID NextAddress;                                      // 8        64
    };
} TRACE_STORE_MEMORY_MAP, *PTRACE_STORE_MEMORY_MAP, **PPTRACE_STORE_MEMORY_MAP;

typedef volatile PTRACE_STORE_MEMORY_MAP VPTRACE_STORE_MEMORY_MAP;

C_ASSERT(sizeof(TRACE_STORE_MEMORY_MAP) == 64);

typedef struct _TRACE_STORE {
    SLIST_HEADER            CloseMemoryMaps;
    SLIST_HEADER            PrepareMemoryMaps;
    SLIST_HEADER            NextMemoryMaps;
    SLIST_HEADER            FreeMemoryMaps;
    SLIST_HEADER            PrefaultMemoryMaps;

    PRTL                    Rtl;
    PTRACE_CONTEXT          TraceContext;
    LARGE_INTEGER           InitialSize;
    LARGE_INTEGER           ExtensionSize;
    LARGE_INTEGER           MappingSize;
    PTP_WORK                PrefaultFuturePageWork;
    PTP_WORK                PrepareNextMemoryMapWork;
    PTP_WORK                CloseMemoryMapWork;
    HANDLE                  NextMemoryMapAvailableEvent;
    HANDLE                  AllMemoryMapsAreFreeEvent;

    PTRACE_STORE_MEMORY_MAP PrevMemoryMap;
    PTRACE_STORE_MEMORY_MAP MemoryMap;

    volatile ULONG  NumberOfActiveMemoryMaps;

    volatile LONG   MappedSequenceId;

    //
    // Each trace store, when initialized, is assigned a unique sequence
    // ID, starting at 1.  The trace store and all metadata stores share
    // the same sequence ID.
    //

    ULONG SequenceId;

    LARGE_INTEGER TotalNumberOfAllocations;
    LARGE_INTEGER TotalAllocationSize;

    union {
        ULONG Flags;
        struct {
            ULONG NoRetire:1;
            ULONG NoPrefaulting:1;
            ULONG RecordSimpleAllocation:1;
            ULONG NoPreferredAddressReuse:1;
            ULONG IsReadonly:1;
            ULONG SetEndOfFileOnClose:1;
            ULONG IsMetadata:1;
        };
    };

    DWORD CreateFileDesiredAccess;
    DWORD CreateFileMappingProtectionFlags;
    DWORD MapViewOfFileDesiredAccess;

    HANDLE FileHandle;
    PVOID PrevAddress;

    PTRACE_STORE            AllocationStore;
    PTRACE_STORE            AddressStore;
    PTRACE_STORE            InfoStore;

    PALLOCATE_RECORDS       AllocateRecords;
    PFREE_RECORDS           FreeRecords;

    //
    // Inline TRACE_STORE_ALLOCATION.
    //

    union {
        TRACE_STORE_ALLOCATION Allocation;
        struct {
            union {
                ULARGE_INTEGER  NumberOfRecords;
                ULARGE_INTEGER  NumberOfAllocations;
            };
            union {
                LARGE_INTEGER   RecordSize;
                ULARGE_INTEGER  AllocationSize;
            };
        };
    };
    PTRACE_STORE_ALLOCATION pAllocation;

    //
    // Inline TRACE_STORE_INFO.
    //

    union {

        TRACE_STORE_INFO Info;

        struct {

            //
            // Inline TRACE_STORE_EOF.
            //

            union {

                TRACE_STORE_EOF Eof;

                struct {
                    LARGE_INTEGER EndOfFile;
                };

            };


            //
            // Inline TRACE_STORE_TIME.
            //

            union {

                TRACE_STORE_TIME Time;

                struct {
                    LARGE_INTEGER   Frequency;
                    LARGE_INTEGER   Multiplicand;
                    FILETIME        StartTime;
                    LARGE_INTEGER   StartCounter;
                };

            };

            //
            // Inline TRACE_STORE_STATS.
            //

            union {

                TRACE_STORE_STATS Stats;

                struct {
                    ULONG DroppedRecords;
                    ULONG ExhaustedFreeMemoryMaps;
                    ULONG AllocationsOutpacingNextMemoryMapPreparation;
                    ULONG Unused1;
                };

            };
        };
    };

    PTRACE_STORE_EOF    pEof;
    PTRACE_STORE_TIME   pTime;
    PTRACE_STORE_STATS  pStats;
    PTRACE_STORE_INFO   pInfo;

    UNICODE_STRING Path;
    WCHAR PathBuffer[_OUR_MAX_PATH];

} TRACE_STORE, *PTRACE_STORE;

static const LPCWSTR TraceStoreFileNames[] = {
    L"TraceEvent.dat",
    L"TraceString.dat",
    L"TraceStringBuffer.dat",
    L"TraceHashedString.dat",
    L"TraceHashedStringBuffer.dat",
    L"TraceBuffer.dat",
    L"TraceFunctionTable.dat",
    L"TraceFunctionTableEntry.dat",
    L"TracePathTable.dat",
    L"TracePathTableEntry.dat",
    L"TraceSession.dat",
    L"TraceFilenameString.dat",
    L"TraceFilenameStringBuffer.dat",
    L"TraceDirectoryString.dat",
    L"TraceDirectoryStringBuffer.dat",
};

static const WCHAR TraceStoreAllocationSuffix[] = L":allocation";
static const DWORD TraceStoreAllocationSuffixLength = (
    sizeof(TraceStoreAllocationSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreAddressSuffix[] = L":address";
static const DWORD TraceStoreAddressSuffixLength = (
    sizeof(TraceStoreAddressSuffix) /
    sizeof(WCHAR)
);

static const WCHAR TraceStoreInfoSuffix[] = L":info";
static const DWORD TraceStoreInfoSuffixLength = (
    sizeof(TraceStoreInfoSuffix) /
    sizeof(WCHAR)
);

static const USHORT LongestTraceStoreSuffixLength = (
    sizeof(TraceStoreAllocationSuffix) /
    sizeof(WCHAR)
);

static const USHORT NumberOfTraceStores = (
    sizeof(TraceStoreFileNames) /
    sizeof(LPCWSTR)
);

static const USHORT ElementsPerTraceStore = 4;

//
// The Event trace store gets an initial file size of 80MB,
// everything else gets 10MB.
//

static const ULONG InitialTraceStoreFileSizes[] = {
    10 << 23,   // Event
    10 << 20,   // String
    10 << 20,   // StringBuffer
    10 << 20,   // HashedString
    10 << 20,   // HashedStringBuffer
    10 << 20,   // Buffer
    10 << 20,   // FunctionTable
    10 << 20,   // FunctionTableEntry
    10 << 20,   // PathTable
    10 << 20,   // PathTableEntry
    10 << 20,   // TraceSession
    10 << 20,   // TraceFilenameString
    10 << 20,   // TraceFilenameStringBuffer
    10 << 20,   // TraceDirectoryString
    10 << 20    // TraceDirectoryStringBuffer
};

#define FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex)        \
    for (Index = 0, StoreIndex = 0;                                 \
         Index < TraceStores->NumberOfTraceStores;                  \
         Index++, StoreIndex += TraceStores->ElementsPerTraceStore)


typedef struct _TRACE_STORES {
    USHORT  Size;
    USHORT  NumberOfTraceStores;
    USHORT  ElementsPerTraceStore;
    USHORT  Reserved;
    PRTL    Rtl;
    TRACE_STORE Stores[MAX_TRACE_STORES];
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
    _In_    PCWSTR          ModuleName
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
    _In_    PCWSTR          ModuleName,
    _In_    PCWSTR          ModulePath
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
    PRTL                Rtl;
    LARGE_INTEGER       SessionId;
    GUID                MachineGuid;
    PISID               Sid;
    PCWSTR              UserName;
    PCWSTR              ComputerName;
    PCWSTR              DomainName;
    FILETIME            SystemTime;
} TRACE_SESSION, *PTRACE_SESSION;

typedef struct _TRACE_CONTEXT {
    ULONG                       Size;
    ULONG                       SequenceId;
    PRTL                        Rtl;
    PTRACE_SESSION              TraceSession;
    PTRACE_STORES               TraceStores;
    PSYSTEM_TIMER_FUNCTION      SystemTimerFunction;
    LARGE_INTEGER               PerformanceCounterFrequency;
    PVOID                       UserData;
    PTP_CALLBACK_ENVIRON        ThreadpoolCallbackEnvironment;
    HANDLE                      HeapHandle;

    //
    // Inline TRACE_STORE_TIME.
    //

    union {

        TRACE_STORE_TIME Time;

        struct {
            LARGE_INTEGER   Frequency;
            LARGE_INTEGER   Multiplicand;
            FILETIME        StartTime;
            LARGE_INTEGER   StartCounter;
        };

    };


} TRACE_CONTEXT, *PTRACE_CONTEXT;

TRACER_API
BOOL
InitializeTraceStores(
    _In_        PRTL            Rtl,
    _In_        PWSTR           BaseDirectory,
    _Inout_opt_ PTRACE_STORES   TraceStores,
    _Inout_     PULONG          SizeOfTraceStores,
    _In_opt_    PULONG          InitialFileSizes,
    _In_        BOOL            Readonly
);

typedef BOOL (*PINITIALIZE_TRACE_SESSION)(
    _In_                                 PRTL           Rtl,
    _Inout_bytecap_(*SizeOfTraceSession) PTRACE_SESSION TraceSession,
    _In_                                 PULONG         SizeOfTraceSession
);

TRACER_API
BOOL
InitializeTraceSession(
    _In_                                 PRTL           Rtl,
    _Inout_bytecap_(*SizeOfTraceSession) PTRACE_SESSION TraceSession,
    _In_                                 PULONG         SizeOfTraceSession
);

TRACER_API
BOOL
BindTraceStoreToTraceContext(
    _Inout_ PTRACE_STORE TraceStore,
    _Inout_ PTRACE_CONTEXT TraceContext
);

typedef BOOL (*PBIND_TRACE_STORE_TO_TRACE_CONTEXT)(
    _Inout_ PTRACE_STORE TraceStore,
    _Inout_ PTRACE_CONTEXT TraceContext
);

typedef BOOL (*PINITIALIZE_TRACE_CONTEXT)(
    _In_     PRTL            Rtl,
    _Inout_bytecap_(*SizeOfTraceContext)
             PTRACE_CONTEXT  TraceContext,
    _In_     PULONG          SizeOfTraceContext,
    _In_     PTRACE_SESSION  TraceSession,
    _In_     PTRACE_STORES   TraceStores,
    _In_     PTP_CALLBACK_ENVIRON  ThreadpoolCallbackEnvironment,
    _In_opt_ PVOID UserData,
    _In_     BOOL Readonly
    );

_Success_(return != 0)
TRACER_API
BOOL
InitializeTraceContext(
    _In_     PRTL            Rtl,
    _Inout_bytecap_(*SizeOfTraceContext)
             PTRACE_CONTEXT  TraceContext,
    _In_     PULONG          SizeOfTraceContext,
    _In_     PTRACE_SESSION  TraceSession,
    _In_     PTRACE_STORES   TraceStores,
    _In_     PTP_CALLBACK_ENVIRON  ThreadpoolCallbackEnvironment,
    _In_opt_ PVOID UserData,
    _In_     BOOL Readonly
    );

typedef BOOL (*PFLUSH_TRACE_STORES)(_In_ PTRACE_CONTEXT TraceContext);

#ifdef _M_X64
#pragma intrinsic(__readgsdword)
FORCEINLINE
DWORD
FastGetCurrentProcessId(VOID)
{
    return __readgsdword(0x40);
}

FORCEINLINE
DWORD
FastGetCurrentThreadId(VOID)
{
    return __readgsdword(0x48);
}
#endif

FORCEINLINE
BOOL
HasVaryingRecordSizes(
    _In_    PTRACE_STORE    TraceStore
    )
{
    return (
        TraceStore->pEof->EndOfFile.QuadPart != (
            TraceStore->pAllocation->RecordSize.QuadPart *
            TraceStore->pAllocation->NumberOfRecords.QuadPart
        )
    );
}

FORCEINLINE
VOID
TraceTimeQueryPerformanceCounter(
    _In_    PTRACE_STORE_TIME   Time,
    _Out_   PLARGE_INTEGER      ElapsedPointer
    )
{
    LARGE_INTEGER Elapsed;

    //
    // Query the performance counter.
    //

    QueryPerformanceCounter(&Elapsed);

    //
    // Get the elapsed time since the start of the trace.
    //

    Elapsed.QuadPart -= Time->StartCounter.QuadPart;

    //
    // Convert to microseconds.
    //

    Elapsed.QuadPart *= Time->Multiplicand.QuadPart;
    Elapsed.QuadPart /= Time->Frequency.QuadPart;

    //
    // Update relative to 1970 C UNIX time.
    //

    Elapsed.QuadPart += Time->StartTime.MicrosecondsSince1970.QuadPart;

    //
    // Update the caller's pointer.
    //

    ElapsedPointer->QuadPart = Elapsed.QuadPart;

}

FORCEINLINE
VOID
TraceContextQueryPerformanceCounter(
    _In_    PTRACE_CONTEXT  TraceContext,
    _Out_   PLARGE_INTEGER  ElapsedPointer
    )
{
    TraceTimeQueryPerformanceCounter(&TraceContext->Time, ElapsedPointer);
}

FORCEINLINE
VOID
TraceStoreQueryPerformanceCounter(
    _In_    PTRACE_STORE    TraceStore,
    _Out_   PLARGE_INTEGER  ElapsedPointer
    )
{
    TraceTimeQueryPerformanceCounter(&TraceStore->TraceContext->Time,
                                     ElapsedPointer);
}

TRACER_API
BOOL
InitializeTraceStoreTime(
    _In_    PRTL                Rtl,
    _In_    PTRACE_STORE_TIME   Time
    );


TRACER_API
BOOL
FlushTraceStores(PTRACE_CONTEXT TraceContext);

TRACER_API
VOID
SubmitTraceStoreFileExtensionThreadpoolWork(
    _Inout_     PTRACE_STORE    TraceStore
);

TRACER_API
VOID
CloseTraceStore(PTRACE_STORE TraceStore);

TRACER_API
VOID
CloseTraceStores(PTRACE_STORES TraceStores);

TRACER_API
DWORD
GetTraceStoresAllocationSize(_In_ const USHORT NumberOfTraceStores);

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
    PULARGE_INTEGER RecordSize
);

TRACER_API
LPVOID
GetNextRecords(
    PTRACE_STORE TraceStore,
    ULARGE_INTEGER RecordSize,
    ULARGE_INTEGER RecordCount
);

TRACER_API
LPVOID
AllocateRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PULARGE_INTEGER RecordSize,
    _In_    PULARGE_INTEGER NumberOfRecords
);

TRACER_API
VOID
FreeRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    PVOID           Buffer
    );

TRACER_API
BOOL
WriteRecords(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _In_    ULARGE_INTEGER  RecordSize,
    _In_    ULARGE_INTEGER  NumberOfRecords,
    _In_    PVOID           FirstRecord,
    _In_    PVOID          *DestinationAddress
);

TRACER_API
LPVOID
WriteBytes(
    _In_     PTRACE_CONTEXT  TraceContext,
    _In_     PTRACE_STORE    TraceStore,
    _In_     ULARGE_INTEGER  NumberOfBytes,
    _In_     PVOID           Buffer,
    _In_opt_ PVOID          *DestinationAddress
);

TRACER_API
PVOID
GetPreviousAllocationAddress(
    _In_ PTRACE_STORE TraceStore
);

TRACER_API
BOOL
GetAllocationSize(
    _In_    PTRACE_CONTEXT  TraceContext,
    _In_    PTRACE_STORE    TraceStore,
    _Inout_ PULARGE_INTEGER TotalSize,
    _Inout_ PULARGE_INTEGER AllocatedSize
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
RegisterModule(
    _Inout_     PTRACE_CONTEXT  TraceContext,
    _In_        DWORD_PTR       ModuleToken,
    _In_        PCWSTR          ModuleName,
    _In_        PCWSTR          ModuleFilename
);

TRACER_API
VOID
Debugbreak();

#ifdef __cpp
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
