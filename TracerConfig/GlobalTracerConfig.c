#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

#include <Windows.h>

PTRACER_CONFIG GlobalTracerConfig = NULL;
PALLOCATOR GlobalTracerAllocator = NULL;

MALLOC Malloc;
CALLOC Calloc;
REALLOC Realloc;
FREE Free;

static ALLOCATOR Allocator;

_Use_decl_annotations_
void * __restrict
Malloc(
    PVOID Context,
    SIZE_T Size
    )
{
    return HeapAlloc((HANDLE)Context, HEAP_ZERO_MEMORY, Size);
}

_Use_decl_annotations_
void * __restrict
Calloc(
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
Free(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree((HANDLE)Context, 0, Buffer);
    return;
}

_Use_decl_annotations_
BOOLEAN
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
BOOLEAN
InitializeGlobalTracerConfig(VOID)
{
    PTRACER_CONFIG TracerConfig;

    if (GlobalTracerConfig) {
        return TRUE;
    }

    RtlSecureZeroMemory(&Allocator, sizeof(Allocator));

    Allocator.Malloc = Malloc;
    Allocator.Calloc = Calloc;
    Allocator.Free = Free;
    Allocator.Context = GetProcessHeap();

    TracerConfig = InitializeTracerConfig(
        &Allocator,
        (PUNICODE_STRING)&TracerRegistryPath
    );

    if (!TracerConfig) {
        return FALSE;
    }

    GlobalTracerConfig = TracerConfig;

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
