/* SPDX-License-Identifier: MIT */
/*
 * 32 bit ARM: ARM state from ARMv5TE, or Thumb-2 (clz, ldrd). Included from
 * nolibgcc.h.
 */

#if !(defined(__thumb2__) || \
      (!defined(__thumb__) && defined(__ARM_FEATURE_CLZ) && defined(__ARM_FEATURE_DSP)))
#error "nolibgcc's ARM code needs ARMv5TE in ARM state, or Thumb-2"
#endif

/* What's in the library, the run time ABI names */
nolibgcc_u32 __aeabi_uidiv(nolibgcc_u32 n, nolibgcc_u32 d) NOLIBGCC_LIB;
nolibgcc_s32 __aeabi_idiv(nolibgcc_s32 n, nolibgcc_s32 d) NOLIBGCC_LIB;
/* These return the quotient in r0 and the remainder in r1 */
nolibgcc_u64 __aeabi_uidivmod(nolibgcc_u32 n, nolibgcc_u32 d) NOLIBGCC_LIB;
nolibgcc_u64 __aeabi_idivmod(nolibgcc_s32 n, nolibgcc_s32 d) NOLIBGCC_LIB;
/*
 * These return the quotient in r0-r1 and the remainder in r2-r3, not
 * callable from C
 */
void __aeabi_uldivmod(void) NOLIBGCC_LIB;
void __aeabi_ldivmod(void) NOLIBGCC_LIB;

/*
 * The building blocks: division is shift and subtract, starting where the
 * top bits of d and n line up.
 */

/* rem must not be NULL, here and below */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc__udivmod32(nolibgcc_u32 n, nolibgcc_u32 d,
						 nolibgcc_u32 *rem)
{
	nolibgcc_u32 q = 0;
	int s;

	/* A udf, where libgcc calls __aeabi_idiv0() */
	if (!d)
		__builtin_trap();

	if (n < d) {
		*rem = n;
		return 0;
	}

	s = __builtin_clz(d) - __builtin_clz(n);
	d <<= s;
	do {
		q <<= 1;
		if (n >= d) {
			n -= d;
			q |= 1;
		}
		d >>= 1;
	} while (s--);

	*rem = n;
	return q;
}

/* x must not be 0 */
NOLIBGCC_INLINE int nolibgcc__clz64(nolibgcc_u64 x)
{
	nolibgcc_u32 hi = x >> 32;

	return hi ? __builtin_clz(hi) : 32 + __builtin_clz((nolibgcc_u32)x);
}

NOLIBGCC_INLINE nolibgcc_u64 nolibgcc__udivmod64(nolibgcc_u64 n, nolibgcc_u64 d,
						 nolibgcc_u64 *rem)
{
	nolibgcc_u64 q = 0;
	int s;

	if (!d)
		__builtin_trap();

	if (!(n >> 32) && !(d >> 32)) {
		nolibgcc_u32 r32;

		q = nolibgcc__udivmod32(n, d, &r32);
		*rem = r32;
		return q;
	}

	if (n < d) {
		*rem = n;
		return 0;
	}

	s = nolibgcc__clz64(d) - nolibgcc__clz64(n);
	d <<= s;
	do {
		q <<= 1;
		if (n >= d) {
			n -= d;
			q |= 1;
		}
		d >>= 1;
	} while (s--);

	*rem = n;
	return q;
}

/* The API: just the operators, the compiler does what it can with a constant */

NOLIBGCC_INLINE nolibgcc_u32 nolibgcc_mul32(nolibgcc_u32 a, nolibgcc_u32 b)
{
	return a * b;
}

NOLIBGCC_INLINE nolibgcc_u32 nolibgcc_udivmod32(nolibgcc_u32 n, nolibgcc_u32 d,
						nolibgcc_u32 *rem)
{
	if (rem)
		*rem = n % d;
	return n / d;
}

NOLIBGCC_INLINE nolibgcc_u32 nolibgcc_udiv32(nolibgcc_u32 n, nolibgcc_u32 d)
{
	return n / d;
}

NOLIBGCC_INLINE nolibgcc_u32 nolibgcc_umod32(nolibgcc_u32 n, nolibgcc_u32 d)
{
	return n % d;
}

NOLIBGCC_INLINE nolibgcc_s32 nolibgcc_div32(nolibgcc_s32 n, nolibgcc_s32 d)
{
	return n / d;
}

NOLIBGCC_INLINE nolibgcc_s32 nolibgcc_mod32(nolibgcc_s32 n, nolibgcc_s32 d)
{
	return n % d;
}

NOLIBGCC_INLINE nolibgcc_u64 nolibgcc_mul64(nolibgcc_u64 a, nolibgcc_u64 b)
{
	return a * b;
}

NOLIBGCC_INLINE nolibgcc_u64 nolibgcc_udivmod64(nolibgcc_u64 n, nolibgcc_u64 d,
						nolibgcc_u64 *rem)
{
	if (rem)
		*rem = n % d;
	return n / d;
}

NOLIBGCC_INLINE nolibgcc_u64 nolibgcc_udiv64(nolibgcc_u64 n, nolibgcc_u64 d)
{
	return n / d;
}

NOLIBGCC_INLINE nolibgcc_u64 nolibgcc_umod64(nolibgcc_u64 n, nolibgcc_u64 d)
{
	return n % d;
}

NOLIBGCC_INLINE nolibgcc_s64 nolibgcc_div64(nolibgcc_s64 n, nolibgcc_s64 d)
{
	return n / d;
}

NOLIBGCC_INLINE nolibgcc_s64 nolibgcc_mod64(nolibgcc_s64 n, nolibgcc_s64 d)
{
	return n % d;
}

NOLIBGCC_INLINE nolibgcc_u64 nolibgcc_shl64(nolibgcc_u64 a, int b)
{
	return a << b;
}

NOLIBGCC_INLINE nolibgcc_u64 nolibgcc_lshr64(nolibgcc_u64 a, int b)
{
	return a >> b;
}

NOLIBGCC_INLINE nolibgcc_s64 nolibgcc_ashr64(nolibgcc_s64 a, int b)
{
	return a >> b;
}
