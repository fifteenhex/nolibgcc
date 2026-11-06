/* SPDX-License-Identifier: MIT */
/*
 * Everything but the 68000 and 68010, included from nolibgcc.h: the
 * operators, left to the compiler and whatever library it calls.
 */

NOLIBGCC_INLINE nolibgcc_u32 nolibgcc_mul32(nolibgcc_u32 a, nolibgcc_u32 b)
{
	return a * b;
}

/* Division first so the compiler can make one divmod call of the two */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc_udivmod32(nolibgcc_u32 n, nolibgcc_u32 d,
						nolibgcc_u32 *rem)
{
	nolibgcc_u32 q = n / d;

	if (rem)
		*rem = n % d;
	return q;
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
	nolibgcc_u64 q = n / d;

	if (rem)
		*rem = n % d;
	return q;
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
