#pragma once

#include "stdafx.h"

MALLOC DefaultHeapMalloc;
CALLOC DefaultHeapCalloc;
REALLOC DefaultHeapRealloc;
FREE DefaultHeapFree;
FREE_POINTER DefaultHeapFreePointer;
INITIALIZE_ALLOCATOR DefaultHeapInitializeAllocator;
DESTROY_ALLOCATOR DefaultHeapDestroyAllocator;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
