#pragma once

#include "stdafx.h"

MALLOC AlignedHeapMalloc;
CALLOC AlignedHeapCalloc;
REALLOC AlignedHeapRealloc;
FREE AlignedHeapFree;
FREE_POINTER AlignedHeapFreePointer;
INITIALIZE_ALLOCATOR AlignedHeapInitializeAllocator;
DESTROY_ALLOCATOR AlignedHeapDestroyAllocator;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
