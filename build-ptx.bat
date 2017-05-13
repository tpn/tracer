rem @echo off

@set sources=TraceStoreKernels

@for %%s in (%sources%) do (
    nvcc -ptx Cu\%%s.cu -o x64\Debug\%%s.ptx    ^
        -Wno-deprecated-gpu-targets             ^
        --cudart=none                           ^
        --device-debug                          ^
        --generate-line-info                    ^
        --restrict

    nvcc -ptx Cu\%%s.cu -o x64\Release\%%s.ptx  ^
        -Wno-deprecated-gpu-targets             ^
        --optimize 3                            ^
        --generate-line-info                    ^
        --restrict
)

@rem vim:set ts=8 sw=4 sts=4 tw=80 expandtab                                   :
