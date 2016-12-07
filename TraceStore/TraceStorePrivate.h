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

typedef _Struct_size_bytes_(SizeOfStruct) struct _TRACE_STORES_RUNDOWN {

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

} TRACE_STORES_RUNDOWN, *PTRACE_STORES_RUNDOWN;

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
BIND_COMPLETE AllocationTimestampBindComplete;
BIND_COMPLETE AllocationTimestampDeltaBindComplete;
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
PTRACE_STORE_TRAITS
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

    if (MemoryMap != &TraceStore->SingleMemoryMap) {
        SecureZeroMemory(MemoryMap, sizeof(*MemoryMap));

        InterlockedPushEntrySList(
            &TraceStore->FreeMemoryMaps,
            &MemoryMap->ListEntry
        );
    }

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
ALLOCATE_RECORDS_WITH_TIMESTAMP SuspendedTraceStoreAllocateRecordsWithTimestamp;
ALLOCATE_RECORDS_WITH_TIMESTAMP TraceStoreAllocateRecordsWithTimestampWorker;

typedef
_Success_(return != 0)
BOOL
(RECORD_TRACE_STORE_ALLOCATION)(
    _In_ PTRACE_STORE     TraceStore,
    _In_ PULARGE_INTEGER  RecordSize,
    _In_ PULARGE_INTEGER  NumberOfRecords,
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
    ULARGE_INTEGER DeltaRecordSize;
    ULARGE_INTEGER TimestampRecordSize;
    ULARGE_INTEGER NumberOfRecords;

    if (Timestamp.QuadPart == 0) {
        return TRUE;
    }

    DeltaRecordSize.QuadPart = sizeof(Delta);
    TimestampRecordSize.QuadPart = sizeof(Timestamp);
    NumberOfRecords.QuadPart = 1;

    TimestampPointer = (PLARGE_INTEGER)(
        TraceStore->AllocationTimestampStore->AllocateRecords(
            TraceStore->TraceContext,
            TraceStore->AllocationTimestampStore,
            &TimestampRecordSize,
            &NumberOfRecords
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
                    &DeltaRecordSize,
                    &NumberOfRecords
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
        &SuspendedTraceStoreAllocateRecordsWithTimestamp
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
        &TraceStoreAllocateRecordsWithTimestamp
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
REALLOC TraceStoreAllocatorRealloc;
FREE TraceStoreAllocatorFree;
FREE_POINTER TraceStoreAllocatorFreePointer;
INITIALIZE_ALLOCATOR TraceStoreInitializeAllocator;
DESTROY_ALLOCATOR TraceStoreDestroyAllocator;

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
// TraceStoreSession-related functions.
//

INITIALIZE_TRACE_SESSION InitializeTraceSession;

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
    _In_opt_ LARGE_INTEGER MappingSize
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
    _In_ PTRACE_STORE InfoStore,
    _In_ LARGE_INTEGER InitialSize,
    _In_ LARGE_INTEGER MappingSize,
    _In_ PTRACE_FLAGS TraceFlags,
    _In_ PTRACE_STORE_RELOC Reloc
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
VOID
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
        return;
    }

    //
    // Take a local copy of the address record, update timestamps and
    // calculate elapsed time, then save the local record back to the
    // backing TRACE_STORE_ADDRESS struct.
    //

    if (!CopyTraceStoreAddress(&Address, AddressPointer)) {
        return;
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
    // Copy the local record back to the backing store and ignore the return
    // value.
    //

    if (!CopyTraceStoreAddress(AddressPointer, &Address)) {
        NOTHING;
    }

    return;
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

#define AcquireWorkingSetChangesLock(TraceContext) \
    AcquireSRWLockExclusive(&TraceContext->WorkingSetChangesLock)

#define TryAcquireWorkingSetChangesLock(TraceContext) \
    TryAcquireSRWLockExclusive(&TraceContext->WorkingSetChangesLock)

#define ReleaseWorkingSetChangesLock(TraceContext) \
    ReleaseSRWLockExclusive(&TraceContext->WorkingSetChangesLock)

#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
