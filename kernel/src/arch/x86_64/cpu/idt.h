#pragma once
#include <stdint.h>

void InitializeIDT();
void IDTSetGate(int interrupt, uint64_t handler, uint8_t flags);
