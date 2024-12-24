TARGET_CFLAGS += -ffreestanding -fshort-wchar -mno-red-zone
TARGET_LINKFLAGS += -T linker.ld -static -Bsymbolic -nostdlib
TARGET_ASMFLAGS += -f elf64

TARGET_CFLAGS += -I lib -I lib/boot -I src -I src/graphics -I src/memory -I src/arch/x86_64/ -I src/arch/x86_64/cpu \
				 -I src/drivers -I src/drivers/keyboard

C_SOURCES := $(shell find src -name "*.c")
C_OBJECTS := $(patsubst src/%.c, $(BUILD_DIR)/kernel/x86_64/%.o, $(C_SOURCES))

ASM_SOURCES := $(shell find src -name "*.asm")
ASM_OBJECTS := $(patsubst src/%.asm, $(BUILD_DIR)/kernel/x86_64/%_asm.o, $(ASM_SOURCES))

C_HEADERS := $(shell find src -name "*.h")
ASM_HEADERS := $(shell find src -name "*.inc")

.PHONY: all kernel
all: kernel

kernel: $(BUILD_DIR)/kernel/kernel.bin
$(BUILD_DIR)/kernel/kernel.bin: $(C_OBJECTS) $(ASM_OBJECTS)
	$(TARGET_LD) $(TARGET_LINKFLAGS) -o $@ $^ $(TARGET_LIBS)

$(BUILD_DIR)/kernel/x86_64/%.o: src/%.c $(C_HEADERS)
	@ mkdir -p $(dir $@)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<

$(BUILD_DIR)/kernel/x86_64/%_asm.o: src/%.asm $(ASM_HEADERS)
	@ mkdir -p $(dir $@)
	$(TARGET_ASM) $(TARGET_ASMFLAGS) -o $@ $<

