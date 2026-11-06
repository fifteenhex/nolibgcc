// SPDX-License-Identifier: MIT
#include "../nolibgcc.h"
#include "arch.h"

/*
 * used: nothing in C references divmod, and udivmod's name has to survive
 * for the asm. noinline: one copy of udivmod, not another inlined into divmod.
 */
static __attribute__((used, noinline))
nolibgcc_u64 udivmod(nolibgcc_u64 n, nolibgcc_u64 d, nolibgcc_u64 *rem)
{
	return nolibgcc__udivmod64(n, d, rem);
}

/*
 * As for 32 bits, the quotient is negative if the signs differ and the
 * remainder has the sign of n
 */
static __attribute__((used))
nolibgcc_s64 divmod(nolibgcc_s64 n, nolibgcc_s64 d, nolibgcc_s64 *rem)
{
	nolibgcc_u64 r, q = udivmod(nolibgcc__abs64(n), nolibgcc__abs64(d), &r);

	*rem = n < 0 ? -r : r;
	return (n ^ d) < 0 ? -q : q;
}

/*
 * n and d arrive in r0-r1 and r2-r3, as the C function wants them. Its third
 * argument, the remainder pointer, goes on the stack, pointing at space just
 * above it (16 keeps sp 8-aligned), and the remainder is loaded from there
 * into r2-r3.
 */
#define DIVMOD(name, fn)						\
	__attribute__((naked)) void name(void)				\
	{								\
		__asm__("push	{r4, lr}\n\t"				\
			"sub	sp, sp, #16\n\t"			\
			"add	r4, sp, #8\n\t"				\
			"str	r4, [sp]\n\t"				\
			"bl	" #fn "\n\t"				\
			"ldrd	r2, r3, [sp, #8]\n\t"			\
			"add	sp, sp, #16\n\t"			\
			"pop	{r4, pc}");				\
	}

DIVMOD(__aeabi_uldivmod, udivmod)
DIVMOD(__aeabi_ldivmod, divmod)
