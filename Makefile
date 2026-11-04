# SPDX-License-Identifier: MIT
#
# make CROSS_COMPILE=m68k-linux-
# make -f nolibgcc/Makefile CROSS_COMPILE=m68k-linux- O=build/nolibgcc

NOLIBGCC_MAKEFILE := $(lastword $(MAKEFILE_LIST))
D := $(patsubst ./%,%,$(dir $(NOLIBGCC_MAKEFILE)))
O ?= $(D)
override O := $(patsubst %//,%/,$(if $(O),$(O)/,))

ifeq ($(CROSS_COMPILE)$(filter clean,$(MAKECMDGOALS)),)
$(error CROSS_COMPILE not set)
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

NOLIBGCC_CPU ?= -m68000
# No LTO: calls to these only appear after LTO has run, too late to inline
# them or to pull in an LTO'd archive member.
# -mpcrel: position independent, usable from PIE programs, without the GOT
# -fpie would use.
NOLIBGCC_CFLAGS = $(NOLIBGCC_CPU) -mpcrel -Os -ffreestanding -std=gnu99 \
		  -Wall -Wextra -ggdb

NOLIBGCC_LIB = $(O)libnolibgcc.a

NOLIBGCC_OBJS = $(addprefix $(O),mulsi3.o)

.PHONY: all
all: $(NOLIBGCC_LIB)

$(O)%.o: $(D)%.c $(D)nolibgcc.h $(NOLIBGCC_MAKEFILE)
	@mkdir -p $(dir $@)
	$(CC) $(NOLIBGCC_CFLAGS) -c -o $@ $<

$(NOLIBGCC_LIB): $(NOLIBGCC_OBJS)
	rm -f $@
	$(AR) rcs $@ $^

.PHONY: disasm
disasm: $(NOLIBGCC_OBJS)
	$(OBJDUMP) -dr $^

.PHONY: size
size: $(NOLIBGCC_OBJS)
	$(SIZE) $^

.PHONY: clean
clean:
	rm -f $(O)*.o $(O)*.a
	$(if $(filter-out $(D),$(O)),rmdir $(O) 2>/dev/null || true)
