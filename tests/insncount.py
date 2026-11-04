#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""
Counts the instructions a static m68k binary runs in qemu user mode, and
how many are multiplies and divides.

insncount.py <objdump> <qemu> <elf> [args...]
"""
import collections
import re
import subprocess
import sys
import tempfile

objdump, qemu, elf, args = sys.argv[1], sys.argv[2].split(), sys.argv[3], sys.argv[4:]

mnemonics = {}
for line in subprocess.run([objdump, '-d', elf], capture_output=True, text=True,
                           check=True).stdout.splitlines():
    m = re.match(r'\s*([0-9a-f]+):\s+(?:[0-9a-f]{4} ?)+\s+(\S+)', line)
    if m:
        mnemonics[int(m.group(1), 16)] = m.group(2)

with tempfile.NamedTemporaryFile(mode='r') as log:
    subprocess.run(qemu + ['-one-insn-per-tb', '-d', 'exec,nochain', '-D', log.name,
                           elf] + args, check=True, stdout=subprocess.DEVNULL)
    counts = collections.Counter()
    for line in log:
        m = re.search(r'\[[0-9a-f]+/([0-9a-f]+)/', line)
        if m:
            counts[mnemonics.get(int(m.group(1), 16), '?')] += 1

total = sum(counts.values())
mul = sum(n for k, n in counts.items() if k.startswith(('mulu', 'muls')))
div = sum(n for k, n in counts.items() if k.startswith(('divu', 'divs')))
print(f'{total} {mul} {div}')
