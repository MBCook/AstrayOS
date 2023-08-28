C_FILES = $(wildcard *.c)
ASM_FILES = $(wildcard *.S)
O_FILES = $(ASM_FILES:.S=.o) $(C_FILES:.c=.o)
LLVM_PATH = /opt/homebrew/opt/llvm/bin
CLANG_FLAGS = -Wall -g -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

.PHONY: all clean lldb

all: clean kernel8.img

%.o: %.S
	$(LLVM_PATH)/clang --target=aarch64-elf $(CLANG_FLAGS) -c $< -o $@

%.o: %.c
	$(LLVM_PATH)/clang --target=aarch64-elf $(CLANG_FLAGS) -c $< -o $@

kernel8.img: $(O_FILES)
	$(LLVM_PATH)/ld.lld -m aarch64elf -nostdlib $(O_FILES) -T link.ld -o kernel8.elf
	$(LLVM_PATH)/llvm-objcopy -O binary kernel8.elf kernel8.img

clean:
	/bin/rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial null -serial stdio

debug: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -s -S -serial null -serial stdio
	
lldb: kernel8.elf
	$(LLVM_PATH)/lldb kernel8.elf
