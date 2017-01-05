/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Registry.c

Abstract:

    This module implements convenience functions for working with the registry.
    Routines are provided to write registry strings in a correctly-escaped
    manner.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
WriteRegistryString(
    PRTL Rtl,
    PALLOCATOR Allocator,
    HKEY RegistryKey,
    PWSTR Name,
    PUNICODE_STRING Value
    )
/*++

Routine Description:

    Writes a Unicode string value to the registry.  If the string contains any
    backslashes, they will be escaped.  If no trailing NUL is present, one will
    be added.  Allocator is used in both of these situations when a new string
    needs to be created.  It will be destroyed automatically before this routine
    returns, so the caller does not need to worry about freeing any memory upon
    return.

Arguments:

    Rtl - Supplies a pointer to an RTL structure.

    Allocator - Supplies a pointer to an ALLOCATOR structure to be used for all
        allocations performed by this routine.

    RegistryKey - Supplies a handle to an open registry key where the value will
        be written.

    Name - Supplies a pointer to a NUL-terminated wide character name of the
        registry string to write.

    Value - Supplies a pointer to a UNICODE_STRING structure that represents
        the underlying wide character string value to write.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    BOOL IsNullTerminated;
    WCHAR Char;
    USHORT Index;
    USHORT NumberOfBackslashes;
    LONG Result;
    DWORD LastError;
    UNICODE_STRING String;
    PUNICODE_STRING StringToWrite;
    PWSTR Dest;
    PRTL_BITMAP BitmapPointer;
    LONG_INTEGER AllocSizeInBytes;
    HANDLE HeapHandle = NULL;
    PVOID Buffer = NULL;

    UNREFERENCED_PARAMETER(String);

    //
    // Reserve a 32 byte (256 >> 3), 256 bit stack-allocated bitmap buffer.
    //

    CHAR StackBitmapBuffer[256 >> 3];
    RTL_BITMAP Bitmap = { 256, (PULONG)&StackBitmapBuffer };
    BitmapPointer = &Bitmap;


    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(Rtl)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Allocator)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(RegistryKey)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Name)) {
        return FALSE;
    }

    if (!ARGUMENT_PRESENT(Value)) {
        return FALSE;
    }

    if (!IsValidUnicodeStringWithMinimumLengthInChars(Value, 1)) {
        return FALSE;
    }

    //
    // Determine if the incoming string is already NUL terminated.  Note that we
    // only dereference memory past Length when it falls within the bounds set
    // by MaximumLength.
    //

    IsNullTerminated = (
        Value->Length <= Value->MaximumLength - sizeof(WCHAR) &&
        Value->Buffer[Value->Length >> 1] == L'\0'
    );

    //
    // Determine if there are any backslashes in the value.
    //

    Success = Rtl->CreateBitmapIndexForUnicodeString(
        Rtl,
        Value,
        L'\\',
        &HeapHandle,
        &BitmapPointer,
        FALSE,
        NULL
    );

    if (!Success) {
        return FALSE;
    }

    NumberOfBackslashes = (USHORT)Rtl->RtlNumberOfSetBits(BitmapPointer);

    //
    // If there are no backslashes and the string is already NUL-terminated,
    // we don't need to do any more preparation and can write it now.
    //

    if (NumberOfBackslashes == 0 && IsNullTerminated) {
        StringToWrite = Value;
        AllocSizeInBytes.LongPart = Value->Length + sizeof(WCHAR);
        goto WriteString;
    }

    //
    // Calculate the allocation size in bytes required for the new temp string.
    //

    AllocSizeInBytes.LongPart = (

        //
        // Account for the original value length.
        //

        Value->Length +

        //
        // Account for extra backslashes required for escaping, if applicable.
        //

        (NumberOfBackslashes * sizeof(WCHAR)) +

        //
        // Account for the trailing NUL.
        //

        sizeof(WCHAR)
    );

    //
    // Sanity check we haven't exceeded MAX_USHORT.
    //

    if (AllocSizeInBytes.HighPart) {
        Success = FALSE;
        goto End;
    }

    //
    // Attempt to allocate a new buffer for the string.
    //

    Buffer = Allocator->Calloc(Allocator->Context, 1, AllocSizeInBytes.LowPart);
    if (!Buffer) {
        Success = FALSE;
        goto End;
    }

    //
    // Wire up our local stack-allocated string structure to use this buffer.
    //

    String.Length = AllocSizeInBytes.LowPart - sizeof(WCHAR);
    String.MaximumLength = AllocSizeInBytes.LowPart;
    String.Buffer = (PWCHAR)Buffer;
    StringToWrite = &String;

    //
    // Fast-path copy if no escaping needs to be done.
    //

    if (NumberOfBackslashes == 0) {
        __movsw(String.Buffer, Value->Buffer, Value->Length >> 1);
        String.Buffer[Value->Length >> 1] = L'\0';
        goto WriteString;
    }

    //
    // Copy the string, inserting backslashes where applicable.
    //

    Dest = String.Buffer;

    for (Index = 0; Index < (Value->Length >> 1); Index++) {
        Char = Value->Buffer[Index];
        if (Char == L'\\') {
            *Dest++ = L'\\';
        }
        *Dest++ = Char;
    }

    //
    // Add the trailing NUL.
    //

    *Dest = L'\0';

WriteString:

    Result = RegSetValueExW(RegistryKey,
                            Name,
                            0,
                            REG_SZ,
                            (const BYTE *)StringToWrite->Buffer,
                            AllocSizeInBytes.LongPart);

    if (Result != ERROR_SUCCESS) {
        LastError = GetLastError();
        Success = FALSE;
    } else {
        Success = TRUE;
    }

End:

    //
    // Free the buffer now, if applicable.
    //

    if (Buffer) {
        Allocator->Free(Allocator->Context, Buffer);
        Buffer = NULL;
    }

    if ((ULONG_PTR)Bitmap.Buffer != (ULONG_PTR)BitmapPointer->Buffer) {

        //
        // We'll hit this point if a new bitmap had to be allocated because
        // our stack-allocated one was too small.  Make sure we free it here.
        //

        HeapFree(HeapHandle, 0, BitmapPointer->Buffer);
    }

    return Success;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
