@echo off

set PogoSafeMode=1
set VCPROFILE_PATH=S:\Source\tracer\x64
if not defined VC14 (
    setx VC14 "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\BIN\amd64"
    set VC14="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\BIN\amd64"
)

rem
rem If pgomgr isn't on our path, we need to add the VC14 bin directory.
rem

setlocal enabledelayedexpansion

set _ADD_PATH=1

for /f "usebackq" %%i in (`pgomgr 2^>NUL`) do (
    set _ADD_PATH=0
)

endlocal & (
    if %_ADD_PATH% == 1 (
        echo Adding VC14 to PATH.
        set "PATH=%VC14%;%PATH%"
    )
)

rem vim:set ts=8 sw=4 sts=4 expandtab tw=80:
