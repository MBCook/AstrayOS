C_FILES = $(wildcard *.c)
ASM_FILES = $(wildcard *.S)
O_FILES = $(ASM_FILES:%.S=build/%.o) $(C_FILES:%.c=build/%.o)
LLVM_PATH = /opt/homebrew/opt/llvm/bin
CLANG_FLAGS = -Wall -g -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

.PHONY: all clean lldb

all: clean build/kernel8.img

build/%.o: %.S
	$(LLVM_PATH)/clang --target=aarch64-elf $(CLANG_FLAGS) -c $< -o $@

build/%.o: %.c
	$(LLVM_PATH)/clang --target=aarch64-elf $(CLANG_FLAGS) -c $< -o $@

build/kernel8.img: $(O_FILES)
	$(LLVM_PATH)/ld.lld -m aarch64elf -nostdlib $(O_FILES) -T link.ld -o build/kernel8.elf
	$(LLVM_PATH)/llvm-objcopy -O binary build/kernel8.elf build/kernel8.img

clean:
	/bin/rm build/* > /dev/null 2> /dev/null || true

run: build/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel build/kernel8.img -serial null -serial stdio

debug: build/kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel build/kernel8.img -s -S -serial null -serial stdio
	
lldb: build/kernel8.elf
	$(LLVM_PATH)/lldb build/kernel8.elf
