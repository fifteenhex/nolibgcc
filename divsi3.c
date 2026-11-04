// SPDX-License-Identifier: MIT
#include "nolibgcc.h"

nolibgcc_u32 __nolibgcc_udivmodsi4(nolibgcc_u32 n, nolibgcc_u32 d, nolibgcc_u32 *rem)
{
	return nolibgcc__udivmod32(n, d, rem);
}

nolibgcc_u32 __udivsi3(nolibgcc_u32 n, nolibgcc_u32 d)
{
	return __nolibgcc_udivmodsi4(n, d, 0);
}

nolibgcc_u32 __umodsi3(nolibgcc_u32 n, nolibgcc_u32 d)
{
	nolibgcc_u32 r;

	__nolibgcc_udivmodsi4(n, d, &r);
	return r;
}

/* The quotient is negative if the signs differ */
nolibgcc_s32 __divsi3(nolibgcc_s32 n, nolibgcc_s32 d)
{
	nolibgcc_u32 q = __nolibgcc_udivmodsi4(nolibgcc__abs32(n), nolibgcc__abs32(d), 0);

	return (n ^ d) < 0 ? -q : q;
}

/* The remainder has the sign of the dividend */
nolibgcc_s32 __modsi3(nolibgcc_s32 n, nolibgcc_s32 d)
{
	nolibgcc_u32 r;

	__nolibgcc_udivmodsi4(nolibgcc__abs32(n), nolibgcc__abs32(d), &r);
	return n < 0 ? -r : r;
}
