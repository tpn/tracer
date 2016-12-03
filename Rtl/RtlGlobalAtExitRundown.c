/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    RtlGlobalAtExitRundown.c

Abstract:

    This module implements "global" versions of routines implemented by the
    RtlAtExitRundown module.  These routines differ in that they do not take
    a PRTL_ATEXIT_RUNDOWN parameter; instead, a global RTL_ATEXIT_RUNDOWN
    structure is defined by this module, and this is passed to the relevant
    RtlAtExitRundown counterpart routine implicitly.

--*/

#include "stdafx.h"

RTL_DATA RTL_ATEXIT_RUNDOWN GlobalRtlAtExitRundown = { 0 };

PRTL_ATEXIT_RUNDOWN
GetGlobalRtlAtExitRundown(
    VOID
    )
/*++

Routine Description:

    This routine returns a pointer to the global RTL_ATEXIT_RUNDOWN structure.

Arguments:

    None.

Return Value:

    A pointer to the global RTL_ATEXIT_RUNDOWN structure.

--*/
{
    return &GlobalRtlAtExitRundown;
}

_Use_decl_annotations_
BOOL
InitializeGlobalRtlAtExitRundown(
    VOID
    )
/*++

Routine Description:

    This routine initializes the global RTL_ATEXIT_RUNDOWN structure.

Arguments:

    None.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    SetAtExit(RegisterGlobalAtExitFunc);
    return InitializeRtlAtExitRundown(GetGlobalRtlAtExitRundown());
}

VOID
DestroyGlobalRtlAtExitRundown(
    VOID
    )
/*++

Routine Description:

    This routine destroys the global RTL_ATEXIT_RUNDOWN structure.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DestroyRtlAtExitRundown(GetGlobalRtlAtExitRundown());
}

_Use_decl_annotations_
BOOL
RegisterGlobalAtExitFunc(
    PATEXITFUNC AtExitFunc
    )
/*++

Routine Description:

    This routine registers an AtExitFunc function pointer with the global atexit
    rundown structure.  This is the routine called when downstream callers call
    atexit().

Arguments:

    PATEXITFUNC - Supplies a pointer to an ATEXITFUNC to call at rundown.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    return RegisterAtExitFunc(GetGlobalRtlAtExitRundown(), AtExitFunc);
}

_Use_decl_annotations_
VOID
RundownGlobalAtExitFunctions(
    BOOL IsProcessTerminating
    )
/*++

Routine Description:

    This routine runs down the atexit functions associated with the global
    atexit rundown list.

Arguments:

    IsProcessTerminating - Supplies a boolean variable that indicates if the
        function is being called due to process termination.  When FALSE,
        implies the function is being called because FreeLibrary() was
        called and there were no more references left to the DLL.

Return Value:

    None.

--*/
{
    RundownAtExitFunctions(GetGlobalRtlAtExitRundown(), IsProcessTerminating);
}

_Use_decl_annotations_
BOOL
IsGlobalRtlAtExitRundownActive(
    VOID
    )
/*++

Routine Description:

    Returns TRUE if a global atexit rundown is active.

Arguments:

    None.

Return Value:

    TRUE if active, FALSE otherwise.

--*/
{
    return IsRtlAtExitRundownActive(GetGlobalRtlAtExitRundown());
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
