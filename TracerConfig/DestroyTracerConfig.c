
#include "TracerConfigPrivate.h"
#include "../Rtl/Rtl.h"

#define FREE_UNICODE_BUFFER(Name) do { \
    if (Paths->Name && Paths->Name.Buffer != NULL) { \
        PVOID Buffer = Paths->Name.Buffer; \
        __try { \
            Allocator->Free(Allocator->Context, Buffer); \
        } __except(EXCEPTION_EXECUTE_HANDLER) { \
            NULL; \
        } \
    } \
} while (0)

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
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        NULL;
    }

    return;
}

_Use_decl_annotations_
VOID
DestroyTracerConfig(
    PTRACER_CONFIG TracerConfig
    )
{
    PALLOCATOR Allocator;
    PTRACER_PATHS Paths;

    if (!TracerConfig) {
        return;
    }
    
    if (!TracerConfig->Allocator) {
        return;
    }

    Allocator = TracerConfig->Allocator;
    Paths = &TracerConfig->Paths;

    FreeUnicodeStringBuffer(Allocator, &Paths->InstallationDirectory);
    FreeUnicodeStringBuffer(Allocator, &Paths->BaseTraceDirectory);
    FreeUnicodeStringBuffer(Allocator, &Paths->RtlDllPath);
    FreeUnicodeStringBuffer(Allocator, &Paths->TracerDllPath);
    FreeUnicodeStringBuffer(Allocator, &Paths->PythonDllPath);
    FreeUnicodeStringBuffer(Allocator, &Paths->PythonTracerDllPath);

    return;
}