
#include "stdafx.h"
#include <ntddk.h>
#include "TracerControlDriver.h"

//
// Forward declarations.
//

/*
TRACER_CONTROL_INITIALIZE DriverEntry;
TRACER_CONTROL_UNLOAD TracerControlUnload;

TRACER_CONTROL_DEVICE_CONTROL TracerControlDeviceControl;
TRACER_CONTROL_CREATE TracerControlCreate;
TRACER_CONTROL_CLOSE TracerControlClose;
TRACER_CONTROL_WRITE TracerControlWrite;
TRACER_CONTROL_READ TracerControlRead;
*/

_Use_decl_annotations_
NTSTATUS
NTAPI
DriverEntry(
    PDRIVER_OBJECT Driver,
    PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS Status;
    ULONG Attributes;
    HANDLE RegistryHandle = NULL;
    PDEVICE_OBJECT DeviceObject;
    PTRACER_CONTROL_DEV_EXT DevExt;
    OBJECT_ATTRIBUTES ObjectAttributes;

    ENTER("DriverEntry");

    if (RegistryPath && RegistryPath->Length) {
        DEBUG1("DriverEntry: RegistryPath: %wZ.\n", RegistryPath);
    }

    //
    // Initialize our major functions.
    //

    Driver->MajorFunction[IRP_MJ_CREATE] = TracerControlCreate;
    Driver->MajorFunction[IRP_MJ_CLOSE] = TracerControlClose;

    Driver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TracerControlDeviceControl;

    Driver->DriverUnload = TracerControlUnload;

    //
    // Create the device.
    //

    Status = IoCreateDevice(
        Driver,
        sizeof(TRACER_CONTROL_DEV_EXT),
        (PUNICODE_STRING)&TracerControlDeviceName,
        FILE_DEVICE_TRACER_CONTROL,
        FILE_DEVICE_SECURE_OPEN,
        FALSE,
        &DeviceObject
    );

    if (!NT_SUCCESS(Status)) {
        DEBUG1("DriverEntry: IoCreateDevice failed: 0x%0x\n", Status);
        goto End;
    }

    //
    // Initialize the device extension.
    //

    DevExt = (PTRACER_CONTROL_DEV_EXT)DeviceObject->DeviceExtension;

    DevExt->Size = sizeof(TRACER_CONTROL_DEV_EXT);

    //
    // Initialize an OBJECT_ATTRIBUTES in order to call ZwOpenKey() against
    // our registry path.
    //

    Attributes = OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE;

    InitializeObjectAttributes(
        &ObjectAttributes,
        RegistryPath,
        Attributes,
        NULL,   // RootDirectory
        NULL    // SecurityDescriptor
    );

    //
    // Open a handle to our driver's registry path.
    //

    Status = ZwOpenKey(&RegistryHandle,
                       KEY_ALL_ACCESS,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        DEBUG1("DriverEntry: ZwOpenKey failed: 0x%0x\n", Status);
        goto Error;
    }

    //
    // Register the symbolic link so that the device is visible to Win32.
    //

    Status = IoCreateSymbolicLink(
        (PUNICODE_STRING)&TracerControlWin32DeviceName,
        (PUNICODE_STRING)&TracerControlDeviceName
    );

    if (!NT_SUCCESS(Status)) {
        DEBUG1("DriverEntry: IoCreateSymbolicLink failed: 0x%0x\n", Status);
        goto Error;
    }

    //
    // Tell the I/O Manager to buffer our reads/writes.
    //

    DeviceObject->Flags |= DO_BUFFERED_IO;

    //
    // That's it, we're done, initialization was successful.
    //

    Status = STATUS_SUCCESS;

    goto End;

Error:

    //
    // Delete our device.
    //

    IoDeleteDevice(DeviceObject);

    ASSERT(!NT_SUCCESS(Status));

End:

    //
    // Close our handle to RegistryPath if we opened it.
    //

    if (RegistryHandle) {
        ZwClose(RegistryHandle);
        RegistryHandle = NULL;
    }

    LEAVE_STATUS("DriverEntry", Status);

    return Status;
}

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
        NOTHING;
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
TracerControlCreate(
    PDEVICE_OBJECT Device,
    PIRP Irp
    )
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IoStack;

    UNREFERENCED_PARAMETER(Device);

    ENTER("Create");

    IoStack = IoGetCurrentIrpStackLocation(Irp);

    //
    // Fill in with blank details now.
    //

    Status = STATUS_SUCCESS;
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    LEAVE_STATUS("Create", Status);

    return Status;
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

    DEBUG2("DeviceControl: Received Ioctl: 0x%0x/%u.\n", Code, Code);
    DEBUG1("DeviceControl: OutputBufferLength: %u.\n", OutputBufferLength);

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
