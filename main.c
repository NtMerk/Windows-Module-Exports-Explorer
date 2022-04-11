#include <stdio.h>
#include <Windows.h>

int main(int argc, char ** argv) {
    
    if (argc < 3) {
        printf("Usage:\n%s module function\n%s module --all", argv[0], argv[0]);
        return 1;
    }

    // Get a handle to the module we want to explore
    HMODULE baseAddress = GetModuleHandleA(argv[1]);

    if (baseAddress == 0) {
        printf("[!] Module %s not found.", argv[1]);
        return 1;
    }

    // Treat the handle as a DOS header
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;

    // Access: the PE header --> the Optional header --> the Data Directories --> the ExportTable
    PIMAGE_NT_HEADERS peHeader = (PIMAGE_NT_HEADERS)((PBYTE)baseAddress + dosHeader->e_lfanew);
    PIMAGE_OPTIONAL_HEADER optionalHeader = (PIMAGE_OPTIONAL_HEADER)&peHeader->OptionalHeader;
    PIMAGE_DATA_DIRECTORY dataDirectory = &(optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
    PIMAGE_EXPORT_DIRECTORY exportTable = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)baseAddress + dataDirectory->VirtualAddress);

    // Get the number of functions in the module
    DWORD dwNames = exportTable->NumberOfNames;

    // Declare the Export Address Table, Export Name Pointer Table and Export Ordinal Table
    PULONG exportAddressTable = (PULONG)((PBYTE)baseAddress + exportTable->AddressOfFunctions);
    PULONG exportNamesTable = (PULONG)((PBYTE)baseAddress + exportTable->AddressOfNames);
    PUSHORT exportOrdinalTable = (PUSHORT)((PBYTE)baseAddress + exportTable->AddressOfNameOrdinals);

    DWORD found = 0;
    PBYTE functionName = {0};
    // Iterate through the function names and addresses within the module
    for (int index = 0; index < dwNames; index++) {
        functionName = (PBYTE)((PBYTE)baseAddress + exportNamesTable[index]);
        if (strcmp(argv[2], functionName) == 0 || strcmp(argv[2], "--all") == 0) {
            printf("[+] %s was found in %s at address 0x%p\n", functionName, argv[1], (PBYTE)dosHeader + exportAddressTable[exportOrdinalTable[index]]);
            found = 1;
        }   
    }

    if (found == 0) {
        printf("[!] %s not found in %s", argv[2], argv[1]);
        return 1;
    }

    return 0;
}