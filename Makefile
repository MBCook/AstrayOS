C_FILES = $(wildcard *.c)
ASM_FILES = $(wildcard *.S)
BUILD_DIR = build
O_FILES = $(ASM_FILES:%.S=$(BUILD_DIR)/%.o) $(C_FILES:%.c=$(BUILD_DIR)/%.o)
LLVM_PATH = /opt/homebrew/opt/llvm/bin
CLANG_FLAGS = -Wall -g -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

.PHONY: all clean lldb

all: clean $(BUILD_DIR)/kernel8.img

$(BUILD_DIR)/%.o: %.S
	$(LLVM_PATH)/clang --target=aarch64-elf $(CLANG_FLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	$(LLVM_PATH)/clang --target=aarch64-elf $(CLANG_FLAGS) -c $< -o $@

$(BUILD_DIR)/kernel8.img: $(O_FILES)
	$(LLVM_PATH)/ld.lld -m aarch64elf -nostdlib $(O_FILES) -T link.ld -o $(BUILD_DIR)/kernel8.elf
	$(LLVM_PATH)/llvm-objcopy -O binary $(BUILD_DIR)/kernel8.elf $(BUILD_DIR)/kernel8.img

clean:
	/bin/rm $(BUILD_DIR)/* > /dev/null 2> /dev/null || true

run: $(BUILD_DIR)/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $(BUILD_DIR)/kernel8.img -serial null -serial stdio

debug: $(BUILD_DIR)/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel $(BUILD_DIR)/kernel8.img -s -S -serial null -serial stdio
	
lldb: $(BUILD_DIR)/kernel8.elf
	$(LLVM_PATH)/lldb $(BUILD_DIR)/kernel8.elf
