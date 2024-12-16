#pragma once
#include <stdint.h>

typedef struct
{
	uint32_t type;
	void* physicalAddr;
	void* virtualAddr;
	uint64_t numPages;
	uint64_t attributes;
} EFI_MEMORY_DESCRIPTOR;

