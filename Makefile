PS3DEV   ?= /usr/local/ps3dev
PSL1GHT  ?= $(PS3DEV)
PORTLIBS ?= $(PS3DEV)/portlibs/ppu
VER_NUM   := 0.2
TIMESTAMP := $(shell date +"%m%d-%H%M")
BUILD_ID  := v$(VER_NUM) ($(TIMESTAMP))
FILE_TAG  := v$(VER_NUM)_$(TIMESTAMP)
TITLE     := OpenLara PS3 Port $(BUILD_ID)
APPID     := OPENLARA0
CONTENTID := UP0001-$(APPID)_00-0000000000000000
APPVER    := 01.00
ICON0     := $(CURDIR)/pkg/ICON0.PNG
PKGFILES  := $(CURDIR)/pkg
OUT_DIR   := $(CURDIR)/out
TARGET    := $(OUT_DIR)/OpenLara-PS3_$(FILE_TAG)
CXX      := $(PS3DEV)/ppu/bin/ppu-g++
CC       := $(PS3DEV)/ppu/bin/ppu-gcc
CPP_SRCS := ps3/ps3_main.cpp src/libs/minimp3/minimp3.cpp
C_SRCS   := src/libs/stb_vorbis/stb_vorbis.c src/libs/tinf/tinflate.c
OBJS     := $(CPP_SRCS:.cpp=.o) $(C_SRCS:.c=.o)
INCLUDES := -Ips3 -Isrc -I$(PORTLIBS)/include -I$(PORTLIBS)/include/SDL2 -I$(PSL1GHT)/ppu/include -I$(PS3DEV)/ppu/include
DEFS     := -DPS3 -D__PS3__ -D_OS_PS3 -D_GAPI_SW -DNDEBUG -DPKG_BUILD_STAMP='"$(BUILD_ID)"'
CXXFLAGS := -O2 -mcpu=cell -std=c++11 -fno-exceptions -fno-rtti -fno-strict-aliasing -Wall -Wno-unused-variable -Wno-unused-function -Wno-sign-compare -Wno-format $(INCLUDES) $(DEFS)
CFLAGS   := -O2 -mcpu=cell -std=gnu11 -fno-strict-aliasing -Wall -Wno-unused-variable -Wno-unused-function -Wno-sign-compare $(INCLUDES) $(DEFS)
LDFLAGS  := -L$(PSL1GHT)/ppu/lib -L$(PORTLIBS)/lib -L$(PS3DEV)/ppu/lib
LIBS     := -lSDL2 -lgcm_sys -lio -laudio -lsysutil -lsysmodule -lrsx -lrt -llv2 -lm -lz -lc
include $(PSL1GHT)/ppu_rules
$(TARGET).elf: $(OBJS)
	@mkdir -p $(OUT_DIR)
	@$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
pkg:
	@rm -f ps3/ps3_main.o $(OUT_DIR)/*
	@$(MAKE) $(TARGET).pkg
	@rm -f $(TARGET).gnpdrm.pkg $(TARGET).elf $(TARGET).self $(TARGET).fake.self EBOOT.BIN
	@python3 -c 'import ftplib, os; f=ftplib.FTP(); f.connect("192.168.100.82", 21, timeout=3); f.login("anonymous","anonymous"); f.cwd("/dev_hdd0/packages"); fp=open("$(TARGET).pkg","rb"); f.storbinary("STOR "+os.path.basename("$(TARGET).pkg"), fp); fp.close(); f.quit(); print("[FTP] OK: PKG enviado")' || echo "[FTP] PS3 no responde, omitiendo subida."
.PHONY: pkg
