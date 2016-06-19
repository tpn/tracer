#pragma once

#include "stdafx.h"
#include <ntddk.h>
#include "TracerControlDriverIoctl.h"

//
//
typedef struct _TRACER_CONTROL_DEV_EXT {
    USHORT Size;
    USHORT Unused1;
    ULONG Unused2;

    HANDLE RegistryHandle;

} TRACER_CONTROL_DEV_EXT, *PTRACER_CONTROL_DEV_EXT;

//
// Driver entry and dispatch declarations.
//

DRIVER_INITIALIZE DriverEntry;

__drv_dispatchType(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH TracerControlDeviceControl;

__drv_dispatchType(IRP_MJ_CREATE)
DRIVER_DISPATCH TracerControlCreate;

__drv_dispatchType(IRP_MJ_CLOSE)
DRIVER_DISPATCH TracerControlClose;

__drv_dispatchType(IRP_MJ_WRITE)
DRIVER_DISPATCH TracerControlWrite;

__drv_dispatchType(IRP_MJ_READ)
DRIVER_DISPATCH TracerControlRead;

DRIVER_UNLOAD TracerControlUnload;

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
