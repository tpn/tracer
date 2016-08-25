
#include "stdafx.h"

_Success_(return != 0)
BOOLEAN
AllocateAndCopyWideString(
    _In_ PALLOCATOR Allocator,
    _In_ USHORT SizeInBytes,
    _In_ PWCHAR Buffer,
    _In_ PUNICODE_STRING String
)
{
    USHORT AlignedSizeInBytes = ALIGN_UP_USHORT_TO_POINTER_SIZE(SizeInBytes);

    String->Buffer = (PWCHAR)(
        Allocator->Calloc(
            Allocator->Context,
            1,
            AlignedSizeInBytes
        )
    );

    if (!String->Buffer) {
        return FALSE;
    }

    //
    // Shift right by 1 to convert the string byte lengths to character lengths.
    // Length has 2 subtracted to account for the trailing NULL.
    //

    String->Length = ((SizeInBytes - 2) >> 1);
    String->MaximumLength = (AlignedSizeInBytes >> 1);

    //
    // Copy the string in WCHAR chunks.
    //

    __movsw((PWORD)String->Buffer, (PWORD)Buffer, String->Length);

    //
    // Ensure the string is NULL terminated.
    //

    ((WCHAR)String->Buffer[String->Length]) = (WCHAR)L"\0";

    return TRUE;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
