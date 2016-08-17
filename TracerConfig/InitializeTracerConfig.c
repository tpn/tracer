#include "TracerConfig.h"

_Use_decl_annotations_
BOOLEAN
CreateTracerConfig(
    PPTRACER_CONFIG TracerConfigPointer
    )
/*++

Routine Description:

    Allocates and zeros a TRACER_CONFIG structure, saving the resulting
    address to the TracerConfigPointer parameter.  Memory is allocated
    via ExAllocatePoolWithTagPriority().  A successfully-created structure
    should be paired with a corresponding DestroyTracerConfig() call once
    the structure is no longer needed.

Arguments:

    TracerConfigPointer - Supplies a pointer to an address that receives the
        address of the newly-created TRACER_CONFIG structure.

Return Value:

    TRUE on success, FALSE on failure.

--*/
{
    PTRACER_CONFIG TracerConfig;

    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfigPointer)) {
        return FALSE;
    }

    //
    // Allocate memory for the TRACER_CONFIG struct.
    //

    TracerConfig = (PTRACER_CONFIG)(
        ExAllocatePoolWithTagPriority(
            PagedPool,
            sizeof(TRACER_CONFIG),
            TRACER_CONFIG_POOL_TAG,
            TRACER_CONFIG_POOL_PRIORITY
        )
    );

    //
    // Validate allocation.
    //

    if (!TracerConfig) {
        return FALSE;
    }

    // 
    // Zero the memory.
    //

    RtlSecureZeroMemory(TracerConfig, sizeof(TRACER_CONFIG));

    //
    // Update the caller's pointer and return success.
    //

    *TracerConfigPointer = TracerConfig;

    return TRUE;
}

_Use_decl_annotations_
BOOLEAN
InitializeSimpleTracerConfigFields(
    PTRACER_CONFIG TracerConfig
    )
{
    TracerConfig->Flags.EnableTraceSessionDirectoryCompression = TRUE;
    TracerConfig->Flags.PrefaultPages = TRUE;

#ifdef _DEBUG
    TracerConfig->Flags.IsDebug = TRUE;
#endif

    TracerConfig->SupportedRuntimes.Python = TRUE;
    TracerConfig->SupportedRuntimes.C = TRUE;

    return TRUE;
}



