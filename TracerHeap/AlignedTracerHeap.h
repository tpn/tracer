#pragma once

#include "stdafx.h"

static MALLOC AlignedHeapMalloc;
static CALLOC AlignedHeapCalloc;
static REALLOC AlignedHeapRealloc;
static FREE AlignedHeapFree;
static FREE_POINTER AlignedHeapFreePointer;
static INITIALIZE_ALLOCATOR AlignedHeapInitializeAllocator;
static DESTROY_ALLOCATOR AlignedHeapDestroyAllocator;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
