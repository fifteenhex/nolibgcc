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
twice for the 68000, runs them in `qemu-m68k -cpu m68000`, where a 68020
instruction traps, and diffs the output. It also checks that the library
(at -Os and -O0) doesn't call anything.

    make CROSS_COMPILE=m68k-linux- NOLIBCDIR=path/to/nolibc test

`UAPIDIR` is where the Linux headers nolibc needs are,
`/usr/m68k-linux-gnu/include` by default.
