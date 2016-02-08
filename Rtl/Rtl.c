#include "Rtl.h"

INIT_ONCE InitOnceSystemTimerFunction = INIT_ONCE_STATIC_INIT;

PVECTORED_EXCEPTION_HANDLER VectoredExceptionHandler = NULL;

INIT_ONCE InitOnceCSpecificHandler = INIT_ONCE_STATIC_INIT;

LONG
WINAPI
CopyMappedMemoryVectoredExceptionHandler(
_In_ PEXCEPTION_POINTERS ExceptionInfo
)
{
    PCONTEXT Context = ExceptionInfo->ContextRecord;
    PEXCEPTION_RECORD Exception = ExceptionInfo->ExceptionRecord;

    if (Exception->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) {
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

BOOL
CALLBACK
SetCSpecificHandlerCallback(
    PINIT_ONCE InitOnce,
    PVOID Parameter,
    PVOID *lpContext
)
{
    VectoredExceptionHandler = AddVectoredExceptionHandler(1, CopyMappedMemoryVectoredExceptionHandler);
    return (BOOL)VectoredExceptionHandler;
}

BOOL
SetCSpecificHandler(PCSPECIFICHANDLER Handler)
{
    BOOL Status;

    Status = InitOnceExecuteOnce(
        &InitOnceCSpecificHandler,
        SetCSpecificHandlerCallback,
        Handler,
        NULL
    );

    return Status;
}

BOOL
CALLBACK
GetSystemTimerFunctionCallback(
    _Inout_     PINIT_ONCE  InitOnce,
    _Inout_     PVOID       Parameter,
    _Inout_opt_ PVOID       *lpContext
)
{
    HMODULE Module;
    FARPROC Proc;
    static SYSTEM_TIMER_FUNCTION SystemTimerFunction = { 0 };

    if (!lpContext) {
        return FALSE;
    }

    Module = GetModuleHandle(TEXT("kernel32"));
    if (Module == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    Proc = GetProcAddress(Module, "GetSystemTimePreciseAsFileTime");
    if (Proc) {
        SystemTimerFunction.GetSystemTimePreciseAsFileTime = (PGETSYSTEMTIMEPRECISEASFILETIME)Proc;
    } else {
        Module = LoadLibrary(TEXT("ntdll"));
        if (!Module) {
            return FALSE;
        }
        Proc = GetProcAddress(Module, "NtQuerySystemTime");
        if (!Proc) {
            return FALSE;
        }
        SystemTimerFunction.NtQuerySystemTime = (PNTQUERYSYSTEMTIME)Proc;
    }

    *((PPSYSTEM_TIMER_FUNCTION)lpContext) = &SystemTimerFunction;
    return TRUE;
}

PSYSTEM_TIMER_FUNCTION
GetSystemTimerFunction()
{
    BOOL Status;
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction;

    Status = InitOnceExecuteOnce(
        &InitOnceSystemTimerFunction,
        GetSystemTimerFunctionCallback,
        NULL,
        (LPVOID *)&SystemTimerFunction
    );

    if (!Status) {
        return NULL;
    } else {
        return SystemTimerFunction;
    }
}

_Check_return_
BOOL
CallSystemTimer(
    _Out_       PFILETIME   SystemTime,
    _Inout_opt_ PPSYSTEM_TIMER_FUNCTION ppSystemTimerFunction
)
{
    PSYSTEM_TIMER_FUNCTION SystemTimerFunction = NULL;

    if (ppSystemTimerFunction) {
        if (*ppSystemTimerFunction) {
            SystemTimerFunction = *ppSystemTimerFunction;
        } else {
            SystemTimerFunction = GetSystemTimerFunction();
            *ppSystemTimerFunction = SystemTimerFunction;
        }
    } else {
        SystemTimerFunction = GetSystemTimerFunction();
    }

    if (!SystemTimerFunction) {
        return FALSE;
    }

    if (SystemTimerFunction->GetSystemTimePreciseAsFileTime) {
        SystemTimerFunction->GetSystemTimePreciseAsFileTime(SystemTime);
    } else if (SystemTimerFunction->NtQuerySystemTime) {
        if (!SystemTimerFunction->NtQuerySystemTime((PLARGE_INTEGER)SystemTime)) {
            return FALSE;
        }
    } else {
        return FALSE;
    }

    return TRUE;
}

_Check_return_
PVOID
CopyToMemoryMappedMemory(
    PVOID Destination,
    LPCVOID Source,
    SIZE_T Size
)
{
    return memcpy(Destination, Source, Size);
}

_Check_return_
BOOL
LoadRtlSymbols(_Inout_ PRTL Rtl)
{

    if (!(Rtl->Kernel32Module = LoadLibraryA("kernel32"))) {
        return FALSE;
    }

    if (!(Rtl->NtdllModule = LoadLibraryA("ntdll"))) {
        return FALSE;
    }

    Rtl->GetSystemTimePreciseAsFileTime = (PGETSYSTEMTIMEPRECISEASFILETIME)
        GetProcAddress(Rtl->Kernel32Module, "GetSystemTimePreciseAsFileTime");

    Rtl->NtQuerySystemTime = (PNTQUERYSYSTEMTIME)
        GetProcAddress(Rtl->NtdllModule, "NtQuerySystemTime");

    if (Rtl->GetSystemTimePreciseAsFileTime) {
        Rtl->SystemTimerFunction.GetSystemTimePreciseAsFileTime =
            Rtl->GetSystemTimePreciseAsFileTime;
    } else if (Rtl->NtQuerySystemTime) {
        Rtl->SystemTimerFunction.NtQuerySystemTime =
            Rtl->NtQuerySystemTime;
    } else {
        return FALSE;
    }

    //
    // Start of auto-generated function resolutions.  Any manual modifications
    // will be lost; run `tracerdev sync-rtl-header` to keep in sync.
    //

    if (!(Rtl->RtlCharToInteger = (PRTLCHARTOINTEGER)
        GetProcAddress(Rtl->NtdllModule, "RtlCharToInteger"))) {
        return FALSE;
    }

    if (!(Rtl->RtlInitializeGenericTable = (PRTL_INITIALIZE_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeGenericTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlInsertElementGenericTable = (PRTL_INSERT_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlInsertElementGenericTableFull = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTableFull"))) {
        return FALSE;
    }

    if (!(Rtl->RtlDeleteElementGenericTable = (PRTL_DELETE_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlDeleteElementGenericTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlLookupElementGenericTable = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlLookupElementGenericTableFull = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTableFull"))) {
        return FALSE;
    }

    if (!(Rtl->RtlEnumerateGenericTable = (PRTL_ENUMERATE_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlEnumerateGenericTableWithoutSplaying = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTableWithoutSplaying"))) {
        return FALSE;
    }

    if (!(Rtl->RtlGetElementGenericTable = (PRTL_GET_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlGetElementGenericTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlNumberGenericTableElements = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberGenericTableElements"))) {
        return FALSE;
    }

    if (!(Rtl->RtlIsGenericTableEmpty = (PRTL_IS_GENERIC_TABLE_EMPTY)
        GetProcAddress(Rtl->NtdllModule, "RtlIsGenericTableEmpty"))) {
        return FALSE;
    }

    if (!(Rtl->PfxInitialize = (PPFX_INITIALIZE)
        GetProcAddress(Rtl->NtdllModule, "PfxInitialize"))) {
        return FALSE;
    }

    if (!(Rtl->PfxInsertPrefix = (PPFX_INSERT_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxInsertPrefix"))) {
        return FALSE;
    }

    if (!(Rtl->PfxRemovePrefix = (PPFX_REMOVE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxRemovePrefix"))) {
        return FALSE;
    }

    if (!(Rtl->PfxFindPrefix = (PPFX_FIND_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxFindPrefix"))) {
        return FALSE;
    }

    if (!(Rtl->RtlCrc32 = (PRTLCRC32)
        GetProcAddress(Rtl->NtdllModule, "RtlCrc32"))) {
        return FALSE;
    }

    if (!(Rtl->RtlCrc64 = (PRTLCRC64)
        GetProcAddress(Rtl->NtdllModule, "RtlCrc64"))) {
        return FALSE;
    }

    if (!(Rtl->RtlPrefixUnicodeString = (PRTL_PREFIX_UNICODE_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlPrefixUnicodeString"))) {
        return FALSE;
    }

    if (!(Rtl->RtlCreateHashTable = (PRTL_CREATE_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlCreateHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlCreateHashTableEx = (PRTL_CREATE_HASH_TABLE_EX)
        GetProcAddress(Rtl->NtdllModule, "RtlCreateHashTableEx"))) {
        return FALSE;
    }

    if (!(Rtl->RtlDeleteHashTable = (PRTL_DELETE_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlDeleteHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlInsertEntryHashTable = (PRTL_INSERT_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertEntryHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlRemoveEntryHashTable = (PRTL_REMOVE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlRemoveEntryHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlLookupEntryHashTable = (PRTL_LOOKUP_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupEntryHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlGetNextEntryHashTable = (PRTL_GET_NEXT_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlGetNextEntryHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlEnumerateEntryHashTable = (PRTL_ENUMERATE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateEntryHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlEndEnumerationHashTable = (PRTL_END_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEndEnumerationHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlInitWeakEnumerationHashTable = (PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInitWeakEnumerationHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlWeaklyEnumerateEntryHashTable = (PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlWeaklyEnumerateEntryHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlEndWeakEnumerationHashTable = (PRTL_END_WEAK_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEndWeakEnumerationHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlInitStrongEnumerationHashTable = (PRTL_INIT_STRONG_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInitStrongEnumerationHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlStronglyEnumerateEntryHashTable = (PRTL_STRONGLY_ENUMERATE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlStronglyEnumerateEntryHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlEndStrongEnumerationHashTable = (PRTL_END_STRONG_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEndStrongEnumerationHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlExpandHashTable = (PRTL_EXPAND_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlExpandHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlContractHashTable = (PRTL_CONTRACT_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlContractHashTable"))) {
        return FALSE;
    }

    if (!(Rtl->RtlInitializeBitMap = (PRTL_INITIALIZE_BITMAP)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeBitMap"))) {
        return FALSE;
    }

    if (!(Rtl->RtlClearBit = (PRTL_CLEAR_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlClearBit"))) {
        return FALSE;
    }

    if (!(Rtl->RtlSetBit = (PRTL_SET_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlSetBit"))) {
        return FALSE;
    }

    if (!(Rtl->RtlTestBit = (PRTL_TEST_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlTestBit"))) {
        return FALSE;
    }

    if (!(Rtl->RtlClearAllBits = (PRTL_CLEAR_ALL_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlClearAllBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlSetAllBits = (PRTL_SET_ALL_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlSetAllBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlFindClearBits = (PRTL_FIND_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlFindSetBits = (PRTL_FIND_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindSetBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlFindClearBitsAndSet = (PRTL_FIND_CLEAR_BITS_AND_SET)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearBitsAndSet"))) {
        return FALSE;
    }

    if (!(Rtl->RtlFindSetBitsAndClear = (PRTL_FIND_SET_BITS_AND_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindSetBitsAndClear"))) {
        return FALSE;
    }

    if (!(Rtl->RtlClearBits = (PRTL_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlClearBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlSetBits = (PRTL_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlSetBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlFindClearRuns = (PRTL_FIND_CLEAR_RUNS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearRuns"))) {
        return FALSE;
    }

    if (!(Rtl->RtlFindLongestRunClear = (PRTL_FIND_LONGEST_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindLongestRunClear"))) {
        return FALSE;
    }

    if (!(Rtl->RtlNumberOfClearBits = (PRTL_NUMBER_OF_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberOfClearBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlNumberOfSetBits = (PRTL_NUMBER_OF_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberOfSetBits"))) {
        return FALSE;
    }

    if (!(Rtl->RtlAreBitsClear = (PRTL_ARE_BITS_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlAreBitsClear"))) {
        return FALSE;
    }

    if (!(Rtl->RtlAreBitsSet = (PRTL_ARE_BITS_SET)
        GetProcAddress(Rtl->NtdllModule, "RtlAreBitsSet"))) {
        return FALSE;
    }

    if (!(Rtl->RtlFindNextForwardRunClear = (PRTL_FIND_NEXT_FORWARD_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindNextForwardRunClear"))) {
        return FALSE;
    }

    //
    // End of auto-generated function resolutions.
    //

    return TRUE;
}


BOOLEAN
RtlCheckBit(
    _In_ PRTL_BITMAP BitMapHeader,
    _In_ ULONG BitPosition
    )
{
    return BitTest64((LONG64 const *)BitMapHeader->Buffer, (LONG64)BitPosition);
}

_Check_return_
BOOL
LoadRtlExFunctions(
    _In_opt_ HMODULE RtlExModule,
    _Inout_  PRTLEXFUNCTIONS RtlExFunctions
    )
{
    if (!RtlExModule) {
        return FALSE;
    }

    if (!RtlExFunctions) {
        return FALSE;
    }

    if (!(RtlExFunctions->CopyToMemoryMappedMemory = (PCOPYTOMEMORYMAPPEDMEMORY)
        GetProcAddress(RtlExModule, "CopyToMemoryMappedMemory"))) {
        return FALSE;
    }

    return TRUE;
}

_Check_return_
BOOL
LoadRtlExSymbols(
    _In_opt_ HMODULE RtlExModule,
    _Inout_  PRTL    Rtl
)
{
    HMODULE Module;

    if (!Rtl) {
        return FALSE;
    }

    if (RtlExModule) {
        Module = RtlExModule;

    } else {

        DWORD Flags = (
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS          |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
        );

        if (!GetModuleHandleEx(Flags, (LPCTSTR)&LoadRtlExFunctions, &Module)) {
            return FALSE;
        }

        if (!Module) {
            return FALSE;
        }
    }

    if (!LoadRtlExFunctions(Module, &Rtl->RtlExFunctions)) {
        return FALSE;
    }

    return TRUE;

}

BOOL
InitializeRtl(
    _Out_bytecap_(*SizeOfRtl) PRTL   Rtl,
    _Inout_                   PULONG SizeOfRtl
    )
{
    if (!Rtl) {
        if (SizeOfRtl) {
            *SizeOfRtl = sizeof(*Rtl);
        }
        return FALSE;
    }

    if (!SizeOfRtl) {
        return FALSE;
    }

    if (*SizeOfRtl < sizeof(*Rtl)) {
        *SizeOfRtl = sizeof(*Rtl);
        return FALSE;
    } else {
        *SizeOfRtl = sizeof(*Rtl);
    }

    SecureZeroMemory(Rtl, sizeof(*Rtl));

    if (!LoadRtlSymbols(Rtl)) {
        return FALSE;
    }

    if (!LoadRtlExSymbols(NULL, Rtl)) {
        return FALSE;
    }

    return TRUE;
}

VOID
Debugbreak()
{
    __debugbreak();
}
