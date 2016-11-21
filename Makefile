##
## This file is part of the libpayload project.
##
## Copyright (C) 2008 Advanced Micro Devices, Inc.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
## 1. Redistributions of source code must retain the above copyright
##    notice, this list of conditions and the following disclaimer.
## 2. Redistributions in binary form must reproduce the above copyright
##    notice, this list of conditions and the following disclaimer in the
##    documentation and/or other materials provided with the distribution.
## 3. The name of the author may not be used to endorse or promote products
##    derived from this software without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
## ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
## FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
## DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
## OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
## LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
## OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGE.
##

$(if $(wildcard .xcompile),,$(eval $(shell ../../../../util/xcompile/xcompile $(XGCCPATH) > .xcompile || rm -f .xcompile)))
.xcompile: ../../../../util/xcompile/xcompile

CONFIG_COMPILER_GCC := y
ARCH-y     := x86_32

include .xcompile

src := $(CURDIR)
srctree := $(src)
sortbootorder_obj := $(src)/build

LIBCONFIG_PATH := $(realpath ../../../libpayload)
LIBPAYLOAD_DIR := $(sortbootorder_obj)/libpayload
HAVE_LIBPAYLOAD := $(wildcard $(LIBPAYLOAD_DIR)/lib/libpayload.a)
LIB_CONFIG ?= configs/defconfig-tinycurses

# CFLAGS := -Wall -Werror -Os
CFLAGS := -Wall -g -Os
TARGET := sortbootorder
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))

ARCH-y     := x86_32

CC := $(CC_$(ARCH-y))
AS := $(AS_$(ARCH-y))
OBJCOPY := $(OBJCOPY_$(ARCH-y))

LPCC := CC="$(CC)" $(LIBPAYLOAD_DIR)/bin/lpgcc
LPAS := AS="$(AS)" $(LIBPAYLOAD_DIR)/bin/lpas

# Make is silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
endif

all: Makefile $(TARGET).elf

$(TARGET).elf: $(OBJS) libpayload
	$(Q)printf "  LPCC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(LPCC) -fno-builtin -o $@ $(OBJS)
	$(Q)$(OBJCOPY) --only-keep-debug $@ sortbootorder.debug
	$(Q)$(OBJCOPY) --strip-debug $@
	$(Q)$(OBJCOPY) --add-gnu-debuglink=sortbootorder.debug $@

%.o: %.c libpayload
	$(Q)printf "  LPCC      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(LPCC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.S.o: %.S libpayload
	$(Q)printf "  LPAS      $(subst $(shell pwd)/,,$(@))\n"
	$(Q)$(LPAS) $(ASFLAGS) --32 -o $@ $<

ifneq ($(strip $(HAVE_LIBPAYLOAD)),)
libpayload:
	$(Q)printf "Found Libpayload $(LIBPAYLOAD_DIR).\n"
else
libpayload:
	$(Q)printf "Building libpayload @ $(LIBCONFIG_PATH).\n"
	$(Q)make -C $(LIBCONFIG_PATH) distclean
	$(Q)make -C $(LIBCONFIG_PATH) defconfig KBUILD_DEFCONFIG=$(LIB_CONFIG)
	$(Q)make -C $(LIBCONFIG_PATH) DESTDIR=$(sortbootorder_obj) install
endif

clean:
	$(Q)rm -f $(TARGET).elf $(TARGET).debug *.o
	$(Q)rm .xcompile

distclean: clean
	$(Q)rm -rf $(sortbootorder_obj)


.PHONY: all clean distclean do-it-all depend with-depends without-depends debian postinst
