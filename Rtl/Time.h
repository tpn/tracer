/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Time.h

Abstract:

    This module contains general time helper inline functions.

--*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdafx.h"

//
// FILETIMEEX is a convenience wrapper around FILETIME and ULARGE_INTEGER that
// allows easy extraction of the underlying ULONGLONG.
//

typedef union _FILETIMEEX {
    ULONGLONG AsULongLong;
    LONGLONG AsLongLong;
    FILETIME AsFiletime;
    ULARGE_INTEGER AsULargeIntger;
    LARGE_INTEGER AsLargeIntger;
} FILETIMEEX, *PFILETIMEEX;

//
// Define multiplicands for absolute FILETIME time offsets.
//

#define _MICROSECOND_ABSOLUTE           (10LL) // 100 nanoseconds
#define _MILLISECOND_ABSOLUTE           (_MICROSECOND_ABSOLUTE * 1000)
#define _SECOND_ABSOLUTE                (_MILLISECOND_ABSOLUTE * 1000)
#define _MINUTE_ABSOLUTE                (60 * _SECOND_ABSOLUTE)
#define _HOUR_ABSOLUTE                  (60 * _MINUTE_ABSOLUTE)
#define _DAY_ABSOLUTE                   (24 * _HOUR_ABSOLUTE)

//
// Define multiplicands for relative FILETIME time offsets.
//

#define _MICROSECOND_RELATIVE           (-10LL) // 100 nanoseconds
#define _MILLISECOND_RELATIVE           (_MICROSECOND_RELATIVE * 1000)
#define _SECOND_RELATIVE                (_MILLISECOND_RELATIVE * 1000)
#define _MINUTE_RELATIVE                (60 * _SECOND_RELATIVE)
#define _HOUR_RELATIVE                  (60 * _MINUTE_RELATIVE)
#define _DAY_RELATIVE                   (24 * _HOUR_RELATIVE)

//
// Inline functions.
//

FORCEINLINE
VOID
MicrosecondsToRelativeThreadpoolTime(
    _In_ ULONG Microseconds,
    _Out_ PFILETIME FileTime
    )
/*++

Routine Description:

    This routine converts a microseconds value into a relative FILETIME time
    that can be passed to threadpool timer functions.

Arguments:

    Microseconds - Supplies the number of microseconds to convert.

    FileTime - Supplies a pointer to a variable that will receive the number
        of microseconds converted into a relative FILETIME representation.

Return Value:

    None.

--*/
{
    ULARGE_INTEGER Micro;
    ULARGE_INTEGER Relative;

    Micro.QuadPart = Microseconds;
    Relative.QuadPart = (_MICROSECOND_RELATIVE * Micro.QuadPart);

    FileTime->dwLowDateTime = Relative.LowPart;
    FileTime->dwHighDateTime = Relative.HighPart;

    return;
}

FORCEINLINE
VOID
MillisecondsToRelativeThreadpoolTime(
    _In_ ULONG Milliseconds,
    _Out_ PFILETIME FileTime
    )
/*++

Routine Description:

    This routine converts a milliseconds value into a relative FILETIME time
    that can be passed to threadpool timer functions.

Arguments:

    Milliseconds - Supplies the number of milliseconds to convert.

    FileTime - Supplies a pointer to a variable that will receive the number
        of milliseconds converted into a relative FILETIME representation.

Return Value:

    None.

--*/
{
    ULARGE_INTEGER Milli;
    ULARGE_INTEGER Relative;

    Milli.QuadPart = Milliseconds;
    Relative.QuadPart = (_MILLISECOND_RELATIVE * Milli.QuadPart);

    FileTime->dwLowDateTime = Relative.LowPart;
    FileTime->dwHighDateTime = Relative.HighPart;

    return;
}

FORCEINLINE
VOID
SecondsToRelativeThreadpoolTime(
    _In_ ULONG Seconds,
    _Out_ PFILETIME FileTime
    )
/*++

Routine Description:

    This routine converts a seconds value into a relative FILETIME time
    that can be passed to threadpool timer functions.

Arguments:

    Seconds - Supplies the number of seconds to convert.

    FileTime - Supplies a pointer to a variable that will receive the number
        of seconds converted into a relative FILETIME representation.

Return Value:

    None.

--*/
{
    ULARGE_INTEGER Second;
    ULARGE_INTEGER Relative;

    Second.QuadPart = Seconds;
    Relative.QuadPart = (_SECOND_RELATIVE * Second.QuadPart);

    FileTime->dwLowDateTime = Relative.LowPart;
    FileTime->dwHighDateTime = Relative.HighPart;

    return;
}

#ifdef __cplusplus
} // extern "C" {
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
