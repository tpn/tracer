#include "hostname.h"

#ifdef COUGAR
static CONST CHAR PythonExePath[] = \
    "C:\\Users\\Trent\\Anaconda\\envs\\py2711\\python.exe";

static CONST CHAR PythonDllPath[] = \
    "S:\\Source\\cpython\\PCbuild\\amd64\\python27.dll";

static CONST CHAR PythonPrefix[] = \
    "C:\\Users\\Trent\\Anaconda\\envs\\py2711";

#elif defined FALCON

static CONST CHAR PythonExePath[] = \
    "C:\\Anaconda3\\envs\\py27\\python.exe";

static CONST CHAR PythonDllPath[] = \
    "S:\\cpython-2.7\\PCbuild\\amd64\\python27.dll";

static CONST CHAR PythonPrefix[] = \
    "C:\\Users\\Trent\\Anaconda\\envs\\py2711";

#else

static CONST CHAR PythonExePath[] = \
    "C:\\Users\\trent\\Anaconda2\\python.exe";

static CONST CHAR PythonDllPath[] = \
    "C:\\Users\\trent\\Anaconda2\\python27.dll";

static CONST CHAR PythonPrefix[] = \
    "C:\\Users\\trent\\Anaconda2";

#endif

