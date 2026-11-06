// SPDX-License-Identifier: MIT
/*
 * Checks the helpers against the host doing the same operations natively.
 * Built with -DREF for the host, and for the target either calling the
 * library directly (68000 only), with -DHDR using the header's functions,
 * or with -DREF using the operators so the compiler calls the library
 * itself. -DHDR on the host checks the header's generic version. The
 * outputs must be identical. Pass -v to print every result instead of a
 * hash.
 *
 * The constant tests are there for the header: with a constant at the call
 * site it takes different paths.
 */
#ifndef NOLIBC
#include <stdio.h>
#include <string.h>
#endif
#ifdef HDR
#include "../nolibgcc.h"
#endif

typedef int si;
typedef unsigned int usi;
typedef long long di;
typedef unsigned long long udi;

#if defined(REF)
#define MUL32(a, b)	((usi)(a) * (usi)(b))
#define DIV32(a, b)	((si)(a) / (si)(b))
#define MOD32(a, b)	((si)(a) % (si)(b))
#define UDIV32(a, b)	((usi)(a) / (usi)(b))
#define UMOD32(a, b)	((usi)(a) % (usi)(b))
#define MUL64(a, b)	((udi)(a) * (udi)(b))
#define DIV64(a, b)	((di)(a) / (di)(b))
#define MOD64(a, b)	((di)(a) % (di)(b))
#define UDIV64(a, b)	((udi)(a) / (udi)(b))
#define UMOD64(a, b)	((udi)(a) % (udi)(b))
#define SHL64(a, b)	((udi)(a) << (b))
#define LSHR64(a, b)	((udi)(a) >> (b))
#define ASHR64(a, b)	((di)(a) >> (b))
#elif defined(HDR)
#define MUL32(a, b)	nolibgcc_mul32(a, b)
#define DIV32(a, b)	nolibgcc_div32(a, b)
#define MOD32(a, b)	nolibgcc_mod32(a, b)
#define UDIV32(a, b)	nolibgcc_udiv32(a, b)
#define UMOD32(a, b)	nolibgcc_umod32(a, b)
#define MUL64(a, b)	nolibgcc_mul64(a, b)
#define DIV64(a, b)	nolibgcc_div64(a, b)
#define MOD64(a, b)	nolibgcc_mod64(a, b)
#define UDIV64(a, b)	nolibgcc_udiv64(a, b)
#define UMOD64(a, b)	nolibgcc_umod64(a, b)
#define SHL64(a, b)	nolibgcc_shl64(a, b)
#define LSHR64(a, b)	nolibgcc_lshr64(a, b)
#define ASHR64(a, b)	nolibgcc_ashr64(a, b)
#else
usi __mulsi3(usi a, usi b);
si __divsi3(si a, si b);
si __modsi3(si a, si b);
usi __udivsi3(usi a, usi b);
usi __umodsi3(usi a, usi b);
di __muldi3(di a, di b);
di __divdi3(di a, di b);
di __moddi3(di a, di b);
udi __udivdi3(udi a, udi b);
udi __umoddi3(udi a, udi b);
di __ashldi3(di a, int b);
di __lshrdi3(di a, int b);
di __ashrdi3(di a, int b);
usi __nolibgcc_udivmodsi4(usi n, usi d, usi *rem);
udi __nolibgcc_udivmoddi4(udi n, udi d, udi *rem);
#define MUL32(a, b)	__mulsi3(a, b)
#define DIV32(a, b)	__divsi3(a, b)
#define MOD32(a, b)	__modsi3(a, b)
#define UDIV32(a, b)	__udivsi3(a, b)
#define UMOD32(a, b)	__umodsi3(a, b)
#define MUL64(a, b)	__muldi3(a, b)
#define DIV64(a, b)	__divdi3(a, b)
#define MOD64(a, b)	__moddi3(a, b)
#define UDIV64(a, b)	__udivdi3(a, b)
#define UMOD64(a, b)	__umoddi3(a, b)
#define SHL64(a, b)	__ashldi3(a, b)
#define LSHR64(a, b)	__lshrdi3(a, b)
#define ASHR64(a, b)	__ashrdi3(a, b)
#endif

static int verbose;
static udi hash, count;

