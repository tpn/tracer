
#include "TracerConfigPrivate.h"
#include "TracerConfigConstants.h"
#include "../Rtl/Rtl.h"

VOID
FreeUnicodeStringBuffer(
    PALLOCATOR Allocator,
    PUNICODE_STRING String
    )
{
    if (!IsValidUnicodeString(String)) {
        return;
    }

    __try {
        Allocator->Free(Allocator->Context, String->Buffer);
        String->Buffer = NULL;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        String->Buffer = NULL;
    }

    return;
}

_Use_decl_annotations_
VOID
DestroyTracerConfig(
    PTRACER_CONFIG TracerConfig
    )
{
    USHORT Index;
    USHORT Offset;
    PALLOCATOR Allocator;
    PTRACER_PATHS Paths;
    PUNICODE_STRING String;

    //
    // Validate arguments.
    //

    if (!TracerConfig) {
        return;
    }

    //
    // If there's no allocator, we can't free anything.
    //

    if (!TracerConfig->Allocator) {
        return;
    }

    //
    // Initialize helper aliases.
    //

    Allocator = TracerConfig->Allocator;
    Paths = &TracerConfig->Paths;

    //
    // Free the installation and base trace directory strings.
    //

    FreeUnicodeStringBuffer(Allocator, &Paths->InstallationDirectory);
    FreeUnicodeStringBuffer(Allocator, &Paths->BaseTraceDirectory);

    //
    // Enumerate over the PathOffsets[], freeing each path as we go.
    //

    for (Index = 0; Index < NumberOfPathOffsets; Index++) {
        Offset = PathOffsets[Index].Offset;
        String = (PUNICODE_STRING)((((ULONG_PTR)Paths) + Offset));
        FreeUnicodeStringBuffer(Allocator, String);
    }

    return;
}

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
