/*++

Copyright (c) 2016 Trent Nelson <trent@trent.me>

Module Name:

    Privilege.c

Abstract:

    This module implements functionality related to system security components.
    Routines are provided for setting or revoking generic token privileges,
    and enabling and disabling the volume management privilege.

--*/

#include "stdafx.h"

_Use_decl_annotations_
BOOL
SetPrivilege(
    PWSTR PrivilegeName,
    BOOL Enable
    )
/*++

Routine Description:

    This routine enables or disables a given privilege name for the current
    process token.

Arguments:

    PrivilegeName - Supplies a pointer to a NULL-terminated wide string that
        represents the privilege name.

    Enable - Supplies a boolean value indicating that the privilege should be
        enabled when set to TRUE, disabled when set to FALSE.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    BOOL Success;
    DWORD LastError;
    DWORD DesiredAccess;
    DWORD TokenAttributes;
    HANDLE ProcessHandle;
    HANDLE TokenHandle;
    TOKEN_PRIVILEGES TokenPrivileges;

    //
    // Initialize local variables.
    //

    ProcessHandle = GetCurrentProcess();
    DesiredAccess = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY;

    if (Enable) {
        TokenAttributes = SE_PRIVILEGE_ENABLED;
    } else {
        TokenAttributes = 0;
    }

    //
    // Obtain a token handle for the current process.
    //

    Success = OpenProcessToken(ProcessHandle, DesiredAccess, &TokenHandle);

    if (!Success) {
        return FALSE;
    }

    //
    // Lookup the privilege value for the name passed in by the caller.
    //

    Success = LookupPrivilegeValueW(
        NULL,
        PrivilegeName,
        &TokenPrivileges.Privileges[0].Luid
    );

    if (!Success) {
        goto End;
    }

    //
    // Fill in the remaining token privilege fields.
    //

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Attributes = TokenAttributes;

    //
    // Attempt to adjust the token privileges.
    //

    Success = AdjustTokenPrivileges(
        TokenHandle,
        FALSE,
        &TokenPrivileges,
        0,
        NULL,
        0
    );

    LastError = GetLastError();

    if (LastError != ERROR_SUCCESS) {
        Success = FALSE;
    }

    //
    // Intentional follow-on.
    //

End:
    CloseHandle(TokenHandle);

    return Success;
}

_Use_decl_annotations_
BOOL
EnablePrivilege(
    PWSTR PrivilegeName
    )
{
    return SetPrivilege(PrivilegeName, TRUE);
}

_Use_decl_annotations_
BOOL
DisablePrivilege(
    PWSTR PrivilegeName
    )
{
    return SetPrivilege(PrivilegeName, FALSE);
}

////////////////////////////////////////////////////////////////////////////////
// SE_MANAGE_VOLUME/SeManageVolume
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
BOOL
EnableManageVolumePrivilege(
    VOID
    )
{
    return EnablePrivilege(SE_MANAGE_VOLUME_NAME);
}

_Use_decl_annotations_
BOOL
DisableManageVolumePrivilege(
    VOID
    )
{
    return DisablePrivilege(SE_MANAGE_VOLUME_NAME);
}

////////////////////////////////////////////////////////////////////////////////
// SE_LOCK_MEMORY/SeLockMemory
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
BOOL
EnableLockMemoryPrivilege(
    VOID
    )
{
    return EnablePrivilege(SE_LOCK_MEMORY_NAME);
}

_Use_decl_annotations_
BOOL
DisableLockMemoryPrivilege(
    VOID
    )
{
    return DisablePrivilege(SE_LOCK_MEMORY_NAME);
}

////////////////////////////////////////////////////////////////////////////////
// SE_DEBUG/SeDebug
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
BOOL
EnableDebugPrivilege(
    VOID
    )
{
    return EnablePrivilege(SE_DEBUG_NAME);
}

_Use_decl_annotations_
BOOL
DisableDebugPrivilege(
    VOID
    )
{
    return DisablePrivilege(SE_DEBUG_NAME);
}

////////////////////////////////////////////////////////////////////////////////
// SE_SYSTEM_PROFILE/SeSystemProfile
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
BOOL
EnableSystemProfilePrivilege(
    VOID
    )
{
    return EnablePrivilege(SE_SYSTEM_PROFILE_NAME);
}

_Use_decl_annotations_
BOOL
DisableSystemProfilePrivilege(
    VOID
    )
{
    return DisablePrivilege(SE_SYSTEM_PROFILE_NAME);
}

////////////////////////////////////////////////////////////////////////////////
// PROF_SINGLE_PROCESS/SeProfileSingleProcess
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
BOOL
EnableProfileSingleProcessPrivilege(
    VOID
    )
{
    return EnablePrivilege(SE_PROF_SINGLE_PROCESS_NAME);
}

_Use_decl_annotations_
BOOL
DisableProfileSingleProcessPrivilege(
    VOID
    )
{
    return DisablePrivilege(SE_PROF_SINGLE_PROCESS_NAME);
}

////////////////////////////////////////////////////////////////////////////////
// SE_INC_WORKING_SET/SeIncreaseWorkingSet
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
BOOL
EnableIncreaseWorkingSetPrivilege(
    VOID
    )
{
    return EnablePrivilege(SE_INC_WORKING_SET_NAME);
}

_Use_decl_annotations_
BOOL
DisableIncreaseWorkingSetPrivilege(
    VOID
    )
{
    return DisablePrivilege(SE_INC_WORKING_SET_NAME);
}

////////////////////////////////////////////////////////////////////////////////
// SE_CREATE_SYMBOLIC_LINK/SeCreateSymbolicLink
////////////////////////////////////////////////////////////////////////////////

_Use_decl_annotations_
BOOL
EnableCreateSymbolicLinkPrivilege(
    VOID
    )
{
    return EnablePrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);
}

_Use_decl_annotations_
BOOL
DisableCreateSymbolicLinkPrivilege(
    VOID
    )
{
    return DisablePrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
