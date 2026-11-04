# SPDX-License-Identifier: MIT
#
# make CROSS_COMPILE=m68k-linux-
# make -f nolibgcc/Makefile CROSS_COMPILE=m68k-linux- O=build/nolibgcc
# make CROSS_COMPILE=m68k-linux- NOLIBCDIR=... test

NOLIBGCC_MAKEFILE := $(lastword $(MAKEFILE_LIST))
D := $(patsubst ./%,%,$(dir $(NOLIBGCC_MAKEFILE)))
O ?= $(D)
override O := $(patsubst %//,%/,$(if $(O),$(O)/,))

ifeq ($(CROSS_COMPILE)$(filter clean,$(MAKECMDGOALS)),)
$(error CROSS_COMPILE not set)
endif

ifeq ($(NOLIBCDIR),)
ifneq ($(filter test %.elf,$(MAKECMDGOALS)),)
$(error NOLIBCDIR not set, needed for the tests)
endif
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
NM = $(CROSS_COMPILE)nm
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

NOLIBGCC_OBJS = $(addprefix $(O),mulsi3.o divsi3.o muldi3.o divdi3.o \
		ashldi3.o lshrdi3.o ashrdi3.o)

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

HOSTCC ?= gcc
QEMU ?= qemu-m68k -cpu m68000
UAPIDIR ?= /usr/m68k-linux-gnu/include

$(O)tests/test.ref: $(D)tests/test.c $(NOLIBGCC_MAKEFILE)
	@mkdir -p $(dir $@)
	$(HOSTCC) -O2 -Wall -Wextra -DREF -o $@ $<

TEST_CFLAGS = $(NOLIBGCC_CPU) -Os -static -nostdlib -fno-builtin -std=gnu99 \
	      -Wall -Wextra \
	      -include $(NOLIBCDIR)/nolibc.h $(addprefix -I,$(UAPIDIR))

# Target builds: library by name, operators
TESTS = lib ops
TEST_lib =
TEST_ops = -DREF

$(O)tests/test-%.elf: $(D)tests/test.c $(D)nolibgcc.h $(NOLIBGCC_LIB) $(NOLIBGCC_MAKEFILE)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_$*) -o $@ $< $(NOLIBGCC_LIB)

# The library at -O0, where the compiler is least helpful
$(O)tests/O0-%.o: $(D)%.c $(D)nolibgcc.h $(NOLIBGCC_MAKEFILE)
	@mkdir -p $(dir $@)
	$(CC) $(NOLIBGCC_CFLAGS) -O0 -c -o $@ $<

# Nothing here may call out, or call a libgcc helper (a helper calling
# itself). nm -u can't see the latter, the callee is in the same object,
# hence the reloc grep.
NOCALLS = $(NOLIBGCC_OBJS) $(addprefix $(O)tests/O0-,$(notdir $(NOLIBGCC_OBJS)))

.PHONY: test
test: $(O)tests/test.ref $(foreach t,$(TESTS),$(O)tests/test-$(t).elf) $(NOCALLS)
	@for o in $(NOCALLS); do \
		c=`$(NM) -u $$o; $(OBJDUMP) -r $$o | grep -oE '__[a-z]+[sd]i3$$'`; \
		test -z "$$c" || { echo "$$o calls:" $$c; exit 1; }; \
	done
	$(O)tests/test.ref > $(O)tests/test.ref.out
	$(foreach t,$(TESTS),$(QEMU) $(O)tests/test-$(t).elf > $(O)tests/test-$(t).out &&) true
	$(foreach t,$(TESTS),diff -u $(O)tests/test.ref.out $(O)tests/test-$(t).out &&) true
	@echo PASS

.PHONY: clean
clean:
	rm -f $(O)*.o $(O)*.a $(O)tests/*.o $(O)tests/test.ref $(O)tests/*.elf \
		$(O)tests/*.out
	$(if $(filter-out $(D),$(O)),rmdir $(O)tests $(O) 2>/dev/null || true)
