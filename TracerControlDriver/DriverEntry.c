#include "stdafx.h"

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
    HANDLE RegistryHandle;
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

    Status = ZwOpenKey(&RegistryHandle, KEY_ALL_ACCESS, &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        DEBUG1("DriverEntry: ZwOpenKey failed: 0x%0x\n", Status);
        goto Error;
    }

    //
    // XXX TODO: read our registry configuration here.
    //

    ZwClose(RegistryHandle);

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
    LEAVE_STATUS("DriverEntry", Status);

    return Status;
}

// vim: set ts=8 sw=4 sts=4 expandtab si ai                                    :
