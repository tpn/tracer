/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    memset.c

Abstract:

    This module supplies a simple memset() implementation.  It is typically
    included by projects that need it (based on unresolved external symbol
    errors when linking) if their PGO builds otherwise fail.

    It is not used or included in the Rtl binary.  Other projects simply
    include it.

--*/

#pragma intrinsic(__stosb)
#pragma function(memset)
void *memset(void *s, int c, __int64 n)
{
    __stosb(s, c, n);
    return s;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