static void result(const char *name, udi a, udi b, udi r)
{
	if (verbose)
		printf("%s %08x%08x %08x%08x = %08x%08x\n", name,
		       (usi)(a >> 32), (usi)a, (usi)(b >> 32), (usi)b,
		       (usi)(r >> 32), (usi)r);
	/*
	 * FNV-1a. The multiply here and the shift in rnd() are __muldi3 and
	 * __lshrdi3 calls on the 68000, a broken one shows up as every line
	 * failing.
	 */
	for (int i = 0; i < 8; i++) {
		hash ^= (r >> (i * 8)) & 0xff;
		hash *= 0x100000001b3ULL;
	}
	count++;
}

static void report(const char *name)
{
	printf("%-12s %8u %08x%08x\n", name, (usi)count, (usi)(hash >> 32), (usi)hash);
	hash = 0xcbf29ce484222325ULL;
	count = 0;
}

static udi state = 0x123456789abcdefULL;

/* xorshift64, shifted down by a random amount so small values get tested too */
static udi rnd(void)
{
	udi x = state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	state = x;

	return x >> (x & 63);
}

static const udi edges[] = {
	0, 1, 2, 3, 7, 10, 32, 33, 0x7f, 0x80, 0xff, 0x100, 0x7fff, 0x8000, 0xfffe,
	0xffff, 0x10000, 0x10001, 0x1fffe, 0x1ffff, 0x20000, 0x3ffff,
	0x12345678, 0x7ffffffe, 0x7fffffff, 0x80000000, 0x80000001, 0xfffeffff,
	0xffff0000, 0xfffffffe, 0xffffffff, 0x100000000ULL, 0x100000001ULL,
	0x1ffffffffULL, 0xffff00000000ULL, 0xffffffffffffULL,
	0x123456789abcdefULL, 0x7fffffffffffffffULL, 0x8000000000000000ULL,
	0x8000000000000001ULL, 0xfffffffeffffffffULL, 0xffffffff00000000ULL,
	0xfffffffffffffffeULL, 0xffffffffffffffffULL,
};

#define NEDGES (sizeof(edges) / sizeof(edges[0]))
#define NRANDOM 200000
#define NRANDOM_CONST 2000

/* Calls fn for each pair of edge values, their negatives, and random pairs */
#define PAIRS(fn)							\
	do {								\
		for (unsigned i = 0; i < NEDGES * 2; i++)		\
			for (unsigned j = 0; j < NEDGES * 2; j++) {	\
				udi a = edges[i / 2], b = edges[j / 2];	\
				if (i & 1)				\
					a = -a;				\
				if (j & 1)				\
					b = -b;				\
				fn(a, b);				\
			}						\
		for (unsigned i = 0; i < NRANDOM; i++) {		\
			udi a = rnd(), b = rnd();			\
			if (i & 1)					\
				a = -a;					\
			if (i & 2)					\
				b = -b;					\
			fn(a, b);					\
		}							\
	} while (0)

/* Skip divide by zero and INT_MIN / -1, they're undefined */
#define BAD32(type, a, b) \
	((b) == 0 || ((type)-1 < 0 && (a) == (type)0x80000000 && (b) == (type)-1))
#define BAD64(type, a, b) \
	((b) == 0 || ((type)-1 < 0 && (a) == (type)0x8000000000000000ULL && (b) == (type)-1))

