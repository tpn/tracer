// Python27.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//typedef int (*PPY_MAIN)(_In_ int argc, _In_ wchar_t **argv);
typedef int (*PPY_MAIN)(_In_ int argc, _In_ char **argv);

int
main(int argc, char **argv)
{
    int retval;
    HMODULE Module;
    PPY_MAIN PyMain;

    if (!(Module = LoadLibraryA("python27.dll"))) {
        return 1;
    };

    if (!(PyMain = (PPY_MAIN)GetProcAddress(Module, "Py_Main"))) {
        return 1;
    }

    retval = PyMain(argc, argv);

    return retval;
}