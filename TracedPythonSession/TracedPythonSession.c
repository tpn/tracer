#include "TracedPythonSession.h"

//
// Forward decls of DLL exports.
//

TRACER_API INITIALIZE_TRACED_PYTHON_SESSION InitializeTracedPythonSession;
TRACER_API DESTROY_TRACED_PYTHON_SESSION DestroyTracedPythonSession;

//
// Forward decls of internal methods.
//

HEAP_ALLOCATION_ROUTINE HeapAllocationRoutine;
HEAP_FREE_ROUTINE HeapFreeRoutine;

//
// Method implementations.
//




_Use_decl_annotations_
PVOID
HeapAllocationRoutine(
    PALLOCATION_CONTEXT AllocationContext,
    ULONG ByteSize
    )
{
    return HeapAlloc((HANDLE)AllocationContext, 0, ByteSize);
}

_Use_decl_annotations_
VOID
HeapFreeRoutine(
    PFREE_CONTEXT FreeContext,
    PVOID Buffer
    )
{
    HeapFree((HANDLE)FreeContext, 0, Buffer);
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :