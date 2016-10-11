#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

#include <Windows.h>

PTRACER_CONFIG GlobalTracerConfig = NULL;
PALLOCATOR GlobalTracerAllocator = NULL;

MALLOC __Malloc;
CALLOC __Calloc;
REALLOC __Realloc;
FREE __Free;

static ALLOCATOR __Allocator;

_Use_decl_annotations_
PVOID
__Malloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc((HANDLE)Context, HEAP_ZERO_MEMORY, Size);
}

_Use_decl_annotations_
PVOID
__Calloc(
    PVOID Context,
    SIZE_T NumberOfElements,
    SIZE_T SizeOfElements
    )
{
    return HeapAlloc(
        (HANDLE)Context,
        HEAP_ZERO_MEMORY,
        NumberOfElements * SizeOfElements
    );

}

_Use_decl_annotations_
VOID
__Free(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree((HANDLE)Context, 0, Buffer);
    return;
}

_Use_decl_annotations_
BOOL
CreateGlobalTraceSessionDirectory(
    PPUNICODE_STRING Directory
    )
{
    return CreateTraceSessionDirectory(
        GlobalTracerConfig,
        Directory
    );
}

_Use_decl_annotations_
BOOL
InitializeGlobalTracerConfig(VOID)
{
    PTRACER_CONFIG TracerConfig;

    if (GlobalTracerConfig) {
        return TRUE;
    }

    RtlSecureZeroMemory(&__Allocator, sizeof(__Allocator));

    __Allocator.Malloc = __Malloc;
    __Allocator.Calloc = __Calloc;
    __Allocator.Free = __Free;
    __Allocator.Context = GetProcessHeap();

    TracerConfig = InitializeTracerConfig(
        &__Allocator,
        (PUNICODE_STRING)&TracerRegistryPath
    );

    if (!TracerConfig) {
        return FALSE;
    }

    GlobalTracerConfig = TracerConfig;

    return TRUE;
}

_Use_decl_annotations_
BOOL
GetGlobalTracerConfig(
    PPTRACER_CONFIG TracerConfig
    )
{
    //
    // Validate arguments.
    //

    if (!ARGUMENT_PRESENT(TracerConfig)) {
        return FALSE;
    }

    //
    // Clear the caller's pointer up front.
    //

    *TracerConfig = NULL;

    //
    // If GlobalTracerConfig hasn't been initialized, return FALSE.  Otherwise,
    // update the caller's pointer and return TRUE for success.
    //

    if (!GlobalTracerConfig) {
        return FALSE;
    }

    *TracerConfig = GlobalTracerConfig;

    return TRUE;
}

_Use_decl_annotations_
VOID
DestroyGlobalTracerConfig(VOID)
{
    DestroyTracerConfig(GlobalTracerConfig);
    return;
}

#ifdef _DEBUG
VOID
DestroyTraceSessionDirectories(
    _In_ PTRACER_CONFIG TracerConfig
    );

VOID
DestroyGlobalTraceSessionDirectory(VOID)
{
    DestroyTraceSessionDirectories(GlobalTracerConfig);
    return;
}
#endif

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
