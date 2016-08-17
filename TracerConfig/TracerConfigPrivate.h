#pragma once

#include "TracerConfig.h"

_Success_(return != 0)
BOOLEAN
CreateTracerConfig(
    _Out_ PPTRACER_CONFIG TracerConfigPointer
    );

_Success_(return != 0)
BOOLEAN
InitializeSimpleTracerConfigFields(
    _Out_ PTRACER_CONFIG TracerConfig
    );
