#pragma once
#include <stdint.h>

typedef struct
{
    unsigned char Signature[8];
    uint8_t CheckSum;
    uint8_t OEMId[6];
    uint8_t Revision;
    uint32_t RSDTAddress;
    uint32_t Length;
    uint64_t XSDTAddress;
    uint8_t ExtendedCheckSum;
    uint8_t Reserved[3];
} __attribute__((packed)) RSDP2;

typedef struct
{
    unsigned char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t CheckSum;
    uint8_t OEMId[6];
    uint8_t OEMTableId[8];
    uint32_t OEMRevision;
    uint32_t CreatorId;
    uint32_t CreatorRevision;
} __attribute__((packed)) SDTHeader;

typedef struct
{
    SDTHeader header;
    uint64_t Reserved;
} __attribute__((packed)) MCFGHeader;

typedef struct
{
    uint64_t PCIBaseAddress;
    uint16_t PCISegGroup;
    uint8_t StartBus;
    uint8_t EndBus;
    uint32_t Reserved;
} __attribute__((packed)) DeviceConfig;

void* FindTable(SDTHeader* sdtHeader, char* signature);