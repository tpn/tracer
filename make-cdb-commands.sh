#!/bin/sh

OUTPUT=cdb-commands.txt
CDB_OUTPUT=cdb-output.txt

(cat <<EOF
bp ModuleLoader!MaybeBreak
g
.outmask /l 0x0
.logopen $CDB_OUTPUT
EOF
) > $OUTPUT

find . -type f -iname "*.h" |
    xargs grep -ni '^typedef struct _.* {$' |
    sed -e 's/_Struct_size.* _/_/'      \
        -e 's/^../dt -v /'              \
        -e 's/\/.*typedef struct _/!_/' \
        -e 's/ {$//' >> $OUTPUT

(cat <<EOF
.logclose
q
EOF
) >> $OUTPUT

# vim:set ts=8 sw=4 sts=4 tw=80 et                                             :
