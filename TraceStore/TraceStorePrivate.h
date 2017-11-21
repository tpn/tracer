/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStorePrivate.h

Abstract:

    This is the private header file for the TraceStore component.  It defines
    function typedefs and function declarations for all major (i.e. not local
    to the module) functions available for use by individual modules within
    this component.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////
// Debugging Helpers
////////////////////////////////////////////////////////////////////////////////

TRACE_STORE_DATA volatile BOOL PauseBeforeThreadpoolWorkEnabled;

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PreThreadpoolWorkSubmission(
    _In_ BOOL volatile *Pause,
    _In_ ULONG volatile *FailedCount
    )
{
    BOOL Success = TRUE;

#ifdef _DEBUG
    if (IsDebuggerPresent() && PauseBeforeThreadpoolWorkEnabled) {
        while (*Pause && *FailedCount == 0) {
            Sleep(1000);
        }
    }
#endif

    Success = (*FailedCount == 0);
    return Success;
}

#define PRE_THREADPOOL_WORK_SUBMISSION(Pause)                              \
    if (!PreThreadpoolWorkSubmission(Pause, &TraceContext->FailedCount)) { \
        goto Error;                                                        \
    }

////////////////////////////////////////////////////////////////////////////////
// Function typedefs and inline functions for internal modules.
////////////////////////////////////////////////////////////////////////////////

//
// TraceStoreContext-related macros.
//

#define TRACE_CONTEXT_STORE(Name) ( \
    TraceStoreIdToTraceStore(       \
        TraceContext->TraceStores,  \
        TraceStore##Name##Id        \
    )                               \
)

#define INIT_WORK(Name, NumberOfItems)                               \
    Work = &TraceContext->##Name##Work;                              \
                                                                     \
    InitializeSListHead(&Work->ListHead);                            \
                                                                     \
    Work->WorkCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL); \
    if (!Work->WorkCompleteEvent) {                                  \
        goto Error;                                                  \
    }                                                                \
                                                                     \
    Work->ThreadpoolWork = CreateThreadpoolWork(                     \
        Name##Callback,                                              \
        TraceContext,                                                \
        ThreadpoolCallbackEnvironment                                \
    );                                                               \
    if (!Work->ThreadpoolWork) {                                     \
        goto Error;                                                  \
    }                                                                \
                                                                     \
    Work->TotalNumberOfItems = NumberOfItems;                        \
    Work->NumberOfActiveItems = NumberOfItems;                       \
    Work->NumberOfFailedItems = 0


#define CLEANUP_WORK(Name)                         \
    Work = &TraceContext->##Name##Work;            \
                                                   \
    if (Work->ThreadpoolWork) {                    \
        CloseThreadpoolWork(Work->ThreadpoolWork); \
    }                                              \
                                                   \
    if (Work->WorkCompleteEvent) {                 \
        CloseHandle(Work->WorkCompleteEvent);      \
    }                                              \
                                                   \
    SecureZeroMemory(Work, sizeof(*Work));

#define CLOSE_WORK(Name) do {                             \
    BOOL CancelPendingCallbacks = FALSE;                  \
    PTRACE_STORE_WORK Work = &TraceContext->##Name##Work; \
                                                          \
    if (Work->ThreadpoolWork) {                           \
        WaitForThreadpoolWorkCallbacks(                   \
            Work->ThreadpoolWork,                         \
            CancelPendingCallbacks                        \
        );                                                \
        CloseThreadpoolWork(Work->ThreadpoolWork);        \
    }                                                     \
                                                          \
    if (Work->WorkCompleteEvent) {                        \
        CloseHandle(Work->WorkCompleteEvent);             \
    }                                                     \
                                                          \
    SecureZeroMemory(Work, sizeof(*Work));                \
} while (0)

#define CLOSE_THREADPOOL_TIMER(Name, Lock)                              \
    if (TraceContext->##Name) {                                         \
        PTP_TIMER Timer = TraceContext->##Name;                         \
        BOOL CancelPendingCallbacks = FALSE;                            \
        AcquireSRWLockExclusive(&TraceContext->##Lock);                 \
        SetThreadpoolTimer(Timer, NULL, 0, 0);                          \
        WaitForThreadpoolTimerCallbacks(Timer, CancelPendingCallbacks); \
        CloseThreadpoolTimer(Timer);                                    \
        TraceContext->##Name = NULL;                                    \
        ReleaseSRWLockExclusive(&TraceContext->##Lock);                 \
    }

#define CLOSE_ALL_THREADPOOL_TIMERS() do {                 \
    CLOSE_THREADPOOL_TIMER(GetWorkingSetChangesTimer,      \
                           WorkingSetChangesLock);         \
                                                           \
    CLOSE_THREADPOOL_TIMER(CapturePerformanceMetricsTimer, \
                           CapturePerformanceMetricsLock); \
} while (0)


//
// TraceStoreAtExitEx-related functions.
//

ATEXITEX_CALLBACK TraceStoreAtExitEx;

//
// TraceStorePath-related functions.
//

typedef
_Success_(return != 0)
BOOL
(FIND_LONGEST_TRACE_STORE_FILENAME)(
    _Out_ PUSHORT LengthPointer
    );
typedef FIND_LONGEST_TRACE_STORE_FILENAME *PFIND_LONGEST_TRACE_STORE_FILENAME;
FIND_LONGEST_TRACE_STORE_FILENAME FindLongestTraceStoreFileName;

typedef
_Success_(return != 0)
ULONG
(GET_LONGEST_TRACE_STORE_FILENAME)(
    VOID
    );
typedef GET_LONGEST_TRACE_STORE_FILENAME *PGET_LONGEST_TRACE_STORE_FILENAME;
GET_LONGEST_TRACE_STORE_FILENAME GetLongestTraceStoreFileName;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_STORE_PATH)(
    _In_    PCWSTR          Path,
    _In_    PTRACE_STORE    TraceStore
    );
typedef INITIALIZE_TRACE_STORE_PATH *PINITIALIZE_TRACE_STORE_PATH;
INITIALIZE_TRACE_STORE_PATH InitializeTraceStorePath;

//
// TraceStoresRundown-related data structures and functions.
//

typedef struct _TRACE_STORES_RUNDOWN_FLAGS {
    ULONG IsActive:1;
} TRACE_STORES_RUNDOWN_FLAGS, *PTRACE_STORES_RUNDOWN_FLAGS;

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACE_STORES_RUNDOWN {

    //
    // Size of the structure, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACE_STORES_RUNDOWN)) USHORT SizeOfStruct;

    //
    // Pad out to 4-bytes.
    //

    USHORT Padding1;

    //
    // Flags.
    //

    TRACE_STORES_RUNDOWN_FLAGS Flags;

    //
    // Critical section protecting the rundown list head.
    //

    CRITICAL_SECTION CriticalSection;

    //
    // Rundown list head.
    //

    _Guarded_by_(CriticalSection)
    LIST_ENTRY ListHead;

} TRACE_STORES_RUNDOWN;
typedef TRACE_STORES_RUNDOWN *PTRACE_STORES_RUNDOWN;

typedef
_Requires_lock_not_held_(Rundown->CriticalSection)
VOID
(RUNDOWN_TRACE_STORES)(
    _In_ PTRACE_STORES_RUNDOWN Rundown
    );
typedef RUNDOWN_TRACE_STORES *PRUNDOWN_TRACE_STORES;
RUNDOWN_TRACE_STORES RundownTraceStores;

typedef
BOOL
(IS_TRACE_STORES_RUNDOWN_ACTIVE)(
    _In_ PTRACE_STORES_RUNDOWN Rundown
    );
typedef IS_TRACE_STORES_RUNDOWN_ACTIVE *PIS_TRACE_STORES_RUNDOWN_ACTIVE;
IS_TRACE_STORES_RUNDOWN_ACTIVE IsTraceStoresRundownActive;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(INITIALIZE_TRACE_STORES_RUNDOWN)(
    _In_ PTRACE_STORES_RUNDOWN Rundown
    );
typedef INITIALIZE_TRACE_STORES_RUNDOWN \
      *PINITIALIZE_TRACE_STORES_RUNDOWN;
INITIALIZE_TRACE_STORES_RUNDOWN InitializeTraceStoresRundown;

typedef
_No_competing_thread_
VOID
(DESTROY_TRACE_STORES_RUNDOWN)(
    _In_ PTRACE_STORES_RUNDOWN Rundown
    );
typedef DESTROY_TRACE_STORES_RUNDOWN \
      *PDESTROY_TRACE_STORES_RUNDOWN;
DESTROY_TRACE_STORES_RUNDOWN DestroyTraceStoresRundown;

typedef
_Requires_lock_held_(Rundown->CriticalSection)
VOID
(ADD_TRACE_STORES_TO_RUNDOWN)(
    _In_ PTRACE_STORES_RUNDOWN Rundown,
    _In_ PTRACE_STORES TraceStores
    );
typedef ADD_TRACE_STORES_TO_RUNDOWN *PADD_TRACE_STORES_TO_RUNDOWN;
ADD_TRACE_STORES_TO_RUNDOWN AddTraceStoresToRundown;

typedef
_Requires_lock_held_(TraceStores->Rundown->CriticalSection)
VOID
(REMOVE_TRACE_STORES_FROM_RUNDOWN)(
    _In_ PTRACE_STORES TraceStores
    );
typedef REMOVE_TRACE_STORES_FROM_RUNDOWN *PREMOVE_TRACE_STORES_FROM_RUNDOWN;
REMOVE_TRACE_STORES_FROM_RUNDOWN RemoveTraceStoresFromRundown;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(REGISTER_TRACE_STORES)(
    _In_ PTRACE_STORES_RUNDOWN Rundown,
    _In_ PTRACE_STORES TraceStores
    );
typedef REGISTER_TRACE_STORES *PREGISTER_TRACE_STORES;
REGISTER_TRACE_STORES RegisterTraceStores;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(UNREGISTER_TRACE_STORES)(
    _In_ PTRACE_STORES TraceStores
    );
typedef UNREGISTER_TRACE_STORES *PUNREGISTER_TRACE_STORES;
UNREGISTER_TRACE_STORES UnregisterTraceStores;

//
// TraceStoreGlobalRundown-related functions.
//

typedef
PTRACE_STORES_RUNDOWN
(GET_GLOBAL_TRACE_STORES_RUNDOWN)(
    VOID
    );
typedef GET_GLOBAL_TRACE_STORES_RUNDOWN *PGET_GLOBAL_TRACE_STORES_RUNDOWN;
GET_GLOBAL_TRACE_STORES_RUNDOWN GetGlobalTraceStoresRundown;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(REGISTER_GLOBAL_TRACE_STORES)(
    _In_ PTRACE_STORES TraceStores
    );
typedef REGISTER_GLOBAL_TRACE_STORES *PREGISTER_GLOBAL_TRACE_STORES;
REGISTER_GLOBAL_TRACE_STORES RegisterGlobalTraceStores;

typedef
_Success_(return != 0)
_Check_return_
BOOL
(UNREGISTER_GLOBAL_TRACE_STORES)(
    _In_ PTRACE_STORES TraceStores
    );
typedef UNREGISTER_GLOBAL_TRACE_STORES *PUNREGISTER_GLOBAL_TRACE_STORES;
UNREGISTER_GLOBAL_TRACE_STORES UnregisterGlobalTraceStores;

//
// TraceStoreMetadata-related functions.
//

typedef
_Success_(return != 0)
PTRACE_STORE
(TRACE_STORE_METADATA_ID_TO_STORE)(
    _In_ PTRACE_STORE TraceStore,
    _In_ TRACE_STORE_METADATA_ID TraceStoreMetadataId
    );
typedef TRACE_STORE_METADATA_ID_TO_STORE *PTRACE_STORE_METADATA_ID_TO_STORE;
TRACE_STORE_METADATA_ID_TO_STORE TraceStoreMetadataIdToStore;

BIND_COMPLETE MetadataInfoMetadataBindComplete;
BIND_COMPLETE AllocationMetadataBindComplete;
BIND_COMPLETE RelocationMetadataBindComplete;
BIND_COMPLETE AddressMetadataBindComplete;
BIND_COMPLETE AddressRangeMetadataBindComplete;
BIND_COMPLETE AllocationTimestampMetadataBindComplete;
BIND_COMPLETE AllocationTimestampDeltaMetadataBindComplete;
BIND_COMPLETE SynchronizationMetadataBindComplete;
BIND_COMPLETE InfoMetadataBindComplete;

typedef
PBIND_COMPLETE
(TRACE_STORE_METADATA_ID_TO_BIND_COMPLETE)(
    _In_ TRACE_STORE_METADATA_ID TraceStoreMetadataId
    );
typedef TRACE_STORE_METADATA_ID_TO_BIND_COMPLETE \
      *PTRACE_STORE_METADATA_ID_TO_BIND_COMPLETE;
TRACE_STORE_METADATA_ID_TO_BIND_COMPLETE TraceStoreMetadataIdToBindComplete;

typedef
PCTRACE_STORE_TRAITS
(TRACE_STORE_METADATA_ID_TO_TRAITS)(
    _In_ TRACE_STORE_METADATA_ID TraceStoreMetadataId
    );
typedef TRACE_STORE_METADATA_ID_TO_TRAITS \
      *PTRACE_STORE_METADATA_ID_TO_TRAITS;
TRACE_STORE_METADATA_ID_TO_TRAITS TraceStoreMetadataIdToTraits;

typedef
ULONG
(TRACE_STORE_METADATA_ID_TO_RECORD_SIZE)(
    _In_ TRACE_STORE_METADATA_ID TraceStoreMetadataId
    );
typedef TRACE_STORE_METADATA_ID_TO_RECORD_SIZE \
      *PTRACE_STORE_METADATA_ID_TO_RECORD_SIZE;
TRACE_STORE_METADATA_ID_TO_RECORD_SIZE TraceStoreMetadataIdToRecordSize;

typedef
PTRACE_STORE_INFO
(TRACE_STORE_METADATA_ID_TO_INFO)(
    _In_ PTRACE_STORE TraceStore,
    _In_ TRACE_STORE_METADATA_ID TraceStoreMetadataId
    );
typedef TRACE_STORE_METADATA_ID_TO_INFO \
      *PTRACE_STORE_METADATA_ID_TO_INFO;
TRACE_STORE_METADATA_ID_TO_INFO TraceStoreMetadataIdToInfo;

typedef
VOID
(INITIALIZE_METADATA_FROM_RECORD_SIZE)(
    _In_ PTRACE_STORE TraceStore
    );
typedef INITIALIZE_METADATA_FROM_RECORD_SIZE \
      *PINITIALIZE_METADATA_FROM_RECORD_SIZE;
INITIALIZE_METADATA_FROM_RECORD_SIZE InitializeMetadataFromRecordSize;

//
// TraceStoreBind-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_STORE)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef BIND_STORE *PBIND_STORE;
BIND_STORE BindStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_METADATA_STORE_READONLY)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef BIND_METADATA_STORE_READONLY *PBIND_METADATA_STORE_READONLY;
BIND_METADATA_STORE_READONLY BindMetadataStoreReadonly;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_TRACE_STORE_READONLY)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef BIND_TRACE_STORE_READONLY *PBIND_TRACE_STORE_READONLY;
BIND_TRACE_STORE_READONLY BindTraceStoreReadonly;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_NON_STREAMING_READONLY_TRACE_STORE)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef BIND_NON_STREAMING_READONLY_TRACE_STORE \
      *PBIND_NON_STREAMING_READONLY_TRACE_STORE;
BIND_NON_STREAMING_READONLY_TRACE_STORE BindNonStreamingReadonlyTraceStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(PREPARE_NON_STREAMING_READONLY_TRACE_STORE_MAPS_COMPLETE)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef PREPARE_NON_STREAMING_READONLY_TRACE_STORE_MAPS_COMPLETE \
      *PPREPARE_NON_STREAMING_READONLY_TRACE_STORE_MAPS_COMPLETE;
