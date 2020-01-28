##
## This file is part of the sortbootorder project.
##
## Copyright (C) 2008 Advanced Micro Devices, Inc.
## Copyright (C) 2016 PC Engines GmbH
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; version 2 of the License.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##

# compilation for coreboot mainline (4.5,4.6) or legacy (4.0)
COREBOOT_REL ?= mainline
VERSION ?= $(shell git describe --tags --dirty)

src := $(CURDIR)
# Assuming src path payloads/external/sortbootorder/sortbootorder/ by default
KDIR ?= $(src)/../../../../
srctree := $(src)
build_dir := $(src)/build

export V := $(V)

CONFIG_SHELL := sh
UNAME_RELEASE := $(shell uname -r)
MAKEFLAGS += -rR --no-print-directory

# Make is silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
.SILENT:
endif

HOSTCC ?= gcc
HOSTCXX ?= g++
HOSTCFLAGS := -I$(src)
HOSTCXXFLAGS := -I$(src)

LIBPAYLOAD_PATH := $(realpath $(KDIR)/payloads/libpayload)
LIBPAYLOAD_OBJ := $(build_dir)/libpayload
HAVE_LIBPAYLOAD := $(wildcard $(LIBPAYLOAD_OBJ)/lib/libpayload.a)
LIBPAYLOAD_CONFIG ?= $(src)/defconfig-apu2
OBJCOPY ?= objcopy

INCLUDES = -I$(src)/include -I$(KDIR)/src/commonlib/include
SRC_DIRS = spi utils
SRC_FILES = $(wildcard *.c)
SRC_FILES += $(wildcard spi/*.c)
SRC_FILES += $(wildcard utils/*.c)
OBJECTS = $(patsubst %.c,%.o,$(SRC_FILES))
OBJS    = $(patsubst %,$(build_dir)/%,$(OBJECTS))
DIRS    = $(patsubst %,$(build_dir)/%,$(SRC_DIRS))
TARGET  = sortbootorder.elf

all: real-all

# in addition to the dependency below, create the file if it doesn't exist
# to silence warnings about a file that would be generated anyway.
$(if $(wildcard .xcompile),,$(eval $(shell $(KDIR)/util/xcompile/xcompile $(XGCCPATH) > .xcompile || rm -f .xcompile)))
.xcompile: $(KDIR)/util/xcompile/xcompile
	$< $(XGCCPATH) > $@.tmp
	\mv -f $@.tmp $@ 2> /dev/null || rm -f $@.tmp $@

CONFIG_COMPILER_GCC := y
ARCH-y     := x86_32

include .xcompile

CC := $(CC_$(ARCH-y))
AS := $(AS_$(ARCH-y))
OBJCOPY := $(OBJCOPY_$(ARCH-y))

LPCC := CC="$(CC)" $(LIBPAYLOAD_OBJ)/bin/lpgcc
LPAS := AS="$(AS)" $(LIBPAYLOAD_OBJ)/bin/lpas

CFLAGS += -Wall -Werror -Os -fno-builtin $(CFLAGS_$(ARCH-y)) $(INCLUDES)
ifeq ($(COREBOOT_REL),legacy)
	CFLAGS += -DCOREBOOT_LEGACY
endif

ifeq ($(SPI_DEBUG),1)
	CFLAGS += -DSPI_DEBUG -DSPI_TRACE_ENABLED
endif

ifeq ($(APU1),y)
	CFLAGS += -DTARGET_APU1
else
	CFLAGS += -DFCH_YANGTZEE
endif

real-all: version $(TARGET)

version:
	sed -e "s/@version@/\"$(VERSION)\"/" version.h.in > version.h

$(TARGET): $(OBJS) libpayload $(DIRS)
	printf "    LPCC       $(subst $(CURDIR)/,,$(@)) (LINK)\n"
	$(LPCC) -o $@ $(OBJS)
	$(OBJCOPY) --only-keep-debug $@ $(TARGET).debug
	$(OBJCOPY) --strip-debug $@
	$(OBJCOPY) --add-gnu-debuglink=$(TARGET).debug $@

$(build_dir)/%.o: $(src)/%.c libpayload $(DIRS)
	printf "    LPCC       $(subst $(CURDIR)/,,$(@))\n"
	$(LPCC) $(CFLAGS) -c -o $@ $<

$(DIRS):
	mkdir -p $(DIRS)

defaultbuild:
	$(MAKE) all

ifneq ($(strip $(HAVE_LIBPAYLOAD)),)
libpayload:
	printf "Found Libpayload $(LIBPAYLOAD_OBJ).\n"
else
LPOPTS=obj="$(CURDIR)/lpbuild" DOTCONFIG="$(CURDIR)/lp.config"
libpayload:
	printf "Building libpayload @ $(LIBPAYLOAD_PATH).\n"
	$(MAKE) -C $(LIBPAYLOAD_PATH) $(LPOPTS) distclean coreinfo_obj=$(build_dir)/libptmp
	$(MAKE) -C $(LIBPAYLOAD_PATH) $(LPOPTS) defconfig KBUILD_DEFCONFIG=$(LIBPAYLOAD_CONFIG)
	$(MAKE) -C $(LIBPAYLOAD_PATH) $(LPOPTS) install DESTDIR=$(build_dir)
endif

clean:
	rm -rf *.elf *.elf.debug build/*.o .xcompile

distclean: clean
	rm -rf build lpbuild lp.config*

.PHONY: clean distclean

