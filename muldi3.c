// SPDX-License-Identifier: MIT
#include "nolibgcc.h"

nolibgcc_s64 __muldi3(nolibgcc_s64 a, nolibgcc_s64 b)
{
	return nolibgcc__mul64(a, b);
}
