// SPDX-License-Identifier: MIT
/*
 * Divides by zero, argv[1] picks how, the Makefile runs each case. Each one
 * has to trap, so getting to the end is a failure.
 */
static volatile unsigned int zero32;
static volatile unsigned long long zero64;
static volatile unsigned int u32 = 7;
static volatile int s32 = -7;
static volatile unsigned long long u64 = 7, big64 = 1ULL << 40;
static volatile long long sbig64 = -(1LL << 40);

int main(int argc, char **argv)
{
	unsigned long long r;

	switch (argc > 1 ? argv[1][0] : 0) {
	case 'a': r = u32 / zero32; break;
	case 'b': r = u32 % zero32; break;
	case 'c': r = s32 / (int)zero32; break;
	case 'd': r = s32 % (int)zero32; break;
	case 'e': r = u64 / zero64; break;
	case 'f': r = big64 / zero64; break;
	case 'g': r = big64 % zero64; break;
	case 'h': r = sbig64 / (long long)zero64; break;
	case 'i': r = sbig64 % (long long)zero64; break;
	default: return 2;
	}

	printf("%s: %llx, no trap\n", argv[1], r);
	return 1;
}
