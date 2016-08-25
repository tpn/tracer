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

#define IOCTL_TRACER_CONTROL_READ_DR7    \
    CTL_CODE(FILE_DEVICE_TRACER_CONTROL, \
             2051,                       \
             METHOD_BUFFERED,            \
             FILE_ANY_ACCESS)

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
