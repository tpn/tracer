/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    PerfectHash.h

Abstract:

    This is the main public header file for the PerfectHash component.

--*/

#ifdef __cplusplus
extern "C" {
#endif

#include "../Rtl/__C_specific_handler.h"
#include "../Rtl/Rtl.h"

//
// Define an opaque PERFECT_HASH_TABLE_KEYS structure.
//

typedef struct _PERFECT_HASH_TABLE_KEYS PERFECT_HASH_TABLE_KEYS;
typedef PERFECT_HASH_TABLE_KEYS *PPERFECT_HASH_TABLE_KEYS;
typedef const PERFECT_HASH_TABLE_KEYS *PCPERFECT_HASH_TABLE_KEYS;

//
// Define the PERFECT_HASH_TABLE_KEYS interface function pointers.
//

typedef union _PERFECT_HASH_TABLE_KEYS_LOAD_FLAGS {
    struct {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_KEYS_LOAD_FLAGS;
typedef PERFECT_HASH_TABLE_KEYS_LOAD_FLAGS
      *PPERFECT_HASH_TABLE_KEYS_LOAD_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_KEYS_LOAD_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI LOAD_PERFECT_HASH_TABLE_KEYS)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ struct _PERFECT_HASH_TABLE_ANY_API *AnyApi,
    _In_ PERFECT_HASH_TABLE_KEYS_LOAD_FLAGS LoadFlags,
    _In_ PCUNICODE_STRING Path,
    _Outptr_result_nullonfailure_ PPERFECT_HASH_TABLE_KEYS *Keys
    );
typedef LOAD_PERFECT_HASH_TABLE_KEYS *PLOAD_PERFECT_HASH_TABLE_KEYS;

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI DESTROY_PERFECT_HASH_TABLE_KEYS)(
    _Inout_ PPERFECT_HASH_TABLE_KEYS *Keys
    );
typedef DESTROY_PERFECT_HASH_TABLE_KEYS *PDESTROY_PERFECT_HASH_TABLE_KEYS;

//
// Define an opaque PERFECT_HASH_TABLE structure.
//

typedef struct _PERFECT_HASH_TABLE PERFECT_HASH_TABLE;
typedef PERFECT_HASH_TABLE *PPERFECT_HASH_TABLE;
typedef const PERFECT_HASH_TABLE *PCPERFECT_HASH_TABLE;

//
// Define an enumeration for identifying which backend algorithm variant to
// use for creating the perfect hash table.
//

typedef enum _PERFECT_HASH_TABLE_ALGORITHM_ID {

    //
    // Explicitly define a null algorithm to take the 0-index slot.
    // This makes enum validation easier.
    //

    PerfectHashTableNullAlgorithmId = 0,

    //
    // Begin valid algorithms.
    //

    PerfectHashTableDefaultAlgorithmId = 1,
    PerfectHashTableChm01AlgorithmId = 1,

    //
    // End valid algorithms.
    //

    //
    // N.B. Keep the next value last.
    //

    PerfectHashTableInvalidAlgorithmId,

} PERFECT_HASH_TABLE_ALGORITHM_ID;
typedef PERFECT_HASH_TABLE_ALGORITHM_ID *PPERFECT_HASH_TABLE_ALGORITHM_ID;

//
// Provide a simple inline algorithm validation routine.
//

FORCEINLINE
BOOLEAN
IsValidPerfectHashTableAlgorithmId(
    _In_ PERFECT_HASH_TABLE_ALGORITHM_ID AlgorithmId
    )
{
    return (
        AlgorithmId > PerfectHashTableNullAlgorithmId &&
        AlgorithmId < PerfectHashTableInvalidAlgorithmId
    );
}

//
// Define an enumeration for identifying which hash function variant to use.
//

