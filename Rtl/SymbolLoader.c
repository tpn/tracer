/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    SymbolLoader.c

Abstract:

    This module implements the symbol loading routines for the Rtl component.

--*/

#include "stdafx.h"

//
// Define the maximum number of modules that can be present in the module array
// passed to LoadSymbols() and indicated by the NumberOfModules parameter.
//

#define MAX_NUMBER_OF_MODULES 64

_Use_decl_annotations_
BOOL
LoadSymbols(
    PPSTR SymbolNameArray,
    ULONG NumberOfSymbolNames,
    PULONG_PTR SymbolAddressArray,
    ULONG NumberOfSymbolAddresses,
    PHMODULE ModuleArray,
    USHORT NumberOfModules,
    PRTL_BITMAP FailedSymbols,
    PULONG NumberOfResolvedSymbolsPointer
    )
/*++

Routine Description:

    This routine is used to dynamically resolve an array of symbol names
    against an array of modules via GetProcAddress(), storing the resulting
    addresses in symbol address array.  If a symbol name cannot be resolved in
    any of the given modules, its corresponding failure bit is set in the
    failed symbol bitmap, also provided by the caller.

Arguments:

    SymbolNameArray - Supplies a pointer to an array of NULL-terminated C
        strings, with each string representing a name to look up (that is,
        call GetProcAddress() on).

    NumberOfSymbolNames - Supplies the number of elements in the parameter
        SymbolNameArray.  This must match NumberOfSymbolAddresses, otherwise,
        FALSE is returned.

    SymbolAddressArray - Supplies the address of an array of variables that will
        receive the address corresponding to each symbol in the names array, or
        NULL if symbol lookup failed.

    NumberOfSymbolAddresses - Supplies the number of elements in the parameter
        SymbolAddressArray.  This must match NumberOfSymbolNames, otherwise,
        FALSE is returned.

    ModuleArray - Supplies the address to an array of HMODULE variables.  The
        modules in this array are enumerated up to NumberOfModules whilst trying
        to resolve a given symbol name.  The most common convention is to use a
        single module and array size of 1.

    NumberOfModules - Supplies the number of elements in the ModuleArray array.
        The value must be >= 1 and <= MAX_NUMBER_OF_MODULES, otherwise, FALSE
        is returned.

    FailedSymbols - Supplies a pointer to an RTL_BITMAP structure, whose
        SizeOfBitmap field is >= NumberOfSymbolNames.  The entire bitmap buffer
        will be zerod up-front and then a bit will be set for any failed lookup
        attempts.

    NumberOfResolvedSymbolsPointer - Supplies the address of a variable that
        will receive the number of successfully resolved symbols.

Return Value:

    If all parameters are successfully validated, TRUE will be returned
    (regardless of the results of the actual symbol loading attempts).
    FALSE is returned if any invalid parameters are detected.

    If a symbol cannot be resolved in any of the modules provided, the symbol's
    corresponding bit will be set in the FailedBitmap, and the corresponding
    pointer in the SymbolAddressesArray array will be set to NULL.

--*/
{
    ULONG Index;
    ULONG ModuleIndex;
    ULONG NumberOfElements;
    ULONG NumberOfResolvedSymbols;
    PSTR Name;
    FARPROC Proc;
    HMODULE Module;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(NumberOfResolvedSymbolsPointer)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up-front.
    //

    *NumberOfResolvedSymbolsPointer = 0;

    if (!ARGUMENT_PRESENT(SymbolNameArray)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(SymbolAddressArray)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(ModuleArray)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(FailedSymbols)) {
        return FALSE;
    }

    if (NumberOfModules == 0 || NumberOfModules > MAX_NUMBER_OF_MODULES) {
        return FALSE;
    }

    //
    // Initialize NumberOfElements to the NumberOfSymbolNames, then make sure
    // it equals NumberOfSymbolAddresses and is less than or equal to the bitmap
    // size (FailedBitmap->SizeOfBitMap).
    //

    NumberOfElements = NumberOfSymbolNames;

    if (NumberOfElements != NumberOfSymbolAddresses) {
        return FALSE;
    }

    if (NumberOfElements > FailedSymbols->SizeOfBitMap) {
        return FALSE;
    }

    //
    // Arguments are valid.  Initialize local variables prior to the loop then
    // proceed with symbol resolution.
    //

    NumberOfResolvedSymbols = 0;

    for (Index = 0; Index < NumberOfElements; Index++) {

        //
        // Resolve symbol name and clear the Proc pointer.
        //

        Name = *(SymbolNameArray + Index);
        Proc = NULL;

        //
        // Inner loop over module array.
        //

        for (ModuleIndex = 0; ModuleIndex < NumberOfModules; ModuleIndex++) {

            //
            // Resolve the module.
            //

            Module = *(ModuleArray + ModuleIndex);

            //
            // Look up the symbol via GetProcAddress().
            //

            Proc = GetProcAddress(Module, Name);

            //
            // If the symbol wasn't resolved, continue the loop.
            //

            if (!Proc) {
                continue;
            }

            break;
        }

        if (!Proc) {

            //
            // No symbol was resolved.  Set the failed bit corresponding to this
            // symbol.  The +1 accounts for the fact that the index is 0-based
            // but the bitmap is 1-based.
            //

            FastSetBit(FailedSymbols, Index + 1);

        } else {

            //
            // The symbol was successfully resolved, increment our counter.
            //

            NumberOfResolvedSymbols++;

        }

        //
        // Save the results to the target array (even if Proc was NULL).
        //

        *(SymbolAddressArray + Index) = (ULONG_PTR)Proc;

    }

    //
    // Update the caller's pointer with the count.
    //

    *NumberOfResolvedSymbolsPointer = NumberOfResolvedSymbols;

    return TRUE;
}