PREPARE_NON_STREAMING_READONLY_TRACE_STORE_MAPS_COMPLETE \
    PrepareNonStreamingReadonlyTraceStoreMapsComplete;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_NON_STREAMING_READONLY_TRACE_STORE_COMPLETE)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef BIND_NON_STREAMING_READONLY_TRACE_STORE_COMPLETE \
      *PBIND_NON_STREAMING_READONLY_TRACE_STORE_COMPLETE;
BIND_NON_STREAMING_READONLY_TRACE_STORE_COMPLETE \
    BindNonStreamingReadonlyTraceStoreComplete;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_STREAMING_READONLY_TRACE_STORE)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef BIND_STREAMING_READONLY_TRACE_STORE \
      *PBIND_STREAMING_READONLY_TRACE_STORE;
BIND_STREAMING_READONLY_TRACE_STORE BindStreamingReadonlyTraceStore;

//
// TraceStoreTime-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_STORE_TIME)(
    _In_ PRTL                Rtl,
    _In_ PTRACE_STORE_TIME   Time
    );
typedef INITIALIZE_TRACE_STORE_TIME *PINITIALIZE_TRACE_STORE_TIME;
INITIALIZE_TRACE_STORE_TIME InitializeTraceStoreTime;

FORCEINLINE
VOID
CopyTraceStoreTime(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    )
{
    PTRACE_STORE_TIME SourceTime = &TraceContext->Time;
    PTRACE_STORE_TIME DestTime = TraceStore->Time;

    __movsb((PBYTE)DestTime, (PBYTE)SourceTime, sizeof(*DestTime));
}

//
// TraceStoreMemoryMap-related functions.
//

typedef
_Success_(return != 0)
BOOL
(GET_NUMBER_OF_MEMORY_MAPS_REQUIRED_BY_TRACE_STORE)(
    _In_  PTRACE_STORE TraceStore,
    _Out_ PULONG NumberOfMapsPointer
    );
typedef GET_NUMBER_OF_MEMORY_MAPS_REQUIRED_BY_TRACE_STORE \
      *PGET_NUMBER_OF_MEMORY_MAPS_REQUIRED_BY_TRACE_STORE;
GET_NUMBER_OF_MEMORY_MAPS_REQUIRED_BY_TRACE_STORE \
    GetNumberOfMemoryMapsRequiredByTraceStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_MEMORY_MAPS_FOR_TRACE_STORE)(
    _In_  PTRACE_STORE TraceStore,
    _Out_ PPTRACE_STORE_MEMORY_MAP FirstMemoryMapPointer,
    _Inout_ PULONG NumberOfMapsPointer
    );
typedef CREATE_MEMORY_MAPS_FOR_TRACE_STORE *PCREATE_MEMORY_MAPS_FOR_TRACE_STORE;
CREATE_MEMORY_MAPS_FOR_TRACE_STORE CreateMemoryMapsForTraceStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_MEMORY_MAPS_FOR_READONLY_TRACE_STORE)(
    _In_  PTRACE_STORE TraceStore,
    _Out_ PPTRACE_STORE_MEMORY_MAP FirstMemoryMapPointer,
    _In_  PULONG NumberOfMemoryMapsPointer
    );
typedef CREATE_MEMORY_MAPS_FOR_READONLY_TRACE_STORE \
      *PCREATE_MEMORY_MAPS_FOR_READONLY_TRACE_STORE;
