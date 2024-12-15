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

int memcmp(const void* aptr, const void* bptr, size_t n){
	const unsigned char* a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++){
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	InitializeLib(ImageHandle, SystemTable);

	GOP_Framebuffer_t* newBuffer = InitializeGOP();

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

	unsigned int y = 50;
	unsigned int BPP = 4;
	for (unsigned int x = 0; x < newBuffer->width * BPP / 2; x += BPP)
	{
		*(unsigned int*)(x + (y * newBuffer->PixelsPerScanLine * BPP) + newBuffer->BaseAddress) = 0xFFFFFFFF;
	}

	// Calling kernel entry point
	int (*KernelStart)() = ((__attribute__((sysv_abi)) int (*)()) header.e_entry);

	Print(L"Return code: %d\r\n", KernelStart());

	return EFI_SUCCESS;
}
