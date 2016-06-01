// Python27.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

static CONST CHAR PythonExePath[] = "C:\\Users\\Trent\\Anaconda\\envs\\py2711\\python.exe";
static CONST CHAR PythonDllPath[] = "C:\\Users\\Trent\\Anaconda\\envs\\py2711\\python27.dll";
static CONST CHAR PythonPrefix[] = "C:\\Users\\Trent\\Anaconda\\envs\\py2711";

typedef int (*PPY_MAIN)(_In_ int argc, _In_ char **argv);
typedef PCHAR (*PPY_GET_PREFIX)(VOID);
typedef PCHAR (*PPY_GET_EXEC_PREFIX)(VOID);
typedef PCHAR (*PPY_GET_PROGRAM_FULL_PATH)(VOID);
typedef PCHAR (*PPY_GET_PROGRAM_NAME)(VOID);

typedef PPY_MAIN *PPPY_MAIN;
typedef PPY_GET_PREFIX *PPPY_GET_PREFIX;
typedef PPY_GET_EXEC_PREFIX *PPPY_GET_EXEC_PREFIX;
typedef PPY_GET_PROGRAM_NAME *PPPY_GET_PROGRAM_NAME;
typedef PPY_GET_PROGRAM_FULL_PATH *PPPY_GET_PROGRAM_FULL_PATH;

//typedef int (*PPY_MAIN)(_In_ int argc, _In_ wchar_t **argv);
typedef int (*PPY_MAIN)(_In_ int argc, _In_ char **argv);

typedef CHAR **PPSTR;

#define LOAD(Module, Name) do {                      \
    Module = LoadLibraryA(Name);                     \
    if (!Module) {                                   \
        OutputDebugStringA("Failed to load " #Name); \
        goto End;                                    \
    }                                                \
} while (0)

