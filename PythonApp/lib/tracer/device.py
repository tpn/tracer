#===============================================================================
# Imports
#===============================================================================
import sys

from win32file import (
    CancelIo,
    ReadFile,
    WriteFile,
    CloseHandle,
    CreateFileW,
    FindStreams,
    DeviceIoControl,
    AllocateReadBuffer,

    GetOverlappedResult,
    CreateIoCompletionPort,
    GetQueuedCompletionStatus,
    PostQueuedCompletionStatus,

    OPEN_ALWAYS,
    OPEN_EXISTING,

    GENERIC_READ,
    GENERIC_WRITE,
    GENERIC_EXECUTE,

    INVALID_HANDLE_VALUE
)

import winioctlcon

from winioctlcon import (
    CTL_CODE,
    FILE_ANY_ACCESS,
    METHOD_BUFFERED,
)

from struct import (
    pack,
    unpack,
    calcsize,
)

from pywintypes import (
    SECURITY_ATTRIBUTES,
)

#===============================================================================
# Globals/Aliases
#===============================================================================
CreateFile = CreateFileW
FILE_DEVICE_TRACER_CONTROL = 52118

#===============================================================================
# Ioctl Codes
#===============================================================================
IOCTL_TRACER_CONTROL_DEVEXT_SIZE = CTL_CODE(
    FILE_DEVICE_TRACER_CONTROL,
    2049,
    METHOD_BUFFERED,
    FILE_ANY_ACCESS,
)

IOCTL_TRACER_CONTROL_READ_CR3 = CTL_CODE(
    FILE_DEVICE_TRACER_CONTROL,
    2050,
    METHOD_BUFFERED,
    FILE_ANY_ACCESS,
)


#===============================================================================
# Helper Methods
#===============================================================================
def ulong(s):
    return unpack('<L', s)[0]

def sizeof_ulong():
    return calcsize('<L')

def ulonglong(s):
    return unpack('<Q', s)[0]

def sizeof_ulonglong():
    return calcsize('<Q')

#===============================================================================
# Classes
#===============================================================================
class TracerControlDevice(object):
    def __init__(self, conf=None, options=None, name=None):
        if options is None:
            from .util import Options

        self.options = options

        if not conf:
            from .config import get_or_create_config
            conf = get_or_create_config()

        self.conf = conf

        self.name = name
        if not self.name:
            self.name = conf.tracer_control_device_win32_name

        self.handle = CreateFile(
            self.name,
            GENERIC_READ | GENERIC_WRITE,
            0,              # Share access
            None,           # Security attributes
            OPEN_EXISTING,
            0,              # Flags/attributes
            0,              # Extended attributes
        )

    def close(self):
        self.handle.close()
        self.handle = None

        self.conf = None
        self.options = None


    def ioctl(self, code, in_buffer, out_buffer, overlapped=None):
        result = DeviceIoControl(
            self.handle,
            code,
            in_buffer,
            out_buffer,
            overlapped
        )

        return result

    @property
    def device_extension_size(self):

        buf = AllocateReadBuffer(sizeof_ulong())
        code = IOCTL_TRACER_CONTROL_DEVEXT_SIZE

        result = self.ioctl(code, None, buf)

        return ulong(result)

    @property
    def cr3(self):
        buf = AllocateReadBuffer(sizeof_ulonglong())
        code = IOCTL_TRACER_CONTROL_READ_CR3

        result = self.ioctl(code, None, buf)
        return ulonglong(result)

    def __enter__(self):
        return self

    def __exit__(self, *exc_info):
        self.close()

    @classmethod
    def create(cls, conf=None, options=None):
        return cls(conf, options)


# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