CREATE_MEMORY_MAPS_FOR_READONLY_TRACE_STORE \
    CreateMemoryMapsForReadonlyTraceStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(PREPARE_NEXT_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE TraceStore,
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef  PREPARE_NEXT_TRACE_STORE_MEMORY_MAP \
       *PPREPARE_NEXT_TRACE_STORE_MEMORY_MAP;
PREPARE_NEXT_TRACE_STORE_MEMORY_MAP PrepareNextTraceStoreMemoryMap;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(PREPARE_FIRST_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE TraceStore
    );
typedef  PREPARE_FIRST_TRACE_STORE_MEMORY_MAP \
       *PPREPARE_FIRST_TRACE_STORE_MEMORY_MAP;
PREPARE_FIRST_TRACE_STORE_MEMORY_MAP PrepareFirstTraceStoreMemoryMap;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(PREPARE_READONLY_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE TraceStore,
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef  PREPARE_READONLY_TRACE_STORE_MEMORY_MAP \
       *PPREPARE_READONLY_TRACE_STORE_MEMORY_MAP;
PREPARE_READONLY_TRACE_STORE_MEMORY_MAP PrepareReadonlyTraceStoreMemoryMap;

typedef
_Success_(return != 0)
BOOL
(CLOSE_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE TraceStore,
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef  CLOSE_TRACE_STORE_MEMORY_MAP \
       *PCLOSE_TRACE_STORE_MEMORY_MAP;
CLOSE_TRACE_STORE_MEMORY_MAP CloseTraceStoreMemoryMap;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(FLUSH_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef FLUSH_TRACE_STORE_MEMORY_MAP *PFLUSH_TRACE_STORE_MEMORY_MAP;
FLUSH_TRACE_STORE_MEMORY_MAP FlushTraceStoreMemoryMap;

typedef
_Success_(return != 0)
BOOL
(UNMAP_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef UNMAP_TRACE_STORE_MEMORY_MAP *PUNMAP_TRACE_STORE_MEMORY_MAP;
UNMAP_TRACE_STORE_MEMORY_MAP UnmapTraceStoreMemoryMap;

typedef
VOID
(RUNDOWN_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE TraceStore,
    _In_opt_ PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef RUNDOWN_TRACE_STORE_MEMORY_MAP *PRUNDOWN_TRACE_STORE_MEMORY_MAP;
RUNDOWN_TRACE_STORE_MEMORY_MAP RundownTraceStoreMemoryMap;

typedef
_Success_(return != 0)
BOOL
(CONSUME_NEXT_TRACE_STORE_MEMORY_MAP)(
    _In_ PTRACE_STORE TraceStore,
    _In_opt_ PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef  CONSUME_NEXT_TRACE_STORE_MEMORY_MAP \
       *PCONSUME_NEXT_TRACE_STORE_MEMORY_MAP;
CONSUME_NEXT_TRACE_STORE_MEMORY_MAP ConsumeNextTraceStoreMemoryMap;

typedef
VOID
(SUBMIT_CLOSE_MEMORY_MAP_THREADPOOL_WORK)(
    _In_  PTRACE_STORE TraceStore,
    _Inout_ _Post_invalid_ PPTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef  SUBMIT_CLOSE_MEMORY_MAP_THREADPOOL_WORK \
       *PSUBMIT_CLOSE_MEMORY_MAP_THREADPOOL_WORK;
SUBMIT_CLOSE_MEMORY_MAP_THREADPOOL_WORK \
    SubmitCloseMemoryMapThreadpoolWork;

//
// TraceStoreCallback-related functions.
//

typedef
VOID
(CALLBACK BIND_METADATA_INFO_STORE_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef BIND_METADATA_INFO_STORE_CALLBACK *PBIND_METADATA_INFO_STORE_CALLBACK;
BIND_METADATA_INFO_STORE_CALLBACK BindMetadataInfoStoreCallback;

typedef
VOID
(CALLBACK BIND_REMAINING_METADATA_STORES_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef BIND_REMAINING_METADATA_STORES_CALLBACK \
      *PBIND_REMAINING_METADATA_STORES_CALLBACK;
BIND_REMAINING_METADATA_STORES_CALLBACK BindRemainingMetadataStoresCallback;

typedef
VOID
(CALLBACK PREFAULT_FUTURE_TRACE_STORE_PAGE_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_STORE TraceStore,
    _In_     PTP_WORK Work
    );
typedef  PREFAULT_FUTURE_TRACE_STORE_PAGE_CALLBACK \
       *PPREFAULT_FUTURE_TRACE_STORE_PAGE_CALLBACK;
PREFAULT_FUTURE_TRACE_STORE_PAGE_CALLBACK \
    PrefaultFutureTraceStorePageCallback;

typedef
VOID
(CALLBACK PREPARE_NEXT_TRACE_STORE_MEMORY_MAP_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_STORE TraceStore,
    _In_     PTP_WORK Work
    );
typedef  PREPARE_NEXT_TRACE_STORE_MEMORY_MAP_CALLBACK \
       *PPREPARE_NEXT_TRACE_STORE_MEMORY_MAP_CALLBACK;
PREPARE_NEXT_TRACE_STORE_MEMORY_MAP_CALLBACK \
    PrepareNextTraceStoreMemoryMapCallback;

typedef
VOID
(CALLBACK PREPARE_READONLY_TRACE_STORE_MEMORY_MAP_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_STORE TraceStore,
    _In_     PTP_WORK Work
    );
typedef  PREPARE_READONLY_TRACE_STORE_MEMORY_MAP_CALLBACK \
       *PPREPARE_READONLY_TRACE_STORE_MEMORY_MAP_CALLBACK;
PREPARE_READONLY_TRACE_STORE_MEMORY_MAP_CALLBACK \
    PrepareReadonlyTraceStoreMemoryMapCallback;

typedef
VOID
(CALLBACK READONLY_NON_STREAMING_BIND_COMPLETE_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef  READONLY_NON_STREAMING_BIND_COMPLETE_CALLBACK \
       *PREADONLY_NON_STREAMING_BIND_COMPLETE_CALLBACK;
READONLY_NON_STREAMING_BIND_COMPLETE_CALLBACK \
    ReadonlyNonStreamingBindCompleteCallback;

typedef
VOID
(CALLBACK CLOSE_TRACE_STORE_MEMORY_MAP_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_STORE TraceStore,
    _In_     PTP_WORK Work
    );
typedef  CLOSE_TRACE_STORE_MEMORY_MAP_CALLBACK \
       *PCLOSE_TRACE_STORE_MEMORY_MAP_CALLBACK;
CLOSE_TRACE_STORE_MEMORY_MAP_CALLBACK \
    CloseTraceStoreMemoryMapCallback;

typedef
VOID
(CALLBACK BIND_TRACE_STORE_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef BIND_TRACE_STORE_CALLBACK *PBIND_TRACE_STORE_CALLBACK;
BIND_TRACE_STORE_CALLBACK BindTraceStoreCallback;

typedef
VOID
(CALLBACK BIND_FLAT_MEMORY_MAP_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef BIND_FLAT_MEMORY_MAP_CALLBACK *PBIND_FLAT_MEMORY_MAP_CALLBACK;
extern BIND_FLAT_MEMORY_MAP_CALLBACK BindFlatMemoryMapCallback;

typedef
VOID
(CALLBACK PREPARE_INTERVALS_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef PREPARE_INTERVALS_CALLBACK *PPREPARE_INTERVALS_CALLBACK;
extern PREPARE_INTERVALS_CALLBACK PrepareIntervalsCallback;

typedef
VOID
(CALLBACK NEW_MODULE_ENTRY_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef NEW_MODULE_ENTRY_CALLBACK *PNEW_MODULE_ENTRY_CALLBACK;
NEW_MODULE_ENTRY_CALLBACK NewModuleEntryCallback;

typedef
VOID
(CALLBACK CLEANUP_THREADPOOL_MEMBERS_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef CLEANUP_THREADPOOL_MEMBERS_CALLBACK \
      *PCLEANUP_THREADPOOL_MEMBERS_CALLBACK;
CLEANUP_THREADPOOL_MEMBERS_CALLBACK CleanupThreadpoolMembersCallback;

typedef
VOID
(CALLBACK GET_WORKING_SET_CHANGES_TIMER_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef GET_WORKING_SET_CHANGES_TIMER_CALLBACK \
      *PGET_WORKING_SET_CHANGES_TIMER_CALLBACK;
GET_WORKING_SET_CHANGES_TIMER_CALLBACK GetWorkingSetChangesTimerCallback;

typedef
VOID
(CALLBACK CAPTURE_PERFORMANCE_METRICS_TIMER_CALLBACK)(
    _In_     PTP_CALLBACK_INSTANCE Instance,
    _In_opt_ PTRACE_CONTEXT TraceContext,
    _In_     PTP_WORK Work
    );
typedef CAPTURE_PERFORMANCE_METRICS_TIMER_CALLBACK \
      *PCAPTURE_PERFORMANCE_METRICS_TIMER_CALLBACK;
CAPTURE_PERFORMANCE_METRICS_TIMER_CALLBACK \
    CapturePerformanceMetricsTimerCallback;

//
// TraceStoreMemory-map related inline functions.
//

FORCEINLINE
VOID
ReturnFreeTraceStoreMemoryMap(
    _In_ PTRACE_STORE TraceStore,
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    //
    // Make sure we don't try and push the trace store's embedded memory map
    // onto the free list.
    //

    if (MemoryMap == &TraceStore->SingleMemoryMap) {
        goto DecrementActiveMaps;
    }

    SecureZeroMemory(MemoryMap, sizeof(*MemoryMap));

    InterlockedPushEntrySList(
        &TraceStore->FreeMemoryMaps,
        &MemoryMap->ListEntry
    );

DecrementActiveMaps:
    if (!InterlockedDecrement(&TraceStore->NumberOfActiveMemoryMaps)) {
        SetEvent(TraceStore->AllMemoryMapsAreFreeEvent);
    }
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PopFreeTraceStoreMemoryMap(
    _In_  PTRACE_STORE TraceStore,
    _Out_ PPTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    PSLIST_HEADER ListHead;
    PSLIST_ENTRY ListEntry;

    ListHead = &TraceStore->FreeMemoryMaps;

    ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *MemoryMap = CONTAINING_RECORD(ListEntry,
                                   TRACE_STORE_MEMORY_MAP,
                                   ListEntry);

    InterlockedIncrement(&TraceStore->NumberOfActiveMemoryMaps);

    return TRUE;
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PopTraceStoreMemoryMap(
    _In_  PSLIST_HEADER ListHead,
    _Out_ PPTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    PSLIST_ENTRY ListEntry;

    ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *MemoryMap = CONTAINING_RECORD(ListEntry,
                                   TRACE_STORE_MEMORY_MAP,
                                   ListEntry);

    return TRUE;
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PopNonRetiredMemoryMap(
    _In_  PTRACE_STORE TraceStore,
    _Out_ PPTRACE_STORE_MEMORY_MAP MemoryMap
)
{
    PSLIST_HEADER ListHead;
    PSLIST_ENTRY ListEntry;

    ListHead = &TraceStore->NonRetiredMemoryMaps;

    ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *MemoryMap = CONTAINING_RECORD(ListEntry,
                                   TRACE_STORE_MEMORY_MAP,
                                   ListEntry);

    InterlockedDecrement(&TraceStore->NumberOfNonRetiredMemoryMaps);

    return TRUE;
}

FORCEINLINE
VOID
PushTraceStoreMemoryMap(
    _In_ PSLIST_HEADER ListHead,
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    )
{
    InterlockedPushEntrySList(ListHead, &MemoryMap->ListEntry);
}

FORCEINLINE
BOOL
GetTraceStoreMemoryMapFileInfo(
    _In_  PTRACE_STORE_MEMORY_MAP MemoryMap,
    _Out_ PFILE_STANDARD_INFO FileInfo
    )
{
    return GetFileInformationByHandleEx(
        MemoryMap->FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        FileInfo,
        sizeof(*FileInfo)
    );
}

FORCEINLINE
BOOL
GetTraceStoreFileInfo(
    _In_  PTRACE_STORE TraceStore,
    _Out_ PFILE_STANDARD_INFO FileInfo
    )
{
    return GetFileInformationByHandleEx(
        TraceStore->FileHandle,
        (FILE_INFO_BY_HANDLE_CLASS)FileStandardInfo,
        FileInfo,
        sizeof(*FileInfo)
    );
}

#define PushNonRetiredMemoryMap(TraceStore, MemoryMap)               \
    InterlockedIncrement(&TraceStore->NumberOfNonRetiredMemoryMaps); \
    PushTraceStoreMemoryMap(                                         \
        &TraceStore->NonRetiredMemoryMaps,                           \
        MemoryMap                                                    \
    )

#define PopNextMemoryMap(TraceStore, MemoryMap) \
    PopTraceStoreMemoryMap(                     \
        &TraceStore->NextMemoryMaps,            \
        MemoryMap                               \
    )

#define PushPrepareReadonlyNonStreamingMap(TraceStore, MemoryMap) \
    PushTraceStoreMemoryMap(                                      \
        &TraceStore->PrepareReadonlyMemoryMaps,                   \
        MemoryMap                                                 \
    )

#define SubmitPrepareReadonlyNonStreamingMap(TraceContext, TraceStore, MemMap) \
    PRE_THREADPOOL_WORK_SUBMISSION(                                            \
        &PauseBeforePrepareReadonlyNonStreamingMap                             \
    );                                                                         \
    InterlockedIncrement(                                                      \
        &TraceStore->PrepareReadonlyNonStreamingMapsInProgress                 \
    );                                                                         \
    PushPrepareReadonlyNonStreamingMap(TraceStore, MemMap);                    \
    SubmitThreadpoolWork(TraceStore->PrepareReadonlyNonStreamingMemoryMapWork)

#define PushReadonlyNonStreamingBindComplete(TraceContext, TraceStore) \
    PushTraceStore(                                                    \
        &TraceContext->ReadonlyNonStreamingBindCompleteWork.ListHead,  \
        TraceStore                                                     \
    )

#define PopReadonlyNonStreamingBindComplete(TraceContext, TraceStore) \
    PopTraceStore(                                                    \
        &TraceContext->ReadonlyNonStreamingBindCompleteWork.ListHead, \
        TraceStore                                                    \
    )

#define SubmitReadonlyNonStreamingBindComplete(TraceContext, TraceStore)  \
    PRE_THREADPOOL_WORK_SUBMISSION(                                       \
        &PauseBeforeReadonlyNonStreamingBindComplete                      \
    );                                                                    \
    InterlockedIncrement(                                                 \
        &TraceStore->ReadonlyNonStreamingBindCompletesInProgress          \
    );                                                                    \
    InterlockedIncrement(                                                 \
        &TraceContext->ReadonlyNonStreamingBindCompletesInProgress        \
    );                                                                    \
    PushReadonlyNonStreamingBindComplete(TraceContext, TraceStore);       \
    SubmitThreadpoolWork(                                                 \
        TraceContext->ReadonlyNonStreamingBindCompleteWork.ThreadpoolWork \
    )

#define PushRelocateTraceStore(TraceContext, TraceStore) \
    PushTraceStore(                                      \
        &TraceContext->RelocateWork.ListHead,            \
        TraceStore                                       \
    )

#define SubmitRelocateWork(TraceContext, TraceStore)          \
    PRE_THREADPOOL_WORK_SUBMISSION(&PauseBeforeRelocate);     \
    InterlockedIncrement(&TraceStore->RelocatesInProgress);   \
    InterlockedIncrement(&TraceContext->RelocatesInProgress); \
    PushRelocateTraceStore(TraceContext, TraceStore);         \
    SubmitThreadpoolWork(                                     \
        TraceContext->RelocateWork.ThreadpoolWork             \
    )

#define PushFlatMemoryMap(TraceContext, TraceStore)    \
    TraceStore->HasFlatMapping = TRUE;                 \
    PushTraceStoreMemoryMap(                           \
        &TraceContext->BindFlatMemoryMapWork.ListHead, \
        &TraceStore->FlatMemoryMap                     \
    )

#define PopFlatMemoryMap(TraceContext, MemoryMapPointer) \
    PopTraceStoreMemoryMap(                              \
        &TraceContext->BindFlatMemoryMapWork.ListHead,   \
        MemoryMapPointer                                 \
    )

#define SubmitDeferredFlatMemoryMap(TraceContext, TraceStore)    \
    PRE_THREADPOOL_WORK_SUBMISSION(&PauseBeforeRelocate);        \
    InterlockedIncrement(                                        \
        &TraceContext->BindFlatMemoryMapWork.NumberOfActiveItems \
    );                                                           \
    InterlockedIncrement(&TraceContext->NumberOfFlatMemoryMaps); \
    PushFlatMemoryMap(TraceContext, TraceStore);

#define PushIntervalPreparation(TraceContext, TraceStore)                     \
    InterlockedIncrement(                                                     \
        &TraceContext->PrepareIntervalsWork.NumberOfActiveItems               \
    );                                                                        \
    InterlockedIncrement(&TraceContext->NumberOfPreparedIntervals);           \
    PushTraceStore(&TraceContext->PrepareIntervalsWork.ListHead, TraceStore); \
    SubmitThreadpoolWork(TraceContext->PrepareIntervalsWork.ThreadpoolWork)

#define PopIntervalPreparation(TraceContext, TraceStorePointer)               \
    PopTraceStore(                                                            \
        &TraceContext->PrepareIntervalsWork.ListHead,                         \
        TraceStorePointer                                                     \
    )

//
// TraceStoreAddress-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(LOAD_NEXT_TRACE_STORE_ADDRESS)(
    _In_  PTRACE_STORE TraceStore,
    _Out_ PPTRACE_STORE_ADDRESS AddressPointer
    );
typedef LOAD_NEXT_TRACE_STORE_ADDRESS *PLOAD_NEXT_TRACE_STORE_ADDRESS;
LOAD_NEXT_TRACE_STORE_ADDRESS LoadNextTraceStoreAddress;

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
CopyTraceStoreAddress(
    _Out_ PTRACE_STORE_ADDRESS DestAddress,
    _In_ _Const_ PTRACE_STORE_ADDRESS SourceAddress
    )
/*++

Routine Description:

    This is a helper routine that can be used to safely copy an address
    structure when either the source or destination is backed by memory
    mapped memory.  Internally, it is simply a __movsq() wrapped in a
    __try/__except block that catches STATUS_IN_PAGE_ERROR exceptions.

Arguments:

    DestAddress - Supplies a pointer to the TRACE_STORE_ADDRESS to which the
        source address range will be copied.

    SourceAddress - Supplies a pointer to the TRACE_STORE_ADDRESS to copy into
        the destination address range.

Return Value:

    TRUE on success, FALSE if a STATUS_IN_PAGE_ERROR occurred.

--*/
{
    TRY_MAPPED_MEMORY_OP {
        __movsq((PDWORD64)DestAddress,
                (PDWORD64)SourceAddress,
                sizeof(*DestAddress) >> 3);
        return TRUE;
    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }
}

//
// TraceStoreAddressRange-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(REGISTER_NEW_TRACE_STORE_ADDRESS_RANGE)(
    _In_  PTRACE_STORE TraceStore,
    _In_  PTRACE_STORE_ADDRESS_RANGE AddressRange
    );
typedef REGISTER_NEW_TRACE_STORE_ADDRESS_RANGE \
      *PREGISTER_NEW_TRACE_STORE_ADDRESS_RANGE;
REGISTER_NEW_TRACE_STORE_ADDRESS_RANGE RegisterNewTraceStoreAddressRange;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(REGISTER_NEW_READONLY_TRACE_STORE_ADDRESS_RANGE)(
    _In_  PTRACE_STORE TraceStore,
    _In_  PTRACE_STORE_ADDRESS_RANGE AddressRange,
    _In_  PTRACE_STORE_MEMORY_MAP MemoryMap
    );
typedef REGISTER_NEW_READONLY_TRACE_STORE_ADDRESS_RANGE \
      *PREGISTER_NEW_READONLY_TRACE_STORE_ADDRESS_RANGE;
REGISTER_NEW_READONLY_TRACE_STORE_ADDRESS_RANGE \
    RegisterNewReadonlyTraceStoreAddressRange;

FORCEINLINE
PTRACE_STORE_ADDRESS_RANGE
TraceStoreReadonlyAddressRangeFromMemoryMap(
    _In_ PTRACE_STORE TraceStore,
    _In_ PTRACE_STORE_MEMORY_MAP MemoryMap
    )
/*++

Routine Description:

    This is a helper routine that can be used to obtain the relevant readonly
    trace store address record from just the memory map.  This is done by using
    known pointer offsets from base arrays.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure.  The trace
        store must be readonly.

    MemoryMap - Supplies a pointer to a TRACE_STORE_MEMORY_MAP structure.

Return Value:

    A pointer to the relevant TRACE_STORE_ADDRESS_RANGE structure for this
    memory map, NULL if an error occurred.

--*/
{
    ULONG Index;
    ULONG_PTR Distance;
    PTRACE_STORE_MEMORY_MAP FirstMemoryMap;
    PTRACE_STORE_ADDRESS_RANGE FirstAddressRange;
    PTRACE_STORE_ADDRESS_RANGE ReadonlyAddressRange;

    FirstMemoryMap = TraceStore->ReadonlyMemoryMaps;
    FirstAddressRange = TraceStore->ReadonlyAddressRanges;

    Distance = ((ULONG_PTR)MemoryMap - (ULONG_PTR)FirstMemoryMap);
    Index = (ULONG)(Distance / sizeof(*MemoryMap));

    ReadonlyAddressRange = &FirstAddressRange[Index];

    if ((Index + 1) < TraceStore->NumberOfReadonlyAddressRanges.LowPart) {
        if (ReadonlyAddressRange->OriginalAddressRange->PreferredBaseAddress !=
            MemoryMap->PreferredBaseAddress) {
            __debugbreak();
            return FALSE;
        }
    }

    return ReadonlyAddressRange;
}


FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
CopyTraceStoreAddressRange(
    _Out_ PTRACE_STORE_ADDRESS_RANGE DestAddressRange,
    _In_ _Const_ PTRACE_STORE_ADDRESS_RANGE SourceAddressRange
    )
/*++

Routine Description:

    This is a helper routine that can be used to safely copy an address range
    structure when either the source or destination is backed by memory mapped
    memory.  Internally, it is simply a __movsq() wrapped in a __try/__except
    block that catches STATUS_IN_PAGE_ERROR exceptions.

Arguments:

    DestAddressRange - Supplies a pointer to the TRACE_STORE_ADDRESS_RANGE to
        which the source address range will be copied.

    SourceAddressRange - Supplies a pointer to the TRACE_STORE_ADDRESS_RANGE to
        copy into the destination address range.

Return Value:

    TRUE on success, FALSE if a STATUS_IN_PAGE_ERROR occurred.

--*/
{
    TRY_MAPPED_MEMORY_OP {
        __movsq((PDWORD64)DestAddressRange,
                (PDWORD64)SourceAddressRange,
                sizeof(*DestAddressRange) >> 3);
        return TRUE;
    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }
}

//
// TraceStoreAllocation-related functions.
//

ALLOCATE_RECORDS TraceStoreAllocateRecords;
ALLOCATE_RECORDS_WITH_TIMESTAMP TraceStoreAllocateRecordsWithTimestamp;

ALLOCATE_RECORDS_WITH_TIMESTAMP
    SuspendedTraceStoreAllocateRecordsWithTimestamp;

ALLOCATE_RECORDS_WITH_TIMESTAMP
    ConcurrentTraceStoreAllocateRecordsWithTimestamp;

ALLOCATE_RECORDS_WITH_TIMESTAMP
    TraceStoreAllocateRecordsWithTimestampImpl;

ALLOCATE_RECORDS_WITH_TIMESTAMP
    TraceStoreAllocatePageAlignedRecordsWithTimestampImpl;

TRY_ALLOCATE_RECORDS TraceStoreTryAllocateRecords;
TRY_ALLOCATE_RECORDS_WITH_TIMESTAMP TraceStoreTryAllocateRecordsWithTimestamp;

typedef
_Success_(return != 0)
BOOL
(RECORD_TRACE_STORE_ALLOCATION)(
    _In_ PTRACE_STORE     TraceStore,
    _In_ ULONG_PTR        NumberOfRecords,
    _In_ ULONG_PTR        RecordSize,
    _In_opt_ ULONG_PTR    WastedBytes,
    _In_ LARGE_INTEGER    Timestamp
    );
typedef RECORD_TRACE_STORE_ALLOCATION *PRECORD_TRACE_STORE_ALLOCATION;
RECORD_TRACE_STORE_ALLOCATION RecordTraceStoreAllocation;

FORCEINLINE
_Success_(return != 0)
RecordTraceStoreAllocationTimestamp(
    _In_ PTRACE_STORE TraceStore,
    _In_ LARGE_INTEGER Timestamp
    )
{
    LONG Delta;
    PLONG DeltaPointer;
    PLARGE_INTEGER PreviousTimestamp;
    PLARGE_INTEGER TimestampPointer;
    ULONG_PTR DeltaRecordSize;
    ULONG_PTR TimestampRecordSize;
    ULONG_PTR NumberOfRecords;

    if (Timestamp.QuadPart == 0) {
        return TRUE;
    }

    DeltaRecordSize = sizeof(Delta);
    TimestampRecordSize = sizeof(Timestamp);
    NumberOfRecords = 1;

    TimestampPointer = (PLARGE_INTEGER)(
        TraceStore->AllocationTimestampStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore->AllocationTimestampStore,
            NumberOfRecords,
            TimestampRecordSize
        )
    );

    if (!TimestampPointer) {
        return FALSE;
    }

    TRY_MAPPED_MEMORY_OP {
        TimestampPointer->QuadPart = Timestamp.QuadPart;

        PreviousTimestamp = (PLARGE_INTEGER)TraceStore->AllocationTimestamp;
        if (PreviousTimestamp) {
            Delta = (LONG)(
                Timestamp.QuadPart -
                PreviousTimestamp->QuadPart
            );

            DeltaPointer = (PLONG)(
                TraceStore->AllocationTimestampDeltaStore->AllocateRecords(
                    TraceStore->TraceContext,
                    TraceStore->AllocationTimestampDeltaStore,
                    NumberOfRecords,
                    DeltaRecordSize
                )
            );

            if (!DeltaPointer) {
                return FALSE;
            }

            *DeltaPointer = Delta;
        }

        TraceStore->AllocationTimestamp = (PTRACE_STORE_ALLOCATION_TIMESTAMP)(
            TimestampPointer
        );

    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }

    return TRUE;
}

FORCEINLINE
VOID
SuspendTraceStoreAllocations(
    _In_ PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine suspends trace store allocations for a given trace store.
    This involves resetting the trace store's resume allocation event, then
    doing an interlocked pointer exchange on the allocate records function
    pointer; swapping the active one with the suspended one, such that future
    allocations go through the suspension code path (and wait on the resume
    event we just set).

    N.B. Only the AllocateRecordsWithTimestamp function pointer needs to be
         exchanged; the AllocateRecords function simply calls this one.

    N.B. No validation is done with regards to the current state of the
         trace store (i.e. ensuring allocations are currently active before
         suspending).

Arguments:

    TraceStore - Supplies a pointer to the TRACE_STORE structure for which
        allocations are to be suspended.

Return Value:

    None.

--*/
{

    //
    // Reset *must* come before the interlocked exchange.
    //

    ResetEvent(TraceStore->ResumeAllocationsEvent);

    InterlockedExchangePointer(
        (volatile PVOID *)&TraceStore->AllocateRecordsWithTimestamp,
        TraceStore->SuspendedAllocateRecordsWithTimestamp
    );

    //
    // Poll the active allocator count.  We need to ensure no threads are
    // within the allocator's body before we return to our caller.
    //

    while (TraceStore->ActiveAllocators > 0) {

        //
        // Give up our quantum and allow other runnable threads on this core
        // to run.
        //
        // N.B. We may need to profile this and see if YieldProcessor() is more
        //      appropriate.
        //

        SwitchToThread();
    }
}

FORCEINLINE
VOID
ResumeTraceStoreAllocations(
    _In_ PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine resumes trace store allocations for a given trace store.
    This involves setting the trace store's resume allocation event, then
    doing an interlocked pointer exchange on the allocate records function
    pointer; swapping the active one (which will be the suspended allocator)
    with the normal one, such that future allocations are satisfied normally.

    N.B. Only the AllocateRecordsWithTimestamp function pointer needs to be
         exchanged; the AllocateRecords function simply calls this one.

    N.B. No validation is done with regards to the current state of the
         trace store (i.e. ensuring allocations are currently suspended before
         resuming).

Arguments:

    TraceStore - Supplies a pointer to the TRACE_STORE structure for which
        allocations are to be resumed.

Return Value:

    None.

--*/
{

    InterlockedExchangePointer(
        (volatile PVOID *)&TraceStore->AllocateRecordsWithTimestamp,
        TraceStore->AllocateRecordsWithTimestampImpl1
    );

    //
    // SetEvent *must* come after the interlocked exchange.
    //

    SetEvent(TraceStore->ResumeAllocationsEvent);
}

//
// TraceStoreAllocator-related function declarations.
//

MALLOC TraceStoreAllocatorMalloc;
CALLOC TraceStoreAllocatorCalloc;
TRY_MALLOC TraceStoreAllocatorTryMalloc;
TRY_CALLOC TraceStoreAllocatorTryCalloc;
MALLOC_WITH_TIMESTAMP TraceStoreAllocatorMallocWithTimestamp;
CALLOC_WITH_TIMESTAMP TraceStoreAllocatorCallocWithTimestamp;
TRY_MALLOC_WITH_TIMESTAMP TraceStoreAllocatorTryMallocWithTimestamp;
TRY_CALLOC_WITH_TIMESTAMP TraceStoreAllocatorTryCallocWithTimestamp;
REALLOC TraceStoreAllocatorRealloc;
FREE TraceStoreAllocatorFree;
FREE_POINTER TraceStoreAllocatorFreePointer;
INITIALIZE_ALLOCATOR TraceStoreInitializeAllocator;
DESTROY_ALLOCATOR TraceStoreDestroyAllocator;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_STORE_ALLOCATOR)(
    _In_ PTRACE_STORE TraceStore
    );
INITIALIZE_TRACE_STORE_ALLOCATOR InitializeTraceStoreAllocator;

//
// TraceStoreRelocation-related functions.
//

typedef
_Success_(return != 0)
BOOL
(SAVE_TRACE_STORE_RELOCATION_INFO)(
    _In_ PTRACE_STORE TraceStore
    );
typedef SAVE_TRACE_STORE_RELOCATION_INFO \
      *PSAVE_TRACE_STORE_RELOCATION_INFO;
SAVE_TRACE_STORE_RELOCATION_INFO SaveTraceStoreRelocationInfo;

typedef
_Success_(return != 0)
BOOL
(LOAD_TRACE_STORE_RELOCATION_INFO)(
    _In_ PTRACE_STORE TraceStore
    );
typedef LOAD_TRACE_STORE_RELOCATION_INFO \
      *PLOAD_TRACE_STORE_RELOCATION_INFO;
LOAD_TRACE_STORE_RELOCATION_INFO LoadTraceStoreRelocationInfo;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(READONLY_NON_STREAMING_TRACE_STORE_READY_FOR_RELOCATION)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef READONLY_NON_STREAMING_TRACE_STORE_READY_FOR_RELOCATION \
      *PREADONLY_NON_STREAMING_TRACE_STORE_READY_FOR_RELOCATION;
READONLY_NON_STREAMING_TRACE_STORE_READY_FOR_RELOCATION \
    ReadonlyNonStreamingTraceStoreReadyForRelocation;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(READONLY_NON_STREAMING_TRACE_STORE_COMPLETE_RELOCATION)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef READONLY_NON_STREAMING_TRACE_STORE_COMPLETE_RELOCATION \
      *PREADONLY_NON_STREAMING_TRACE_STORE_COMPLETE_RELOCATION;
READONLY_NON_STREAMING_TRACE_STORE_COMPLETE_RELOCATION \
    ReadonlyNonStreamingTraceStoreCompleteRelocation;

//
// TraceStoreTraits-related functions.
//

typedef
_Success_(return != 0)
BOOL
(SAVE_TRACE_STORE_TRAITS)(
    _In_ PTRACE_STORE TraceStore
    );
typedef SAVE_TRACE_STORE_TRAITS \
      *PSAVE_TRACE_STORE_TRAITS;
SAVE_TRACE_STORE_TRAITS SaveTraceStoreTraits;

typedef
_Success_(return != 0)
BOOL
(LOAD_TRACE_STORE_TRAITS)(
    _In_ PTRACE_STORE TraceStore
    );
typedef LOAD_TRACE_STORE_TRAITS \
      *PLOAD_TRACE_STORE_TRAITS;
LOAD_TRACE_STORE_TRAITS LoadTraceStoreTraits;

typedef
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_STORE_TRAITS)(
    _In_ PTRACE_STORE TraceStore
    );
typedef INITIALIZE_TRACE_STORE_TRAITS \
      *PINITIALIZE_TRACE_STORE_TRAITS;
INITIALIZE_TRACE_STORE_TRAITS InitializeTraceStoreTraits;

//
// TraceStore-related functions.
//

BIND_COMPLETE TraceStoreBindComplete;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_STORE)(
    _In_     PCWSTR Path,
    _In_     PTRACE_STORE TraceStore,
    _In_opt_ LARGE_INTEGER InitialSize,
    _In_opt_ LARGE_INTEGER MappingSize,
    _In_opt_ PCTRACE_STORE_TRAITS Traits
    );
typedef INITIALIZE_STORE *PINITIALIZE_STORE;
INITIALIZE_STORE InitializeStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_STORE)(
    _In_ PRTL Rtl,
    _In_ PCWSTR Path,
    _In_ PTRACE_STORE TraceStore,
    _In_ PTRACE_STORE MetadataInfoStore,
    _In_ PTRACE_STORE AllocationStore,
    _In_ PTRACE_STORE RelocationStore,
    _In_ PTRACE_STORE AddressStore,
    _In_ PTRACE_STORE AddressRangeStore,
    _In_ PTRACE_STORE AllocationTimestampStore,
    _In_ PTRACE_STORE AllocationTimestampDeltaStore,
    _In_ PTRACE_STORE SynchronizationStore,
    _In_ PTRACE_STORE InfoStore,
    _In_ LARGE_INTEGER InitialSize,
    _In_ LARGE_INTEGER MappingSize,
    _In_ PTRACE_FLAGS TraceFlags,
    _In_ PTRACE_STORE_RELOC Reloc,
    _In_opt_ PCTRACE_STORE_TRAITS Traits
    );
typedef INITIALIZE_TRACE_STORE *PINITIALIZE_TRACE_STORE;
INITIALIZE_TRACE_STORE InitializeTraceStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_TRACE_STORE)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef BIND_TRACE_STORE *PBIND_TRACE_STORE;
BIND_TRACE_STORE BindTraceStore;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(BIND_FLAT_MEMORY_MAP)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE_MEMORY_MAP FlatMemoryMap
    );
typedef BIND_FLAT_MEMORY_MAP *PBIND_FLAT_MEMORY_MAP;
extern BIND_FLAT_MEMORY_MAP BindFlatMemoryMap;

typedef
VOID
(INITIALIZE_TRACE_STORE_SLIST_HEADERS)(
    _In_ PTRACE_STORE TraceStore
    );
typedef INITIALIZE_TRACE_STORE_SLIST_HEADERS \
      *PINITIALIZE_TRACE_STORE_SLIST_HEADERS;
INITIALIZE_TRACE_STORE_SLIST_HEADERS InitializeTraceStoreSListHeaders;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_TRACE_STORE_EVENTS)(
    _In_ PTRACE_STORE TraceStore
    );
typedef CREATE_TRACE_STORE_EVENTS *PCREATE_TRACE_STORE_EVENTS;
CREATE_TRACE_STORE_EVENTS CreateTraceStoreEvents;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_TRACE_STORE_THREADPOOL_WORK_ITEMS)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    );
typedef CREATE_TRACE_STORE_THREADPOOL_WORK_ITEMS \
      *PCREATE_TRACE_STORE_THREADPOOL_WORK_ITEMS;
CREATE_TRACE_STORE_THREADPOOL_WORK_ITEMS CreateTraceStoreThreadpoolWorkItems;

typedef
_Success_(return != 0)
BOOL
(TRUNCATE_STORE)(
    _In_ PTRACE_STORE TraceStore
    );
typedef TRUNCATE_STORE *PTRUNCATE_STORE;
TRUNCATE_STORE TruncateStore;

typedef
VOID
(CLOSE_STORE)(
    _In_ PTRACE_STORE TraceStore
    );
typedef CLOSE_STORE *PCLOSE_STORE;
CLOSE_STORE CloseStore;

typedef
VOID
(CLOSE_TRACE_STORE)(
    _In_ PTRACE_STORE TraceStore
    );
typedef CLOSE_TRACE_STORE *PCLOSE_TRACE_STORE;
CLOSE_TRACE_STORE CloseTraceStore;

FORCEINLINE
VOID
CloseTraceStoresInline(
    _In_ PTRACE_STORES TraceStores
    )
{
    USHORT Index;
    USHORT StoreIndex;

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {
        CloseTraceStore(&TraceStores->Stores[StoreIndex]);
    }
}

typedef
VOID
(RUNDOWN_STORE)(
    _In_ PTRACE_STORE TraceStore
    );
typedef RUNDOWN_STORE *PRUNDOWN_STORE;
RUNDOWN_STORE RundownStore;

typedef
VOID
(RUNDOWN_TRACE_STORE)(
    _In_ PTRACE_STORE TraceStore
    );
typedef RUNDOWN_TRACE_STORE *PRUNDOWN_TRACE_STORE;
RUNDOWN_TRACE_STORE RundownTraceStore;

FORCEINLINE
VOID
RundownTraceStoresInline(
    _In_ PTRACE_STORES TraceStores
    )
{
    USHORT Index;
    USHORT StoreIndex;

    FOR_EACH_TRACE_STORE(TraceStores, Index, StoreIndex) {
        RundownTraceStore(&TraceStores->Stores[StoreIndex]);
    }
}

#define CLOSE_METADATA_STORE(Name)             \
    if (TraceStore->##Name##Store) {           \
        CloseStore(TraceStore->##Name##Store); \
        TraceStore->##Name##Store = NULL;      \
    }

#define CLOSE_METADATA_STORES()                     \
    CLOSE_METADATA_STORE(Allocation);               \
    CLOSE_METADATA_STORE(Relocation);               \
    CLOSE_METADATA_STORE(Address);                  \
    CLOSE_METADATA_STORE(AddressRange);             \
    CLOSE_METADATA_STORE(AllocationTimestamp);      \
    CLOSE_METADATA_STORE(AllocationTimestampDelta); \
    CLOSE_METADATA_STORE(Synchronization);          \
    CLOSE_METADATA_STORE(Info);                     \
    CLOSE_METADATA_STORE(MetadataInfo);

#define RUNDOWN_METADATA_STORE(Name)             \
    if (TraceStore->##Name##Store) {             \
        RundownStore(TraceStore->##Name##Store); \
        TraceStore->##Name##Store = NULL;        \
    }

#define RUNDOWN_METADATA_STORES()                     \
    RUNDOWN_METADATA_STORE(Allocation);               \
    RUNDOWN_METADATA_STORE(Relocation);               \
    RUNDOWN_METADATA_STORE(Address);                  \
    RUNDOWN_METADATA_STORE(AddressRange);             \
    RUNDOWN_METADATA_STORE(AllocationTimestamp);      \
    RUNDOWN_METADATA_STORE(AllocationTimestampDelta); \
    RUNDOWN_METADATA_STORE(Synchronization);          \
    RUNDOWN_METADATA_STORE(Info);                     \
    RUNDOWN_METADATA_STORE(MetadataInfo);

//
// TraceStore-related inline functions.
//

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PopTraceStore(
    _In_  PSLIST_HEADER ListHead,
    _Out_ PPTRACE_STORE TraceStore
    )
{
    PSLIST_ENTRY ListEntry;

    ListEntry = InterlockedPopEntrySList(ListHead);
    if (!ListEntry) {
        return FALSE;
    }

    *TraceStore = CONTAINING_RECORD(ListEntry,
                                    TRACE_STORE,
                                    ListEntry);

    return TRUE;
}

FORCEINLINE
VOID
PushTraceStore(
    _In_ PSLIST_HEADER ListHead,
    _In_ PTRACE_STORE TraceStore
    )
{
    InterlockedPushEntrySList(ListHead, &TraceStore->ListEntry);
}

FORCEINLINE
VOID
PushFailedTraceStore(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine pushes a trace store that has encountered an unrecoverable
    error (such as CreateFileMapping() failing) to the trace context's failure
    list, atomically increments the context's failure count, and, if it is the
    first failure, submits a context cleanup work item to the cancellation
    threadpool, which will have the effect of terminating other threadpool
    callbacks in progress and eventually set the loading complete event.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

    TraceStore - Supplies a pointer to a TRACE_STORE structure.

Return Value:

    None.

--*/
{
    __debugbreak();
    PushTraceStore(&TraceContext->FailedListHead, TraceStore);

    if (InterlockedIncrement(&TraceContext->FailedCount) == 1) {

        //
        // This is the first failure.  Submit a work item to close threadpool
        // cleanup group members from the cancellation threadpool.
        //

        SubmitThreadpoolWork(TraceContext->CleanupThreadpoolMembersWork);
    }
}


FORCEINLINE
VOID
TraceContextFailure(
    _In_ PTRACE_CONTEXT TraceContext
    )
/*++

Routine Description:

    This routine atomically increments the context's failure count, and,
    if it is the first failure, submits a context cleanup work item to the
    cancellation threadpool, which will have the effect of terminating other
    threadpool callbacks in progress.

Arguments:

    TraceContext - Supplies a pointer to a TRACE_CONTEXT structure.

Return Value:

    None.

--*/
{
    __debugbreak();
    if (InterlockedIncrement(&TraceContext->FailedCount) == 1) {

        //
        // This is the first failure.  Submit a work item to close threadpool
        // cleanup group members from the cancellation threadpool.
        //

        SubmitThreadpoolWork(TraceContext->CleanupThreadpoolMembersWork);
    }
}

//
// BindMetadataInfoStore-related macros.
//

#define PushBindMetadataInfoTraceStore(TraceContext, TraceStore) \
    PushTraceStore(                                              \
        &TraceContext->BindMetadataInfoStoreWork.ListHead,       \
        TraceStore->MetadataInfoStore                            \
    )

#define PopBindMetadataInfoTraceStore(TraceContext, MetadataInfoStorePointer) \
    PopTraceStore(                                                            \
        &TraceContext->BindMetadataInfoStoreWork.ListHead,                    \
        MetadataInfoStorePointer                                              \
    )

#define SubmitBindMetadataInfoWork(TraceContext, TraceStore)      \
    PRE_THREADPOOL_WORK_SUBMISSION(&PauseBeforeBindMetadataInfo); \
    PushBindMetadataInfoTraceStore(TraceContext, TraceStore);     \
    SubmitThreadpoolWork(                                         \
        TraceContext->BindMetadataInfoStoreWork.ThreadpoolWork    \
    )

//
// BindRemainingMetadata-related macros.
//

#define PushBindRemainingMetadataTraceStore(TraceContext, MetadataStore) \
    PushTraceStore(                                                      \
        &TraceContext->BindRemainingMetadataStoresWork.ListHead,         \
        MetadataStore                                                    \
    )

#define PopBindRemainingMetadataTraceStore(TraceContext, MetadataStorePointer) \
    PopTraceStore(                                                             \
        &TraceContext->BindRemainingMetadataStoresWork.ListHead,               \
        MetadataStorePointer                                                   \
    )

#define SubmitBindRemainingMetadataWork(TraceContext, MetadataStore)   \
    PRE_THREADPOOL_WORK_SUBMISSION(&PauseBeforeBindRemainingMetadata); \
    PushBindRemainingMetadataTraceStore(TraceContext, MetadataStore);  \
    SubmitThreadpoolWork(                                              \
        TraceContext->BindRemainingMetadataStoresWork.ThreadpoolWork   \
    )

#define SUBMIT_METADATA_BIND(Name)   \
    MetadataBindsSubmitted++;        \
    SubmitBindRemainingMetadataWork( \
        TraceContext,                \
        TraceStore->##Name##Store    \
    )

#define SUBMIT_BINDS_FOR_REMAINING_METADATA_STORES(TraceContext, TraceStore) \
    SUBMIT_METADATA_BIND(Allocation);                                        \
    SUBMIT_METADATA_BIND(Relocation);                                        \
    SUBMIT_METADATA_BIND(Address);                                           \
    SUBMIT_METADATA_BIND(AddressRange);                                      \
    SUBMIT_METADATA_BIND(AllocationTimestamp);                               \
    SUBMIT_METADATA_BIND(AllocationTimestampDelta);                          \
    SUBMIT_METADATA_BIND(Synchronization);                                   \
    SUBMIT_METADATA_BIND(Info);

//
// BindTraceStore-related macros.
//

#define PushBindTraceStore(TraceContext, TraceStore) \
    PushTraceStore(                                  \
        &TraceContext->BindTraceStoreWork.ListHead,  \
        TraceStore                                   \
    )

#define PopBindTraceStore(TraceContext, TraceStorePointer) \
    PopTraceStore(                                         \
        &TraceContext->BindTraceStoreWork.ListHead,        \
        TraceStorePointer                                  \
    )

#define SubmitBindTraceStoreWork(TraceContext, TraceStore)                \
    PRE_THREADPOOL_WORK_SUBMISSION(&PauseBeforeBindTraceStore);           \
    PushBindTraceStore(TraceContext, TraceStore);                         \
    SubmitThreadpoolWork(TraceContext->BindTraceStoreWork.ThreadpoolWork)

//
// TraceStoreSystemTimer-related functions.
//

typedef
_Success_(return != 0)
PTIMER_FUNCTION
(TRACE_STORE_GET_TIMER_FUNCTION)(VOID);
typedef TRACE_STORE_GET_TIMER_FUNCTION *PTRACE_STORE_GET_TIMER_FUNCTION;
TRACE_STORE_GET_TIMER_FUNCTION TraceStoreGetTimerFunction;

typedef
_Success_(return != 0)
BOOL
(TRACE_STORE_CALL_TIMER)(
    _Out_       PFILETIME   SystemTime,
    _Inout_opt_ PPTIMER_FUNCTION ppTimerFunction
    );
typedef TRACE_STORE_CALL_TIMER *PTRACE_STORE_CALL_TIMER;
TRACE_STORE_CALL_TIMER TraceStoreCallTimer;

//
// TraceStorePrefault-related inline functions.
//

FORCEINLINE
VOID
PrefaultFutureTraceStorePage(
    _In_ PTRACE_STORE TraceStore
    )
/*++

Routine Description:

    This routine pops a memory map off the TraceStore's PrefaultMemoryMaps
    list and reads a single byte from the memory map's NextAddress address.
    This has the effect of bringing the page into the process's memory if it
    isn't already in it -- either via a hard fault, soft fault, or simply
    priming the TLB with the mapping if the underlying page is already valid
    within our memory space (which is often the case because the cache manager
    aggressively reads ahead when we start mapping views of the file).

    Because a hard or soft fault can only by satisfied by blocking the thread
    (because the relevant page may need to be read off the backing disk/store),
    pre-faults are performed ahead of time by threadpool threads off the tracing
    hot-path.

Arguments:

    TraceStore - Supplies a pointer to a TRACE_STORE structure that has a
        TraceStoreMemoryMap pushed to the PrefaultMemoryMaps SLIST_HEAD.

Return Value:

    None.

--*/
{
    BOOL Success;
    PTRACE_STORE_MEMORY_MAP PrefaultMemoryMap;
    PSLIST_HEADER PrefaultList;

    //
    // Validate arguments.
    //

    if (!TraceStore) {
        return;
    }

    //
    // Pop an entry off the TraceStore's PrefaultMemoryMap list.  (There should
    // be a 1:1 correspondence between SubmitThreadpoolWork() calls and push
    // calls to the prefault list, so we should always be able to pop something
    // off.)
    //

    PrefaultList = &TraceStore->PrefaultMemoryMaps;
    Success = PopTraceStoreMemoryMap(PrefaultList, &PrefaultMemoryMap);

    if (!Success) {
        return;
    }

    //
    // Prefault the page.
    //

    TRY_MAPPED_MEMORY_OP {
        TraceStore->Rtl->PrefaultPages(PrefaultMemoryMap->NextAddress, 1);
    } CATCH_EXCEPTION_ACCESS_VIOLATION {

        //
        // This will happen if servicing the prefault off-core has taken longer
        // for the originating core (the one that submitted the prefault work)
        // to consume the entire memory map, then *another* memory map, which
        // will retire the memory map backing this prefault address, which
        // results in the address being invalidated, which results in an access
        // violation when we try and read/prefault it from the thread pool.
        //

        TraceStore->Stats->AccessViolationsEncounteredDuringAsyncPrefault++;
    }

    //
    // Return the memory map to the free list.
    //

    ReturnFreeTraceStoreMemoryMap(TraceStore, PrefaultMemoryMap);
}

//
// TraceStoreMemoryMap-related inline functions.
//

FORCEINLINE
BOOL
FinalizeTraceStoreAddressTimes(
    _In_ PTRACE_STORE TraceStore,
    _In_opt_ PTRACE_STORE_ADDRESS AddressPointer
    )
/*++

Routine Description:

    This routine is responsible for updating the final timestamps of a trace
    store address record.  It is called when a memory map is being closed or
    rundown.

Arguments:

    TraceStore - Supplies a pointer to a trace store structure.  This is used
        to call TraceStoreQueryPerformanceCounter() in order to calculate the
        final elapsed times.

    AddressPointer - Supplies an optional pointer to the trace store address
        structure to rundown.  If NULL, this routine returns immediately.

Return Value:

    None.

--*/
{
    LARGE_INTEGER Timestamp;
    LARGE_INTEGER PreviousTimestamp;
    LARGE_INTEGER Elapsed;
    PLARGE_INTEGER ElapsedPointer;
    TRACE_STORE_ADDRESS Address;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(AddressPointer)) {
        return FALSE;
    }

    //
    // Take a local copy of the address record, update timestamps and
    // calculate elapsed time, then save the local record back to the
    // backing TRACE_STORE_ADDRESS struct.
    //

    if (!CopyTraceStoreAddress(&Address, AddressPointer)) {
        __debugbreak();
        return FALSE;
    }

    //
    // Get a local copy of the elapsed start time.
    //

    TraceStoreQueryPerformanceCounter(TraceStore, &Elapsed, &Timestamp);

    //
    // Copy it to the Released timestamp.
    //

    Address.Timestamp.Released.QuadPart = Elapsed.QuadPart;

    //
    // Determine what state this memory map was in at the time of being closed.
    // For a memory map that has progressed through the normal lifecycle, it'll
    // typically be in 'AwaitingRelease' at this point.  However, we could be
    // getting called against an active memory map or prepared memory map if
    // we're getting released as a result of closing the trace store.
    //

    if (Address.Timestamp.Retired.QuadPart != 0) {

        //
        // Normal memory map awaiting retirement.  Elapsed.AwaitingRelease
        // will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Retired.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingRelease;

    } else if (Address.Timestamp.Consumed.QuadPart != 0) {

        //
        // An active memory map.  Elapsed.Active will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Consumed.QuadPart;
        ElapsedPointer = &Address.Elapsed.Active;

    } else if (Address.Timestamp.Prepared.QuadPart != 0) {

        //
        // A prepared memory map awaiting consumption.
        // Elapsed.AwaitingConsumption will receive our elapsed time.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Prepared.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingConsumption;

    } else {

        //
        // A memory map that wasn't even prepared.  Highly unlikely.
        //

        PreviousTimestamp.QuadPart = Address.Timestamp.Requested.QuadPart;
        ElapsedPointer = &Address.Elapsed.AwaitingPreparation;
    }

    //
    // Calculate the elapsed time.
    //

    Elapsed.QuadPart -= PreviousTimestamp.QuadPart;

    //
    // Update the target elapsed time.
    //

    ElapsedPointer->QuadPart = Elapsed.QuadPart;

    //
    // Copy the local record back to the backing store.
    //

    if (!CopyTraceStoreAddress(AddressPointer, &Address)) {
        __debugbreak();
        return FALSE;
    }

    return TRUE;
}

//
// TraceStoreLoader-related functions.
//

BIND_COMPLETE ModuleTableStoreBindComplete;
BIND_COMPLETE ModuleLoadEventStoreBindComplete;
DLL_NOTIFICATION_CALLBACK TraceStoreDllNotificationCallback;
DLL_NOTIFICATION_CALLBACK TraceStoreDllNotificationCallbackImpl1;

#define AcquireModuleNamePrefixTableLockExclusive(TraceContext)        \
    AcquireSRWLockExclusive(&TraceContext->ModuleNamePrefixTableLock);

#define ReleaseModuleNamePrefixTableLockExclusive(TraceContext)        \
    ReleaseSRWLockExclusive(&TraceContext->ModuleNamePrefixTableLock);

#define AcquireModuleNamePrefixTableLockShared(TraceContext)        \
    AcquireSRWLockShared(&TraceContext->ModuleNamePrefixTableLock);

#define ReleaseModuleNamePrefixTableLockShared(TraceContext)        \
    ReleaseSRWLockShared(&TraceContext->ModuleNamePrefixTableLock);

#define AcquireModuleTableEntryLoadEventsLockExclusive(Entry) \
    AcquireSRWLockExclusive(&Entry->LoadEventsLock);

#define ReleaseModuleTableEntryLoadEventsLockExclusive(Entry) \
    ReleaseSRWLockExclusive(&Entry->LoadEventsLock);

#define AcquireModuleTableEntryLoadEventsLockShared(Entry) \
    AcquireSRWLockShared(&Entry->LoadEventsLock);

#define ReleaseModuleTableEntryLoadEventsLockShared(Entry) \
    ReleaseSRWLockShared(&Entry->LoadEventsLock);

#define AcquireModuleTableEntryDuplicateEntriesLockExclusive(Entry) \
    AcquireSRWLockExclusive(&Entry->DuplicateEntriesLock);

#define ReleaseModuleTableEntryDuplicateEntriesLockExclusive(Entry) \
    ReleaseSRWLockExclusive(&Entry->DuplicateEntriesLock);

#define AcquireModuleTableEntryDuplicateEntriesLockShared(Entry) \
    AcquireSRWLockShared(&Entry->DuplicateEntriesLock);

#define ReleaseModuleTableEntryDuplicateEntriesLockShared(Entry) \
    ReleaseSRWLockShared(&Entry->DuplicateEntriesLock);

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
PopNewModuleTableEntry(
    _In_  PTRACE_CONTEXT TraceContext,
    _Inout_ PPTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    PRTL_FILE File;
    PSLIST_HEADER ListHead;

    ListHead = &TraceContext->NewModuleEntryWork.ListHead;
    if (!PopRtlFile(ListHead, &File)) {
        return FALSE;
    }

    *ModuleTableEntry = CONTAINING_RECORD(File,
                                          TRACE_MODULE_TABLE_ENTRY,
                                          File);

    return TRUE;
}

FORCEINLINE
VOID
PushNewModuleTableEntry(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    PRTL_FILE File;
    PSLIST_HEADER ListHead;

    File = &ModuleTableEntry->File;
    ListHead = &TraceContext->NewModuleEntryWork.ListHead;
    PushRtlFile(ListHead, File);
}

typedef
_Check_return_
_Success_(return != 0)
_Maybe_raises_SEH_exception_
BOOL
(PROCESS_NEW_MODULE_TABLE_ENTRY)(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    );
typedef PROCESS_NEW_MODULE_TABLE_ENTRY *PPROCESS_NEW_MODULE_TABLE_ENTRY;
PROCESS_NEW_MODULE_TABLE_ENTRY ProcessNewModuleTableEntry;

#define EnterNewModuleTableCallback(TraceContext)                                \
    InterlockedIncrement(&TraceContext->NewModuleEntryWork.NumberOfActiveItems);

#define LeaveNewModuleTableCallback(TraceContext)                                \
    InterlockedDecrement(&TraceContext->NewModuleEntryWork.NumberOfActiveItems);

#define SubmitNewModuleTableEntry(TraceContext, ModuleTableEntry)          \
    PushNewModuleTableEntry(TraceContext, ModuleTableEntry);               \
    SubmitThreadpoolWork(TraceContext->NewModuleEntryWork.ThreadpoolWork);

FORCEINLINE
_Check_return_
_Success_(return != 0)
BOOL
GetLastLoadEvent(
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry,
    _Inout_ PPTRACE_MODULE_LOAD_EVENT LoadEventPointer
    )
{
    BOOL Found = FALSE;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY ListEntry;
    PTRACE_MODULE_LOAD_EVENT LoadEvent;

    //
    // Clear the caller's pointer up-front.
    //

    *LoadEventPointer = NULL;

    //
    // Acquire the load events lock and Initialize the list head.
    //

    AcquireModuleTableEntryLoadEventsLockShared(ModuleTableEntry);

    ListHead = &ModuleTableEntry->LoadEventsListHead;

    //
    // Enumerate entries in reverse and look for the first load event.
    //

    FOR_EACH_LIST_ENTRY_REVERSE(ListHead, ListEntry) {

        LoadEvent = CONTAINING_RECORD(ListEntry,
                                      TRACE_MODULE_LOAD_EVENT,
                                      ListEntry);

        if (LoadEvent->Reason.Loaded) {
            Found = TRUE;
            break;
        }
    }

    //
    // Release the load events lock.
    //

    ReleaseModuleTableEntryLoadEventsLockShared(ModuleTableEntry);

    //
    // If a record was found, update the caller's pointer.
    //

    if (Found) {
        *LoadEventPointer = LoadEvent;
    }

    return Found;
}

//
// TraceStoreSymbol-related functions and macros.
//

#define SYMBOL_CONTEXT_STORE(Name) (              \
    TraceStoreIdToTraceStore(                     \
        SymbolContext->TraceContext->TraceStores, \
        TraceStore##Name##Id                      \
    )                                             \
)

BIND_COMPLETE SymbolTableStoreBindComplete;
BIND_COMPLETE SymbolTypeStoreBindComplete;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK CREATE_TRACE_SYMBOL_CONTEXT_CALLBACK)(
    _In_ PINIT_ONCE InitOnce,
    _In_ PTRACE_CONTEXT TraceContext,
    _Outptr_result_nullonfailure_ PPTRACE_SYMBOL_CONTEXT SymbolContextPointer
    );
typedef CREATE_TRACE_SYMBOL_CONTEXT_CALLBACK \
      *PCREATE_TRACE_SYMBOL_CONTEXT_CALLBACK;
CREATE_TRACE_SYMBOL_CONTEXT_CALLBACK CreateTraceSymbolContextCallback;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_TRACE_SYMBOL_CONTEXT)(
    _In_ PTRACE_CONTEXT TraceContext
    );
typedef CREATE_TRACE_SYMBOL_CONTEXT \
      *PCREATE_TRACE_SYMBOL_CONTEXT;
CREATE_TRACE_SYMBOL_CONTEXT CreateTraceSymbolContext;

TRACE_SYMBOL_THREAD_ENTRY TraceSymbolThreadEntry;
TRACE_SYMBOL_THREAD_ENTRY TraceSymbolThreadEntryImpl;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_SYMBOL_CONTEXT)(
    _In_ PTRACE_CONTEXT TraceContext
    );
typedef INITIALIZE_TRACE_SYMBOL_CONTEXT *PINITIALIZE_TRACE_SYMBOL_CONTEXT;
INITIALIZE_TRACE_SYMBOL_CONTEXT InitializeTraceSymbolContext;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(PROCESS_TRACE_SYMBOL_WORK)(
    _In_ PTRACE_SYMBOL_CONTEXT SymbolContext
    );
typedef PROCESS_TRACE_SYMBOL_WORK *PPROCESS_TRACE_SYMBOL_WORK;
PROCESS_TRACE_SYMBOL_WORK ProcessTraceSymbolWork;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_SYMBOL_TABLE_FOR_MODULE_TABLE_ENTRY)(
    _In_ PTRACE_SYMBOL_CONTEXT SymbolContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    );
typedef CREATE_SYMBOL_TABLE_FOR_MODULE_TABLE_ENTRY \
      *PCREATE_SYMBOL_TABLE_FOR_MODULE_TABLE_ENTRY;
CREATE_SYMBOL_TABLE_FOR_MODULE_TABLE_ENTRY CreateSymbolTableForModuleTableEntry;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK SYMBOL_CONTEXT_ENUM_TYPES_CALLBACK)(
    _In_ PSYMBOL_INFO SymbolInfo,
    _In_ ULONG SymbolSize,
    _In_opt_ PTRACE_SYMBOL_CONTEXT SymbolContext
    );
typedef SYMBOL_CONTEXT_ENUM_TYPES_CALLBACK \
      *PSYMBOL_CONTEXT_ENUM_TYPES_CALLBACK;
SYMBOL_CONTEXT_ENUM_TYPES_CALLBACK SymbolContextEnumTypesCallback;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK SYMBOL_CONTEXT_ENUM_SYMBOLS_CALLBACK)(
    _In_ PSYMBOL_INFO SymbolInfo,
    _In_ ULONG SymbolSize,
    _In_opt_ PTRACE_SYMBOL_CONTEXT SymbolContext
    );
typedef SYMBOL_CONTEXT_ENUM_SYMBOLS_CALLBACK \
      *PSYMBOL_CONTEXT_ENUM_SYMBOLS_CALLBACK;
SYMBOL_CONTEXT_ENUM_SYMBOLS_CALLBACK SymbolContextEnumSymbolsCallback;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK SYMBOL_CONTEXT_ENUM_SOURCE_FILES_CALLBACK)(
    _In_ PSOURCEFILEW SourceFile,
    _In_opt_ PTRACE_SYMBOL_CONTEXT SymbolContext
    );
typedef SYMBOL_CONTEXT_ENUM_SOURCE_FILES_CALLBACK \
      *PSYMBOL_CONTEXT_ENUM_SOURCE_FILES_CALLBACK;
SYMBOL_CONTEXT_ENUM_SOURCE_FILES_CALLBACK SymbolContextEnumSourceFilesCallback;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK SYMBOL_CONTEXT_ENUM_LINES_CALLBACK)(
    _In_ PSRCCODEINFOW SourceCodeInfo,
    _In_opt_ PTRACE_SYMBOL_CONTEXT SymbolContext
    );
typedef SYMBOL_CONTEXT_ENUM_LINES_CALLBACK \
      *PSYMBOL_CONTEXT_ENUM_LINES_CALLBACK;
SYMBOL_CONTEXT_ENUM_LINES_CALLBACK SymbolContextEnumLinesCallback;

FORCEINLINE
VOID
AppendModuleTableEntryToSymbolContext(
    _In_ PTRACE_SYMBOL_CONTEXT SymbolContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    PLIST_ENTRY ListEntry;
    PGUARDED_LIST GuardedList;

    ListEntry = &ModuleTableEntry->SymbolContextListEntry;
    GuardedList = &SymbolContext->WorkList;
    AppendTailGuardedList(GuardedList, ListEntry);
    SetEvent(SymbolContext->WorkAvailableEvent);
}

FORCEINLINE
VOID
MaybeAppendModuleTableEntryToSymbolContext(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    PTRACE_SYMBOL_CONTEXT SymbolContext;

    SymbolContext = TraceContext->SymbolContext;

    if (!SymbolContext) {
        return;
    }

    InitializeListHead(&ModuleTableEntry->SymbolContextListEntry);

    AppendModuleTableEntryToSymbolContext(SymbolContext, ModuleTableEntry);
}

FORCEINLINE
_Check_return_
_Success_(return != 0)
_Requires_lock_not_held_(SymbolContext->CriticalSection)
BOOL
RemoveHeadModuleTableEntryFromSymbolContext(
    _In_ PTRACE_SYMBOL_CONTEXT SymbolContext,
    _Outptr_result_nullonfailure_ PPTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    BOOL Success;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PGUARDED_LIST GuardedList;

    GuardedList = &SymbolContext->WorkList;
    AcquireGuardedListLockExclusive(GuardedList);
    ListHead = &GuardedList->ListHead;
    if (IsListEmpty(ListHead)) {
        Success = FALSE;
        *ModuleTableEntry = NULL;
        goto End;
    }

    ListEntry = RemoveHeadList(ListHead);

    *ModuleTableEntry = CONTAINING_RECORD(ListEntry,
                                          TRACE_MODULE_TABLE_ENTRY,
                                          SymbolContextListEntry);

    Success = TRUE;

End:
    ReleaseGuardedListLockExclusive(GuardedList);

    return Success;
}

FORCEINLINE
BOOL
AllocateAndCopySymbolModuleInfo(
    _In_ PTRACE_SYMBOL_CONTEXT SymbolContext,
    _In_ PIMAGEHLP_MODULEW64 ModuleInfo
    )
{
    SIZE_T QuadWords;
    LONG_INTEGER AllocSize;
    PTRACE_STORE TraceStore;
    PRTL_IMAGE_FILE ImageFile;

    AllocSize.LongPart = ALIGN_UP_POINTER(sizeof(*ModuleInfo));
    if (AllocSize.HighPart) {
        __debugbreak();
    }

    QuadWords = AllocSize.LongPart >> 3;

    TraceStore = SYMBOL_CONTEXT_STORE(SymbolModuleInfo);
    ImageFile = &SymbolContext->CurrentModuleTableEntry->File.ImageFile;

    TRY_MAPPED_MEMORY_OP {

        //
        // Allocate space.
        //

        ImageFile->ModuleInfo = (PIMAGEHLP_MODULEW64)(
            TraceStore->AllocateRecordsWithTimestamp(
                TraceStore->TraceContext,
                TraceStore,
                1,
                AllocSize.LongPart,
                &SymbolContext->CurrentTimestamp
            )
        );

        if (!ImageFile->ModuleInfo) {
            return FALSE;
        }

        //
        // Copy the contents.
        //

        __movsq((PDWORD64)ImageFile->ModuleInfo,
                (PDWORD64)ModuleInfo,
                QuadWords);

    } CATCH_STATUS_IN_PAGE_ERROR {
        return FALSE;
    }

    return TRUE;
}

FORCEINLINE
BOOL
AllocateAndCopySourceFile(
    _In_ PTRACE_SYMBOL_CONTEXT SymbolContext,
    _In_ PSOURCEFILEW pSourceFile
    )
{
    SIZE_T QuadWords;
    LONG_INTEGER AllocSize;
    PTRACE_STORE TraceStore;
    PSOURCEFILEW SourceFile;
    PRTL_IMAGE_FILE ImageFile;

    AllocSize.LongPart = 0;
    QuadWords = AllocSize.LongPart >> 3;

    TraceStore = SYMBOL_CONTEXT_STORE(SymbolFile);
    ImageFile = &SymbolContext->CurrentModuleTableEntry->File.ImageFile;

    //
    // Need to add the source file as a prefix table entry on a structure
    // somewhere.
    //

    SourceFile = NULL;
    OutputDebugStringA("Copied file: ");
    OutputDebugStringW(pSourceFile->FileName);
    return TRUE;
}

//
// TraceStoreDebugEngine-related functions and macros.
//

BIND_COMPLETE TypeInfoTableStoreBindComplete;
BIND_COMPLETE FunctionTableStoreBindComplete;
BIND_COMPLETE FunctionSourceCodeStoreBindComplete;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK CREATE_TRACE_DEBUG_CONTEXT_CALLBACK)(
    _In_ PINIT_ONCE InitOnce,
    _In_ PTRACE_CONTEXT TraceContext,
    _Outptr_result_nullonfailure_ PPTRACE_DEBUG_CONTEXT DebugContextPointer
    );
typedef CREATE_TRACE_DEBUG_CONTEXT_CALLBACK \
      *PCREATE_TRACE_DEBUG_CONTEXT_CALLBACK;
CREATE_TRACE_DEBUG_CONTEXT_CALLBACK CreateTraceDebugContextCallback;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_TRACE_DEBUG_CONTEXT)(
    _In_ PTRACE_CONTEXT TraceContext
    );
typedef CREATE_TRACE_DEBUG_CONTEXT \
      *PCREATE_TRACE_DEBUG_CONTEXT;
CREATE_TRACE_DEBUG_CONTEXT CreateTraceDebugContext;

TRACE_DEBUG_ENGINE_THREAD_ENTRY TraceDebugEngineThreadEntry;
TRACE_DEBUG_ENGINE_THREAD_ENTRY TraceDebugEngineThreadEntryImpl;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(INITIALIZE_TRACE_DEBUG_CONTEXT)(
    _In_ PTRACE_CONTEXT TraceContext
    );
typedef INITIALIZE_TRACE_DEBUG_CONTEXT *PINITIALIZE_TRACE_DEBUG_CONTEXT;
INITIALIZE_TRACE_DEBUG_CONTEXT InitializeTraceDebugContext;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(PROCESS_TRACE_DEBUG_ENGINE_WORK)(
    _In_ PTRACE_DEBUG_CONTEXT DebugContext
    );
typedef PROCESS_TRACE_DEBUG_ENGINE_WORK *PPROCESS_TRACE_DEBUG_ENGINE_WORK;
PROCESS_TRACE_DEBUG_ENGINE_WORK ProcessTraceDebugEngineWork;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CREATE_TYPE_INFO_TABLE_FOR_MODULE_TABLE_ENTRY)(
    _In_ PTRACE_DEBUG_CONTEXT DebugContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    );
