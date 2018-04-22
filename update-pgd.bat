@echo off

setlocal enableextensions

rem If input is Rtl!1.pgc, then:
rem 	i = Rtl
rem		j = 1
rem		k = pgc
rem
rem	We want to generate:
rem		pgomgr /merge x64\PGInstrument\Rtl!1.pgc x64\Rtl.pgd

echo on

@for /f "usebackq tokens=1,2,3 delims=!." %%i in (`dir /b x64\PGInstrument\*.pgc`) do (
    pgomgr /merge "x64\PGInstrument\%%i!%%j.%%k" x64\%%i.pgd
)
