/*++

Copyright (c) 2018 Trent Nelson <trent@trent.me>

Module Name:

    UnicodeString.c

Abstract:

    This is the UnicodeString module of the Rtl component.  Routines are
    provided for appending integers, string and characters to wide char buffers.

--*/

#include "stdafx.h"

_Use_decl_annotations_
VOID
AppendIntegerToWideCharBuffer(
    PPWCHAR BufferPointer,
    ULONGLONG Integer
    )
{
    PWCHAR Buffer;
    USHORT Offset;
    USHORT NumberOfDigits;
    ULONGLONG Digit;
    ULONGLONG Value;
    ULONGLONG Count;
    ULONGLONG Bytes;
    WCHAR WideChar;
    PWCHAR Dest;

    Buffer = *BufferPointer;

    //
    // Count the number of digits required to represent the integer in base 10.
    //

    NumberOfDigits = CountNumberOfLongLongDigitsInline(Integer);

    //
    // Initialize our destination pointer to the last digit.  (We write
    // back-to-front.)
    //

    Offset = (NumberOfDigits - 1) * sizeof(WideChar);
    Dest = (PWCHAR)RtlOffsetToPointer(Buffer, Offset);

    Count = 0;
    Bytes = 0;

    //
    // Convert each digit into the corresponding character and copy to the
    // string buffer, retreating the pointer as we go.
    //

    Value = Integer;

    do {
        Count++;
        Bytes += sizeof(WideChar);
        Digit = Value % 10;
        Value = Value / 10;
        WideChar = ((WCHAR)Digit + '0');
        *Dest-- = WideChar;
    } while (Value != 0);

    *BufferPointer = (PWCHAR)RtlOffsetToPointer(Buffer, Bytes);

    return;
}


_Use_decl_annotations_
VOID
AppendIntegerToWideCharBufferEx(
    PPWCHAR BufferPointer,
    ULONGLONG Integer,
    BYTE NumberOfDigits,
    WCHAR Pad,
    WCHAR Trailer
    )
/*++

Routine Description:

    This is a helper routine that appends an integer to a character buffer,
    with optional support for padding and trailer characters.

Arguments:

    BufferPointer - Supplies a pointer to a variable that contains the address
        of a character buffer to which the string representation of the integer
        will be written.  The pointer is adjusted to point after the length of
        the written bytes prior to returning.

    Integer - Supplies the long long integer value to be appended to the string.

    NumberOfDigits - The expected number of digits for the value.  If Integer
        has less digits than this number, it will be left-padded with the char
        indicated by the Pad parameter.

    Pad - A character to use for padding, if applicable.

    Trailer - An optional trailing wide character to append.

Return Value:

    None.

--*/
{
    BYTE Offset;
    BYTE NumberOfWideCharsToPad;
    BYTE ActualNumberOfDigits;
    ULONGLONG Digit;
    ULONGLONG Value;
    ULONGLONG Count;
    ULONGLONG Bytes;
    WCHAR WideChar;
    PWCHAR End;
    PWCHAR Dest;
    PWCHAR Start;
    PWCHAR Expected;

    Start = *BufferPointer;

    //
    // Make sure the integer value doesn't have more digits than specified.
    //

    ActualNumberOfDigits = CountNumberOfLongLongDigitsInline(Integer);
    ASSERT(ActualNumberOfDigits <= NumberOfDigits);

    //
    // Initialize our destination pointer to the last digit.  (We write
    // back-to-front.)
    //

    Offset = (NumberOfDigits - 1) * sizeof(WideChar);
    Dest = (PWCHAR)RtlOffsetToPointer(Start, Offset);
    End = Dest + 1;

    Count = 0;
    Bytes = 0;

    //
    // Convert each digit into the corresponding character and copy to the
    // string buffer, retreating the pointer as we go.
    //

    Value = Integer;

    do {
        Count++;
        Bytes += sizeof(WideChar);
        Digit = Value % 10;
        Value = Value / 10;
        WideChar = ((WCHAR)Digit + '0');
        *Dest-- = WideChar;
    } while (Value != 0);

    //
    // Pad the string with zeros if necessary.
    //

    NumberOfWideCharsToPad = NumberOfDigits - ActualNumberOfDigits;

    if (NumberOfWideCharsToPad && Pad) {
        do {
            Count++;
            Bytes += sizeof(WideChar);
            *Dest-- = Pad;
        } while (--NumberOfWideCharsToPad);
    }

    //
    // Add the trailer if applicable.
    //

    if (Trailer) {
        Bytes += sizeof(WideChar);
        *End++ = Trailer;
    }

    Expected = (PWCHAR)RtlOffsetToPointer(Start, Bytes);
    ASSERT(Expected == End);

    *BufferPointer = End;

    return;
}

_Use_decl_annotations_
VOID
AppendUnicodeStringToWideCharBuffer(
    PPWCHAR BufferPointer,
    PCUNICODE_STRING UnicodeString
    )
{
    PVOID Buffer;

    Buffer = *BufferPointer;
    CopyMemory(Buffer, UnicodeString->Buffer, UnicodeString->Length);
    *BufferPointer = (PWCHAR)RtlOffsetToPointer(Buffer, UnicodeString->Length);

    return;
}

_Use_decl_annotations_
VOID
AppendWideCharBufferToWideCharBuffer(
    PPWCHAR BufferPointer,
    PCWCHAR String,
    ULONG SizeInBytes
    )
{
    PVOID Buffer;

    Buffer = *BufferPointer;
    CopyMemory(Buffer, String, SizeInBytes);
    *BufferPointer = (PWCHAR)RtlOffsetToPointer(Buffer, SizeInBytes);

    return;
}

_Use_decl_annotations_
VOID
AppendWideCharToWideCharBuffer(
    PPWCHAR BufferPointer,
    WCHAR WideChar
    )
{
    PWCHAR Buffer;

    Buffer = *BufferPointer;
    *Buffer = WideChar;
    *BufferPointer = Buffer + 1;
}

_Use_decl_annotations_
VOID
AppendWideCStrToWideCharBuffer(
    PPWCHAR BufferPointer,
    PCSZ String
    )
{
    PWCHAR Dest = *BufferPointer;
    PWCHAR Source = (PWCHAR)String;

    while (*Source) {
        *Dest++ = *Source++;
    }

    *BufferPointer = Dest;

    return;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
