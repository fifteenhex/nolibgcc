// SPDX-License-Identifier: MIT
/*
 * Checks the helpers against the host doing the same operations natively.
 * Built with -DREF for the host, and for the target either calling the
 * library directly or with -DREF using the operators so the compiler calls
 * the library itself. The outputs must be identical. Pass -v to print every
 * result instead of a hash.
 */
#if defined(REF) && !defined(NOLIBC)
#include <stdio.h>
#include <string.h>
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
	 * __lshrdi3 calls on the target, a broken one shows up as every line
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
#if defined(REF)
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
#if defined(REF)
	q = a / b;
	r = a % b;
#else
	q = __nolibgcc_udivmoddi4(a, b, &r);
#endif
	result("udivmod64", a, b, q);
	result("udivmod64", a, b, r);
}

#define RUN(name) do { PAIRS(x_##name); report(#name); } while (0)

/*
 * INT_MIN / -1 doesn't fit and is undefined in C, so it's skipped above and
 * can't be compared with the host. Like libgcc the helpers give INT_MIN and
 * 0.
 */
static void intmin(void)
{
#if defined(REF) && !defined(NOLIBC)
	printf("intmin ok\n");
#else
	volatile si a32 = (si)0x80000000, b32 = -1;
	volatile di a64 = (di)0x8000000000000000ULL, b64 = -1;
	int ok = DIV32(a32, b32) == a32 && MOD32(a32, b32) == 0 &&
		 DIV64(a64, b64) == a64 && MOD64(a64, b64) == 0;

	printf("intmin %s\n", ok ? "ok" : "FAIL");
#endif
}

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
	intmin();

	return 0;
}
