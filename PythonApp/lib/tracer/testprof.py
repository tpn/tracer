#===============================================================================
# Imports
#===============================================================================
try:
    from tpn.cli import run
except ImportError:
    import sys
    from os.path import (
        join,
        abspath,
        dirname,
        normpath,
    )
    lib_dir = normpath(join(dirname(abspath(__file__)), '..'))
    sys.path.insert(0, lib_dir)
    from tpn.cli import run

#===============================================================================
# Main
#===============================================================================
def main():
    run('tpn tpn bt')

if __name__ == '__main__':
    main()

# vim:set ts=8 sw=4 sts=4 tw=78 et:
