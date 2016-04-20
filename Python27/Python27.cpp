// Python27.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//typedef int (*PPY_MAIN)(_In_ int argc, _In_ wchar_t **argv);
typedef int (*PPY_MAIN)(_In_ int argc, _In_ char **argv);

int
main(int argc, char **argv)
{
    int retval;
    DWORD Flags;
    HMODULE Module;
    HMODULE Handle;
    PPY_MAIN PyMain;
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    PIMAGE_THUNK_DATA ThunkData;
    ULONG_PTR BaseAddress;
    ULONG_PTR ImportAddress;

    if (!(Module = LoadLibraryA("python27.dll"))) {
        return 1;
    };

    if (!(PyMain = (PPY_MAIN)GetProcAddress(Module, "Py_Main"))) {
        return 1;
    }

    Flags = (
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
    );

    if (!GetModuleHandleEx(Flags, (LPCTSTR)PyMain, &Handle)) {
        return 1;
    }

    DosHeader = (PIMAGE_DOS_HEADER)Handle;
    BaseAddress = (ULONG_PTR)DosHeader;

    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        return 1;
    }

    NtHeader = (PIMAGE_NT_HEADERS)(BaseAddress + DosHeader->e_lfanew);
    if (NtHeader->Signature != IMAGE_NT_SIGNATURE) {
        return 1;
    }

    OptionalHeader = (PIMAGE_OPTIONAL_HEADER)&NtHeader->OptionalHeader;

    ImportAddress = OptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

    ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(ImportAddress + BaseAddress);
    
    while (ImportDescriptor->Name) {
        PCHAR Name = (PCHAR)(ImportDescriptor->Name + BaseAddress);
        ThunkData = (PIMAGE_THUNK_DATA)(ImportDescriptor->FirstThunk + BaseAddress);

        ImportDescriptor++;
    }

    retval = PyMain(argc, argv);

    return retval;
}