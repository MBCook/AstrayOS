C_FILES = $(wildcard *.c)
ASM_FILES = $(wildcard *.asm)
O_FILES = $(ASM_FILES:.asm=.o) $(C_FILES:.c=.o)
LLVMPATH = /opt/homebrew/opt/llvm/bin
CLANGFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd

.PHONY: all clean

all: clean kernel8.img

%.o: %.asm
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

%.o: %.c
	$(LLVMPATH)/clang --target=aarch64-elf $(CLANGFLAGS) -c $< -o $@

kernel8.img: $(O_FILES)
	$(LLVMPATH)/ld.lld -m aarch64elf -nostdlib $(O_FILES) -T link.ld -o kernel8.elf
	$(LLVMPATH)/llvm-objcopy -O binary kernel8.elf kernel8.img

clean:
	/bin/rm kernel8.elf *.o *.img > /dev/null 2> /dev/null || true

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -d in_asm
