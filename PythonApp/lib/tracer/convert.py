#===============================================================================
# Imports
#===============================================================================
import os
import re
from tpn.clipboard import cb

#===============================================================================
# Constants
#===============================================================================
TYPEDEF_RIO_NOTIFICATION_COMPLETION = '''\
typedef struct _RIO_NOTIFICATION_COMPLETION {
  RIO_NOTIFICATION_COMPLETION_TYPE Type;
  union {
    struct {
      HANDLE EventHandle;
      BOOL   NotifyReset;
    } Event;
    struct {
      HANDLE IocpHandle;
      PVOID  CompletionKey;
      PVOID  Overlapped;
    } Iocp;
  };
} RIO_NOTIFICATION_COMPLETION, *PRIO_NOTIFICATION_COMPLETION;'''

CTYPEDEF_RIO_NOTIFICATION_COMPLETION = '''\
    ctypedef struct RIO_NOTIFICATION_COMPLETION:
        RIO_NOTIFICATION_COMPLETION_TYPE Type
        HANDLE EventHandle
        BOOL   NotifyReset
        HANDLE IocpHandle
        PVOID  CompletionKey
        PVOID  Overlapped
    ctypedef RIO_NOTIFICATION_COMPLETION *PRIO_NOTIFICATION_COMPLETION'''

TYPEDEF_FILE_STANDARD_INFO = '''\
typedef struct _FILE_STANDARD_INFO {
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    DWORD NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;
} FILE_STANDARD_INFO, *PFILE_STANDARD_INFO;'''

PYTHON_CTYPES_FILE_STANDARD_INFO_FORMAT = '''\
    class FILE_STANDARD_INFO(Structure):
        _fields_ = [
            ('AllocationSize', LARGE_INTEGER),
            ('EndOfFile', LARGE_INTEGER),
            ('NumberOfLinks', DWORD),
            ('DeletePending', BOOLEAN),
            ('Directory', BOOLEAN),
        ]'''

TYPEDEF_XSAVE_FORMAT = '''\
typedef struct DECLSPEC_ALIGN(16) _XSAVE_FORMAT {
    USHORT ControlWord;
    USHORT StatusWord;
    UCHAR TagWord;
    UCHAR Reserved1;
    USHORT ErrorOpcode;
    ULONG ErrorOffset;
    USHORT ErrorSelector;
    USHORT Reserved2;
    ULONG DataOffset;
    USHORT DataSelector;
    USHORT Reserved3;
    ULONG MxCsr;
    ULONG MxCsr_Mask;
    M128A FloatRegisters[8];

#if defined(_WIN64)

    M128A XmmRegisters[16];
    UCHAR Reserved4[96];

#else

    M128A XmmRegisters[8];
    UCHAR Reserved4[224];

#endif

} XSAVE_FORMAT, *PXSAVE_FORMAT;'''

CTYPEDEF_XSAVE_FORMAT = '''\
    ctypedef struct XSAVE_FORMAT:
        USHORT ControlWord
        USHORT StatusWord
        UCHAR TagWord
        UCHAR Reserved1
        USHORT ErrorOpcode
        ULONG ErrorOffset
        USHORT ErrorSelector
        USHORT Reserved2
        ULONG DataOffset
        USHORT DataSelector
        USHORT Reserved3
        ULONG MxCsr
        ULONG MxCsr_Mask
        M128A FloatRegisters[8]
    IF UNAME_MACHINE[-2:] == 'x64':
        M128A XmmRegisters[16]
        UCHAR Reserved4[96]
    ELSE:
        M128A XmmRegisters[8]
        UCHAR Reserved4[224]
    ctypedef XSAVE_FORMAT *PXSAVE_FORMAT'''

WINDOWS_CreateThreadpoolIo = '''\
PTP_IO WINAPI CreateThreadpoolIo(
  _In_        HANDLE                fl,
  _In_        PTP_WIN32_IO_CALLBACK pfnio,
  _Inout_opt_ PVOID                 pv,
  _In_opt_    PTP_CALLBACK_ENVIRON  pcbe
);'''

