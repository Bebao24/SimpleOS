#include "acpi.h"

void* FindTable(SDTHeader* sdtHeader, char* signature)
{
    int entries = (sdtHeader->Length - sizeof(SDTHeader)) / 8;
    for (int i = 0; i < entries; i++)
    {
        SDTHeader* newSDTHeader = (SDTHeader*)*(uint64_t*)((uint64_t)sdtHeader + sizeof(SDTHeader) + (i * 8));

        for (int j = 0; j < 4; j++)
        {
            if (newSDTHeader->Signature[j] != signature[j])
            {
                break;
            }

            if (j == 3)
            {
                return newSDTHeader;
            }
        }
    }

    return 0;
}

