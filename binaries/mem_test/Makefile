GEM5_PATH = /homelocal/antoma19_local/u/cnm/gem5/gem5
ISA = x86
GXX = gcc


all: mem_test

mem_test: mem_test.c
	$(GXX) -o mem_test mem_test.c -static -I$(GEM5_PATH)/include -L$(GEM5_PATH)/util/m5/build/$(ISA)/out -lm5

clean:
	rm -f mem_test
