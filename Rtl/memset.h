/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    memset.h

Abstract:

    This module supplies an extern definition of the memset function.  It is
    included by projects that need it (based on unresolved external symbol
    errors when linking) if their PGO builds otherwise fail.

--*/

#pragma function(memset)
extern "C" void *memset(void *s, int c, size_t n);

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
