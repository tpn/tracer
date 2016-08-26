#include "stdafx.h"

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

// vim: set ts=8 sw=4 sts=4 tw=80 expandtab si ai                              :
