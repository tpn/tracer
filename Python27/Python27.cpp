// Python27.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//typedef int (*PPY_MAIN)(_In_ int argc, _In_ wchar_t **argv);
typedef int (*PPY_MAIN)(_In_ int argc, _In_ char **argv);

typedef CHAR **PPSTR;

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
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    PIMAGE_DATA_DIRECTORY Directory;
    PIMAGE_DATA_DIRECTORY DataDirectory;
    PIMAGE_DATA_DIRECTORY ImportsDirectory;
    PIMAGE_DATA_DIRECTORY ExportsDirectory;
    PIMAGE_DATA_DIRECTORY DirectoryEntry;
    PIMAGE_DATA_DIRECTORY Imports;
    PIMAGE_DATA_DIRECTORY Exports;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_SYMBOL Symbol;
    PIMAGE_SYMBOL_EX SymbolEx;
    PIMAGE_SECTION_HEADER FirstSectionHeader;
    PIMAGE_SECTION_HEADER SectionHeader;
    PIMAGE_AUX_SYMBOL AuxSymbol;
    PIMAGE_AUX_SYMBOL_EX AuxSymbolEx;
    PIMAGE_ARCHIVE_MEMBER_HEADER ArchiveMemberHeader;
    PIMAGE_BOUND_IMPORT_DESCRIPTOR BoundImportDescriptor;
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    PIMAGE_LOAD_CONFIG_DIRECTORY LoadConfigDirectory;
    PIMAGE_RUNTIME_FUNCTION_ENTRY RuntimeFunctionEntry;
    PIMAGE_DEBUG_DIRECTORY DebugDirectory;
    PIMAGE_COFF_SYMBOLS_HEADER CoffSymbolsHeader;
    PFPO_DATA FpoData;
    PIMAGE_DEBUG_MISC DebugMisc;
    PIMAGE_IMPORT_BY_NAME ImportByName;
    PIMAGE_BASE_RELOCATION BaseRelocation;
    PIMAGE_LINENUMBER LineNumber;

    PIMAGE_THUNK_DATA ThunkData;
    PIMAGE_THUNK_DATA OriginalThunk;
    PIMAGE_THUNK_DATA PatchedThunk;

    ULONG Index;
    ULONG Count;
    ULONG NumberOfExports;
    PDWORD BaseExportNames;
    PWORD BaseExportOrdinals;
    PDWORD BaseExportFunctions;

    ULONG_PTR ExportNameAddress;
    PSTR ExportName;
    PSTR Name;
    PSTR *NamePointer;
    PWORD OrdinalPointer;
    PDWORD FunctionPointerRva;
    WORD Ordinal;
    ULONG_PTR Function;
    PIMAGE_IMPORT_BY_NAME ImportName;

    ULONG_PTR BaseAddress;
    ULONG_PTR ImportAddress;
    ULONG_PTR ExportAddress;

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

    FileHeader = (PIMAGE_FILE_HEADER)&NtHeader->FileHeader;
    OptionalHeader = (PIMAGE_OPTIONAL_HEADER)&NtHeader->OptionalHeader;
    FirstSectionHeader = IMAGE_FIRST_SECTION(NtHeader);

    DataDirectory = OptionalHeader->DataDirectory;
    Directory = &DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    ImportAddress = Directory->VirtualAddress;

    ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(ImportAddress + BaseAddress);

    while (ImportDescriptor->Name) {
        PCHAR DllName = (PCHAR)(ImportDescriptor->Name + BaseAddress);
        DWORD OriginalFirstThunk = ImportDescriptor->OriginalFirstThunk;
        DWORD FirstThunk = ImportDescriptor->FirstThunk;
        PROC Function;
        HMODULE Module;
        PROC ProcAddress;

        Module = LoadLibraryA(DllName);

        ULONG_PTR Original;
        ULONG_PTR Patched;

        OriginalThunk = (PIMAGE_THUNK_DATA)(OriginalFirstThunk + BaseAddress);
        PatchedThunk = (PIMAGE_THUNK_DATA)(FirstThunk + BaseAddress);

        while ((Original = OriginalThunk->u1.Ordinal) || PatchedThunk->u1.Ordinal) {

            if (!Original) {
                Original = PatchedThunk->u1.Ordinal;
            }
            if (IMAGE_SNAP_BY_ORDINAL(Original)) {
                Ordinal = IMAGE_ORDINAL(Original);
                ProcAddress = NULL;
                Name = NULL;
            }
            else {
                ImportByName = (PIMAGE_IMPORT_BY_NAME)(Original + BaseAddress);
                Name = (PCHAR)ImportByName->Name;
                ProcAddress = GetProcAddress(Module, Name);
                Ordinal = NULL;
            }

            Function = (PROC)(PatchedThunk->u1.Function);
            if (!Ordinal) {
                if (Function != ProcAddress) {
                    __debugbreak();
                }
            }

            OriginalThunk++;
            PatchedThunk++;
        }

        ImportDescriptor++;
    }

    Directory = &DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    ExportAddress = Directory->VirtualAddress;

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)(ExportAddress + BaseAddress);

    NumberOfExports = ExportDirectory->NumberOfFunctions;
    BaseExportFunctions = (PDWORD)(ExportDirectory->AddressOfFunctions + BaseAddress);
    BaseExportOrdinals = (PWORD)(ExportDirectory->AddressOfNameOrdinals + BaseAddress);
    BaseExportNames = (PDWORD)(ExportDirectory->AddressOfNames + BaseAddress);

    for (Index = 0; Index < NumberOfExports; Index++) {
        DWORD NameRva = BaseExportNames[Index];
        PCHAR Name = (PCHAR)(NameRva + BaseAddress);
        //PDWORD NameRva2 = NameRva + BaseAddress;

        //PSTR Name = (PSTR)(ExportDirectory->AddressOfNames[Index] + BaseAddress);

        //PSTR Name = (PSTR)(BaseExportNames[Index]);
        //PSTR Name2 = (PSTR)(BaseExportNames[Index] + BaseAddress);
        ULONG_PTR Function = (ULONG_PTR)(BaseExportFunctions[Index] + BaseAddress);
        //WORD Ordinal = (WORD)
    }

    //NamePointer = BaseExportNames-1;
    OrdinalPointer = BaseExportOrdinals-1;
    FunctionPointerRva = BaseExportFunctions-1;


    Count = NumberOfExports;
    Index = 0;
    do {
        ++Index;

        ++NamePointer;
        ++OrdinalPointer;
        ++FunctionPointerRva;

        Name = *NamePointer;
        Name = *NamePointer + BaseAddress;
        Function = (ULONG_PTR)(*FunctionPointerRva + BaseAddress);
        Ordinal = (WORD)(*OrdinalPointer + ExportDirectory->Base);

    } while (--Count);

    /*
    for (Index = 0; Index < NumberOfExports; Index++) {

        PSTR Name = (PSTR)(BaseExportNames[Index]);
        PSTR Name2 = (PSTR)(BaseExportNames[Index] + BaseAddress);
        ULONG_PTR Function = (ULONG_PTR)(BaseExportFunctions[Index] + BaseAddress);
        //WORD Ordinal = (WORD)
    }
    */

    retval = PyMain(argc, argv);

    return retval;
}