CYTHON_CreateThreadpoolIo = '''\
    PTP_IO __stdcall CreateThreadpoolIo(
        HANDLE                fl,
        PTP_WIN32_IO_CALLBACK pfnio,
        PVOID                 pv,
        PTP_CALLBACK_ENVIRON  pcbe
    )

    ctypedef PTP_IO (__stdcall *LPFN_CreateThreadpoolIo)(
        HANDLE                fl,
        PTP_WIN32_IO_CALLBACK pfnio,
        PVOID                 pv,
        PTP_CALLBACK_ENVIRON  pcbe
    )'''

#===============================================================================
# Globals
#===============================================================================
DECLSPEC = re.compile('DECLSPEC_[^ ]+ ')
STDCALL = re.compile('(WINAPI|CALLBACK|NTAPI|PASCAL)')
IN_OUT = re.compile('('
    '_Reserved_|'
    '_Inout_opt_|'
    '_Out_opt_|'
    '_In_opt_|'
    '_Inout_|'
    '_Out_|'
    '_In_'
    ')'
)

#===============================================================================
# Helpers
#===============================================================================
def convert_windows_typedef_to_cython_ctypedef(text=None, indent_output=True):
    """
    >>> func = convert_windows_typedef_to_cython_ctypedef
    >>> text = TYPEDEF_RIO_NOTIFICATION_COMPLETION
    >>> func(text) == CTYPEDEF_RIO_NOTIFICATION_COMPLETION
    True
    >>> text = TYPEDEF_XSAVE_FORMAT
    >>> func(text) == CTYPEDEF_XSAVE_FORMAT
    True
    """
    from_clipboard = False

    if not text:
        text = cb()
        from_clipboard = True

    lines = text.splitlines()

    first_line = DECLSPEC.sub('', lines[0])

    first_line = (
        first_line.replace('typedef', 'ctypedef')
                  .replace('struct _', 'struct ')
                  .replace('union _', 'union ')
                  .replace('enum _', 'enum ')
                  .replace(' {', ':')
    )

    new_lines = [ first_line, ]
    saw_ifdef = False
    for line in lines[1:-1]:
        if 'union {' in line:
            continue
        if 'struct {' in line:
            continue
        if 'DUMMY' in line:
            continue
        if '}' in line:
            continue
        if line == '#if defined(_WIN64)':
            line = "IF UNAME_MACHINE[-2:] == 'x64':"
            saw_ifdef = True
            new_lines.append(line)
            continue
        elif line == '#else':
            if saw_ifdef:
                new_lines.append('ELSE:')
            continue
        elif line == '#endif':
            if saw_ifdef:
                saw_ifdef = False
                continue

        line = line.replace(';', '') \
                   .replace(',', '')
        line = DECLSPEC.sub('', line)
        # Strip off bit fields
        ix = line.find(':')
        if ix != -1:
            line = line[:ix]
        #ipdb.set_trace()
        line = line.strip()
        if not line:
            continue
        elif not line.startswith(('IF', 'ELSE', 'ELIF')):
            line = '    %s' % line
        new_lines.append(line)

    typename = first_line.split(' ')[2][:-1]
    last_line = lines[-1]
    ix = last_line.find(',')
    if ix != -1:
        for alias in last_line.split(', ')[1:]:
            line = 'ctypedef %s %s' % (typename, alias.replace(';', ''))
            new_lines.append(line)

    if indent_output:
        sep = '\n    '
        new_lines[0] = '    %s' % new_lines[0]
    else:
        sep = '\n'
    output = sep.join(new_lines)
    if from_clipboard:
        cb(output)
    return output

