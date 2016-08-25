#pragma once

#include "PythonTracer.h"

TRACER_API START_TRACING StartTracing;
TRACER_API STOP_TRACING StopTracing;

TRACER_API START_PROFILING StartProfiling;
TRACER_API STOP_PROFILING StopProfiling;

TRACER_API ENABLE_MEMORY_TRACING EnableMemoryTracing;
TRACER_API DISABLE_MEMORY_TRACING DisableMemoryTracing;

TRACER_API ENABLE_IO_COUNTERS_TRACING EnableIoCountersTracing;
TRACER_API DISABLE_IO_COUNTERS_TRACING DisableIoCountersTracing;

TRACER_API ENABLE_HANDLE_COUNT_TRACING EnableHandleCountTracing;
TRACER_API DISABLE_HANDLE_COUNT_TRACING DisableHandleCountTracing;

TRACER_API ADD_MODULE_NAME AddModuleName;

// vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                     :
