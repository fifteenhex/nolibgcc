# nolibgcc

Replacements for the libgcc helpers the compiler calls when the CPU can't do
an operation itself, for freestanding programs that don't want to link
against libgcc (for its licence, or because the toolchain's copy was built
for the wrong CPU).

It has the helpers that came up building
[smolutils](https://github.com/fifteenhex/smolutils) for two CPUs. The 68000
has no 32 bit multiply or divide:

| Function                                      | File           |
|-----------------------------------------------|----------------|
| `__mulsi3`                                    | m68k/mulsi3.c  |
| `__divsi3` `__modsi3` `__udivsi3` `__umodsi3` | m68k/divsi3.c  |
| `__muldi3`                                    | m68k/muldi3.c  |
| `__divdi3` `__moddi3` `__udivdi3` `__umoddi3` | m68k/divdi3.c  |
| `__ashldi3`                                   | m68k/ashldi3.c |
| `__lshrdi3`                                   | m68k/lshrdi3.c |
| `__ashrdi3`                                   | m68k/ashrdi3.c |

32 bit ARM without a divide instruction, like the Cortex-A9, only needs
division:

| Function                                                            | File        |
|---------------------------------------------------------------------|-------------|
| `__aeabi_uidiv` `__aeabi_uidivmod` `__aeabi_idiv` `__aeabi_idivmod` | arm/idiv.c  |
| `__aeabi_uldivmod` `__aeabi_ldivmod`                                | arm/ldiv.c  |

On ARM division by zero is a udf (SIGILL). libgcc's helpers call
`raise(SIGFPE)` instead, pulling `raise()` into every program that
divides.

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

On the 68000, after inlining they can see which operands are constant or
known to be small (`__builtin_constant_p`, at -O1 and up). A divisor known
to fit in 16 bits is two divu.w, four for a 64 bit dividend; two 16 bit
operands are one divu.w; a constant power of two is a shift. Anything else
calls the library, except `nolibgcc_mul32()`, which is small enough to
always inline; a constant the compiler can do in a few shifts and adds, and
any constant shift count, are left to the compiler. `nolibgcc_mul64()` is
just `*`.

ARM has a 32x32->64 multiply, so with GCC 16 the compiler does division by
a constant itself when it thinks that's worth it: at -O2 always for 32 bit
and for small constants for 64 bit (`/ 10` but not `/ 1000`); at -Os much
less (32 bit `/ 10` is inline, `/ 7` and `% 10` are calls, 64 bit always
is). The header's functions are just the operators.

On the 68000 `make bench` runs some typical uses written both ways in qemu
and counts the instructions per call (insns), and how many of them are
multiplies and divides, the slow instructions there:

```
         |        operators         |          header
workload |   insns   mul   div bytes |   insns   mul   div bytes
dec32    |     801   0.0  39.2    78 |     205   0.0  19.6    72
dec64    |    2723   0.0 116.3   100 |     707   0.0  77.5   116
ns       |     881  14.0   7.0    84 |     855  14.0   7.0    98
ms       |      79   0.0   4.0    50 |      17   0.0   2.0    44
percent  |      90   2.0   1.0    40 |      84   2.0   1.0    44
kib      |      49   0.0   0.0    76 |      49   0.0   0.0    76
```

dec32 and dec64 print numbers (`/ 10` and `% 10`), ns and ms convert
times (`/` and `% 1000000000` on 64 bits, `/ 1000` on 32 bits), percent
divides by a variable and kib multiplies by a constant.

## Building

    make CROSS_COMPILE=m68k-linux-
    make CROSS_COMPILE=arm-linux-gnueabihf-

gives `libnolibgcc.a`, put it after everything else on the link line. The
architecture comes from the compiler, what's specific to one is in
`Makefile.<arch>`. The Makefile can be used from another directory with
`-f` and the output put somewhere else with `O=`:

    make -f path/to/nolibgcc/Makefile CROSS_COMPILE=m68k-linux- O=build/nolibgcc

`NOLIBGCC_CPU` picks the CPU. On the 68000 that's `-m68000` by default, or
`-m68010`; ColdFire isn't supported. On ARM it's `-march=armv7-a
-mfpu=vfpv3-d16`; the code needs ARMv5TE in ARM state or Thumb-2, and the
`-mfpu` is only there for hard float toolchains.

## Testing

`make test` builds `tests/test.c` for the host and, with
[nolibc](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/tools/include/nolibc),
for the target: the operators against the library built at -Os and at -O0,
the header at -Os and at -O0, on the 68000 the library called by name, on
ARM the operators from ARM state (`-marm`). It runs them in qemu, diffs the
output against the host's, checks that the library calls nothing and that
division by zero traps.

    make CROSS_COMPILE=m68k-linux- NOLIBCDIR=path/to/nolibc test
    make CROSS_COMPILE=m68k-linux- NOLIBCDIR=path/to/nolibc bench
    make CROSS_COMPILE=arm-linux-gnueabihf- NOLIBCDIR=path/to/nolibc test

On the 68000 qemu is `-cpu m68000`, where a 68020 instruction traps;
`tests/dispatch.c` (cases the header must do inline) mustn't call anything
and `tests/shiftadd.c` (multiplies left to the compiler) mustn't have a
mulu.w. On ARM qemu is `-cpu cortex-a9`, where udiv traps. A big endian
library passes too, and a library built `-marm` called from Thumb; neither
is in `make test`:

    make CROSS_COMPILE=arm-linux-gnueabihf- NOLIBCDIR=... \
        NOLIBGCC_CPU="-march=armv7-a -mfpu=vfpv3-d16 -mbig-endian" \
        QEMU="qemu-armeb -cpu cortex-a9" test

`UAPIDIR` is where the Linux headers nolibc needs are,
`/usr/m68k-linux-gnu/include` or `/usr/arm-linux-gnueabihf/include` by
default.
