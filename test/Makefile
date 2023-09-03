LLVM_PATH = /usr/bin
CLANG_FLAGS = -Wall -g

.PHONY: all clean

all: clean memtest

memtest: memory_test.c
	$(LLVM_PATH)/clang $(CLANG_FLAGS) -o $@ $<

clean:
	/bin/rm memtest > /dev/null 2> /dev/null || true
