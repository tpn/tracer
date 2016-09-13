
#include "stdafx.h"

ULONG TlsIndex = TLS_OUT_OF_INDEXES;
PTRACER_CONFIG TracerConfig = NULL;

//
// Copies of the old allocation routines that will be restored when the last
// thread detaches.
//

PMALLOC OriginalMalloc = NULL;
PCALLOC OriginalCalloc = NULL;
PREALLOC OriginalRealloc = NULL;
PFREE OriginalFree = NULL;
PFREE_POINTER OriginalFreePointer = NULL;
