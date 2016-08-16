#include "TracerConfig.h"

//__declspec(dllexport)
//PINITIALIZE_TRACER_CONFIG InitializeTracerConfig;

 //RTL_INIT_ONCE InitTracerConfigOnce = INIT_ONCE_STATIC_INIT;

RTL_RUN_ONCE InitTracerConfigOnce = RTL_RUN_ONCE_INIT;

__declspec(dllexport)
PTRACER_CONFIG TracerConfig = NULL;

_Use_decl_annotations_
__declspec(dllexport)
BOOLEAN
InitializeTracerConfig(VOID)
{

    return FALSE;
}
