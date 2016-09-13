
#include <ntstatus.h>
#pragma warning(push)
#pragma warning(disable: 4995)
#include <ntstrsafe.h>


/*++

NTSTATUS
RtlUnicodeStringPrintf(
_Inout_                     PUNICODE_STRING DestinationString,
_In_ _Printf_format_string_ PCWSTR          pszFormat,
...
);

Routine Description:

This routine is a safer version of the C built-in function 'sprintf' for
PUNICODE_STRINGs.

This function returns an NTSTATUS value, and not a pointer. It returns
STATUS_SUCCESS if the string was printed without truncation, otherwise it
will return a failure code. In failure cases it will return a truncated
version of the ideal result.

Arguments:

DestinationString   -  pointer to the counted unicode destination string

pszFormat           -  format string which must be null terminated

...                 -  additional parameters to be formatted according to
the format string

Notes:
Behavior is undefined if destination, format strings or any arguments
strings overlap.

DestinationString and pszFormat should not be NULL.  See RtlUnicodeStringPrintfEx if you
require the handling of NULL values.

Return Value:

STATUS_SUCCESS -   if there was sufficient space in the dest buffer for
the resultant string

failure        -   the operation did not succeed

STATUS_BUFFER_OVERFLOW
Note: This status has the severity class Warning - IRPs completed with this
status do have their data copied back to user mode
-   this return value is an indication that the print
operation failed due to insufficient space. When this
error occurs, the destination buffer is modified to
contain a truncated version of the ideal result. This is
useful for situations where truncation is ok.

It is strongly recommended to use the NT_SUCCESS() macro to test the
return value of this function

--*/

NTSTATUS
__stdcall
UnicodeStringPrintf(
    _Inout_ PUNICODE_STRING DestinationString,
    _In_ _Printf_format_string_ NTSTRSAFE_PCWSTR pszFormat,
    ...)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(
        DestinationString,
        &pszDest,
        &cchDest,
        NULL,
        NTSTRSAFE_UNICODE_STRING_MAX_CCH,
        0
    );

    if (NT_SUCCESS(status)) {
        va_list argList;
        size_t cchNewDestLength = 0;

        va_start(argList, pszFormat);

        status = RtlWideCharArrayVPrintfWorker(
            pszDest,
            cchDest,
            &cchNewDestLength,
            pszFormat,
            argList
        );

        va_end(argList);

        //
        // Safe to multiply cchNewDestLength * sizeof(wchar_t) since
        // cchDest < NTSTRSAFE_UNICODE_STRING_MAX_CCH and sizeof(wchar_t) is 2.
        //

        DestinationString->Length = (USHORT)(
            cchNewDestLength *
            sizeof(wchar_t)
        );

    }

    return status;
}

#pragma warning(pop)

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
