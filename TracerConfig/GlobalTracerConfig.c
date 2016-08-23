#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

#include <Windows.h>

//#pragma data_seg(".shared")
PTRACER_CONFIG GlobalTracerConfig = NULL;
PALLOCATOR GlobalTracerAllocator = NULL;
//#pragma data_seg()

//#pragma comment(linker, "/section:.shared,rws")

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
        (PUNICODE_STRING)&RegistryPath
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
