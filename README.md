# Windows Module Exports Explorer
This program is designed to load a module and read through its exports table using the structures of the Portable Executable file format.
## Usage
To check if a specific API is in the module's export table:
### Input
```shell
exports.exe kernel32 VirtualAlloc
```
### Output
```shell
[+] VirtualAlloc was found in kernel32 at address 0x76A4F3C0
```

To print all APIs:
### Input
```shell
exports.exe ntdll --all
```
### Output
```shell=
[+] A_SHAFinal was found in ntdll at address 0x773D7B10
[+] A_SHAInit was found in ntdll at address 0x773F8B60
[+] A_SHAUpdate was found in ntdll at address 0x773D7BF0
[+] AlpcAdjustCompletionListConcurrencyCount was found in ntdll at address 0x7742EA90
[+] AlpcFreeCompletionListMessage was found in ntdll at address 0x7742EAC0
...
```

## How it Works
In order to understand how we can access the structures within the PE File Format Headers, we need to keep this picture in mind. The Portable Executable (PE) format is a file format for executables, object code, DLLs and others used in 32-bit and 64-bit versions of Windows operating systems.
![](https://i.imgur.com/vx3UlDm.png)
With the following line of code, we treat the handle to a module as a DOS Header structure. This header does not contain much useful information, but it remains there for compatibility reasons.
```c
PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)baseAddress;
```
To get to the newer PE Header structure, we need to add an offset to the DOS Header. This offset is **e_lfanew (0x3C)**. We use other different offsets from within the PE Header to reach the Optional Header, the Data Directories, and ultimately the **ExportTable**. All of this can be related to the picture above.
```c
PIMAGE_NT_HEADERS peHeader = (PIMAGE_NT_HEADERS)((PBYTE)baseAddress + dosHeader->e_lfanew);
PIMAGE_OPTIONAL_HEADER optionalHeader = (PIMAGE_OPTIONAL_HEADER)&peHeader->OptionalHeader;
PIMAGE_DATA_DIRECTORY dataDirectory = &(optionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]);
PIMAGE_EXPORT_DIRECTORY exportTable = (PIMAGE_EXPORT_DIRECTORY)((PBYTE)baseAddress + dataDirectory->VirtualAddress);
```

The ExportTable contains the amount of APIs in the module, as well as the Relative Virtual Addresses to three important structures. 
* **The Export Address Table**
    * Contains the address of the APIs
    * Must be indexed using an ordinal number from the Export Ordinal Table
* **The Export Name Pointer Table**
    * Contains the names of the APIs
* **The Export Ordinal Table**
    * Contains **16-bit** (thus the PUSHORT) indexes into the Export Address Table
```c=
DWORD dwNames = exportTable->NumberOfNames;
PULONG exportAddressTable = (PULONG)((PBYTE)baseAddress + exportTable->AddressOfFunctions);
PULONG exportNamesTable = (PULONG)((PBYTE)baseAddress + exportTable->AddressOfNames);
PUSHORT exportOrdinalTable = (PUSHORT)((PBYTE)baseAddress + exportTable->AddressOfNameOrdinals);
```
Once we have these three tables, we can iterate through them and explore the different APIs within the module and their respective addresses.

```c
for (int index = 0; index < dwNames; index++) {
    functionName = (PBYTE)((PBYTE)baseAddress + exportNamesTable[index]);
    printf("[+] %s was found in %s at address 0x%p\n", functionName, argv[1], (PBYTE)dosHeader + exportAddressTable[exportOrdinalTable[index]]);
}
```

# References
https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#export-directory-table
https://en.wikipedia.org/wiki/Portable_Executable
https://en.wikipedia.org/wiki/Portable_Executable#/media/File:Portable_Executable_32_bit_Structure_in_SVG_fixed.svg
https://dev.to/wireless90/exploring-the-export-table-windows-pe-internals-4l47
