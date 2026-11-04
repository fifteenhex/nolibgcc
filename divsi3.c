// SPDX-License-Identifier: MIT
#include "nolibgcc.h"

nolibgcc_u32 __nolibgcc_udivmodsi4(nolibgcc_u32 n, nolibgcc_u32 d, nolibgcc_u32 *rem)
{
	return nolibgcc__udivmod32(n, d, rem);
}

nolibgcc_u32 __udivsi3(nolibgcc_u32 n, nolibgcc_u32 d)
{
	return nolibgcc_udiv32(n, d);
}

nolibgcc_u32 __umodsi3(nolibgcc_u32 n, nolibgcc_u32 d)
{
	return nolibgcc_umod32(n, d);
}

nolibgcc_s32 __divsi3(nolibgcc_s32 n, nolibgcc_s32 d)
{
	return nolibgcc_div32(n, d);
}

nolibgcc_s32 __modsi3(nolibgcc_s32 n, nolibgcc_s32 d)
{
	return nolibgcc_mod32(n, d);
}
