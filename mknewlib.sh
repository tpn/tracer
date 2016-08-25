#!/bin/sh

set -e

_src=TracerConfig
_src_uuid=76F2056F-6A65-4BA3-A49F-3CBF5D1E2CDF
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
        -e "s/Tracing.c/${_dst}.c/g"            \
        -e "s/Tracing.h/${_dst}.h/g"            \
        -e s/\"$_src\"/\"$_dst\"/g
}

_convert_vcxproj_filter() {
    sed -e "s/B1FAB504-C337-4440-B0C8-01B4B736D7CC/$(genuuid)/" \
        -e "s/A3DD3FD5-C7B0-44EA-BC78-54E1237732CF/$(genuuid)/" \
        -e "s/655DD96C-4829-417C-BEA6-44E9F631B067/$(genuuid)/" \
        -e "s/Tracing.c/${_dst}.c/g"                            \
        -e "s/Tracing.h/${_dst}.h/g"                            \
        -e "s/${_src}.h/${_dst}.h/g"                            \
        -e "s/${_src}.c/${_dst}.c/g"
}

if [ ! -f $1/$1.vcxproj ]; then
    cat Tracer/Tracer.vcxproj | _convert_vcxproj > $1/$1.vcxproj
else
    _new_proj_uuid=$(cat $1/$1.vcxproj | egrep "$_uuid_regex" | cut -f 2 -d '{' | cut -f 1 -d '}')
fi

if [ ! -f $1/$1.vcxproj.filters ]; then
    cat ${_src}/${_src}.vcxproj.filters | _convert_vcxproj_filter > $1/$1.vcxproj.filters
fi

for f in stdafx.h stdafx.c targetver.h; do
    if [ ! -f $1/$f ]; then
        cp ${_src}/$f $1
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
        -e "s/Tracer/$1/g"
echo EndProject

cat Tracer.sln | \
    egrep "$_src_uuid" | \
    sed -e "s/$_src_uuid/$_new_proj_uuid/g"