#define RESOLVE(Module, Type, Name) do {                             \
    Name = (Type)GetProcAddress(Module, #Name);                      \
    if (!Name) {                                                     \
        OutputDebugStringA("Failed to resolve " #Module " !" #Name); \
        goto End;                                                    \
    }                                                                \
} while (0)

typedef struct _TYPE_OFFSET {
    union {
        USHORT Value;
        struct {
            USHORT Offset:12;
            USHORT Type:4;
        };
    };
} TYPE_OFFSET, *PTYPE_OFFSET, **PPTYPE_OFFSET;

int
main(int argc, char **argv)
{
    int retval = 1;
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
    PIMAGE_DATA_DIRECTORY RelocationDirectory;
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

    PIMAGE_BASE_RELOCATION Relocation;

    ULONG_PTR BaseAddress;
    ULONG_PTR ImportAddress;
    ULONG_PTR ExportAddress;
    ULONG_PTR NewBaseAddress = 0x3e000000;

    LONG NumberOfRvaAndSizes;

    HMODULE Kernel32;
    HMODULE Shell32;
    HMODULE Python;
    HMODULE RtlModule;
    HMODULE HookModule;

    PPY_MAIN Py_Main;
    PPY_GET_PREFIX Py_GetPrefix;
    PPY_GET_EXEC_PREFIX Py_GetExecPrefix;
    PPY_GET_PROGRAM_FULL_PATH Py_GetProgramFullPath;

    PPYGC_COLLECT PyGC_Collect;
    ULONG_PTR PyGC_Collect_Offset;

    ULONG NumberOfRelocations = 0;
    PIMAGE_BASE_RELOCATION EndRelocation;

    LOAD(Python, PythonDllPath);
    RESOLVE(Python, PPY_MAIN, Py_Main);
    RESOLVE(Python, PPY_GET_PREFIX, Py_GetPrefix);
    RESOLVE(Python, PPY_GET_EXEC_PREFIX, Py_GetExecPrefix);
    RESOLVE(Python, PPY_GET_PROGRAM_FULL_PATH, Py_GetProgramFullPath);
    RESOLVE(Python, PPYGC_COLLECT, PyGC_Collect);

    Flags = (
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT
    );

    if (!GetModuleHandleEx(Flags, (LPCTSTR)Py_Main, &Handle)) {
        return 1;
    }

    DosHeader = (PIMAGE_DOS_HEADER)Handle;
    BaseAddress = (ULONG_PTR)DosHeader;


    PyGC_Collect_Offset = ((ULONG_PTR)PyGC_Collect - BaseAddress);

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

    NumberOfRvaAndSizes = OptionalHeader->NumberOfRvaAndSizes;
    DataDirectory = OptionalHeader->DataDirectory;
    Directory = &DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    ULONG_PTR RelocationsAddress = Directory->VirtualAddress;

    Relocation = (PIMAGE_BASE_RELOCATION)(
        RelocationsAddress +
        BaseAddress
    );

    EndRelocation = (PIMAGE_BASE_RELOCATION)(
        RelocationsAddress +
        BaseAddress +
        Directory->Size
    );
    EndRelocation++;

    while (Relocation != EndRelocation) {
        PUCHAR FixupVA;
        PUCHAR FixupBytes;
        ULONG_PTR NewVirtualAddress;
        ULONG_PTR FixupVirtualAddress;
        ULONG VirtualAddress = Relocation->VirtualAddress;
        ULONG SizeOfBlock = Relocation->SizeOfBlock;
        USHORT RemainingSizeOfBlock = SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION);
        USHORT NumberOfTypeOffsets = RemainingSizeOfBlock / sizeof(USHORT);
        USHORT Count = 0;
        PTYPE_OFFSET BaseTypeOffset = (PTYPE_OFFSET)(
            RtlOffsetToPointer(
                Relocation,
                sizeof(IMAGE_BASE_RELOCATION)
            )
        );
        PUSHORT TypeOffsetPointer = (PUSHORT)BaseTypeOffset;
        PTYPE_OFFSET TypeOffset;
        USHORT RelocationType;
        USHORT Offset;
        ULONG_PTR Target = (ULONG_PTR)0x000000001E03D7B4;
#define WithinTarget(Va) \
        (((((ULONG_PTR)(Va))-16) <= Target) && ((((ULONG_PTR)(Va))+16) >= Target))

        if (!RemainingSizeOfBlock) {
            continue;
        }

        FixupVirtualAddress = (ULONG_PTR)BaseAddress + VirtualAddress;
        //FixupBytes = (PUCHAR)(*((PUCHAR)FixupVirtualAddress));


        do {

            // Second relocation
            // Offset = 72 -> 48h
            // VirtualAddress = 0x1e163000
            // FixupVA = VirtualAddress + Offset -> 0x1e163048
            // Memory dump at address:
            //      0x000000001E163048  000000001e06a510
            // Dumpbin /relocations:
            //
            //          BASE RELOCATIONS #6
            //151000 RVA,        C SizeOfBlock
            //   998  DIR64      000000001E14FB3C
            //     0  ABS
            //163000 RVA,       1C SizeOfBlock
            //    48  DIR64      000000001E06A510

            TypeOffset = (PTYPE_OFFSET)TypeOffsetPointer++;
            RelocationType = TypeOffset->Value >> 12;
            Offset = TypeOffset->Value & 0xfff;
            FixupVA = (PUCHAR)(FixupVirtualAddress + Offset);

            if (FixupVA == (PUCHAR)0x000000001E03D7B4) {
                __debugbreak();
            } else if (FixupVirtualAddress == (ULONG_PTR)0x000000001E03D7B4) {
                __debugbreak();
            }

            if (WithinTarget(FixupVA)) {
                __debugbreak();
            }
            else if (WithinTarget(FixupVirtualAddress)) {
                __debugbreak();
            }

            switch (TypeOffset->Type) {
                case IMAGE_REL_BASED_ABSOLUTE:
                    NULL;
                    break;
                case IMAGE_REL_BASED_DIR64:

                    break;
            }

            RemainingSizeOfBlock -= sizeof(USHORT);

            NumberOfRelocations++;

        } while (RemainingSizeOfBlock);

        Relocation = (PIMAGE_BASE_RELOCATION)(
            RtlOffsetToPointer(
                Relocation,
                SizeOfBlock
            )
        );
    }

    Directory = &DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    ImportAddress = Directory->VirtualAddress;

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

        while (OriginalThunk->u1.Ordinal || PatchedThunk->u1.Ordinal) {

            Original = OriginalThunk->u1.Ordinal;
            if (!Original) {
                Original = PatchedThunk->u1.Ordinal;
            }
            if (IMAGE_SNAP_BY_ORDINAL(Original)) {
                Ordinal = IMAGE_ORDINAL(Original);
                ProcAddress = NULL;
                Name = NULL;
            } else {
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

    BaseExportFunctions = (PDWORD)(
        ExportDirectory->AddressOfFunctions +
        BaseAddress
    );

    BaseExportOrdinals = (PWORD)(
        ExportDirectory->AddressOfNameOrdinals +
        BaseAddress
    );

    BaseExportNames = (PDWORD)(
        ExportDirectory->AddressOfNames +
        BaseAddress
    );

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


    /*
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
    */
    /*
    for (Index = 0; Index < NumberOfExports; Index++) {

        PSTR Name = (PSTR)(BaseExportNames[Index]);
        PSTR Name2 = (PSTR)(BaseExportNames[Index] + BaseAddress);
        ULONG_PTR Function = (ULONG_PTR)(BaseExportFunctions[Index] + BaseAddress);
        //WORD Ordinal = (WORD)
    }
    */

    retval = Py_Main(argc, argv);
End:
    return retval;
}
