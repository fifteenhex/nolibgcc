// SPDX-License-Identifier: MIT
#include "nolibgcc.h"

nolibgcc_s64 __ashldi3(nolibgcc_s64 a, int b)
{
	return nolibgcc__shl64(a, b);
}