def convert_windows_typedef_to_python_ctypes_structure(
    text=None,
    indent_output=False
):
    """
    >>> func = convert_windows_typedef_to_cython_ctypedef
    >>> text = TYPEDEF_FILE_STANDARD_INFO
    >>> func(text) == PYTHON_CTYPES_FILE_STANDARD_INFO_FORMAT
    True
    """
    from_clipboard = False

    if not text:
        text = cb()
        from_clipboard = True

    lines = text.splitlines()

    first_line = DECLSPEC.sub('', lines[0])

    first_line = (
        first_line.replace('typedef', 'class')
                  .replace('struct _', '(Structure): ')
                  .replace('union _', '(Union): ')
                  .replace(' {', '')
    )

    tokens = first_line.split(' ')
    if len(tokens) == 3:
        (classtoken, typetoken, typename) = tokens
    else:
        raise NotImplementedError

    first_line = '%s %s%s' % (classtoken, typename, typetoken)
    new_lines = [
        first_line,
        '    _fields_ = [',
    ]
    saw_ifdef = False
    for line in lines[1:-1]:
        if 'union {' in line:
            continue
        if 'struct {' in line:
            continue
        if 'DUMMY' in line:
            continue
        if '}' in line:
            continue
        if line == '#if defined(_WIN64)':
            line = "IF UNAME_MACHINE[-2:] == 'x64':"
            saw_ifdef = True
            new_lines.append(line)
            continue
        elif line == '#else':
            if saw_ifdef:
                new_lines.append('ELSE:')
            continue
        elif line == '#endif':
            if saw_ifdef:
                saw_ifdef = False
                continue

        line = line.replace(';', '') \
                   .replace(',', '')
        line = DECLSPEC.sub('', line)
        # Strip off bit fields
        ix = line.find(':')
        if ix != -1:
            line = line[:ix]
        #ipdb.set_trace()
        line = line.strip()
        while line.find('  ') != -1:
            line = line.replace('  ', ' ')
        if not line:
            continue
        elif not line.startswith(('IF', 'ELSE', 'ELIF')):
            tokens = line.split(' ')
            line = "        ('%s', %s)," % (tokens[1], tokens[0])
        new_lines.append(line)

    new_lines.append('    ]')

    if indent_output:
        sep = '\n    '
        new_lines[0] = '    %s' % new_lines[0]
    else:
        sep = '\n'
    output = sep.join(new_lines)

    if from_clipboard:
        cb(output)
    return output

def convert_windows_funcdef_to_cython_funcdef(text=None, indent_output=True):
    """
    >>> func = convert_windows_funcdef_to_cython_funcdef
    >>> text = WINDOWS_CreateThreadpoolIo
    >>> func(text) == CYTHON_CreateThreadpoolIo
    True
    """
    from_clipboard = False

    if not text:
        text = cb()
        from_clipboard = True

    lines = text.splitlines()

    first_line = STDCALL.sub('__stdcall', lines[0])

    first_line = (
        first_line.replace('VOID', 'void')
    )

    new_lines = [ first_line, ]
    for line in lines[1:-1]:
        line = IN_OUT.sub('', line)
        line = line.strip()
        line = line.replace('//', '#')
        if not line:
            continue
        line = '    %s' % line
        new_lines.append(line)

    if indent_output:
        indent = '    '
        sep = '\n%s' % '    '
        new_lines[0] = '    %s' % new_lines[0]
    else:
        sep = '\n'
        indent = ''

    last_param_index = len(new_lines)-1

    output_lines = [
        sep.join(new_lines),
        '\n%s)\n' % indent,
    ]

    first_words = first_line.split(' ')
    if len(first_words) == 3:
        (return_type, calling_convention, funcname) = (
            first_words[0],
            first_words[1] + ' ',
            first_words[2].replace('(', '')
        )
    else:
        (return_type, calling_convention, funcname) = (
            first_words[0],
            '',
            first_words[1].replace('(', '')
        )

    lpfn_funcdef = '%sctypedef %s (%s*LPFN_%s)(' % (
        indent,
        return_type,
        calling_convention,
        funcname,
    )

    lpfn_params = sep.join(new_lines[1:last_param_index+1])
    lpfn_lines = [
        lpfn_funcdef,
        sep,
        lpfn_params,
        '\n',
        '%s)' % indent
    ]

    output = ''.join((
        ''.join(output_lines),
        ''.join(lpfn_lines),
    ))

    if from_clipboard:
        cb(output)
    return output

if __name__ == '__main__':
    import doctest
    doctest.testfile()

# vim:set ts=8 sw=4 sts=4 tw=80 expandtab nospell:                             #