typedef CREATE_TYPE_INFO_TABLE_FOR_MODULE_TABLE_ENTRY
      *PCREATE_TYPE_INFO_TABLE_FOR_MODULE_TABLE_ENTRY;
CREATE_TYPE_INFO_TABLE_FOR_MODULE_TABLE_ENTRY \
    CreateTypeInfoTableForModuleTableEntry;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(CALLBACK TRACE_DEBUG_ENGINE_SYMBOL_CALLBACK)(
    _In_ PTRACE_DEBUG_CONTEXT DebugContext
    );
typedef TRACE_DEBUG_ENGINE_SYMBOL_CALLBACK
      *PTRACE_DEBUG_ENGINE_SYMBOL_CALLBACK;

#pragma component(browser, off)

DEBUG_ENGINE_LINE_OUTPUT_CALLBACK
    TraceDebugEngineExamineSymbolsLineOutputCallback;

DEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK
    TraceDebugEngineExamineSymbolsPartialOutputCallback;

DEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK
    TraceDebugEngineExamineSymbolsOutputCompleteCallback;

DEBUG_ENGINE_PARTIAL_OUTPUT_CALLBACK
    TraceDebugEngineUnassembleFunctionPartialOutputCallback;

DEBUG_ENGINE_OUTPUT_COMPLETE_CALLBACK
    TraceDebugEngineUnassembleFunctionOutputCompleteCallback;
