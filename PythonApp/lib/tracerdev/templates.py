from textwrap import dedent
C_FILE = """\
/*++

Copyright (c) 2017 Trent Nelson <trent@trent.me>

Module Name:

    %(module_name)s

Abstract:%(abstract)s
--*/"""

SYMBOL_NAMES_HEADER_ABSTRACT = """

    Auto-generated file for the %(component)s component's symbol name constants.
"""

C_FILE_SOURCE_FILE = """\



"""

def make_symbol_name_header(component, module_name):
    template = C_FILE_HEADER
    abstract = SYMBOL_NAMES_HEADER_ABSTRACT % locals()
    text = template % locals()
    return text

