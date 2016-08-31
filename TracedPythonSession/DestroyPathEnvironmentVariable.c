#include "stdafx.h"

#include "TracedPythonSessionPrivate.h"

_Use_decl_annotations_
VOID
DestroyPathEnvironmentVariable(
    PPPATH_ENV_VAR PathPointer
    )
/*++

Routine Description:

    This routine destroys a PATH_ENV_VAR struct.  A partially-initialized
    struct (or NULL) can be passed.  (The intent being that this method is
    safe to call on the PATH_ENV_VAR in any state.)

Arguments:

    PathPointer - Supplies a pointer to the address of a pointer to a
        PATH_ENV_VAR struct.  This value will be cleared (as long as it is
        non-NULL).

Return Value:

    None.

--*/
{
    PPATH_ENV_VAR Path;
    PALLOCATOR Allocator;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(PathPointer)) {
        return;
    }

    Path = *PathPointer;

    //
    // Clear the user's pointer first.
    //

    *PathPointer = NULL;

    if (!Path) {
        return;
    }

    Allocator = Path->Allocator;

    if (!Allocator) {

        //
        // Can't do much if no allocator was set.
        //

        return;
    }

    //
    // The PATH_ENV_VAR struct is constructed via two memory allocations.  The
    // first is rooted at the Path struct and holds the struct plus bitmaps
    // and unicode string buffers.  The second is allocated once we know how
    // many directories are present, and is rooted at PathsPrefixTableEntries.
    // We check that first before blowing away Path.
    //

    if (Path->PathsPrefixTableEntries) {
        Allocator->FreePointer(
            Allocator->Context,
            &Path->PathsPrefixTableEntries
        );
    }

    Allocator->FreePointer(Allocator->Context, &Path);

    return;
}


// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