#pragma component(browser, on)

FORCEINLINE
VOID
AppendModuleTableEntryToDebugContext(
    _In_ PTRACE_DEBUG_CONTEXT DebugContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    PLIST_ENTRY ListEntry;
    PGUARDED_LIST GuardedList;

    ListEntry = &ModuleTableEntry->DebugContextListEntry;
    GuardedList = &DebugContext->WorkList;
    AppendTailGuardedList(GuardedList, ListEntry);
    SetEvent(DebugContext->WorkAvailableEvent);
}

FORCEINLINE
VOID
MaybeAppendModuleTableEntryToDebugContext(
    _In_ PTRACE_CONTEXT TraceContext,
    _In_ PTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    PTRACE_DEBUG_CONTEXT DebugContext;

    DebugContext = TraceContext->DebugContext;

    if (!DebugContext) {
        return;
    }

    InitializeListHead(&ModuleTableEntry->DebugContextListEntry);

    AppendModuleTableEntryToDebugContext(DebugContext, ModuleTableEntry);
}


FORCEINLINE
_Check_return_
_Success_(return != 0)
_When_(return == 0, _Post_satisfies_(*ModuleTableEntry == NULL))
_Requires_lock_not_held_(DebugContext->CriticalSection)
BOOL
RemoveHeadModuleTableEntryFromDebugContext(
    _In_ PTRACE_DEBUG_CONTEXT DebugContext,
    _Inout_ _Pre_notnull_ PPTRACE_MODULE_TABLE_ENTRY ModuleTableEntry
    )
{
    BOOL Success;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PGUARDED_LIST GuardedList;

    GuardedList = &DebugContext->WorkList;
    ListHead = &GuardedList->ListHead;
    AcquireGuardedListLockExclusive(GuardedList);
    if (IsListEmpty(ListHead)) {
        Success = FALSE;
        *ModuleTableEntry = NULL;
        goto End;
    }

    ListEntry = RemoveHeadList(ListHead);

    *ModuleTableEntry = CONTAINING_RECORD(ListEntry,
                                          TRACE_MODULE_TABLE_ENTRY,
                                          DebugContextListEntry);

    Success = TRUE;

End:
    ReleaseGuardedListLockExclusive(GuardedList);

    return Success;
}

