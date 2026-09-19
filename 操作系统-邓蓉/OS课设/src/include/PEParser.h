#ifndef PE_PARSER_H
#define PE_PARSER_H

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

#include "INode.h"

struct ImageDosHeader 
{												  // DOS .EXE header
    unsigned short   e_magic;                     // Magic number
    unsigned short   e_cblp;                      // Bytes on last page of file
    unsigned short   e_cp;                        // Pages in file
    unsigned short   e_crlc;                      // Relocations
    unsigned short   e_cparhdr;                   // Size of header in paragraphs
    unsigned short   e_minalloc;                  // Minimum extra paragraphs needed
    unsigned short   e_maxalloc;                  // Maximum extra paragraphs needed
    unsigned short   e_ss;                        // Initial (relative) SS value
    unsigned short   e_sp;                        // Initial SP value
    unsigned short   e_csum;                      // Checksum
    unsigned short   e_ip;                        // Initial IP value
    unsigned short   e_cs;                        // Initial (relative) CS value
    unsigned short   e_lfarlc;                    // File address of relocation table
    unsigned short   e_ovno;                      // Overlay number
    unsigned short   e_res[4];                    // Reserved words
    unsigned short   e_oemid;                     // OEM identifier (for e_oeminfo)
    unsigned short   e_oeminfo;                   // OEM information; e_oemid specific
    unsigned short   e_res2[10];                  // Reserved words
    unsigned long    e_lfanew;                    // File address of new exe header
 };

struct ImageDataDirectory{
    unsigned long   VirtualAddress;
    unsigned long   Size;
};

struct ImageFileHeader
{
	unsigned short Machine;
    unsigned short NumberOfSections;
    unsigned long  TimeDateStamp;
    unsigned long  PointerToSymbolTable;
    unsigned long  NumberOfSymbols;
    unsigned short SizeOfOptionalHeader;
    unsigned short Characteristics;
};

struct ImageOptionalHeader32
{
	//
    // Standard fields.
    //

    unsigned short    Magic;
    unsigned char    MajorLinkerVersion;
    unsigned char    MinorLinkerVersion;
    unsigned long   SizeOfCode;
    unsigned long   SizeOfInitializedData;
    unsigned long   SizeOfUninitializedData;
    unsigned long   AddressOfEntryPoint;
    unsigned long   BaseOfCode;
    unsigned long   BaseOfData;

    //
    // NT additional fields.
    //

    unsigned long   ImageBase;
    unsigned long   SectionAlignment;
    unsigned long   FileAlignment;
    unsigned short    MajorOperatingSystemVersion;
    unsigned short    MinorOperatingSystemVersion;
    unsigned short    MajorImageVersion;
    unsigned short    MinorImageVersion;
    unsigned short    MajorSubsystemVersion;
    unsigned short    MinorSubsystemVersion;
    unsigned long   Win32VersionValue;
    unsigned long   SizeOfImage;
    unsigned long   SizeOfHeaders;
    unsigned long   CheckSum;
    unsigned short    Subsystem;
    unsigned short    DllCharacteristics;
    unsigned long   SizeOfStackReserve;
    unsigned long   SizeOfStackCommit;
    unsigned long   SizeOfHeapReserve;
    unsigned long   SizeOfHeapCommit;
    unsigned long   LoaderFlags;
    unsigned long   NumberOfRvaAndSizes;
    ImageDataDirectory DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};

struct ImageNTHeader 
{
    unsigned long Signature;        /* This is the Windows executable (NE) header */
    ImageFileHeader FileHeader;
    ImageOptionalHeader32 OptionalHeader;
};

struct ImageSectionHeader
{
	char    Name[8];
    union {
            unsigned long   PhysicalAddress;
            unsigned long   VirtualSize;
    } Misc;
    unsigned long   VirtualAddress;
    unsigned long   SizeOfRawData;
    unsigned long   PointerToRawData;
    unsigned long   PointerToRelocations;
    unsigned long   PointerToLinenumbers;
    unsigned short    NumberOfRelocations;
    unsigned short    NumberOfLinenumbers;
    unsigned long   Characteristics;
};


class PEParser
{
public:
	static const unsigned int TEXT_SECTION_IDX = 0;
	static const unsigned int DATA_SECTION_IDX = 1;
	static const unsigned int RDATA_SECTION_IDX = 2;
	static const unsigned int BSS_SECTION_IDX = 3;
	static const unsigned int IDATA_SECTION_IDX = 4;

    static const int ntHeader_size = 0xf8;
    static const int section_size = 0x28;

public:
    PEParser();
	PEParser(unsigned long peAddress);
	unsigned long Parse();
	/*
	 *@comment 将Parse后的exe定位到内存中正确的位置
	 *@Important 在Relocate之前需要首先调用Parse()以得到所需要的
	 * exe各个section的信息，同时需要首先map好页表，否则会失败
	 */
	unsigned int Relocate();
	unsigned int Relocate(Inode* p_inode, int sharedText);

    bool HeaderLoad(Inode* p_inode);

public:
	unsigned long EntryPointAddress;

	unsigned long TextAddress;
	unsigned long TextSize;
	
	unsigned long DataAddress;
	unsigned long DataSize;

	unsigned long StackSize;
	unsigned long HeapSize;

private:
	unsigned long peAddress;
	ImageNTHeader ntHeader;
 	ImageSectionHeader* sectionHeaders;
};

#endif
