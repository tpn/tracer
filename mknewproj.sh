#!/bin/sh

set -e

_src=TraceStore
_src_uuid=C6FBB103-535C-4BCD-A463-B60C09481854
_uuid_regex="[0-9A-F]{8,8}-[0-9A-F]{4,4}-[0-9A-F]{4,4}-[0-9A-F]{4,4}-[0-9A-F]{12,12}"

if [ -z "$1" ]; then
    echo "error: missing project name"
    echo "  usage:  ./mknewproject.sh <Project>"
    echo "   e.g.:  ./mknewproject.sh PythonTracer"
    exit 1
fi

if [ ! -d "$1" ]; then
    mkdir $1
fi

_dst=$1

genuuid() {
    uuidgen | tr '[:lower:]' '[:upper:]'
}

_new_proj_uuid=$(genuuid)

_convert_vcxproj() {
    sed -e "s/$_src_uuid/$_new_proj_uuid/"      \
        -e "s/>$_src</>$_dst</g"                \
        -e "s/TraceStore.c/${_dst}.c/g"         \
        -e "s/TraceStore.h/${_dst}.h/g"         \
        -e s/\"$_src\"/\"$_dst\"/g
}

_convert_vcxproj_filter() {
    sed -e "s/6D7DB358-C3A0-4B8D-B406-D61EE6DF6AF4/$(genuuid)/" \
        -e "s/2042C3F2-BEAD-4F20-AFE4-28678670EAE1/$(genuuid)/" \
        -e "s/8C6EED73-60D9-4B6A-BBB5-28E00B82C084/$(genuuid)/" \
        -e "s/TraceStore.c/${_dst}.c/g"                         \
        -e "s/TraceStore./${_dst}.h/g"                          \
        -e "s/${_src}.h/${_dst}.h/g"                            \
        -e "s/${_src}.c/${_dst}.c/g"
}

if [ ! -f $1/$1.vcxproj ]; then
    cat TraceStore/TraceStore.vcxproj | _convert_vcxproj > $1/$1.vcxproj
else
    _new_proj_uuid=$(cat $1/$1.vcxproj | egrep "$_uuid_regex" | cut -f 2 -d '{' | cut -f 1 -d '}')
fi

if [ ! -f $1/$1.vcxproj.filters ]; then
    cat TraceStore/TraceStore.vcxproj.filters | _convert_vcxproj_filter > $1/$1.vcxproj.filters
fi

for f in stdafx.h stdafx.c dllmain.c targetver.h; do
    if [ ! -f $1/$f ]; then
        cp TraceStore/$f $1
    fi
done

if [ ! -f $1/${1}.c ]; then
    touch $1/${1}.c
fi

if [ ! -f $1/${1}.h ]; then
    touch $1/${1}.h
fi

#echo $_project_def | _convert_input
#echo $_project_config | _convert_input
cat Tracer.sln | \
    egrep "$_src_uuid" | \
    grep '^Project' | \
    sed -e "s/$_src_uuid/$_new_proj_uuid/g" \
        -e "s/TraceStore/$1/g"
echo EndProject

cat Tracer.sln | \
    egrep "$_src_uuid" | \
    sed -e "s/$_src_uuid/$_new_proj_uuid/g"