FORCEINLINE
BOOL
ConvertStringToWide(
    _In_ PSTRING String,
    _In_ PUNICODE_STRING Unicode,
    _In_ PALLOCATOR Allocator,
    _In_ PPUNICODE_STRING UnicodeStringPointer
    )
{
    CHAR Char;
    WCHAR Wide;
    USHORT Index;
    PUNICODE_STRING UnicodeString;

    if (String->Length > (Unicode->MaximumLength >> 1)) {

        UnicodeString = (PUNICODE_STRING)(
            Allocator->Calloc(
                Allocator->Context,
                1,
                (SIZE_T)(
                    (USHORT)(sizeof(UNICODE_STRING)) +
                    (USHORT)(String->Length << 1)
                )
            )
        );

        if (!UnicodeString) {
            return FALSE;
        }

        UnicodeString->Buffer = (PWCHAR)(
            RtlOffsetToPointer(
                UnicodeString,
                sizeof(UNICODE_STRING)
            )
        );

        UnicodeString->MaximumLength = (String->Length << 1);

    } else {
        UnicodeString = Unicode;
    }

    for (Index = 0; Index < String->Length; Index++) {
        Char = String->Buffer[Index];
        Wide = (WCHAR)Char;
        UnicodeString->Buffer[Index] = Wide;
    }

    UnicodeString->Length = String->Length << 1;
    *UnicodeStringPointer = UnicodeString;

    return TRUE;
}

