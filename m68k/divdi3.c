// SPDX-License-Identifier: MIT
#include "../nolibgcc.h"

nolibgcc_u64 __nolibgcc_udivmoddi4(nolibgcc_u64 n, nolibgcc_u64 d, nolibgcc_u64 *rem)
{
	return nolibgcc__udivmod64(n, d, rem);
}

nolibgcc_u64 __udivdi3(nolibgcc_u64 n, nolibgcc_u64 d)
{
	return nolibgcc_udiv64(n, d);
}

nolibgcc_u64 __umoddi3(nolibgcc_u64 n, nolibgcc_u64 d)
{
	return nolibgcc_umod64(n, d);
}

nolibgcc_s64 __divdi3(nolibgcc_s64 n, nolibgcc_s64 d)
{
	return nolibgcc_div64(n, d);
}

nolibgcc_s64 __moddi3(nolibgcc_s64 n, nolibgcc_s64 d)
{
	return nolibgcc_mod64(n, d);
}
