// SPDX-License-Identifier: MIT
#include "../nolibgcc.h"

nolibgcc_s64 __ashrdi3(nolibgcc_s64 a, int b)
{
	return nolibgcc__shr64(a, b, 1);
}
