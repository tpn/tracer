#include "stdafx.h"

static PRTL_COMPARE_STRING _RtlCompareString = NULL;

RTL_API
LONG
CompareStringCaseInsensitive(
    _In_ PCSTRING String1,
    _In_ PCSTRING String2
    )
{
    return _RtlCompareString(String1, String2, FALSE);
}

//
// STRING_TABLE
//

_Use_decl_annotations_
PSTRING_TABLE
CreateStringTable(
    PRTL        Rtl,
    PALLOCATOR  Allocator,
    USHORT      NumberOfElements,
    USHORT      CharacterWidth
    )
{
    USHORT NumberOfElements;
    ULONG AllocSize;
    PVOID Buffer;
    PSTRING_TABLE StringTable;
    PPSTRING Table;

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(HeapHandle)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(StringTablePointer)) {
        return FALSE;
    }

    if (ARGUMENT_PRESENT(NumberOfElementsPointer)) {
        NumberOfElements = *NumberOfElementsPointer;
    } else {
        NumberOfElements = INITIAL_STRING_TABLE_NUM_ELEMENTS;
    }

    Buffer = HeapAlloc(HeapHandle, 0, sizeof(STRING_TABLE));
    if (!Buffer) {
        return FALSE;
    }

    StringTable = (PSTRING_TABLE)Buffer;

    AllocSize = sizeof(PPSTRING) * NumberOfElements;

    Buffer = HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, AllocSize);
    if (!Buffer) {
        HeapFree(HeapHandle, 0, StringTable);
        return FALSE;
    }

    Table = (PPSTRING)Buffer;
    StringTable->Rtl = Rtl;
    StringTable->HeapHandle = HeapHandle;
    StringTable->NumberOfElements = NumberOfElements;
    StringTable->Available = NumberOfElements;
    StringTable->Table = Table;
    StringTable->BinarySearch = Rtl->bsearch;
    StringTable->QuickSort = Rtl->qsort;
    StringTable->Compare = CompareStringCaseInsensitive;

    *StringTablePointer = StringTable;

    return TRUE;
}

RTL_API
_Success_(return != 0)
BOOL
InsertStringTable(
    _In_        PSTRING_TABLE   StringTable,
    _In_        PSTRING         String,
    _Out_opt_   PBOOL           NewElementPointer
    )
{
    PRTL Rtl;
    BOOL Found = FALSE;
    BOOL NewElement = FALSE;
    USHORT Index;
    USHORT CurrentSize;
    ULONG Result;
    PPSTRING Table;

    if (!ARGUMENT_PRESENT(StringTable)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(String)) {
        return FALSE;
    }

    Rtl = StringTable->Rtl;
    Table = StringTable->Table;
    CurrentSize = StringTable->NumberOfElements - StringTable->Available;

    //
    // Do a linear search of the array looking for a suitable insertion point.
    //

    for (Index = 0; Index < CurrentSize; Index++) {

        Result = Rtl->RtlCompareString(String, Table[Index], FALSE);

        if (Result == -1) {

            //
            // This is our insertion point.

            Found = TRUE;
            break;

        } else if (Result == 0) {

            //
            // String already exists in the table.
            //

            goto End;

        }

        //
        // String is greater than the current table entry, continue.
        //
    }

    if (!StringTable->Available) {

        //
        // The array needs to be grown.
        //

        __debugbreak();

    } else if (StringTable->Available == StringTable->NumberOfElements) {

        //
        // The array is empty, insert the string into the first slot.
        //

        *Table = String;
        NewElement = TRUE;
        goto End;

    }



    --StringTable->Available;

End:

    if (ARGUMENT_PRESENT(NewElementPointer)) {
        *NewElementPointer = NewElement;
    }

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