typedef enum _PERFECT_HASH_TABLE_HASH_FUNCTION_ID {

    //
    // Explicitly define a null algorithm to take the 0-index slot.
    // This makes enum validation easier.
    //

    PerfectHashTableNullHashFunctionId = 0,

    //
    // Begin valid hash functions.
    //

    PerfectHashTableDefaultHashFunctionId = 1,

    //
    // End valid hash functions.
    //

    //
    // N.B. Keep the next value last.
    //

    PerfectHashTableInvalidHashFunctionId,

} PERFECT_HASH_TABLE_HASH_FUNCTION_ID;
typedef PERFECT_HASH_TABLE_HASH_FUNCTION_ID
      *PPERFECT_HASH_TABLE_HASH_FUNCTION_ID;

//
// Provide a simple inline hash function validation routine.
//

FORCEINLINE
BOOLEAN
IsValidPerfectHashTableHashFunctionId(
    _In_ PERFECT_HASH_TABLE_HASH_FUNCTION_ID HashFunctionId
    )
{
    return (
        HashFunctionId > PerfectHashTableNullHashFunctionId &&
        HashFunctionId < PerfectHashTableInvalidHashFunctionId
    );
}

//
// Define an enumeration for identifying the type of table masking used by the
// underlying perfect hash table.  This has performance and size implications.
// Modulus masking typically results in smaller tables at the expenses of slower
// modulus-based hash functions, whereas shifting results in larger tables but
// faster hash functions.
//

typedef enum _PERFECT_HASH_TABLE_MASKING_TYPE {

    //
    // Null masking type.
    //

    PerfectHashTableNullMaskingType = 0,

    //
    // Being valid masking types.
    //

    PerfectHashTableModulusMaskingType = 1,
    PerfectHashTableShiftMaskingType,

    //
    // End valid masking types.
    //

    //
    // N.B. Keep the next value last.
    //

    PerfectHashTableInvalidMaskingType,


} PERFECT_HASH_TABLE_MASKING_TYPE;
typedef PERFECT_HASH_TABLE_MASKING_TYPE
      *PPERFECT_HASH_TABLE_MASKING_TYPE;

//
// Provide a simple inline masking type validation routine.
//

FORCEINLINE
BOOLEAN
IsValidPerfectHashTableMaskingType(
    _In_ PERFECT_HASH_TABLE_MASKING_TYPE MaskingType
    )
{
    return (
        MaskingType > PerfectHashTableNullMaskingType &&
        MaskingType < PerfectHashTableInvalidMaskingType
    );
}

//
// Define an opaque runtime context to encapsulate threadpool resources.  This
// is created via CreatePerfectHashTableContext() with a desired concurrency,
// and then passed to CreatePerfectHashTable(), allowing it to search for
// perfect hash solutions in parallel.
//

typedef struct _PERFECT_HASH_TABLE_CONTEXT PERFECT_HASH_TABLE_CONTEXT;
typedef PERFECT_HASH_TABLE_CONTEXT *PPERFECT_HASH_TABLE_CONTEXT;

//
// Define the create and destroy functions for the runtime context.
//

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI CREATE_PERFECT_HASH_TABLE_CONTEXT)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ struct _PERFECT_HASH_TABLE_ANY_API *AnyApi,
    _In_opt_ PULONG MaximumConcurrency,
    _Outptr_opt_result_nullonfailure_ PPERFECT_HASH_TABLE_CONTEXT *Context
    );
typedef CREATE_PERFECT_HASH_TABLE_CONTEXT *PCREATE_PERFECT_HASH_TABLE_CONTEXT;

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI DESTROY_PERFECT_HASH_TABLE_CONTEXT)(
    _Pre_notnull_ _Post_satisfies_(*ContextPointer == 0)
        PPERFECT_HASH_TABLE_CONTEXT *ContextPointer,
    _In_opt_ PBOOLEAN IsProcessTerminating
    );
typedef DESTROY_PERFECT_HASH_TABLE_CONTEXT *PDESTROY_PERFECT_HASH_TABLE_CONTEXT;

//
// Define the PERFECT_HASH_TABLE interface function pointers (and supporting
// flag enumerations, if applicable).
//

