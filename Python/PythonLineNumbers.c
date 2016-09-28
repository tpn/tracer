/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    PythonLineNumbers.c

Abstract:

    This module implements routines related to extracting line number
    information from Python code object structures.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
ResolveLineNumbers(
    PPYTHON Python,
    PPYTHON_FUNCTION Function
    )
/*++

Routine Description:

    This method resolves line number information for a given Python function.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    Function - Supplies a pointer to a PYTHON_FUNCTION structure to resolve
        line number information for.

Return Value:

    None.

--*/
{

    //
    // We haven't implemented line number resolution for anything other than
    // Python 2.x yet.
    //

    if (Python->MajorVersion == 2) {
        ResolveLineNumbersForPython2(Python, Function);
    } else {
        ResolveLineNumbersForPython3(Python, Function);
    }
}

_Use_decl_annotations_
VOID
ResolveLineNumbersForPython2(
    PPYTHON Python,
    PPYTHON_FUNCTION Function
    )
/*++

Routine Description:

    This method resolves line number information for a given Python function
    assuming a Python 2.x runtime.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    Function - Supplies a pointer to a PYTHON_FUNCTION structure to resolve
        line number information for.

Return Value:

    None.

--*/
{
    USHORT Index;
    USHORT Address;
    USHORT TableSize;
    USHORT LineNumber;
    USHORT SizeOfByteCode;
    USHORT NumberOfLines;
    USHORT FirstLineNumber;
    USHORT PreviousAddress;
    USHORT PreviousLineNumber;
    PPYSTRINGOBJECT ByteCodes;
    PLINE_NUMBER Table;
    PLINE_NUMBER_TABLE2 LineNumbers;
    PPYCODEOBJECT25_27 CodeObject;

    CodeObject = (PPYCODEOBJECT25_27)Function->CodeObject;
    ByteCodes = (PPYSTRINGOBJECT)CodeObject->Code;
    SizeOfByteCode = (USHORT)ByteCodes->ObjectSize;

    LineNumbers = (PLINE_NUMBER_TABLE2)CodeObject->LineNumberTable;

    //
    // The underlying ObjectSize will refer to the number of bytes; as each
    // entry is two bytes, shift right once.
    //

    TableSize = (USHORT)LineNumbers->ObjectSize >> 1;

    Address = 0;
    NumberOfLines = 0;
    PreviousAddress = 0;
    PreviousLineNumber = 0;
    FirstLineNumber = LineNumber = (USHORT)CodeObject->FirstLineNumber;

    for (Index = 0; Index < TableSize; Index++) {

        USHORT ByteIncrement;
        USHORT LineIncrement;

        Table = &LineNumbers->Table[Index];

        ByteIncrement = Table->ByteIncrement;
        LineIncrement = Table->LineIncrement;

        if (ByteIncrement) {
            if (LineNumber != PreviousLineNumber) {
                NumberOfLines++;
                PreviousLineNumber = LineNumber;
            }
        }

        if (LineIncrement) {
            if (Address != PreviousAddress) {
                PreviousAddress = Address;
            }
        }

        Address += ByteIncrement;
        LineNumber += LineIncrement;
    }

    if (LineNumber != PreviousLineNumber) {
        NumberOfLines++;
        PreviousLineNumber = LineNumber;
    }

    if (Address != PreviousAddress) {
        PreviousAddress = Address;
    }

    Function->FirstLineNumber = FirstLineNumber;
    Function->NumberOfLines = (
        PreviousLineNumber -
        FirstLineNumber
    );
    Function->SizeOfByteCode = SizeOfByteCode;
    Function->NumberOfCodeLines = NumberOfLines;

    //
    // Now that we know the total number of code lines and total number of
    // lines, we have enough information to create a line number bitmap,
    // a line-number-to-relative-index table, and a line-hit histogram.
    // All of which are an XXX todo.
    //

    return;
}

_Use_decl_annotations_
VOID
ResolveLineNumbersForPython3(
    PPYTHON Python,
    PPYTHON_FUNCTION Function
    )
/*++

Routine Description:

    This method resolves line number information for a given Python function
    assuming a Python 3.x runtime.

    N.B.: this routine is currently not implemented and simply sets the
          line number related field in the function structure to zero.

Arguments:

    Python - Supplies a pointer to a PYTHON structure.

    Function - Supplies a pointer to a PYTHON_FUNCTION structure to resolve
        line number information for.

Return Value:

    None.

--*/
{

    Function->FirstLineNumber = 0;
    Function->NumberOfLines = 0;
    Function->NumberOfCodeLines = 0;
    Function->SizeOfByteCode = 0;
}


#ifdef __cplusplus
}; // extern "C"
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
