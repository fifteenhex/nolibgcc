# SPDX-License-Identifier: MIT
#
# make CROSS_COMPILE=m68k-linux-
# make -f nolibgcc/Makefile CROSS_COMPILE=m68k-linux- O=build/nolibgcc
# make CROSS_COMPILE=m68k-linux- NOLIBCDIR=... test

NOLIBGCC_MAKEFILE := $(lastword $(MAKEFILE_LIST))
# Makefile.<arch> is included before all: and has rules of its own
.DEFAULT_GOAL := all
D := $(patsubst ./%,%,$(dir $(NOLIBGCC_MAKEFILE)))
O ?= $(D)
override O := $(patsubst %//,%/,$(if $(O),$(O)/,))

ifeq ($(CROSS_COMPILE)$(filter clean,$(MAKECMDGOALS)),)
$(error CROSS_COMPILE not set)
endif

ifeq ($(NOLIBCDIR),)
ifneq ($(filter test bench %.elf,$(MAKECMDGOALS)),)
$(error NOLIBCDIR not set, needed for the tests)
endif
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
NM = $(CROSS_COMPILE)nm
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

# m68k-linux-gnu -> m68k
ifneq ($(CROSS_COMPILE),)
NOLIBGCC_ARCH := $(firstword $(subst -, ,$(shell $(CC) -dumpmachine)))
ifeq ($(wildcard $(D)Makefile.$(NOLIBGCC_ARCH)),)
$(error nolibgcc doesn't support '$(NOLIBGCC_ARCH)' (from $(CC) -dumpmachine))
endif
# Makefile.<arch> uses these in prerequisites, which make expands as it reads them
NOLIBGCC_MAKEFILES = $(NOLIBGCC_MAKEFILE) $(D)Makefile.$(NOLIBGCC_ARCH)
NOLIBGCC_HEADERS = $(D)nolibgcc.h $(D)$(NOLIBGCC_ARCH)/arch.h
NOLIBGCC_LIB = $(O)libnolibgcc.a
include $(D)Makefile.$(NOLIBGCC_ARCH)
endif

# No LTO: calls to these only appear after LTO has run, too late to inline
# them or to pull in an LTO'd archive member.
NOLIBGCC_CFLAGS = $(NOLIBGCC_CPU) $(NOLIBGCC_ARCH_CFLAGS) -Os -ffreestanding \
		  -std=gnu99 -Wall -Wextra -ggdb

NOLIBGCC_OBJS = $(addprefix $(O)$(NOLIBGCC_ARCH)/,$(NOLIBGCC_SRCS:.c=.o))

.PHONY: all
all: $(NOLIBGCC_LIB)

$(O)%.o: $(D)%.c $(NOLIBGCC_HEADERS) $(NOLIBGCC_MAKEFILES)
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

$(O)tests/test.ref: $(D)tests/test.c $(NOLIBGCC_MAKEFILE)
	@mkdir -p $(dir $@)
	$(HOSTCC) -O2 -Wall -Wextra -DREF -o $@ $<

TEST_CFLAGS = $(NOLIBGCC_CPU) -Os -static -nostdlib -fno-builtin -std=gnu99 \
	      -Wall -Wextra $(TEST_ARCH_CFLAGS) \
	      -include $(NOLIBCDIR)/nolibc.h $(addprefix -I,$(UAPIDIR))

# Target builds of tests/test.c, the ones in TESTS are run
TEST_ops = -DREF
TEST_hdr = -DHDR
TEST_hdr0 = -DHDR -O0

$(O)tests/test-%.elf: $(D)tests/test.c $(NOLIBGCC_HEADERS) $(NOLIBGCC_LIB) $(NOLIBGCC_MAKEFILES)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_$*) -o $@ $< $(NOLIBGCC_LIB)

# The library at -O0, where the compiler is least helpful
O0_OBJS = $(addprefix $(O)tests/O0-,$(notdir $(NOLIBGCC_OBJS)))

$(O)tests/O0-%.o: $(D)$(NOLIBGCC_ARCH)/%.c $(NOLIBGCC_HEADERS) $(NOLIBGCC_MAKEFILES)
	@mkdir -p $(dir $@)
	$(CC) $(NOLIBGCC_CFLAGS) -O0 -c -o $@ $<

# Nothing here may call out, or call a helper (a helper calling itself).
# nm -u can't see the latter, the callee is in the same object, hence the
# reloc grep.
NOCALLS = $(NOLIBGCC_OBJS) $(O0_OBJS) $(NOCALLS_ARCH)

.PHONY: test
test: $(O)tests/test.ref $(foreach t,$(TESTS),$(O)tests/test-$(t).elf) $(NOCALLS)
	@for o in $(NOCALLS); do \
		c=`$(NM) -u $$o; $(OBJDUMP) -r $$o | grep -oE '$(NOLIBGCC_HELPERS)$$'`; \
		test -z "$$c" || { echo "$$o calls:" $$c; exit 1; }; \
	done
	$(TEST_ARCH_CHECK)
	$(O)tests/test.ref > $(O)tests/test.ref.out
	$(foreach t,$(TESTS),$(QEMU) $(O)tests/test-$(t).elf > $(O)tests/test-$(t).out &&) true
	$(foreach t,$(TESTS),diff -u $(O)tests/test.ref.out $(O)tests/test-$(t).out &&) true
	@echo PASS

.PHONY: clean
clean:
	rm -f $(O)*.a $(O)*/*.o $(O)tests/test.ref $(O)tests/*.elf $(O)tests/*.out
	$(if $(filter-out $(D),$(O)),rmdir $(O)*/ $(O) 2>/dev/null || true)
