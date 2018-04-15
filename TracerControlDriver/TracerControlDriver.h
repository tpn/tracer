#pragma once

#include "stdafx.h"
#include <ntddk.h>
#include "TracerControlDriverIoctl.h"
#include "TracerControlDebug.h"
#include "TracerControlConstants.h"
#include "DriverUtil.h"

//
// TracerControl Driver's Device Extension Object.
//

typedef struct _TRACER_CONTROL_DEV_EXT {
    USHORT Size;
    USHORT Unused1;
    ULONG Unused2;

    HANDLE RegistryHandle;

} TRACER_CONTROL_DEV_EXT, *PTRACER_CONTROL_DEV_EXT;

//
// Driver entry and dispatch type definitions.
//

typedef DRIVER_INITIALIZE TRACER_CONTROL_INITIALIZE;
typedef DRIVER_UNLOAD TRACER_CONTROL_UNLOAD;

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD TracerControlUnload;

typedef
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH
TRACER_CONTROL_DEVICE_CONTROL, *PTRACER_CONTROL_DEVICE_CONTROL;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH
TracerControlDeviceControl;

typedef
_Dispatch_type_(IRP_MJ_CREATE)
DRIVER_DISPATCH
TRACER_CONTROL_CREATE, *PTRACER_CONTROL_CREATE;

_Dispatch_type_(IRP_MJ_CREATE)
DRIVER_DISPATCH
TracerControlCreate;

typedef
_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH
TRACER_CONTROL_CLOSE, *PTRACER_CONTROL_CLOSE;

_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH
TracerControlClose;

typedef
_Dispatch_type_(IRP_MJ_WRITE)
DRIVER_DISPATCH
TRACER_CONTROL_WRITE, *PTRACER_CONTROL_WRITE;

_Dispatch_type_(IRP_MJ_WRITE)
DRIVER_DISPATCH
TracerControlWrite;

typedef
_Dispatch_type_(IRP_MJ_READ)
DRIVER_DISPATCH
TRACER_CONTROL_READ, *PTRACER_CONTROL_READ;

_Dispatch_type_(IRP_MJ_READ)
DRIVER_DISPATCH
TracerControlRead;

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