//
// Create
//

typedef union _PERFECT_HASH_TABLE_CREATE_FLAGS {
    struct {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_CREATE_FLAGS;
typedef PERFECT_HASH_TABLE_CREATE_FLAGS *PPERFECT_HASH_TABLE_CREATE_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_CREATE_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI CREATE_PERFECT_HASH_TABLE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ struct _PERFECT_HASH_TABLE_ANY_API *AnyApi,
    _In_ PPERFECT_HASH_TABLE_CONTEXT Context,
    _In_opt_ PERFECT_HASH_TABLE_CREATE_FLAGS CreateFlags,
    _In_ PERFECT_HASH_TABLE_ALGORITHM_ID AlgorithmId,
    _In_ PERFECT_HASH_TABLE_MASKING_TYPE MaskingType,
    _In_ PERFECT_HASH_TABLE_HASH_FUNCTION_ID HashFunctionId,
    _Inout_opt_ PULARGE_INTEGER NumberOfTableElements,
    _In_ PPERFECT_HASH_TABLE_KEYS Keys,
    _In_opt_ PCUNICODE_STRING HashTablePath,
    _Outptr_opt_result_nullonfailure_ PPERFECT_HASH_TABLE *PerfectHashTable
    );
typedef CREATE_PERFECT_HASH_TABLE *PCREATE_PERFECT_HASH_TABLE;

//
// Load
//

typedef union _PERFECT_HASH_TABLE_LOAD_FLAGS {
    struct {
        ULONG Unused:32;
    };
    LONG AsLong;
    ULONG AsULong;
} PERFECT_HASH_TABLE_LOAD_FLAGS;
typedef PERFECT_HASH_TABLE_LOAD_FLAGS *PPERFECT_HASH_TABLE_LOAD_FLAGS;
C_ASSERT(sizeof(PERFECT_HASH_TABLE_LOAD_FLAGS) == sizeof(ULONG));

typedef
_Check_return_
_Success_(return != 0)
BOOLEAN
(NTAPI LOAD_PERFECT_HASH_TABLE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ struct _PERFECT_HASH_TABLE_ANY_API *AnyApi,
    _In_opt_ PERFECT_HASH_TABLE_LOAD_FLAGS LoadFlags,
    _In_ PCUNICODE_STRING Path,
    _Outptr_result_nullonfailure_ PPERFECT_HASH_TABLE *PerfectHashTable
    );
typedef LOAD_PERFECT_HASH_TABLE *PLOAD_PERFECT_HASH_TABLE;

//
// Destroy
//

typedef
BOOLEAN
(NTAPI DESTROY_PERFECT_HASH_TABLE)(
    _Pre_notnull_ _Post_satisfies_(*PerfectHashTablePointer == 0)
        PPERFECT_HASH_TABLE *PerfectHashTablePointer,
    _In_opt_ PBOOLEAN IsProcessTerminating
    );
typedef DESTROY_PERFECT_HASH_TABLE *PDESTROY_PERFECT_HASH_TABLE;

//
// Allocator-specific typedefs.
//

typedef
_Check_return_
_Success_(return != 0)
BOOL
(NTAPI INITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP)(
    _In_ PRTL_BOOTSTRAP RtlBootstrap,
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP
      *PINITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP;

typedef
_Check_return_
_Success_(return != 0)
BOOL
(NTAPI INITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator
    );
typedef INITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR
      *PINITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR;

//
// Self-test typedefs.
//

typedef
_Success_(return != 0)
BOOLEAN
(NTAPI SELF_TEST_PERFECT_HASH_TABLE)(
    _In_ PRTL Rtl,
    _In_ PALLOCATOR Allocator,
    _In_ struct _PERFECT_HASH_TABLE_ANY_API *AnyApi,
    _In_ struct _PERFECT_HASH_TABLE_TEST_DATA *TestData,
    _In_ PCUNICODE_STRING TestDataDirectory
    );
typedef SELF_TEST_PERFECT_HASH_TABLE *PSELF_TEST_PERFECT_HASH_TABLE;

typedef
_Success_(return != 0)
BOOLEAN
(NTAPI TEST_PERFECT_HASH_TABLE)(
    _In_ PPERFECT_HASH_TABLE Table
    );
typedef TEST_PERFECT_HASH_TABLE *PTEST_PERFECT_HASH_TABLE;

//
// Define the main PerfectHash API structure.
//

typedef struct _Struct_size_bytes_(SizeOfStruct) _PERFECT_HASH_TABLE_API {

    //
    // Size of the structure, in bytes.  This is filled in automatically by
    // LoadPerfectHashTableApi() based on the initial SizeOfAnyApi parameter.
    //

    _In_range_(sizeof(struct _PERFECT_HASH_TABLE_API),
               sizeof(struct _PERFECT_HASH_TABLE_API_EX)) ULONG SizeOfStruct;

    //
    // Number of function pointers contained in the structure.  This is filled
    // in automatically by LoadPerfectHashTableApi() based on the initial
    // SizeOfAnyApi parameter divided by the size of a function pointer.
    //

    ULONG NumberOfFunctions;

    //
    // Begin function pointers.
    //

    union {
        PVOID FirstFunctionPointer;
        PSET_C_SPECIFIC_HANDLER SetCSpecificHandler;
    };

    PLOAD_PERFECT_HASH_TABLE_KEYS LoadPerfectHashTableKeys;
    PDESTROY_PERFECT_HASH_TABLE_KEYS DestroyPerfectHashTableKeys;

    PCREATE_PERFECT_HASH_TABLE_CONTEXT CreatePerfectHashTableContext;
    PDESTROY_PERFECT_HASH_TABLE_CONTEXT DestroyPerfectHashTableContext;

    PCREATE_PERFECT_HASH_TABLE CreatePerfectHashTable;
    PLOAD_PERFECT_HASH_TABLE LoadPerfectHashTable;
    PTEST_PERFECT_HASH_TABLE TestPerfectHashTable;
    PDESTROY_PERFECT_HASH_TABLE DestroyPerfectHashTable;

    PINITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR InitializePerfectHashAllocator;

    PINITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP
        InitializePerfectHashAllocatorFromRtlBootstrap;

} PERFECT_HASH_TABLE_API;
typedef PERFECT_HASH_TABLE_API *PPERFECT_HASH_TABLE_API;

//
// Define the extended API.
//

typedef struct _PERFECT_HASH_TABLE_API_EX {

    //
    // Inline PERFECT_HASH_TABLE_API.
    //

    //
    // Size of the structure, in bytes.  This is filled in automatically by
    // LoadPerfectHashTableApi() based on the initial SizeOfAnyApi parameter.
    //

    _In_range_(sizeof(struct _PERFECT_HASH_TABLE_API),
               sizeof(struct _PERFECT_HASH_TABLE_API_EX)) ULONG SizeOfStruct;

    //
    // Number of function pointers contained in the structure.  This is filled
    // in automatically by LoadPerfectHashTableApi() based on the initial
    // SizeOfAnyApi parameter divided by the size of a function pointer.
    //

    ULONG NumberOfFunctions;

    //
    // Begin function pointers.
    //

    union {
        PVOID FirstFunctionPointer;
        PSET_C_SPECIFIC_HANDLER SetCSpecificHandler;
    };

    PLOAD_PERFECT_HASH_TABLE_KEYS LoadPerfectHashTableKeys;
    PDESTROY_PERFECT_HASH_TABLE_KEYS DestroyPerfectHashTableKeys;

    PCREATE_PERFECT_HASH_TABLE_CONTEXT CreatePerfectHashTableContext;
    PDESTROY_PERFECT_HASH_TABLE_CONTEXT DestroyPerfectHashTableContext;

    PCREATE_PERFECT_HASH_TABLE CreatePerfectHashTable;
    PLOAD_PERFECT_HASH_TABLE LoadPerfectHashTable;
    PTEST_PERFECT_HASH_TABLE TestPerfectHashTable;
    PDESTROY_PERFECT_HASH_TABLE DestroyPerfectHashTable;

    PINITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR InitializePerfectHashAllocator;

    PINITIALIZE_PERFECT_HASH_TABLE_ALLOCATOR_FROM_RTL_BOOTSTRAP
        InitializePerfectHashAllocatorFromRtlBootstrap;

    //
    // Extended API functions used for testing and benchmarking.
    //

    PSELF_TEST_PERFECT_HASH_TABLE SelfTestPerfectHashTable;

} PERFECT_HASH_TABLE_API_EX;
typedef PERFECT_HASH_TABLE_API_EX *PPERFECT_HASH_TABLE_API_EX;

typedef union _PERFECT_HASH_TABLE_ANY_API {
    PERFECT_HASH_TABLE_API Api;
    PERFECT_HASH_TABLE_API_EX ApiEx;
} PERFECT_HASH_TABLE_ANY_API;
typedef PERFECT_HASH_TABLE_ANY_API *PPERFECT_HASH_TABLE_ANY_API;

FORCEINLINE
BOOLEAN
LoadPerfectHashTableApi(
    _In_ PRTL Rtl,
    _Inout_ HMODULE *ModulePointer,
    _In_opt_ PUNICODE_STRING ModulePath,
    _In_ ULONG SizeOfAnyApi,
    _Out_writes_bytes_all_(SizeOfAnyApi) PPERFECT_HASH_TABLE_ANY_API AnyApi
    )
/*++

Routine Description:

    Loads the string table module and resolves all API functions for either
    the PERFECT_HASH_TABLE_API or PERFECT_HASH_TABLE_API_EX structure.  The
    desired API is indicated by the SizeOfAnyApi parameter.

    Example use:

        PERFECT_HASH_TABLE_API_EX GlobalApi;
        PPERFECT_HASH_TABLE_API_EX Api;

        Success = LoadPerfectHashApi(Rtl,
                                     NULL,
                                     NULL,
                                     sizeof(GlobalApi),
                                     (PPERFECT_HASH_TABLE_ANY_API)&GlobalApi);
        ASSERT(Success);
        Api = &GlobalApi;

    In this example, the extended API will be provided as our sizeof(GlobalApi)
    will indicate the structure size used by PERFECT_HASH_TABLE_API_EX.

Arguments:

    Rtl - Supplies a pointer to an initialized RTL structure.

    ModulePointer - Optionally supplies a pointer to an existing module handle
        for which the API symbols are to be resolved.  May be NULL.  If not
        NULL, but the pointed-to value is NULL, then this parameter will
        receive the handle obtained by LoadLibrary() as part of this call.
        If the string table module is no longer needed, but the program will
        keep running, the caller should issue a FreeLibrary() against this
        module handle.

    ModulePath - Optionally supplies a pointer to a UNICODE_STRING structure
        representing a path name of the string table module to be loaded.
        If *ModulePointer is not NULL, it takes precedence over this parameter.
        If NULL, and no module has been provided via *ModulePointer, loading
        will be attempted via LoadLibraryA("PerfectHashTable.dll")'.

    SizeOfAnyApi - Supplies the size, in bytes, of the underlying structure
        pointed to by the AnyApi parameter.

    AnyApi - Supplies the address of a structure which will receive resolved
        API function pointers.  The API furnished will depend on the size
        indicated by the SizeOfAnyApi parameter.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    HMODULE Module = NULL;
    ULONG NumberOfSymbols;
    ULONG NumberOfResolvedSymbols;

    //
    // Define the API names.
    //
    // N.B. These names must match PERFECT_HASH_TABLE_API_EX exactly (including
    //      the order).
    //

    CONST PCSTR Names[] = {
        "SetCSpecificHandler",
        "LoadPerfectHashTableKeys",
        "DestroyPerfectHashTableKeys",
        "CreatePerfectHashTableContext",
        "DestroyPerfectHashTableContext",
        "CreatePerfectHashTable",
        "LoadPerfectHashTable",
        "TestPerfectHashTable",
        "DestroyPerfectHashTable",
        "InitializePerfectHashTableAllocator",
        "InitializePerfectHashTableAllocatorFromRtlBootstrap",
        "SelfTestPerfectHashTable",
    };

    //
    // Define an appropriately sized bitmap we can passed to Rtl->LoadSymbols().
    //

    ULONG BitmapBuffer[(ALIGN_UP(ARRAYSIZE(Names), sizeof(ULONG) << 3) >> 5)+1];
    RTL_BITMAP FailedBitmap = { ARRAYSIZE(Names)+1, (PULONG)&BitmapBuffer };

    //
    // Determine the number of symbols we want to resolve based on the size of
    // the API indicated by the caller.
    //

    if (SizeOfAnyApi == sizeof(AnyApi->Api)) {
        NumberOfSymbols = sizeof(AnyApi->Api) / sizeof(ULONG_PTR);
    } else if (SizeOfAnyApi == sizeof(AnyApi->ApiEx)) {
        NumberOfSymbols = sizeof(AnyApi->ApiEx) / sizeof(ULONG_PTR);
    } else {
        return FALSE;
    }

    //
    // Subtract the structure header (size, number of symbols, etc).
    //

    NumberOfSymbols -= (
        (FIELD_OFFSET(PERFECT_HASH_TABLE_API, FirstFunctionPointer)) /
        sizeof(ULONG_PTR)
    );

    //
    // Attempt to load the underlying perfect hash table module if necessary.
    //

    if (ARGUMENT_PRESENT(ModulePointer)) {
        Module = *ModulePointer;
    }

    if (!Module) {
        if (ARGUMENT_PRESENT(ModulePath)) {
            Module = LoadLibraryW(ModulePath->Buffer);
        } else {
            Module = LoadLibraryA("PerfectHashTable.dll");
        }
    }

    if (!Module) {
        return FALSE;
    }

    //
    // We've got a handle to the perfect hash table.  Load the symbols we want
    // dynamically via Rtl->LoadSymbols().
    //

    Success = Rtl->LoadSymbols(Names,
                               NumberOfSymbols,
                               (PULONG_PTR)&AnyApi->Api.FirstFunctionPointer,
                               NumberOfSymbols,
                               Module,
                               &FailedBitmap,
                               TRUE,
                               &NumberOfResolvedSymbols);

    ASSERT(Success);

    //
    // Debug helper: if the breakpoint below is hit, then the symbol names
    // have potentially become out of sync.  Look at the value of first failed
    // symbol to assist in determining the cause.
    //

    if (NumberOfSymbols != NumberOfResolvedSymbols) {
        PCSTR FirstFailedSymbolName;
        ULONG FirstFailedSymbol;
        ULONG NumberOfFailedSymbols;

        NumberOfFailedSymbols = Rtl->RtlNumberOfSetBits(&FailedBitmap);
        FirstFailedSymbol = Rtl->RtlFindSetBits(&FailedBitmap, 1, 0);
        FirstFailedSymbolName = Names[FirstFailedSymbol-1];
        __debugbreak();
    }

    //
    // Set the C specific handler for the module, such that structured
    // exception handling will work.
    //

    AnyApi->Api.SetCSpecificHandler(Rtl->__C_specific_handler);

    //
    // Save the structure size and number of function pointers.
    //

    AnyApi->Api.SizeOfStruct = SizeOfAnyApi;
    AnyApi->Api.NumberOfFunctions = NumberOfSymbols;

    //
    // Update the caller's pointer and return success.
    //

    if (ARGUMENT_PRESENT(ModulePointer)) {
        *ModulePointer = Module;
    }

    return TRUE;
}

#ifdef __cplusplus
} // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