#define T32(name, op, type, div)					\
	static void x_##name(udi a64, udi b64)				\
	{								\
		type a = a64, b = b64;					\
		if (div && BAD32(type, a, b))				\
			return;						\
		result(#name, (usi)a, (usi)b, (usi)op(a, b));		\
	}
#define T64(name, op, type, div)					\
	static void x_##name(udi a64, udi b64)				\
	{								\
		type a = a64, b = b64;					\
		if (div && BAD64(type, a, b))				\
			return;						\
		result(#name, a, b, op(a, b));				\
	}

T32(mul32, MUL32, usi, 0)
T32(div32, DIV32, si, 1)
T32(mod32, MOD32, si, 1)
T32(udiv32, UDIV32, usi, 1)
T32(umod32, UMOD32, usi, 1)
T64(mul64, MUL64, di, 0)
T64(div64, DIV64, di, 1)
T64(mod64, MOD64, di, 1)
T64(udiv64, UDIV64, udi, 1)
T64(umod64, UMOD64, udi, 1)

static void x_shl64(udi a, udi b) { result("shl64", a, b & 63, SHL64(a, b & 63)); }
static void x_lshr64(udi a, udi b) { result("lshr64", a, b & 63, LSHR64(a, b & 63)); }
static void x_ashr64(udi a, udi b) { result("ashr64", a, b & 63, ASHR64(a, b & 63)); }

/* Quotient and remainder from one call */
static void x_udivmod32(udi a64, udi b64)
{
	usi a = a64, b = b64, q, r;

	if (!b)
		return;
#if defined(HDR)
	q = nolibgcc_udivmod32(a, b, &r);
#elif defined(REF)
	q = a / b;
	r = a % b;
#else
	q = __nolibgcc_udivmodsi4(a, b, &r);
#endif
	result("udivmod32", a, b, q);
	result("udivmod32", a, b, r);
}

static void x_udivmod64(udi a, udi b)
{
	udi q, r;

	if (!b)
		return;
#if defined(HDR)
	q = nolibgcc_udivmod64(a, b, &r);
#elif defined(REF)
	q = a / b;
	r = a % b;
#else
	q = __nolibgcc_udivmoddi4(a, b, &r);
#endif
	result("udivmod64", a, b, q);
	result("udivmod64", a, b, r);
}

/*
 * Operands that aren't constant but are known to be small, the header has
 * paths for those too.
 */
static void x_narrow(udi a, udi b)
{
	unsigned short a16 = a, b16 = b;
	usi a32 = a, b32 = b;

	if (b16) {
		result("n_udiv32", a16, b16, UDIV32(a16, b16));
		result("n_umod32", a16, b16, UMOD32(a16, b16));
		result("n_udiv32", a32, b16, UDIV32(a32, b16));
		result("n_umod32", a32, b16, UMOD32(a32, b16));
		result("n_udiv64", a, b16, UDIV64(a, b16));
		result("n_umod64", a, b16, UMOD64(a, b16));
	}
	result("n_mul32", a32, b16, MUL32(a32, b16));
	if (b32) {
		result("n_udiv64", a32, b32, UDIV64(a32, b32));
		result("n_umod64", a32, b32, UMOD64(a32, b32));
	}
}

#define RUN(name) do { PAIRS(x_##name); report(#name); } while (0)

/*
 * INT_MIN / -1 doesn't fit and is undefined in C, so it's skipped above and
 * can't be compared with the host. Like libgcc the helpers give INT_MIN and
 * 0.
 */
static void intmin(void)
{
	/* Nor can the header where it's just the operators */
#if (defined(REF) && !defined(NOLIBC)) || defined(NOLIBGCC_GENERIC)
	printf("intmin ok\n");
#else
	/* volatile so the header can't see constants */
	volatile si a32 = (si)0x80000000, b32 = -1;
	volatile di a64 = (di)0x8000000000000000ULL, b64 = -1;
	int ok = DIV32(a32, b32) == a32 && MOD32(a32, b32) == 0 &&
		 DIV64(a64, b64) == a64 && MOD64(a64, b64) == 0;

	printf("intmin %s\n", ok ? "ok" : "FAIL");
#endif
}

/*
 * Constant operands: the operation with a literal as one side, applied to
 * the edge values and random values for the other side.
 */
#define CONST_VALUES(k)							\
	for (unsigned i = 0; i < NEDGES * 2 + NRANDOM_CONST; i++) {	\
		udi v = i < NEDGES * 2 ? edges[i / 2] : rnd();		\
		if (i & 1)						\
			v = -v;						\
		k							\
	}

/* type, op, constant on the right, i.e. v op k */
#define KR32(name, op, type, div, k)					\
	CONST_VALUES(							\
		type a = v;						\
		if (!(div && BAD32(type, a, (type)(k))))		\
			result(name, (usi)a, (usi)(type)(k), (usi)op(a, (type)(k)));)
#define KL32(name, op, type, div, k)					\
	CONST_VALUES(							\
		type b = v;						\
		if (!(div && BAD32(type, (type)(k), b)))		\
			result(name, (usi)(type)(k), (usi)b, (usi)op((type)(k), b));)
#define KR64(name, op, type, div, k)					\
	CONST_VALUES(							\
		type a = v;						\
		if (!(div && BAD64(type, a, (type)(k))))		\
			result(name, a, (type)(k), op(a, (type)(k)));)
#define KL64(name, op, type, div, k)					\
	CONST_VALUES(							\
		type b = v;						\
		if (!(div && BAD64(type, (type)(k), b)))		\
			result(name, (type)(k), b, op((type)(k), b));)

#define CONSTS32(X) X(1) X(2) X(3) X(7) X(10) X(16) X(100) X(1000) X(0x7fff) \
	X(0x8000) X(0xffff) X(0x10000) X(0x10001) X(1000000) X(0x7fffffff) \
	X(0x80000000) X(0xfffffffe) X(0xffffffff)
#define CONSTS64(X) CONSTS32(X) X(0x100000000ULL) X(1000000000000ULL) \
	X(0x7fffffffffffffffULL) X(0x8000000000000000ULL) X(0xffffffffffffffffULL)
#define SHIFTS(X) X(0) X(1) X(7) X(15) X(16) X(31) X(32) X(33) X(48) X(63)

#define K32(k)								\
	KR32("k_mul32", MUL32, usi, 0, k)				\
	KL32("k_mul32", MUL32, usi, 0, k)				\
	KR32("k_udiv32", UDIV32, usi, 1, k)				\
	KL32("k_udiv32", UDIV32, usi, 1, k)				\
	KR32("k_umod32", UMOD32, usi, 1, k)				\
	KL32("k_umod32", UMOD32, usi, 1, k)				\
	KR32("k_div32", DIV32, si, 1, k)				\
	KL32("k_div32", DIV32, si, 1, k)				\
	KR32("k_div32", DIV32, si, 1, (si)-(usi)(k))				\
	KL32("k_div32", DIV32, si, 1, (si)-(usi)(k))				\
	KR32("k_mod32", MOD32, si, 1, k)				\
	KL32("k_mod32", MOD32, si, 1, k)				\
	KR32("k_mod32", MOD32, si, 1, (si)-(usi)(k))
#define K64(k)								\
	KR64("k_mul64", MUL64, udi, 0, k)				\
	KL64("k_mul64", MUL64, udi, 0, k)				\
	KR64("k_udiv64", UDIV64, udi, 1, k)				\
	KL64("k_udiv64", UDIV64, udi, 1, k)				\
	KR64("k_umod64", UMOD64, udi, 1, k)				\
	KL64("k_umod64", UMOD64, udi, 1, k)				\
	KR64("k_div64", DIV64, di, 1, k)				\
	KL64("k_div64", DIV64, di, 1, k)				\
	KR64("k_div64", DIV64, di, 1, (di)-(udi)(k))				\
	KL64("k_div64", DIV64, di, 1, (di)-(udi)(k))				\
	KR64("k_mod64", MOD64, di, 1, k)				\
	KL64("k_mod64", MOD64, di, 1, k)				\
	KR64("k_mod64", MOD64, di, 1, (di)-(udi)(k))
#define KSHIFT(k)							\
	CONST_VALUES(							\
		result("k_shl64", v, k, SHL64(v, k));			\
		result("k_lshr64", v, k, LSHR64(v, k));			\
		result("k_ashr64", v, k, ASHR64(v, k));)

static void consts32(void) { CONSTS32(K32) }
static void consts64(void) { CONSTS64(K64) }
static void shifts(void) { SHIFTS(KSHIFT) }

int main(int argc, char **argv)
{
	verbose = argc > 1 && !strcmp(argv[1], "-v");
	hash = 0xcbf29ce484222325ULL;

	RUN(mul32);
	RUN(div32);
	RUN(mod32);
	RUN(udiv32);
	RUN(umod32);
	RUN(mul64);
	RUN(div64);
	RUN(mod64);
	RUN(udiv64);
	RUN(umod64);
	RUN(shl64);
	RUN(lshr64);
	RUN(ashr64);
	RUN(udivmod32);
	RUN(udivmod64);
	RUN(narrow);
	intmin();

	consts32();
	report("const32");
	consts64();
	report("const64");
	shifts();
	report("constshift");

	return 0;
}
