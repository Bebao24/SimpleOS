#include "system.h"

void panic()
{
    asm volatile("cli; hlt");
}


