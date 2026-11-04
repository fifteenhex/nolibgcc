// SPDX-License-Identifier: MIT
#include "nolibgcc.h"

nolibgcc_u64 __nolibgcc_udivmoddi4(nolibgcc_u64 n, nolibgcc_u64 d, nolibgcc_u64 *rem)
{
	return nolibgcc__udivmod64(n, d, rem);
}

nolibgcc_u64 __udivdi3(nolibgcc_u64 n, nolibgcc_u64 d)
{
	return __nolibgcc_udivmoddi4(n, d, 0);
}

nolibgcc_u64 __umoddi3(nolibgcc_u64 n, nolibgcc_u64 d)
{
	nolibgcc_u64 r;

	__nolibgcc_udivmoddi4(n, d, &r);
	return r;
}

/* The quotient is negative if the signs differ */
nolibgcc_s64 __divdi3(nolibgcc_s64 n, nolibgcc_s64 d)
{
	nolibgcc_u64 q = __nolibgcc_udivmoddi4(nolibgcc__abs64(n), nolibgcc__abs64(d), 0);

	return (n ^ d) < 0 ? -q : q;
}

/* The remainder has the sign of the dividend */
nolibgcc_s64 __moddi3(nolibgcc_s64 n, nolibgcc_s64 d)
{
	nolibgcc_u64 r;

	__nolibgcc_udivmoddi4(nolibgcc__abs64(n), nolibgcc__abs64(d), &r);
	return n < 0 ? -r : r;
}
