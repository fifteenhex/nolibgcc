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
