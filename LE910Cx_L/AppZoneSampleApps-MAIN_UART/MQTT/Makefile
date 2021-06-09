include export.mk
include prod_tool.in

uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))

TOOLCHAIN=GNUEABI

ifeq ($(SYSLIB),)
  SYSLIB=$(TOOLCHAIN_PATH)/gcc-linaro-arm-linux-gnueabi/sysroot
endif


#### BINARIES

CC := $(TOOLCHAIN_BIN)arm-linux-gnueabi-gcc  
CXX := $(TOOLCHAIN_BIN)arm-linux-gnueabi-g++  
CPP := $(TOOLCHAIN_BIN)arm-linux-gnueabi-gcc -E 
AR := $(TOOLCHAIN_BIN)arm-linux-gnueabi-gcc-ar
AS := $(TOOLCHAIN_BIN)arm-linux-gnueabi-gcc-as
LD := $(TOOLCHAIN_BIN)arm-linux-gnueabi-ld  --sysroot=$(SYSLIB)
NM := $(TOOLCHAIN_BIN)arm-linux-gnueabi-nm

OBJCOPY:=$(TOOLCHAIN_BIN)arm-linux-gnueabi-objcopy
OBJDUMP:=$(TOOLCHAIN_BIN)arm-linux-gnueabi-objdump
STRIP:=$(TOOLCHAIN_BIN)arm-linux-gnueabi-strip
RANLIB:=$(TOOLCHAIN_BIN)arm-linux-gnueabi-ranlib

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

LIBDIRS = \
 -L $(SYSLIB)/lib \
 -L $(SYSLIB)/usr/lib
 
AZ_M2MB_LIBS:= \
	-lazc20 \
	-ltm2mbcom \
	-lgm2mbpub \
	-ltm2mbpub

LIBS = $(AZ_M2MB_LIBS) -lc -lm



#### SOURCE FILES

SRCS = $(wildcard src/*.c)
SRCS += $(wildcard src/*.cpp)

-include Makefile.in


ifneq ($(TELITBIN),)
  m2mapzname = $(TELITBIN)
else
  m2mapzname = m2mapz
endif

bin  = $(m2mapzname).bin
OUT_DIR = obj
Q=@


SRCS += $(patsubst %.o,%.c, $(OBJS))

OBJECTS = $(SRCS:%=%.o)

OBJS_PREFIX = $(addprefix $(OUT_DIR)/, $(OBJECTS))

OBJ_DIRS := $(call uniq, $(dir $(OBJS_PREFIX)))

.PHONY: directories

directories: ${OBJ_DIRS} 

${OBJ_DIRS}:
	$(Q)mkdir -p $@ 

all : directories $(bin)

# executable binary
$(bin) : $(OBJECTS)
	$(Q)echo ==========================================
	$(Q)echo   LINK objects and libraries
	$(Q)echo ==========================================
	$(CXX)  -o $(bin)  $(LDFLAGS) $(OBJS_PREFIX) $(LIBDIRS) $(LIBS)

# c source
%.c.o : %.c
# $(Q)echo ==========================================
# $(Q)echo   BUILD C source
# $(Q)echo ==========================================
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $(addprefix $(OUT_DIR)/, $@)

# c++ source
%.cpp.o : %.cpp
# $(Q)echo ==========================================
# $(Q)echo   BUILD C++ source
# $(Q)echo ==========================================
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $(addprefix $(OUT_DIR)/, $@)


clean:
	$(Q)rm -f $(bin) $(OBJECTS)
	$(Q)rm -rf $(OUT_DIR)

	
