#include "TracerConfig.h"
#include "TracerConfigPrivate.h"

//__declspec(dllexport)
//PINITIALIZE_TRACER_CONFIG InitializeTracerConfig;

 //RTL_INIT_ONCE InitTracerConfigOnce = INIT_ONCE_STATIC_INIT;

RTL_RUN_ONCE InitTracerConfigOnce = RTL_RUN_ONCE_INIT;

__declspec(dllexport)
PTRACER_CONFIG GlobalTracerConfig = NULL;

BOOLEAN
InitializeTracerConfigOnce(VOID)
{
    BOOLEAN Status;
    PTRACER_CONFIG TracerConfig;

    Status = CreateTracerConfig(&TracerConfig);
    if (!Status) {
        return Status;
    }

    Status = InitializeSimpleTracerConfigFields(TracerConfig);
    if (!Status) {
        return Status;
    }

    GlobalTracerConfig = TracerConfig;

    return Status;
}

_Use_decl_annotations_
__declspec(dllexport)
BOOLEAN
InitializeTracerConfig(VOID)
{
    if (!GlobalTracerConfig) {
        return InitializeTracerConfigOnce();
    }

    return TRUE;
}
