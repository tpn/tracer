#include "stdafx.h"

_Use_decl_annotations_
VOID
TracerControlUnload(
    PDRIVER_OBJECT Driver
    )
{
    PDEVICE_OBJECT Device;
    PTRACER_CONTROL_DEV_EXT DevExt;

    ENTER("Unload");

    Device = Driver->DeviceObject;

    if (!Device) {

        return;
    }

    //
    // Get our extension.
    //

    DevExt = (PTRACER_CONTROL_DEV_EXT)Device->DeviceExtension;

    if (DevExt) {

        //
        // Close our registry key if set.
        //

        if (DevExt->RegistryHandle) {
            ZwClose(DevExt->RegistryHandle);
            DevExt->RegistryHandle = NULL;
        }
    }

    //
    // Delete symbolic link.
    //

    IoDeleteSymbolicLink((PUNICODE_STRING)&TracerControlWin32DeviceName);

    //
    // Delete the device.
    //

    IoDeleteDevice(Device);

    LEAVE("Unload");

    return;
}


_Use_decl_annotations_
NTSTATUS
NTAPI
TracerControlClose(
    PDEVICE_OBJECT Device,
    PIRP Irp
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IoStack;

    UNREFERENCED_PARAMETER(Device);

    ENTER("Close");

    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Fill in with blank details now.
    //

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);


    LEAVE_STATUS("Close", Status);

    return Status;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
TracerControlDeviceControl(
    PDEVICE_OBJECT Device,
    PIRP Irp
    )
{
    ULONG Code;
    NTSTATUS Status;
    PIO_STACK_LOCATION IoStack;
    PTRACER_CONTROL_DEV_EXT DevExt;
    ULONG OutputBufferLength;

    //
    // Get current Irp stack location.
    //

    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Get our device extension.
    //

    DevExt = (PTRACER_CONTROL_DEV_EXT)Device->DeviceExtension;

    //
    // Extract Ioctl code and output buffer length.
    //

    Code = IoStack->Parameters.DeviceIoControl.IoControlCode;

    OutputBufferLength = IoStack->Parameters.DeviceIoControl.OutputBufferLength;

    DEBUG2("DeviceControl: Received Ioctl: 0x%0x/%d.\n", Code, Code);
    DEBUG1("DeviceControl: OutputBufferLength: %d.\n", OutputBufferLength);

    switch (Code) {

        case IOCTL_TRACER_CONTROL_DEVEXT_SIZE: {
            PULONG OutputBuffer;

            //
            // Validate output buffer length.
            //

            if (OutputBufferLength < sizeof(ULONG)) {

                Status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = 0;
                break;
            }

            //
            // This is a METHOD_BUFFERED request, so the output buffer
            // will be in the SystemBuffer field.
            //

            OutputBuffer = (PULONG)Irp->AssociatedIrp.SystemBuffer;

            //
            // Write the value to the buffer.
            //

            *OutputBuffer = DevExt->Size;

            //
            // Fill in Irp details indicating success.
            //

            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = sizeof(ULONG);
            break;

        }

        case IOCTL_TRACER_CONTROL_READ_CR3: {
            PULONGLONG OutputBuffer;

            //
            // Validate output buffer length.
            //

            if (OutputBufferLength < sizeof(ULONGLONG)) {

                Status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = 0;
                break;
            }

            //
            // This is a METHOD_BUFFERED request, so the output buffer
            // will be in the SystemBuffer field.
            //

            OutputBuffer = (PULONGLONG)Irp->AssociatedIrp.SystemBuffer;

            //
            // Read the value of Cr3 directly into the output buffer.
            //

            ReadCr3(OutputBuffer);

            //
            // Fill in Irp details indicating success.
            //

            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = sizeof(ULONGLONG);
            break;

        }

        case IOCTL_TRACER_CONTROL_READ_DR7: {
            PULONGLONG OutputBuffer;

            //
            // Validate output buffer length.
            //

            if (OutputBufferLength < sizeof(ULONGLONG)) {

                Status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Status = Status;
                Irp->IoStatus.Information = 0;
                break;
            }

            //
            // This is a METHOD_BUFFERED request, so the output buffer
            // will be in the SystemBuffer field.
            //

            OutputBuffer = (PULONGLONG)Irp->AssociatedIrp.SystemBuffer;

            //
            // Read the value of Dr7 directly into the output buffer.
            //

            ReadDr7(OutputBuffer);

            //
            // Fill in Irp details indicating success.
            //

            Status = STATUS_SUCCESS;
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = sizeof(ULONGLONG);
            break;

        }

        default: {

            Status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = 0;

            DEBUG1("DeviceControl: Invalid Ioctl: 0x%0x.\n", Code);
            break;

        }
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    LEAVE_STATUS("DeviceControl", Status);

    return Status;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
TracerControlRead(
    PDEVICE_OBJECT Device,
    PIRP Irp
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IoStack;

    UNREFERENCED_PARAMETER(Device);

    ENTER("Read");

    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Fill in with blank details now.
    //

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    LEAVE_STATUS("Read", Status);

    return Status;
}

_Use_decl_annotations_
NTSTATUS
NTAPI
TracerControlWrite(
    PDEVICE_OBJECT Device,
    PIRP Irp
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IoStack;

    UNREFERENCED_PARAMETER(Device);

    ENTER("Write");

    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Fill in with blank details now.
    //

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    LEAVE_STATUS("Write", Status);

    return Status;
}

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
