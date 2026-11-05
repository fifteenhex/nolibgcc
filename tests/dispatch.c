// SPDX-License-Identifier: MIT
/*
 * 68000 cases the header must do inline. Only compiled, never linked:
 * nothing in here may call anything.
 */
#include "../nolibgcc.h"

typedef unsigned short u16;
typedef nolibgcc_u32 u32;
typedef nolibgcc_s32 s32;
typedef nolibgcc_u64 u64;

u32 udiv32_k16(u32 n) { return nolibgcc_udiv32(n, 10); }
u32 umod32_k16(u32 n) { return nolibgcc_umod32(n, 1000); }
u32 udivmod32_k16(u32 n, u32 *r) { return nolibgcc_udivmod32(n, 0xffff, r); }
s32 div32_k16(s32 n) { return nolibgcc_div32(n, -10); }
s32 mod32_k16(s32 n) { return nolibgcc_mod32(n, 7); }
u32 udiv32_pow2(u32 n) { return nolibgcc_udiv32(n, 0x10000); }
u32 umod32_pow2(u32 n) { return nolibgcc_umod32(n, 0x80000000); }
u32 udiv32_16(u16 n, u16 d) { return nolibgcc_udiv32(n, d); }
u32 umod32_16(u16 n, u16 d) { return nolibgcc_umod32(n, d); }
s32 div32_16(short n, short d) { return nolibgcc_div32(n, d); }
u32 udiv32_k(void) { return nolibgcc_udiv32(123456789, 12345); }
u64 udiv64_k16(u64 n) { return nolibgcc_udiv64(n, 10); }
u64 umod64_k16(u64 n) { return nolibgcc_umod64(n, 1000); }
u64 udivmod64_k16(u64 n, u64 *r) { return nolibgcc_udivmod64(n, 1000, r); }
nolibgcc_s64 div64_k16(nolibgcc_s64 n) { return nolibgcc_div64(n, 10); }
nolibgcc_s64 mod64_k16(nolibgcc_s64 n) { return nolibgcc_mod64(n, -1000); }
u64 udiv64_32_k16(u32 n) { return nolibgcc_udiv64(n, 100000 / 10); }
u64 udiv64_pow2(u64 n) { return nolibgcc_udiv64(n, 1ULL << 40); }
u32 mul32(u32 a, u32 b) { return nolibgcc_mul32(a, b); }
u32 mul32_16(u32 a, u16 b) { return nolibgcc_mul32(a, b); }
u32 mul32_k(u32 a) { return nolibgcc_mul32(a, 10); }
u32 mul32_kdense(u32 a) { return nolibgcc_mul32(a, 1000000000); }
u32 mul32_kdense16(u32 a) { return nolibgcc_mul32(a, 0xabcd); }
u64 mul64_k(u64 a) { return nolibgcc_mul64(a, 4096); }
u64 shl64_k(u64 a) { return nolibgcc_shl64(a, 33); }
u64 lshr64_k(u64 a) { return nolibgcc_lshr64(a, 7); }
nolibgcc_s64 ashr64_k(nolibgcc_s64 a) { return nolibgcc_ashr64(a, 40); }
