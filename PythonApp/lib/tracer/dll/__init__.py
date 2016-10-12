
def bool_errcheck(result, func, args):
    if not result:
        raise RuntimeError("%s failed" % func.__name__)
    return args

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