//
// TraceStoreWorkingSet-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
_Requires_lock_held_(TraceContext->WorkingSetChangesLock)
_Maybe_raises_SEH_exception_
BOOL
(GET_WORKING_SET_CHANGES)(
    _In_ PTRACE_CONTEXT TraceContext
    );
typedef GET_WORKING_SET_CHANGES *PGET_WORKING_SET_CHANGES;
GET_WORKING_SET_CHANGES GetWorkingSetChanges;

BIND_COMPLETE WsWatchInfoExStoreBindComplete;

//
// TraceStoreWorkingSet-related macros.
//

#define AcquireWorkingSetChangesLock(TraceContext)                \
    AcquireSRWLockExclusive(&TraceContext->WorkingSetChangesLock)

#define TryAcquireWorkingSetChangesLock(TraceContext)                \
    TryAcquireSRWLockExclusive(&TraceContext->WorkingSetChangesLock)

#define ReleaseWorkingSetChangesLock(TraceContext)                \
    ReleaseSRWLockExclusive(&TraceContext->WorkingSetChangesLock)

//
// TraceStorePerformance-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
_Requires_lock_held_(TraceContext->CapturePerformanceMetricsLock)
_Maybe_raises_SEH_exception_
BOOL
(CAPTURE_PERFORMANCE_METRICS)(
    _In_ PTRACE_CONTEXT TraceContext
    );
typedef CAPTURE_PERFORMANCE_METRICS *PCAPTURE_PERFORMANCE_METRICS;
CAPTURE_PERFORMANCE_METRICS CapturePerformanceMetrics;

BIND_COMPLETE PerformanceStoreBindComplete;

//
// TraceStorePerformance-related macros.
//

#define AcquireCapturePerformanceMetricsLock(TraceContext)                \
    AcquireSRWLockExclusive(&TraceContext->CapturePerformanceMetricsLock)

#define TryAcquireCapturePerformanceMetricsLock(TraceContext)                \
    TryAcquireSRWLockExclusive(&TraceContext->CapturePerformanceMetricsLock)

#define ReleaseCapturePerformanceMetricsLock(TraceContext)                \
    ReleaseSRWLockExclusive(&TraceContext->CapturePerformanceMetricsLock)

//
// TraceStoreIntervals-related functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(EXTRACT_INTERVALS_OLD)(
    _In_ ULONGLONG NumberOfIntervals,
    _In_ ULONGLONG TicksPerInterval,
    _In_ ULONGLONG NumberOfRecords,
    _In_ ULONGLONG RecordSizeInBytes,
    _In_ PVOID AllocationTimestampBaseAddress,
    _In_ PVOID RecordBaseAddress,
    _Out_writes_(NumberOfIntervals) PTRACE_STORE_INTERVAL Interval,
    _Out_ PULONGLONG ExtractionTicksPointer
    );

typedef
_Check_return_
_Success_(return != 0)
BOOL
(EXTRACT_INTERVALS)(
    _In_ PRTL Rtl,
    _Inout_ PTRACE_STORE_INTERVALS Intervals
    );
typedef EXTRACT_INTERVALS *PEXTRACT_INTERVALS;
extern EXTRACT_INTERVALS ExtractIntervals;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(PREPARE_TRACE_STORE_INTERVALS)(
    _In_ struct _TRACE_STORE *TraceStore
    );
typedef PREPARE_TRACE_STORE_INTERVALS *PPREPARE_TRACE_STORE_INTERVALS;
extern PREPARE_TRACE_STORE_INTERVALS PrepareTraceStoreIntervals;


//
// Sqlite3-related decls.
//

//
// The order of items in this enum must be kept in sync with the string used
// to initialize the string table (TraceStoreSqlite3FunctionsString).
//

typedef enum _TRACE_STORE_SQLITE3_FUNCTION_ID {
    UnknownFunction = -1,

    CountFunctionId,
    AverageFunctionId,
    SumFunctionId,

    //
    // Any enumeration value >= InvalidFunctionId is invalid.  Make sure this
    // always comes last in the enum layout.
    //

    InvalidFunctionId

} TRACE_STORE_SQLITE3_FUNCTION_ID;

//
// Define helper structure that we use to statically define our function
// overloads.  These are defined in TraceStoreConstants.c and used in the
// initialization function in TraceStoreSqlite3ExtInit.c.
//

typedef union _TRACE_STORE_SQLITE3_FUNCTION_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_SQLITE3_FUNCTION_FLAGS;
C_ASSERT(sizeof(TRACE_STORE_SQLITE3_FUNCTION_FLAGS) == sizeof(ULONG));
typedef TRACE_STORE_SQLITE3_FUNCTION_FLAGS *PTRACE_STORE_SQLITE3_FUNCTION_FLAGS;

typedef struct _TRACE_STORE_SQLITE3_FUNCTION {
    TRACE_STORE_SQLITE3_FUNCTION_ID Id;
    TRACE_STORE_SQLITE3_FUNCTION_FLAGS Flags;
    PCSZ Name;
    LONG NumberOfArguments;
    LONG TextEncoding;
    PSQLITE3_SCALAR_FUNCTION ScalarFunction;
    PSQLITE3_AGGREGATE_STEP_FUNCTION AggregateStepFunction;
    PSQLITE3_AGGREGATE_FINAL_FUNCTION AggregateFinalFunction;
    PSQLITE3_FUNCTION_DESTROY DestroyFunction;
} TRACE_STORE_SQLITE3_FUNCTION;
typedef TRACE_STORE_SQLITE3_FUNCTION *PTRACE_STORE_SQLITE3_FUNCTION;
typedef CONST TRACE_STORE_SQLITE3_FUNCTION *PCTRACE_STORE_SQLITE3_FUNCTION;

#define NULL_FUNCTION_ENTRY \
    {                       \
        InvalidFunctionId,  \
        {                   \
            0,              \
        },                  \
        NULL,               \
        0,                  \
        0,                  \
        NULL,               \
        NULL,               \
        NULL                \
    }

#define LAST_FUNCTION_ENTRY NULL_FUNCTION_ENTRY

FORCEINLINE
BOOL
IsLastTraceStoreSqlite3Function(
    _In_ PCTRACE_STORE_SQLITE3_FUNCTION Function
    )
{
    return (
        Function->Id == InvalidFunctionId &&
        Function->Name == NULL
    );
}

#define FOR_EACH_TRACE_STORE_SQLITE3_FUNCTION(Function) \
    for (Function = TraceStoreSqlite3Functions;         \
         !IsLastTraceStoreSqlite3Function(Function);    \
         Function++)

extern SQLITE3_AGGREGATE_STEP_FUNCTION TraceStoreSqlite3CountStep;
extern SQLITE3_AGGREGATE_FINAL_FUNCTION TraceStoreSqlite3CountFinal;
extern SQLITE3_FUNCTION_DESTROY TraceStoreSqlite3CountDestroy;

//
// The sqlite3 virtual function signatures only have an opaque pointer to a
// struct named SQLITE3_CONTEXT; this is useless for us as we rely on being
// able to get to the SQLITE3 API via function parameters.  We could abuse
// the opaqueness of the context and back track to the data we need, but this
// would be fragile and prone to breakage when their internal structures are
// changed.  Instead, we use TLS; when a cursor is opened, relevant details
// are stashed in a TLS slot, which can then be probed in the virtual function
// callbacks.  The following variable is our TLS index.  It is assigned via
// TlsAlloc() in our DLL entry point for PROCESS_ATTACH events.
//

extern ULONG TraceStoreSqlite3TlsIndex;

//
// The PROCESS_ATTACH and PROCESS_ATTACH functions share the same signature.
//

typedef
_Check_return_
_Success_(return != 0)
(TRACE_STORE_SQLITE3_TLS_FUNCTION)(
    _In_    HMODULE     Module,
    _In_    DWORD       Reason,
    _In_    LPVOID      Reserved
    );
typedef TRACE_STORE_SQLITE3_TLS_FUNCTION *PTRACE_STORE_SQLITE3_TLS_FUNCTION;

TRACE_STORE_SQLITE3_TLS_FUNCTION TraceStoreSqlite3TlsProcessAttach;
TRACE_STORE_SQLITE3_TLS_FUNCTION TraceStoreSqlite3TlsProcessDetach;

//
// Define TLS Get/Set cursor functions.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(TRACE_STORE_SQLITE3_TLS_SET_CURSOR)(
    _In_ struct _TRACE_STORE_SQLITE3_CURSOR *Cursor
    );
typedef TRACE_STORE_SQLITE3_TLS_SET_CURSOR *PTRACE_STORE_SQLITE3_TLS_SET_CURSOR;

typedef
_Check_return_
_Success_(return != 0)
struct _TRACE_STORE_SQLITE3_CURSOR *
(TRACE_STORE_SQLITE3_TLS_GET_CURSOR)(
    VOID
    );
typedef TRACE_STORE_SQLITE3_TLS_GET_CURSOR *PTRACE_STORE_SQLITE3_TLS_GET_CURSOR;

extern TRACE_STORE_SQLITE3_TLS_SET_CURSOR TraceStoreSqlite3TlsSetCursor;
extern TRACE_STORE_SQLITE3_TLS_GET_CURSOR TraceStoreSqlite3TlsGetCursor;

//
// Cursor flags.
//

typedef union _TRACE_STORE_SQLITE3_CURSOR_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates a coalesced allocation is currently active.
        // The number of iterations into the coalesced allocation is tracked
        // via CurrentCoalescedAllocationNumber.  When this number reaches
        // TotalCoalescedAllocations, the allocation record will be advanced.
        //

        ULONG IsCoalescedAllocationActive:1;

        //
        // When set, indicates that this is a cursor for a metadata store.
        // This value is primed from the TraceStore->IsMetadata flag.
        //

        ULONG IsMetadataStore:1;

        //
        // When set, indicates that this trace store's traits indicate it that
        // it has a fixed record size, and that the record size is a power of
        // 2.  This makes it easier to calculate the next record's starting
        // address in the cursor's Next() method.
        //

        ULONG FixedRecordSizeAndAlwaysPowerOf2:1;

        //
        // The Eof() virtual table routine will check this flag each time it
        // is called.  If it is set, it will return as if the data stream's
        // end of file was reached.  This is useful for things like overriding
        // count(*) where we don't need to stream through the entire record
        // set because we can just reference Cursor->TotalNumberOfRecords.
        //

        ULONG EofOverride:1;

        //
        // When set, indicates this is an interval cursor.  These differ from
        // normal cursors in that they enumerate over dynamically calculated
        // interval data (versus the normal trace store memory map+allocation
        // approach used by normal cursors).
        //

        ULONG IsIntervalCursor:1;

        //
        // Unused bits.
        //

        ULONG Unused:27;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_SQLITE3_CURSOR_FLAGS;
C_ASSERT(sizeof(TRACE_STORE_SQLITE3_CURSOR_FLAGS) == sizeof(ULONG));

//
// Define a helper union that allows easy casting to common pointer types
// captured by trace stores.
//

typedef union _TRACE_STORE_ROW {
    PTRACE_STORE_METADATA_INFO AsMetadataInfo;
    PTRACE_STORE_ALLOCATION AsAllocation;
    PTRACE_STORE_RELOC AsReloc;
    PTRACE_STORE_ADDRESS AsAddress;
    PTRACE_STORE_ADDRESS_RANGE AsAddressRange;
    PTRACE_STORE_ALLOCATION_TIMESTAMP AsAllocationTimestamp;
    PTRACE_STORE_ALLOCATION_TIMESTAMP_DELTA AsAllocationTimestampDelta;
    PTRACE_STORE_SYNC AsSync;
    PTRACE_STORE_INFO AsInfo;
    PSTR AsStringBuffer;
    PWSTR AsWideStringBuffer;
    PSTRING AsString;
    PUNICODE_STRING AsUnicodeString;
    PBYTE AsByte;
    PCHAR AsChar;
    PSHORT AsShort;
    PUSHORT AsUShort;
    PLONG AsLong;
    PULONG AsULong;
    PLARGE_INTEGER AsLargeInteger;
    PULARGE_INTEGER AsULargeInteger;
    PLONGLONG AsLongLong;
    PULONGLONG AsULongLong;
    PFLOAT AsFloat;
    PDOUBLE AsDouble;
    PVOID AsVoid;
    PTRACE_STORE_INTERVAL AsInterval;
    PSQLITE3_INT64 AsSqlite3Int64;
    PSYMBOL_INFO AsSymbolInfo;
} TRACE_STORE_ROW;
typedef TRACE_STORE_ROW *PTRACE_STORE_ROW;
C_ASSERT(sizeof(TRACE_STORE_ROW) == 8);

typedef enum _TRACE_STORE_SQLITE3_ROWID_CONSTRAINT {

    RowidEqualConstraint                = 1,
    RowidGreaterThanConstraint          = 2,
    RowidGreaterThanOrEqualConstraint   = 3,
    RowidLessThanConstraint             = 4,
    RowidLessThanOrEqualConstraint      = 5,
    RowidInvalidConstraint              = 6,
    RowidMaskConstraint                 = 7

} TRACE_STORE_SQLITE3_ROWID_CONSTRAINT;

