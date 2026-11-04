// SPDX-License-Identifier: MIT
/*
 * Workloads for bench.py, built with the operators or with -DHDR the
 * header. argv[1] picks one, "none" is the baseline.
 */
#ifdef HDR
#include "../nolibgcc.h"
#define MUL64(a, b)	nolibgcc_mul64(a, b)
#define UDIV32(a, b)	nolibgcc_udiv32(a, b)
#define UMOD32(a, b)	nolibgcc_umod32(a, b)
#define UDIV64(a, b)	nolibgcc_udiv64(a, b)
#define UMOD64(a, b)	nolibgcc_umod64(a, b)
#else
#define MUL64(a, b)	((a) * (b))
#define UDIV32(a, b)	((a) / (b))
#define UMOD32(a, b)	((a) % (b))
#define UDIV64(a, b)	((a) / (b))
#define UMOD64(a, b)	((a) % (b))
#endif

typedef unsigned int u32;
typedef unsigned long long u64;

static volatile u32 sink;
static u64 state = 0x123456789abcdefULL;

static u64 rnd(void)
{
	u64 x = state;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;
	state = x;

	return x;
}

static char buf[32];

/* Printing numbers in decimal */
static void __attribute__((noinline)) dec32(u32 v)
{
	char *p = buf + sizeof(buf);

	do {
		*--p = '0' + UMOD32(v, 10);
		v = UDIV32(v, 10);
	} while (v);
	sink = *p;
}

static void __attribute__((noinline)) dec64(u64 v)
{
	char *p = buf + sizeof(buf);

	do {
		*--p = '0' + UMOD64(v, 10);
		v = UDIV64(v, 10);
	} while (v);
	sink = *p;
}

/* Nanoseconds to seconds and milliseconds, a divisor bigger than 16 bits */
static void __attribute__((noinline)) ns(u64 v)
{
	sink = UDIV64(v, 1000000000) + UDIV32((u32)UMOD64(v, 1000000000), 1000000);
}

/* Milliseconds to seconds and milliseconds, a 16 bit divisor */
static void __attribute__((noinline)) ms(u32 v)
{
	sink = UDIV32(v, 1000) + UMOD32(v, 1000);
}

/* Percentage used, dividing by a variable */
static void __attribute__((noinline)) percent(u32 used, u32 total)
{
	sink = UDIV32(used * 100, total);
}

/* Blocks to KiB */
static void __attribute__((noinline)) kib(u64 blocks)
{
	sink = UDIV64(MUL64(blocks, 4096ULL * 3), 1024);
}

static void w_none(u64 r) { (void)r; }
static void w_dec32(u64 r) { dec32(r); }
static void w_dec64(u64 r) { dec64(r); }
static void w_ns(u64 r) { ns(r >> 4); }
static void w_ms(u64 r) { ms(r); }
static void w_percent(u64 r) { percent((u32)r >> 8, ((u32)(r >> 32) >> 7) | 1); }
static void w_kib(u64 r) { kib(r >> 20); }

static const struct {
	const char *name;
	void (*fn)(u64 r);
} workloads[] = {
	{ "none", w_none },
	{ "dec32", w_dec32 },
	{ "dec64", w_dec64 },
	{ "ns", w_ns },
	{ "ms", w_ms },
	{ "percent", w_percent },
	{ "kib", w_kib },
};

static int streq(const char *a, const char *b)
{
	while (*a && *a == *b) {
		a++;
		b++;
	}
	return *a == *b;
}

int main(int argc, char **argv, char **envp)
{
	void (*fn)(u64 r) = w_none;

	(void)envp;

	/* Picked before the loop so the baseline only differs by the work */
	for (unsigned i = 0; argc > 1 && i < sizeof(workloads) / sizeof(workloads[0]); i++)
		if (streq(argv[1], workloads[i].name))
			fn = workloads[i].fn;

	for (int i = 0; i < N; i++)
		fn(rnd());

	return 0;
}
