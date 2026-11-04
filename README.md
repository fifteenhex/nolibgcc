# nolibgcc

Replacements for the libgcc helpers the compiler calls when the CPU can't do
an operation itself, for freestanding programs that don't want to link
against libgcc (for its licence, or because the toolchain's copy was built
for the wrong CPU).

This targets the 68000, which has no 32 bit multiply or divide, and only
has the helpers that came up building
[smolutils](https://github.com/fifteenhex/smolutils):

| Function                                      | File      |
|-----------------------------------------------|-----------|
| `__mulsi3`                                    | mulsi3.c  |
| `__divsi3` `__modsi3` `__udivsi3` `__umodsi3` | divsi3.c  |
| `__muldi3`                                    | muldi3.c  |
| `__divdi3` `__moddi3` `__udivdi3` `__umoddi3` | divdi3.c  |
| `__ashldi3`                                   | ashldi3.c |
| `__lshrdi3`                                   | lshrdi3.c |
| `__ashrdi3`                                   | ashrdi3.c |

To find out what a program needs, link it without the library and look at
what's undefined:

    ... 2>&1 | grep -o "undefined reference to \`[^']*'" | sort -u

## The header

Because the compiler only calls the helpers once it has finished
optimising, `x / 10` costs a full division even though the divisor is
known. `nolibgcc.h` has the same operations as inline functions:

    #include "nolibgcc.h"

    q = nolibgcc_udiv32(x, 10);  /* instead of x / 10 */
    r = nolibgcc_umod64(ns, 1000000000);

| Function                                                   | Operator     |
|------------------------------------------------------------|--------------|
| `nolibgcc_mul32()` `nolibgcc_mul64()`                      | `*`          |
| `nolibgcc_udiv32()` `nolibgcc_div32()` and 64 bit          | `/`          |
| `nolibgcc_umod32()` `nolibgcc_mod32()` and 64 bit          | `%`          |
| `nolibgcc_udivmod32()` `nolibgcc_udivmod64()`              | both at once |
| `nolibgcc_shl64()` `nolibgcc_lshr64()` `nolibgcc_ashr64()` | `<<` `>>`    |

After inlining they can see which operands are constant or known to be
small (`__builtin_constant_p`, at -O1 and up). A divisor known to fit in
16 bits is two divu.w, four for a 64 bit dividend; two 16 bit operands are
one divu.w; a constant power of two is a shift. Anything else calls the
library, except `nolibgcc_mul32()`, which is small enough to always
inline; a constant the compiler can do in a few shifts and adds, and any
constant shift count, are left to the compiler. `nolibgcc_mul64()` is just
`*`.

## Building

    make CROSS_COMPILE=m68k-linux-

gives `libnolibgcc.a`, put it after everything else on the link line. The
Makefile can be used from another directory with `-f` and the output put
somewhere else with `O=`:

    make -f path/to/nolibgcc/Makefile CROSS_COMPILE=m68k-linux- O=build/nolibgcc

`NOLIBGCC_CPU` picks the CPU, `-m68000` by default, or `-m68010`. ColdFire
isn't supported.

## Testing

`make test` builds `tests/test.c` for the host and, with
[nolibc](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/include/nolibc),
four ways for the 68000, runs them in `qemu-m68k -cpu m68000`, where a
68020 instruction traps, and diffs the output. It also checks that the
library (at -Os and -O0) and `tests/dispatch.c` (the cases the header must
do inline) don't call anything, and that `tests/shiftadd.c` (multiplies
left to the compiler) has no mulu.w.

    make CROSS_COMPILE=m68k-linux- NOLIBCDIR=path/to/nolibc test

`UAPIDIR` is where the Linux headers nolibc needs are,
`/usr/m68k-linux-gnu/include` by default.
