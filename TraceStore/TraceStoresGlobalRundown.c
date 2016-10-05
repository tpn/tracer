/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    TraceStoresGlobalRundown.c

Abstract:

    This module implements "global" versions of routines implemented by the
    TraceStoresRundown module.  These routines differ in that they do not take
    a PTRACE_STORES_RUNDOWN parameter; instead, a global TRACE_STORE_RUNDOWN
    structure is defined by this module, and this is passed to the relevant
    TraceStoresRundown counterpart routine implicitly.

    Other components within the TraceStore module will call the global methods
    instead of of the rundown methods directly.

--*/

#include "stdafx.h"

TRACE_STORE_DATA TRACE_STORES_RUNDOWN GlobalTraceStoresRundown = { 0 };

PTRACE_STORES_RUNDOWN
GetGlobalTraceStoresRundown(
    VOID
    )
/*++

Routine Description:

    This routine returns a pointer to the global trace stores rundown structure.

Arguments:

    None.

Return Value:

    A pointer to the global trace stores rundown structure.

--*/
{
    return &GlobalTraceStoresRundown;
}

_Use_decl_annotations_
BOOL
InitializeGlobalTraceStoresRundown(
    VOID
    )
/*++

Routine Description:

    This routine initializes the global TRACE_STORE_RUNDOWN structure.

Arguments:

    None.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return InitializeTraceStoresRundown(GetGlobalTraceStoresRundown());
}

VOID
DestroyGlobalTraceStoresRundown(
    VOID
    )
/*++

Routine Description:

    This routine destroys the global TRACE_STORES_RUNDOWN structure.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DestroyTraceStoresRundown(GetGlobalTraceStoresRundown());
}

_Use_decl_annotations_
BOOL
RegisterGlobalTraceStores(
    PTRACE_STORES TraceStores
    )
/*++

Routine Description:

    This routine registers the trace stores with the global rundown structure.

Arguments:

    TraceStores - Supplies a pointer to a TRACE_STORES structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return RegisterTraceStores(GetGlobalTraceStoresRundown(), TraceStores);
}

VOID
RundownGlobalTraceStores(
    VOID
    )
/*++

Routine Description:

    This routine runs down the trace stores associated with the global rundown.
    list.

Arguments:

    None.

Return Value:

    None.

--*/
{
    RundownTraceStores(GetGlobalTraceStoresRundown());
}

BOOL
IsGlobalTraceStoresRundownActive(
    VOID
    )
/*++

Routine Description:

    Returns TRUE if a global trace stores rundown is active.

Arguments:

    None.

Return Value:

    TRUE if active, FALSE otherwise.

--*/
{
    return IsTraceStoresRundownActive(GetGlobalTraceStoresRundown());
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
