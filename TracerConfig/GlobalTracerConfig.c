#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

#include <Windows.h>

#pragma data_seg("Shared")
__declspec(dllexport) PTRACER_CONFIG GlobalTracerConfig;
#pragma data_seg()

#pragma comment(linker, "/section:Shared,rws")

MALLOC Malloc;
CALLOC Calloc;
REALLOC Realloc;
FREE Free;

static ALLOCATOR Allocator;

static CONST UNICODE_STRING RegistryPath = \
    RTL_CONSTANT_STRING(L"Software\\Tracer");

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
VOID
Free(
    PVOID Context,
    PVOID Buffer
    )
{
    HeapFree((HANDLE)Context, 0, Buffer);
    return;
}

__declspec(dllexport)
_Use_decl_annotations_
BOOLEAN
CreateGlobalTraceSessionDirectory(
    PUNICODE_STRING Directory
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

    RtlSecureZeroMemory(&Allocator, sizeof(Allocator));

    Allocator.Malloc = Malloc;
    Allocator.Free = Free;
    Allocator.Context = GetProcessHeap();

    TracerConfig = InitializeTracerConfig(&Allocator, &RegistryPath);

    if (!TracerConfig) {
        return FALSE;
    }

    GlobalTracerConfig = TracerConfig;

    return TRUE;
}

_Use_decl_annotations_
VOID
DestroyGlobalTraceSession(VOID)
{
    DestroyTracerConfig(GlobalTracerConfig);
    return;
}
