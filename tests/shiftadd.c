// SPDX-License-Identifier: MIT
/*
 * Multiplies by a constant the header should leave to the compiler's shifts
 * and adds. Only compiled, never linked: no mulu.w and no calls.
 */
#include "../nolibgcc.h"

typedef nolibgcc_u32 u32;

u32 mul32_15(u32 a) { return nolibgcc_mul32(a, 15); }
u32 mul32_60(u32 a) { return nolibgcc_mul32(a, 60); }
u32 mul32_1000(u32 a) { return nolibgcc_mul32(a, 1000); }
u32 mul32_0xffff(u32 a) { return nolibgcc_mul32(a, 0xffff); }
u32 mul32_0x7fffffff(u32 a) { return nolibgcc_mul32(0x7fffffff, a); }
u32 mul32_0x80000001(u32 a) { return nolibgcc_mul32(a, 0x80000001); }
