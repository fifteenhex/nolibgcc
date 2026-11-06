// SPDX-License-Identifier: MIT
#include "../nolibgcc.h"
#include "arch.h"

/* One copy of the division for all four */
static __attribute__((noinline))
nolibgcc_u32 udivmod(nolibgcc_u32 n, nolibgcc_u32 d, nolibgcc_u32 *rem)
{
	return nolibgcc__udivmod32(n, d, rem);
}

/* Quotient in r0 and remainder in r1 is a 64 bit return value */
NOLIBGCC_INLINE nolibgcc_u64 pair(nolibgcc_u32 q, nolibgcc_u32 r)
{
#ifdef __ARMEB__
	return (nolibgcc_u64)q << 32 | r;
#else
	return (nolibgcc_u64)r << 32 | q;
#endif
}

nolibgcc_u32 __aeabi_uidiv(nolibgcc_u32 n, nolibgcc_u32 d)
{
	nolibgcc_u32 r;

	return udivmod(n, d, &r);
}

nolibgcc_u64 __aeabi_uidivmod(nolibgcc_u32 n, nolibgcc_u32 d)
{
	nolibgcc_u32 r, q = udivmod(n, d, &r);

	return pair(q, r);
}

/* The quotient is negative if the signs differ */
nolibgcc_s32 __aeabi_idiv(nolibgcc_s32 n, nolibgcc_s32 d)
{
	nolibgcc_u32 r, q = udivmod(nolibgcc__abs32(n), nolibgcc__abs32(d), &r);

	return (n ^ d) < 0 ? -q : q;
}

/* The remainder has the sign of the dividend */
nolibgcc_u64 __aeabi_idivmod(nolibgcc_s32 n, nolibgcc_s32 d)
{
	nolibgcc_u32 r, q = udivmod(nolibgcc__abs32(n), nolibgcc__abs32(d), &r);

	return pair((n ^ d) < 0 ? -q : q, n < 0 ? -r : r);
}
