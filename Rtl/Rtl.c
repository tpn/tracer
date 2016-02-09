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

    if (!(Rtl->NtosKrnlModule = LoadLibraryA("ntoskrnl.exe"))) {
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
    // will be lost; run `tracerdev sync-rtl-header` to keep in sync.  If you
    // need to tweak the template, make sure the first block (RtlCharToInteger)
    // matches the new template -- the sync-rtl-header command relies on the
    // first block to match in order to determine the starting point for where
    // the blocks should be placed.
    //

    if (!(Rtl->RtlCharToInteger = (PRTLCHARTOINTEGER)
        GetProcAddress(Rtl->NtdllModule, "RtlCharToInteger"))) {

        if (!(Rtl->RtlCharToInteger = (PRTLCHARTOINTEGER)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCharToInteger"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlCharToInteger'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInitializeGenericTable = (PRTL_INITIALIZE_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeGenericTable"))) {

        if (!(Rtl->RtlInitializeGenericTable = (PRTL_INITIALIZE_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitializeGenericTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInitializeGenericTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInsertElementGenericTable = (PRTL_INSERT_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTable"))) {

        if (!(Rtl->RtlInsertElementGenericTable = (PRTL_INSERT_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertElementGenericTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInsertElementGenericTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInsertElementGenericTableFull = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertElementGenericTableFull"))) {

        if (!(Rtl->RtlInsertElementGenericTableFull = (PRTL_INSERT_ELEMENT_GENERIC_TABLE_FULL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertElementGenericTableFull"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInsertElementGenericTableFull'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlDeleteElementGenericTable = (PRTL_DELETE_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlDeleteElementGenericTable"))) {

        if (!(Rtl->RtlDeleteElementGenericTable = (PRTL_DELETE_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlDeleteElementGenericTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlDeleteElementGenericTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlLookupElementGenericTable = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTable"))) {

        if (!(Rtl->RtlLookupElementGenericTable = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupElementGenericTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlLookupElementGenericTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlLookupElementGenericTableFull = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupElementGenericTableFull"))) {

        if (!(Rtl->RtlLookupElementGenericTableFull = (PRTL_LOOKUP_ELEMENT_GENERIC_TABLE_FULL)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupElementGenericTableFull"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlLookupElementGenericTableFull'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlEnumerateGenericTable = (PRTL_ENUMERATE_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTable"))) {

        if (!(Rtl->RtlEnumerateGenericTable = (PRTL_ENUMERATE_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateGenericTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateGenericTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlEnumerateGenericTableWithoutSplaying = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateGenericTableWithoutSplaying"))) {

        if (!(Rtl->RtlEnumerateGenericTableWithoutSplaying = (PRTL_ENUMERATE_GENERIC_TABLE_WITHOUT_SPLAYING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateGenericTableWithoutSplaying"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateGenericTableWithoutSplaying'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlGetElementGenericTable = (PRTL_GET_ELEMENT_GENERIC_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlGetElementGenericTable"))) {

        if (!(Rtl->RtlGetElementGenericTable = (PRTL_GET_ELEMENT_GENERIC_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlGetElementGenericTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlGetElementGenericTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlNumberGenericTableElements = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberGenericTableElements"))) {

        if (!(Rtl->RtlNumberGenericTableElements = (PRTL_NUMBER_GENERIC_TABLE_ELEMENTS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNumberGenericTableElements"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlNumberGenericTableElements'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlIsGenericTableEmpty = (PRTL_IS_GENERIC_TABLE_EMPTY)
        GetProcAddress(Rtl->NtdllModule, "RtlIsGenericTableEmpty"))) {

        if (!(Rtl->RtlIsGenericTableEmpty = (PRTL_IS_GENERIC_TABLE_EMPTY)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlIsGenericTableEmpty"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlIsGenericTableEmpty'");
            return FALSE;
        }
    }

    if (!(Rtl->PfxInitialize = (PPFX_INITIALIZE)
        GetProcAddress(Rtl->NtdllModule, "PfxInitialize"))) {

        if (!(Rtl->PfxInitialize = (PPFX_INITIALIZE)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxInitialize"))) {

            OutputDebugStringA("Rtl: failed to resolve 'PfxInitialize'");
            return FALSE;
        }
    }

    if (!(Rtl->PfxInsertPrefix = (PPFX_INSERT_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxInsertPrefix"))) {

        if (!(Rtl->PfxInsertPrefix = (PPFX_INSERT_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxInsertPrefix"))) {

            OutputDebugStringA("Rtl: failed to resolve 'PfxInsertPrefix'");
            return FALSE;
        }
    }

    if (!(Rtl->PfxRemovePrefix = (PPFX_REMOVE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxRemovePrefix"))) {

        if (!(Rtl->PfxRemovePrefix = (PPFX_REMOVE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxRemovePrefix"))) {

            OutputDebugStringA("Rtl: failed to resolve 'PfxRemovePrefix'");
            return FALSE;
        }
    }

    if (!(Rtl->PfxFindPrefix = (PPFX_FIND_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "PfxFindPrefix"))) {

        if (!(Rtl->PfxFindPrefix = (PPFX_FIND_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "PfxFindPrefix"))) {

            OutputDebugStringA("Rtl: failed to resolve 'PfxFindPrefix'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlPrefixUnicodeString = (PRTL_PREFIX_UNICODE_STRING)
        GetProcAddress(Rtl->NtdllModule, "RtlPrefixUnicodeString"))) {

        if (!(Rtl->RtlPrefixUnicodeString = (PRTL_PREFIX_UNICODE_STRING)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlPrefixUnicodeString"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlPrefixUnicodeString'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlCreateHashTable = (PRTL_CREATE_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlCreateHashTable"))) {

        if (!(Rtl->RtlCreateHashTable = (PRTL_CREATE_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlCreateHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlCreateHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlDeleteHashTable = (PRTL_DELETE_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlDeleteHashTable"))) {

        if (!(Rtl->RtlDeleteHashTable = (PRTL_DELETE_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlDeleteHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlDeleteHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInsertEntryHashTable = (PRTL_INSERT_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertEntryHashTable"))) {

        if (!(Rtl->RtlInsertEntryHashTable = (PRTL_INSERT_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertEntryHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInsertEntryHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlRemoveEntryHashTable = (PRTL_REMOVE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlRemoveEntryHashTable"))) {

        if (!(Rtl->RtlRemoveEntryHashTable = (PRTL_REMOVE_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlRemoveEntryHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlRemoveEntryHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlLookupEntryHashTable = (PRTL_LOOKUP_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlLookupEntryHashTable"))) {

        if (!(Rtl->RtlLookupEntryHashTable = (PRTL_LOOKUP_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlLookupEntryHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlLookupEntryHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlGetNextEntryHashTable = (PRTL_GET_NEXT_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlGetNextEntryHashTable"))) {

        if (!(Rtl->RtlGetNextEntryHashTable = (PRTL_GET_NEXT_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlGetNextEntryHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlGetNextEntryHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlEnumerateEntryHashTable = (PRTL_ENUMERATE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEnumerateEntryHashTable"))) {

        if (!(Rtl->RtlEnumerateEntryHashTable = (PRTL_ENUMERATE_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEnumerateEntryHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlEnumerateEntryHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlEndEnumerationHashTable = (PRTL_END_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEndEnumerationHashTable"))) {

        if (!(Rtl->RtlEndEnumerationHashTable = (PRTL_END_ENUMERATION_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEndEnumerationHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlEndEnumerationHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInitWeakEnumerationHashTable = (PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlInitWeakEnumerationHashTable"))) {

        if (!(Rtl->RtlInitWeakEnumerationHashTable = (PRTL_INIT_WEAK_ENUMERATION_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitWeakEnumerationHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInitWeakEnumerationHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlWeaklyEnumerateEntryHashTable = (PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlWeaklyEnumerateEntryHashTable"))) {

        if (!(Rtl->RtlWeaklyEnumerateEntryHashTable = (PRTL_WEAKLY_ENUMERATE_ENTRY_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlWeaklyEnumerateEntryHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlWeaklyEnumerateEntryHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlEndWeakEnumerationHashTable = (PRTL_END_WEAK_ENUMERATION_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlEndWeakEnumerationHashTable"))) {

        if (!(Rtl->RtlEndWeakEnumerationHashTable = (PRTL_END_WEAK_ENUMERATION_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlEndWeakEnumerationHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlEndWeakEnumerationHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlExpandHashTable = (PRTL_EXPAND_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlExpandHashTable"))) {

        if (!(Rtl->RtlExpandHashTable = (PRTL_EXPAND_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlExpandHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlExpandHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlContractHashTable = (PRTL_CONTRACT_HASH_TABLE)
        GetProcAddress(Rtl->NtdllModule, "RtlContractHashTable"))) {

        if (!(Rtl->RtlContractHashTable = (PRTL_CONTRACT_HASH_TABLE)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlContractHashTable"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlContractHashTable'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInitializeBitMap = (PRTL_INITIALIZE_BITMAP)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeBitMap"))) {

        if (!(Rtl->RtlInitializeBitMap = (PRTL_INITIALIZE_BITMAP)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitializeBitMap"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInitializeBitMap'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlClearBit = (PRTL_CLEAR_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlClearBit"))) {

        if (!(Rtl->RtlClearBit = (PRTL_CLEAR_BIT)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlClearBit"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlClearBit'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlSetBit = (PRTL_SET_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlSetBit"))) {

        if (!(Rtl->RtlSetBit = (PRTL_SET_BIT)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlSetBit"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlSetBit'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlTestBit = (PRTL_TEST_BIT)
        GetProcAddress(Rtl->NtdllModule, "RtlTestBit"))) {

        if (!(Rtl->RtlTestBit = (PRTL_TEST_BIT)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlTestBit"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlTestBit'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlClearAllBits = (PRTL_CLEAR_ALL_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlClearAllBits"))) {

        if (!(Rtl->RtlClearAllBits = (PRTL_CLEAR_ALL_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlClearAllBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlClearAllBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlSetAllBits = (PRTL_SET_ALL_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlSetAllBits"))) {

        if (!(Rtl->RtlSetAllBits = (PRTL_SET_ALL_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlSetAllBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlSetAllBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindClearBits = (PRTL_FIND_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearBits"))) {

        if (!(Rtl->RtlFindClearBits = (PRTL_FIND_CLEAR_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindClearBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindClearBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindSetBits = (PRTL_FIND_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindSetBits"))) {

        if (!(Rtl->RtlFindSetBits = (PRTL_FIND_SET_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindSetBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindSetBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindClearBitsAndSet = (PRTL_FIND_CLEAR_BITS_AND_SET)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearBitsAndSet"))) {

        if (!(Rtl->RtlFindClearBitsAndSet = (PRTL_FIND_CLEAR_BITS_AND_SET)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindClearBitsAndSet"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindClearBitsAndSet'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindSetBitsAndClear = (PRTL_FIND_SET_BITS_AND_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindSetBitsAndClear"))) {

        if (!(Rtl->RtlFindSetBitsAndClear = (PRTL_FIND_SET_BITS_AND_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindSetBitsAndClear"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindSetBitsAndClear'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlClearBits = (PRTL_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlClearBits"))) {

        if (!(Rtl->RtlClearBits = (PRTL_CLEAR_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlClearBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlClearBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlSetBits = (PRTL_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlSetBits"))) {

        if (!(Rtl->RtlSetBits = (PRTL_SET_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlSetBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlSetBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindClearRuns = (PRTL_FIND_CLEAR_RUNS)
        GetProcAddress(Rtl->NtdllModule, "RtlFindClearRuns"))) {

        if (!(Rtl->RtlFindClearRuns = (PRTL_FIND_CLEAR_RUNS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindClearRuns"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindClearRuns'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindLongestRunClear = (PRTL_FIND_LONGEST_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindLongestRunClear"))) {

        if (!(Rtl->RtlFindLongestRunClear = (PRTL_FIND_LONGEST_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindLongestRunClear"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindLongestRunClear'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlNumberOfClearBits = (PRTL_NUMBER_OF_CLEAR_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberOfClearBits"))) {

        if (!(Rtl->RtlNumberOfClearBits = (PRTL_NUMBER_OF_CLEAR_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNumberOfClearBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlNumberOfClearBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlNumberOfSetBits = (PRTL_NUMBER_OF_SET_BITS)
        GetProcAddress(Rtl->NtdllModule, "RtlNumberOfSetBits"))) {

        if (!(Rtl->RtlNumberOfSetBits = (PRTL_NUMBER_OF_SET_BITS)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlNumberOfSetBits"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlNumberOfSetBits'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlAreBitsClear = (PRTL_ARE_BITS_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlAreBitsClear"))) {

        if (!(Rtl->RtlAreBitsClear = (PRTL_ARE_BITS_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlAreBitsClear"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlAreBitsClear'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlAreBitsSet = (PRTL_ARE_BITS_SET)
        GetProcAddress(Rtl->NtdllModule, "RtlAreBitsSet"))) {

        if (!(Rtl->RtlAreBitsSet = (PRTL_ARE_BITS_SET)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlAreBitsSet"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlAreBitsSet'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindFirstRunClear = (PRTL_FIND_FIRST_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindFirstRunClear"))) {

        if (!(Rtl->RtlFindFirstRunClear = (PRTL_FIND_FIRST_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindFirstRunClear"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindFirstRunClear'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindNextForwardRunClear = (PRTL_FIND_NEXT_FORWARD_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindNextForwardRunClear"))) {

        if (!(Rtl->RtlFindNextForwardRunClear = (PRTL_FIND_NEXT_FORWARD_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindNextForwardRunClear"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindNextForwardRunClear'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindLastBackwardRunClear = (PRTL_FIND_LAST_BACKWARD_RUN_CLEAR)
        GetProcAddress(Rtl->NtdllModule, "RtlFindLastBackwardRunClear"))) {

        if (!(Rtl->RtlFindLastBackwardRunClear = (PRTL_FIND_LAST_BACKWARD_RUN_CLEAR)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindLastBackwardRunClear"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindLastBackwardRunClear'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInitializeUnicodePrefix = (PRTL_INITIALIZE_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlInitializeUnicodePrefix"))) {

        if (!(Rtl->RtlInitializeUnicodePrefix = (PRTL_INITIALIZE_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInitializeUnicodePrefix"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInitializeUnicodePrefix'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlInsertUnicodePrefix = (PRTL_INSERT_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlInsertUnicodePrefix"))) {

        if (!(Rtl->RtlInsertUnicodePrefix = (PRTL_INSERT_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlInsertUnicodePrefix"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlInsertUnicodePrefix'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlRemoveUnicodePrefix = (PRTL_REMOVE_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlRemoveUnicodePrefix"))) {

        if (!(Rtl->RtlRemoveUnicodePrefix = (PRTL_REMOVE_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlRemoveUnicodePrefix"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlRemoveUnicodePrefix'");
            return FALSE;
        }
    }

    if (!(Rtl->RtlFindUnicodePrefix = (PRTL_FIND_UNICODE_PREFIX)
        GetProcAddress(Rtl->NtdllModule, "RtlFindUnicodePrefix"))) {

        if (!(Rtl->RtlFindUnicodePrefix = (PRTL_FIND_UNICODE_PREFIX)
            GetProcAddress(Rtl->NtosKrnlModule, "RtlFindUnicodePrefix"))) {

            OutputDebugStringA("Rtl: failed to resolve 'RtlFindUnicodePrefix'");
            return FALSE;
        }
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

