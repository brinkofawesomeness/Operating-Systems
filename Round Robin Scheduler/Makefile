CXX=/home/smarz1/Programs/riscv/bin/riscv64-g++
LDS=/home/smarz1/courses/cosc361/src/lds/virt.lds
CXXFLAGS=-L/home/smarz1/courses/cosc361 -Wall -Wextra -pedantic -Wextra -O0 -g -std=c++17
CXXFLAGS+=-static -ffreestanding -nostdlib -fno-rtti -fno-exceptions
CXXFLAGS+=-march=rv64gc -mabi=lp64d -I/home/smarz1/courses/cosc361/src/include
LDFLAGS=-T$(LDS)
SOURCES_CPP=$(wildcard *.cpp)
OBJS_CPP=$(patsubst %.cpp,%.o,$(SOURCES_CPP))
OUT=os.elf
OSLIB=/home/smarz1/courses/cosc361/stud/libos.a
QEMU=/home/smarz1/Programs/riscv/bin/qemu-system-riscv64
QARGS=-machine virt -smp 1
QARGS+=-m 128M 
QARGS+=-nographic -serial mon:stdio
QARGS+=-bios none
QARGS+=-kernel $(OUT)
XARGO=xargo
XARGO_LIB_DIR=src/rs/target/riscv64gc-unknown-none-elf/debug
XARGO_LIB=$(XARGO_LIB_DIR)/libos.a
RUSTC=rustc
RUSTC_FLAGS=--edition 2018 --crate-type staticlib --emit=obj -C opt-level=0 --target riscv64gc-unknown-none-elf
LIBS=-los -lgcc

all: $(OUT)

$(OUT): Makefile $(OBJS_CPP) $(OSLIB) $(LDS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(OUT) $(OBJS_CPP) $(LIBS)

%.o: %.cpp Makefile
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run hdd

clean:
	rm -f $(OUT) $(OBJS_CPP)

run: $(OUT)
	$(QEMU) $(QARGS)

rung: $(OUT)
	$(QEMU) -s -S $(QARGS)
