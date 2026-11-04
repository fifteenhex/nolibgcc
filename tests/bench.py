#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""
Per workload: instructions, multiplies and divides per call, and the
function's size, for both builds.

bench.py <cross prefix> <qemu> <calls> <ops elf> <hdr elf>
"""
import os
import subprocess
import sys

prefix, qemu, calls = sys.argv[1], sys.argv[2], int(sys.argv[3])
elfs = {'ops': sys.argv[4], 'hdr': sys.argv[5]}
insncount = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'insncount.py')
workloads = ('dec32', 'dec64', 'ns', 'ms', 'percent', 'kib')


def count(v, w):
    out = subprocess.run([sys.executable, insncount, prefix + 'objdump', qemu, elfs[v], w],
                         capture_output=True, text=True, check=True).stdout
    return [int(x) for x in out.split()]


def sizes(v):
    s = {}
    out = subprocess.run([prefix + 'nm', '-S', elfs[v]], capture_output=True, text=True,
                         check=True).stdout
    for line in out.splitlines():
        p = line.split()
        if len(p) == 4:
            s[p[3]] = int(p[1], 16)
    return s


def function_size(v, name):
    """LTO may split a function into name.constprop.N etc, sum them"""
    found = [n for s, n in size[v].items() if s == name or s.startswith(name + '.')]
    if not found:
        sys.exit(f'{name} not found in {elfs[v]}')
    return sum(found)


base = {v: count(v, 'none') for v in elfs}
size = {v: sizes(v) for v in elfs}

print(f"{'':8} | {'operators':^24} | {'header':^24}")
print(f"{'workload':8} | {'insns':>7} {'mul':>5} {'div':>5} {'bytes':>5} |"
      f" {'insns':>7} {'mul':>5} {'div':>5} {'bytes':>5}")
for w in workloads:
    row = f'{w:8}'
    for v in elfs:
        c = [(a - b) / calls for a, b in zip(count(v, w), base[v])]
        row += f' | {c[0]:7.0f} {c[1]:5.1f} {c[2]:5.1f} {function_size(v, w):5}'
    print(row)
