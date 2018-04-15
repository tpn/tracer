/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    String.c

Abstract:

    This is the String module of the Rtl component.  Routines are provided
    for appending integers, string and characters to existing character buffers.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
AppendIntegerToCharBuffer(
    PPCHAR BufferPointer,
    ULONGLONG Integer
    )
{
    PCHAR Buffer;
    USHORT Offset;
    USHORT NumberOfDigits;
    ULONGLONG Digit;
    ULONGLONG Value;
    ULONGLONG Count;
    ULONGLONG Bytes;
    CHAR Char;
    PCHAR Dest;

    Buffer = *BufferPointer;

    //
    // Count the number of digits required to represent the integer in base 10.
    //

    NumberOfDigits = CountNumberOfLongLongDigitsInline(Integer);

    //
    // Initialize our destination pointer to the last digit.  (We write
    // back-to-front.)
    //

    Offset = (NumberOfDigits - 1) * sizeof(Char);
    Dest = (PCHAR)RtlOffsetToPointer(Buffer, Offset);

    Count = 0;
    Bytes = 0;

    //
    // Convert each digit into the corresponding character and copy to the
    // string buffer, retreating the pointer as we go.
    //

    Value = Integer;

    do {
        Count++;
        Bytes += sizeof(Char);
        Digit = Value % 10;
        Value = Value / 10;
        Char = ((CHAR)Digit + '0');
        *Dest-- = Char;
    } while (Value != 0);

    *BufferPointer = RtlOffsetToPointer(Buffer, Bytes);

    return;
}

_Use_decl_annotations_
VOID
AppendStringToCharBuffer(
    PPCHAR BufferPointer,
    PCSTRING String
    )
{
    PVOID Buffer;

    Buffer = *BufferPointer;
    CopyMemory(Buffer, String->Buffer, String->Length);
    *BufferPointer = RtlOffsetToPointer(Buffer, String->Length);

    return;
}

_Use_decl_annotations_
VOID
AppendCharBufferToCharBuffer(
    PPCHAR BufferPointer,
    PCHAR String,
    ULONG SizeInBytes
    )
{
    PVOID Buffer;

    Buffer = *BufferPointer;
    CopyMemory(Buffer, String, SizeInBytes);
    *BufferPointer = RtlOffsetToPointer(Buffer, SizeInBytes);

    return;
}

_Use_decl_annotations_
VOID
AppendCharToCharBuffer(
    PPCHAR BufferPointer,
    CHAR Char
    )
{
    PCHAR Buffer;

    Buffer = *BufferPointer;
    *Buffer = Char;
    *BufferPointer = Buffer + 1;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
