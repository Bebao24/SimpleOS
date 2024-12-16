#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stddef.h>

typedef struct
{
	void* BaseAddress;
	size_t BufferSize;
	unsigned int width;
	unsigned int height;
	unsigned int PixelsPerScanLine;
} GOP_Framebuffer_t;

GOP_Framebuffer_t fb;
GOP_Framebuffer_t* InitializeGOP()
{
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
	if (EFI_ERROR(status))
	{
		Print(L"Failed to locate GOP!\r\n");
		return NULL;
	}
	else
	{
		Print(L"GOP located!\r\n");
	}

	fb.BaseAddress = (void*)gop->Mode->FrameBufferBase;
	fb.BufferSize = gop->Mode->FrameBufferSize;
	fb.width = gop->Mode->Info->HorizontalResolution;
	fb.height = gop->Mode->Info->VerticalResolution;
	fb.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

	return &fb;
}

EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* FileName, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;
	
	EFI_FILE* LoadedFile;

	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

	if (Directory == NULL)
	{
		// Open the root directory
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	status = Directory->Open(Directory, &LoadedFile, FileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);

	if (status != EFI_SUCCESS)
	{
		return NULL;
	}

	return LoadedFile;
}

#define PSF1_MAGIC_0 0x36
#define PSF1_MAGIC_1 0x04

typedef struct
{
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charSize;
} PSF1_HEADER;

typedef struct
{
	PSF1_HEADER* fontHeader;
	void* glyphBuffer;
} PSF1_FONT;

PSF1_FONT* LoadFont(EFI_FILE* Directory, CHAR16* FileName, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_FILE* font = LoadFile(Directory, FileName, ImageHandle, SystemTable);
	if (font == NULL) return NULL;

	PSF1_HEADER* fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&fontHeader);
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, fontHeader); // Read the fontt header

	// Verify font
	if (fontHeader->magic[0] != PSF1_MAGIC_0 || fontHeader->magic[1] != PSF1_MAGIC_1)
	{
		return NULL;
	}

	UINTN glyphBufferSize = fontHeader->charSize * 256;
	if (fontHeader->mode == 1)
	{
		glyphBufferSize = fontHeader->charSize * 512;
	}

	// Read the glyph buffer
	void* glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);
	}

	PSF1_FONT* finishedFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finishedFont);
	finishedFont->fontHeader = fontHeader;
	finishedFont->glyphBuffer = glyphBuffer;
	return finishedFont;
}

int memcmp(const void* aptr, const void* bptr, size_t n){
	const unsigned char* a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++){
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

typedef struct
{
    GOP_Framebuffer_t* fb;
	PSF1_FONT* font;
	EFI_MEMORY_DESCRIPTOR* mMap;
	UINTN mMapSize;
	UINTN mDescriptorSize;
} BootInfo;

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	InitializeLib(ImageHandle, SystemTable);

	GOP_Framebuffer_t* newBuffer = InitializeGOP();
	if (newBuffer == NULL)
	{
		return EFI_NOT_FOUND;
	}

	PSF1_FONT* newFont = LoadFont(NULL, L"default.psf", ImageHandle, SystemTable);

	if (newFont == NULL)
	{
		Print(L"Failed to load PSF font!\r\n");
		return EFI_INVALID_PARAMETER;
	}
	else
	{
		Print(L"PSF font loaded successfully! char size = %d\r\n", newFont->fontHeader->charSize);
	}

	EFI_FILE* kernel = LoadFile(NULL, L"kernel.bin", ImageHandle, SystemTable);

	if (kernel == NULL)
	{
		Print(L"kernel.bin not found!\r\n");
		return EFI_INVALID_PARAMETER;
	}
	else
	{
		Print(L"kernel.bin found!\r\n");
	}

	// Getting file header
	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

		UINTN size = sizeof(header);
		kernel->Read(kernel, &size, &header);
	}

	// Check ELF header
	if (
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	)
	{
		Print(L"kernel.bin format isn't ELF64!\r\n");
		return EFI_INVALID_PARAMETER;
	}
	else
	{
		Print(L"kernel.bin format verified!\r\n");
	}

	Elf64_Phdr* phdrs;
	{
		kernel->SetPosition(kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		kernel->Read(kernel, &size, phdrs);
	}

	for (
		Elf64_Phdr* phdr = phdrs;
		(char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	)
	{
		switch (phdr->p_type)
		{
			case PT_LOAD:
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				kernel->SetPosition(kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				kernel->Read(kernel, &size, (void*)segment);
				break;
		}
	}

	Print(L"Kernel loaded!\r\n");

	EFI_MEMORY_DESCRIPTOR* map = NULL;
	UINTN MapSize, MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
	{
		SystemTable->BootServices->GetMemoryMap(&MapSize, map, &MapKey, &DescriptorSize, &DescriptorVersion);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void**)&map);
		SystemTable->BootServices->GetMemoryMap(&MapSize, map, &MapKey, &DescriptorSize, &DescriptorVersion);
	}

	// Calling kernel entry point
	void (*KernelStart)(BootInfo*) = ((__attribute__((sysv_abi)) void (*)(BootInfo*)) header.e_entry);

	BootInfo bootInfo;
	bootInfo.fb = newBuffer;
	bootInfo.font = newFont;
	bootInfo.mMap = map;
	bootInfo.mMapSize = MapSize;
	bootInfo.mDescriptorSize = DescriptorSize;

	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
	KernelStart(&bootInfo);

	return EFI_SUCCESS;
}