BOOL
TestLoadSymbols(VOID)
{
    BOOL Success;

    PSTR Names[] = {
        "RtlNumberOfClearBits",
        "RtlNumberOfSetBits",
        "Dummy",
        "PfxInsertPrefix",
        "bsearch",
        "RtlPrefetchMemoryNonTemporal",
        "RtlEnumerateGenericTableAvl",
    };

    HMODULE Modules[] = {
        LoadLibraryA("kernel32"),
        LoadLibraryA("ntdll"),
        LoadLibraryA("ntoskrnl.exe"),
    };

    struct {
        PRTL_NUMBER_OF_CLEAR_BITS RtlNumberOfClearBits;
        PRTL_NUMBER_OF_SET_BITS RtlNumberOfSetBits;
        PVOID Dummy;
        PPFX_INSERT_PREFIX PfxInsertPrefix;
        PBSEARCH bsearch;
        PRTL_PREFETCH_MEMORY_NON_TEMPORAL RtlPrefetchMemoryNonTemporal;
        PRTL_ENUMERATE_GENERIC_TABLE_AVL RtlEnumerateGenericTableAvl;
    } Functions;

    ULONG NumberOfResolvedSymbols;
    RTL_BITMAP FailedBitmap;
    ULONG BitmapBuffer = 0;

    //
    // Wire up the failed bitmap.
    //

    FailedBitmap.SizeOfBitMap = ARRAYSIZE(Names);
    FailedBitmap.Buffer = &BitmapBuffer;

    Success = LoadSymbols(Names,
                          ARRAYSIZE(Names),
                          (PULONG_PTR)&Functions,
                          sizeof(Functions) / sizeof(ULONG_PTR),
                          Modules,
                          ARRAYSIZE(Modules),
                          &FailedBitmap,
                          &NumberOfResolvedSymbols);

    if (!Success) {
        __debugbreak();
    }

    if (NumberOfResolvedSymbols != 6) {
        __debugbreak();
        Success = FALSE;
        goto End;
    }

    if (Functions.RtlNumberOfSetBits(&FailedBitmap) != 1) {
        __debugbreak();
        Success = FALSE;
        goto End;
    }

    if (Functions.RtlNumberOfClearBits(&FailedBitmap) != 6) {
        __debugbreak();
        Success = FALSE;
        goto End;
    }

End:
    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
