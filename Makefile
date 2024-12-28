include build/config.mk

.PHONY: all image kernel bootloader clean always run

all: image

image: $(BUILD_DIR)/image.hdd
$(BUILD_DIR)/image.hdd: bootloader kernel
	dd if=/dev/zero of=$@ bs=512 count=93750
	mkfs.fat -F 32 $@
	mmd -i $@ ::/EFI ::/EFI/BOOT
	mcopy -i $@ $(GNU_EFI)/x86_64/bootloader/main.efi ::/EFI/BOOT
	mcopy -i $@ $(BUILD_DIR)/kernel/kernel.bin ::
	mcopy -i $@ build/default.psf ::
	mcopy -i $@ build/startup.nsh ::

bootloader: $(GNU_EFI)/x86_64/bootloader/main.efi
$(GNU_EFI)/x86_64/bootloader/main.efi: always
	@ $(MAKE) -C $(GNU_EFI)
	@ $(MAKE) -C $(GNU_EFI) bootloader

kernel: $(BUILD_DIR)/kernel/kernel.bin
$(BUILD_DIR)/kernel/kernel.bin: always
	@ $(MAKE) -C kernel BUILD_DIR=$(abspath $(BUILD_DIR))

run:
	qemu-system-x86_64 -machine q35 \
    -drive file=$(BUILD_DIR)/image.hdd,format=raw -m 256M -cpu qemu64 \
    -drive if=pflash,format=raw,unit=0,file="OVMFbin/OVMF_CODE-pure-efi.fd",readonly=on \
    -drive if=pflash,format=raw,unit=1,file="OVMFbin/OVMF_VARS-pure-efi.fd" -net none

always:
	@ mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)