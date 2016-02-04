#===============================================================================
# Imports
#===============================================================================

from .util import (
    ProcessWrapper,
)

#===============================================================================
# Classes
#===============================================================================
class SubversionClientException(Exception):
    pass

class SubversionClient(ProcessWrapper):
    # Purpose-built extension, intended for helping with testing.
    def __init__(self, exe, *args, **kwds):
        ProcessWrapper.__init__(self, exe, *args, **kwds)
        self.username = str()
        self.password = str()
        self.exception_class = SubversionClientException

    def build_command_line(self, exe, action, *args, **kwds):
        #import wingdbstub
        args = list(args)
        kwds = dict(kwds)

        is_ra = False
        if action == 'ci':
            is_ra = True
        elif action in ('cp', 'copy', 'mv', 'move', 'mkdir', 'rm', 'remove'):
            line = ' '.join(args)
            is_ra = (
                line.count('file://') or
                line.count('svn://')  or
                line.count('http://')
            )

        if is_ra:
            assert self.username and self.password
            kwds['username'] = self.username
            kwds['password'] = self.password
            kwds['no_auth_cache']   = True
            kwds['non_interactive'] = True

            if 'm' not in kwds:
                kwds['m'] = '""'

            if 'm' in kwds:
                m = kwds['m'] or '""'
                if not m.startswith('"'):
                    m = '"' + m
                if not m.endswith('"'):
                    m = m + '"'
                kwds['m'] = m

            if 'u' in kwds:
                kwds['username'] = kwds['u']
                del kwds['u']

        return ProcessWrapper.build_command_line(self, exe, action,
                                                 *args, **kwds)

#===============================================================================
# Instances
#===============================================================================
svn = SubversionClient('svn')
svnmucc = SubversionClient('svnmucc')
svnadmin = ProcessWrapper('svnadmin')
evnadmin = ProcessWrapper('evnadmin')

# vim:set ts=8 sw=4 sts=4 tw=78 et:
