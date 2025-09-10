include export.mk
include prod_tool.in

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

TOOLCHAIN=GNUEABI

ifeq ($(SYSLIB),)
  SYSLIB=$(TOOLCHAIN_PATH)/gcc-linaro-arm-linux-gnueabi/sysroot
endif


#### BINARIES

CC := $(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-gcc --sysroot=$(SYSLIB)
CXX := $(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-g++ --sysroot=$(SYSLIB)
CPP := $(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-gcc -E --sysroot=$(SYSLIB)
AR := $(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-ar
AS := $(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-as
LD := $(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-ld --sysroot=$(SYSLIB)
NM := $(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-nm

OBJCOPY:=$(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-objcopy
OBJDUMP:=$(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-objdump
STRIP:=$(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-strip
RANLIB:=$(TOOLCHAIN_BIN)arm-none-linux-gnueabihf-ranlib

###FLAGS
CFLAGS=-fexpensive-optimizations -frename-registers -fomit-frame-pointer -ftree-vectorize -Wno-error=maybe-uninitialized -finline-functions -finline-limit=64  -fstack-protector-strong -pie -fpie -Wa,--noexecstack
CXXFLAGS=-fexpensive-optimizations -frename-registers -fomit-frame-pointer -ftree-vectorize -Wno-error=maybe-uninitialized -finline-functions -finline-limit=64  -fstack-protector-strong -pie -fpie -Wa,--noexecstack
CPPFLAGS=-std=gnu99

# c compiler flag
CFLAGS += -Wall -Wundef -Wstrict-prototypes -g -O0
          
# c++ compiler flag
CXXFLAGS += -Wall -Wundef -g -O0

CPPFLAGS += -I hdr -I $(SYSLIB)/usr/include/m2mb -DLE910CXL

LDFLAGS := --sysroot=$(SYSLIB)

####LIBRARIES

CFLAGS += -I $(SYSLIB)/usr/include/m2mb
CFLAGS += -O2
LDLIBS = -lazc20 -lm2mgcom -lm2mgpub

TARGET = usb_write.bin
OBJS = M2MB_main
SRCS = src/M2MB_main

all: $(TARGET)

$(TARGET): $(OBJS).o
	${CC} $(OBJS).o -o $(TARGET) $(LDLIBS)

M2MB_main.o: $(SRCS).c
	${CC} $(CFLAGS) -c $(SRCS).c -o $(OBJS).o


clean:
	rm -rf $(OBJS).o $(TARGET)

	
