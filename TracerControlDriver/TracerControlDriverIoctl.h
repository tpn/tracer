#pragma once

#define FILE_DEVICE_TRACER_CONTROL 52118

#define IOCTL_TRACER_CONTROL_DEVEXT_SIZE \
    CTL_CODE(FILE_DEVICE_TRACER_CONTROL, \
             2049,                       \
             METHOD_BUFFERED,            \
             FILE_ANY_ACCESS)

#define IOCTL_TRACER_CONTROL_READ_CR3    \
    CTL_CODE(FILE_DEVICE_TRACER_CONTROL, \
             2050,                       \
             METHOD_BUFFERED,            \
             FILE_ANY_ACCESS)

