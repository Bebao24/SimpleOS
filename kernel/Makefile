TARGET_CFLAGS += -ffreestanding -fshort-wchar
TARGET_LINKFLAGS += -T linker.ld -static -Bsymbolic -nostdlib

TARGET_CFLAGS += -I lib -I lib/boot -I src -I src/graphics -I src/memory

C_SOURCES := $(shell find src -name "*.c")
C_OBJECTS := $(patsubst src/%.c, $(BUILD_DIR)/kernel/x86_64/%.o, $(C_SOURCES))

C_HEADERS := $(shell find src -name "*.h")

.PHONY: all kernel
all: kernel

kernel: $(BUILD_DIR)/kernel/kernel.bin
$(BUILD_DIR)/kernel/kernel.bin: $(C_OBJECTS)
	$(TARGET_LD) $(TARGET_LINKFLAGS) -o $@ $^ $(TARGET_LIBS)

$(BUILD_DIR)/kernel/x86_64/%.o: src/%.c $(C_HEADERS)
	@ mkdir -p $(dir $@)
	$(TARGET_CC) $(TARGET_CFLAGS) -c -o $@ $<