typedef union _TRACE_STORE_SQLITE3_ROWID_INDEX {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG EqualConstraint:1;
        ULONG GreaterThanConstraint:1;
        ULONG LessThanConstraint:1;

        ULONG Unused:28;
        ULONG IsRowidIndex:1;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_SQLITE3_ROWID_INDEX;
C_ASSERT(sizeof(TRACE_STORE_SQLITE3_ROWID_INDEX) == sizeof(ULONG));
typedef TRACE_STORE_SQLITE3_ROWID_INDEX *PTRACE_STORE_SQLITE3_ROWID_INDEX;


//
// This structure captures the necessary information about a column's data type
// and offset such that it can be translated into an appropriate sqlite3 result
// during cursor iteration.
//

typedef union _TRACE_STORE_COLUMN_TYPE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_COLUMN_TYPE;
typedef TRACE_STORE_COLUMN_TYPE *PTRACE_STORE_COLUMN_TYPE;

typedef union _TRACE_STORE_COLUMN_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_COLUMN_FLAGS;
typedef TRACE_STORE_COLUMN_FLAGS *PTRACE_STORE_COLUMN_FLAGS;

typedef struct _TRACE_STORE_COLUMN {
    TRACE_STORE_COLUMN_TYPE Type;
    TRACE_STORE_COLUMN_FLAGS Flags;

    LONGLONG Offset;
    ULONGLONG SizeInBytes;

    struct _TRACE_STORE_COLUMN *PointedToColumn;

    PCSZ Name;

} TRACE_STORE_COLUMN;
typedef TRACE_STORE_COLUMN *PTRACE_STORE_COLUMN;

typedef struct _TRACE_STORE_SQLITE3_CURSOR {

    //
    // Copy of the trace store's traits.
    //

    TRACE_STORE_TRAITS Traits;

    //
    // Cursor flags.
    //

    TRACE_STORE_SQLITE3_CURSOR_FLAGS Flags;

    //
    // Rowid tracks the current row/record being processed.
    //

    ULONGLONG Rowid;

    //
    // The first row.  This will always initially point to the base address of
    // the first memory map.  It may be subsequently updated to point at an
    // alternate row if a query has rowid constraints.
    //

    union {
        TRACE_STORE_ROW FirstRow;
        ULONGLONG FirstRowRaw;
    };

    //
    // Likewise, FirstRowid will either be 0 if no constraints have been set
    // filtering row IDs, or something greater than zero if constraints have
    // been set.
    //

    ULONGLONG FirstRowid;

    //
    // The last row.  This will always initially point to the address of the
    // last record of the trace store.  It may be subsequently updated to point
    // at an alternate row if a query has rowid constraints.
    //

    union {
        TRACE_STORE_ROW LastRow;
        ULONGLONG LastRowRaw;
    };

    //
    // Likewise, LastRowid will either be the same as TotalNumberOfRecords if
    // no rowid constraints have been set, otherwise, it will be some value
    // less than this value.
    //

    ULONGLONG LastRowid;

    //
    // The maximum possible rowid value for this query.  This will correspond
    // to TotalNumberOfRecords - 1.
    //

    ULONGLONG MaxRowid;

    //
    // The current row being processed.
    //

    union {
        TRACE_STORE_ROW CurrentRow;
        ULONGLONG CurrentRowRaw;
    };

    //
    // Track how many records and allocations we've iterated over so far.
    //

    LONGLONG CurrentRecordCount;
    LONGLONG CurrentAllocationCount;

    //
    // Total number of records allocated by this store.  This will correspond
    // to the number of rows returned by this cursor if we're doing a full
    // table scan.
    //

    LONGLONG TotalNumberOfRecords;

    //
    // The total number of allocations that were performed by this trace store.
    //

    LONGLONG TotalNumberOfAllocations;

    //
    // If this is a metadata store, the size of each record (allocation).
    // (Metadata stores have fixed records.)
    //

    ULONGLONG MetadataRecordSizeInBytes;

    //
    // If it's possible to ascertain the trace store's record size up front
    // (e.g. if it has a fixed record size that is also a power of 2), this
    // value will be set to a non-zero value.  If it's zero, then the cursor's
    // Next() callback will need to do a little more work to determine where
    // the next row begins (based on the trace store's traits, etc).
    //

    ULONGLONG TraceStoreRecordSizeInBytes;

    //
    // Pointer to the active memory map record.
    //

    PTRACE_STORE_MEMORY_MAP MemoryMap;

    //
    // The following fields only apply to trace stores, not metadata stores.
    //

    //
    // Pointer to the active address record.
    //

    PTRACE_STORE_ADDRESS Address;

    //
    // Pointer to the active address range record.
    //

    PTRACE_STORE_ADDRESS_RANGE AddressRange;

    //
    // Pointer to the active allocation record.
    //

    PTRACE_STORE_ALLOCATION Allocation;

    //
    // If the trace store coalesced allocations, this counter is used to track
    // how far we are through the coalesced allocation.
    //

    LONGLONG CurrentCoalescedAllocationNumber;
    LONGLONG TotalCoalescedAllocations;

    //
    // Pointer back to our owning trace store and trace store db.
    //

    PTRACE_STORE TraceStore;
    struct _TRACE_STORE_SQLITE3_DB *Db;

    //
    // Pointer back to the trace store's intervals, if applicable.
    //

    PTRACE_STORE_INTERVALS Intervals;
    ULONGLONG IntervalRecordSize;

    //
    // Pointer to our column function.
    //

    PTRACE_STORE_SQLITE3_COLUMN Sqlite3Column;

    //
    // Pointer to the sqlite3 API.
    //

    PCSQLITE3 Sqlite3;

    //
    // Timestamp counter of when this cursor was opened.
    //

    LARGE_INTEGER OpenedTimestamp;

    //
    // Filtering information.
    //

    PSQLITE3_VALUE *FilterArguments;
    LONG NumberOfArguments;
    LONG FilterIndexNumber;
    PCSZ FilterIndexStringBuffer;
    STRING FilterIndexString;

    //
    // Inline SQLITE3_VTAB_CURSOR structure.
    //
    // N.B.: This structure must always come last.
    //

    union {
        struct {
            PSQLITE3_VTAB VirtualTable;
        };
        SQLITE3_VTAB_CURSOR AsSqlite3VtabCursor;
    };

} TRACE_STORE_SQLITE3_CURSOR;
typedef TRACE_STORE_SQLITE3_CURSOR *PTRACE_STORE_SQLITE3_CURSOR;

#include "TraceStoreSqlite3Schemas.h"

//
// TraceStore-wrapper module.
//

extern SQLITE3_CREATE TraceStoreSqlite3ModuleCreate;
extern SQLITE3_CONNECT TraceStoreSqlite3ModuleConnect;
extern SQLITE3_BEST_INDEX TraceStoreSqlite3ModuleBestIndex;
extern SQLITE3_DISCONNECT TraceStoreSqlite3ModuleDisconnect;
extern SQLITE3_DESTROY TraceStoreSqlite3ModuleDestroy;
extern SQLITE3_OPEN_CURSOR TraceStoreSqlite3ModuleOpenCursor;
extern SQLITE3_CLOSE_CURSOR TraceStoreSqlite3ModuleCloseCursor;
extern SQLITE3_FILTER TraceStoreSqlite3ModuleFilter;
extern SQLITE3_NEXT TraceStoreSqlite3ModuleNext;
extern SQLITE3_EOF TraceStoreSqlite3ModuleEof;
extern SQLITE3_COLUMN TraceStoreSqlite3ModuleColumn;
extern SQLITE3_ROWID TraceStoreSqlite3ModuleRowid;
extern SQLITE3_UPDATE TraceStoreSqlite3ModuleUpdate;
extern SQLITE3_BEGIN TraceStoreSqlite3ModuleBegin;
extern SQLITE3_SYNC TraceStoreSqlite3ModuleSync;
extern SQLITE3_COMMIT TraceStoreSqlite3ModuleCommit;
extern SQLITE3_ROLLBACK TraceStoreSqlite3ModuleRollback;
extern SQLITE3_FIND_FUNCTION TraceStoreSqlite3ModuleFindFunction;
extern SQLITE3_RENAME TraceStoreSqlite3ModuleRename;
extern SQLITE3_SAVEPOINT TraceStoreSqlite3ModuleSavepoint;
extern SQLITE3_RELEASE TraceStoreSqlite3ModuleRelease;
extern SQLITE3_ROLLBACK_TO TraceStoreSqlite3ModuleRollbackTo;

extern SQLITE3_MODULE TraceStoreSqlite3Module;

//
// Custom interval module (one per normal trace store).
//

extern SQLITE3_CREATE TraceStoreSqlite3IntervalModuleCreate;
extern SQLITE3_CONNECT TraceStoreSqlite3IntervalModuleConnect;
extern SQLITE3_BEST_INDEX TraceStoreSqlite3IntervalModuleBestIndex;
extern SQLITE3_DISCONNECT TraceStoreSqlite3IntervalModuleDisconnect;
extern SQLITE3_DESTROY TraceStoreSqlite3IntervalModuleDestroy;
extern SQLITE3_OPEN_CURSOR TraceStoreSqlite3IntervalModuleOpenCursor;
extern SQLITE3_CLOSE_CURSOR TraceStoreSqlite3IntervalModuleCloseCursor;
extern SQLITE3_FILTER TraceStoreSqlite3IntervalModuleFilter;
extern SQLITE3_NEXT TraceStoreSqlite3IntervalModuleNext;
extern SQLITE3_EOF TraceStoreSqlite3IntervalModuleEof;
extern SQLITE3_COLUMN TraceStoreSqlite3IntervalModuleColumn;
extern SQLITE3_ROWID TraceStoreSqlite3IntervalModuleRowid;
extern SQLITE3_UPDATE TraceStoreSqlite3IntervalModuleUpdate;
extern SQLITE3_BEGIN TraceStoreSqlite3IntervalModuleBegin;
extern SQLITE3_SYNC TraceStoreSqlite3IntervalModuleSync;
extern SQLITE3_COMMIT TraceStoreSqlite3IntervalModuleCommit;
extern SQLITE3_ROLLBACK TraceStoreSqlite3IntervalModuleRollback;
extern SQLITE3_FIND_FUNCTION TraceStoreSqlite3IntervalModuleFindFunction;
extern SQLITE3_RENAME TraceStoreSqlite3IntervalModuleRename;
extern SQLITE3_SAVEPOINT TraceStoreSqlite3IntervalModuleSavepoint;
extern SQLITE3_RELEASE TraceStoreSqlite3IntervalModuleRelease;
extern SQLITE3_ROLLBACK_TO TraceStoreSqlite3IntervalModuleRollbackTo;

extern SQLITE3_MODULE TraceStoreSqlite3IntervalModule;


FORCEINLINE
VOID
CopySqlite3ModuleInline(
    _Out_writes_bytes_all_(sizeof(SQLITE3_MODULE)) PSQLITE3_MODULE Dest,
    PSQLITE3_MODULE Source
    )
{
    CopyMemory((PBYTE)Dest, (PBYTE)Source, sizeof(*Dest));
}

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

extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3DefaultColumnImpl;

extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3MetadataInfoColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3AllocationColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3RelocationColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3AddressColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3AddressRangeColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3AllocationTimestampColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3AllocationTimestampDeltaColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3SynchronizationColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3InfoColumn;
extern TRACE_STORE_SQLITE3_COLUMN TraceStoreSqlite3IntervalColumn;

extern SQLITE3_SCALAR_FUNCTION TraceStoreSqlite3DefaultScalarFunction;
extern SQLITE3_AGGREGATE_STEP_FUNCTION
       TraceStoreSqlite3DefaultAggregateStepFunction;
extern SQLITE3_AGGREGATE_FINAL_FUNCTION
       TraceStoreSqlite3DefaultAggregateFinalFunction;

typedef union _TRACE_STORE_SQLITE3_DB_FLAGS {
    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates CUDA has been initialized and is ready for use
        // (via the Cu interface pointer).
        //

        ULONG CuAvailable:1;

        //
        // Unused.
        //

        ULONG Unused:31;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_SQLITE3_DB_FLAGS;
C_ASSERT(sizeof(TRACE_STORE_SQLITE3_DB_FLAGS) == sizeof(ULONG));

typedef union _TRACE_STORE_SQLITE3_DB_STATE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_SQLITE3_DB_STATE;
C_ASSERT(sizeof(TRACE_STORE_SQLITE3_DB_STATE) == sizeof(ULONG));

typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACE_STORE_SQLITE3_DB {

    _Field_range_(==, sizeof(struct _TRACE_STORE_SQLITE3_DB))
    ULONG SizeOfStruct;

    //
    // Flags.
    //

    TRACE_STORE_SQLITE3_DB_FLAGS Flags;

    //
    // State.
    //

    TRACE_STORE_SQLITE3_DB_STATE State;

    //
    // Sqlite3-specific fields.
    //

    PALLOCATOR Sqlite3Allocator;
    PSQLITE3_DB Sqlite3Db;
    PCSQLITE3 Sqlite3;

    //
    // Our internal fields.
    //

    PRTL Rtl;
    PTRACER_CONFIG TracerConfig;
    PUNICODE_STRING TraceSessionDirectory;

    HANDLE OwningModule;
    PRTL_PATH OwningModulePath;

    //
    // Pointer to a CUDA Device structure if available.
    //

    PCU Cu;

    //
    // We prepare CUDA in a separate thread.  The following event is set when
    // the InitCu() threadpool callback has completed.  Note that completion
    // doesn't imply success; the caller must check the CuAvailable flag.
    //

    HANDLE CuLoadingCompleteEvent;

    //
    // Our CU_MODULE for TraceStoreKernels, plus context and associated funcs.
    //

    PCU_MODULE CuModule;
    PCU_CONTEXT CuContext;
    PCU_FUNCTION CuFunctions[2];

    //
    // Device and attributes.
    //

    CU_DEVICE CuDevice;
    CU_DEVICE_ATTRIBUTES CuDeviceAttributes;

    //
    // Our core tracing/support modules.
    //

    HMODULE TracerHeapModule;
    HMODULE StringTableModule;

    //
    // TracerHeap-specific functions.
    //

    PINITIALIZE_HEAP_ALLOCATOR_EX InitializeHeapAllocatorEx;

    //
    // StringTable-specific functions.
    //

    PINITIALIZE_STRING_TABLE_ALLOCATOR InitializeStringTableAllocator;
    PCREATE_STRING_TABLE CreateStringTable;
    PDESTROY_STRING_TABLE DestroyStringTable;

    PCREATE_STRING_TABLE_FROM_DELIMITED_STRING
        CreateStringTableFromDelimitedString;

    PCREATE_STRING_TABLE_FROM_DELIMITED_ENVIRONMENT_VARIABLE
        CreateStringTableFromDelimitedEnvironmentVariable;

    ALLOCATOR StringTableAllocator;
    ALLOCATOR StringArrayAllocator;

    USHORT NumberOfFunctionStringTables;
    USHORT Padding[3];

    PSTRING_TABLE FunctionStringTable1;

    //
    // Threadpool and callback environment.
    //

    ULONG MaximumProcessorCount;
    PTP_POOL Threadpool;
    TP_CALLBACK_ENVIRON ThreadpoolCallbackEnviron;

    //
    // A threadpool with 1 thread limited to cancellations.
    //

    PTP_POOL CancellationThreadpool;
    TP_CALLBACK_ENVIRON CancellationThreadpoolCallbackEnviron;

    ALLOCATOR Allocator;

    PTRACE_SESSION TraceSession;
    PTRACE_CONTEXT TraceContext;
    PTRACE_STORES TraceStores;

} TRACE_STORE_SQLITE3_DB;
typedef TRACE_STORE_SQLITE3_DB *PTRACE_STORE_SQLITE3_DB;

//
// Cuda-related type and function definitions.
//

typedef union _Struct_size_bytes_(sizeof(ULONG)) {

    struct _Struct_size_bytes_(sizeof(ULONG)) {

        //
        // When set, indicates CUDA has been initialized and is ready for use
        // (via the Cu interface pointer).
        //

        ULONG CuAvailable:1;

        //
        // Unused bits.
        //

        ULONG Unused:31;
    };

    LONG AsLong;
    ULONG AsULong;

} TRACE_STORE_CUDA_FLAGS;
C_ASSERT(sizeof(TRACE_STORE_CUDA_FLAGS) == sizeof(ULONG));

typedef union _TRACE_STORE_CUDA_STATE {
    struct _Struct_size_bytes_(sizeof(ULONG)) {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} TRACE_STORE_CUDA_STATE;
C_ASSERT(sizeof(TRACE_STORE_CUDA_STATE) == sizeof(ULONG));


typedef struct _Struct_size_bytes_(SizeOfStruct) _TRACE_STORE_CUDA {

    //
    // Structure size, in bytes.
    //

    _Field_range_(==, sizeof(struct _TRACE_STORE_CUDA)) ULONG SizeOfStruct;

    //
    // Flags.
    //

    TRACE_STORE_CUDA_FLAGS Flags;

    //
    // State.
    //

    TRACE_STORE_CUDA_STATE State;

    //
    // Pointer to a CUDA Device structure if available.
    //

    PCU Cu;

    //
    // We prepare CUDA in a separate thread.  The following event is set when
    // the InitCu() threadpool callback has completed.  Note that completion
    // doesn't imply success; the caller must check the CuAvailable flag.
    //

    HANDLE CuLoadingCompleteEvent;

    //
    // Our CU_MODULE for TraceStoreKernels, plus context.
    //

    PCU_MODULE CuModule;
    PCU_CONTEXT CuContext;

    //
    // Device and attributes.
    //

    CU_DEVICE CuDevice;
    CU_DEVICE_ATTRIBUTES CuDeviceAttributes;

    //
    // Ptx paths and functions.
    //

    USHORT NumberOfPtxPaths;
    USHORT NumberOfFunctions;
    ULONG Padding;

    PTRACER_PTX_PATH_ID PtxPathIds;
    PCSZ *PtxFunctionNames;
    PPCU_FUNCTION CuFunctions;

} TRACE_STORE_CUDA;
typedef TRACE_STORE_CUDA *PTRACE_STORE_CUDA;

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
