/* SPDX-License-Identifier: MIT */
#ifndef NOLIBGCC_H
#define NOLIBGCC_H

/*
 * The library is built from this header. The nolibgcc__*() functions are
 * its implementations.
 */

#if !defined(__m68k__) || defined(__mcoldfire__)
#error "nolibgcc is for the 680x0"
#endif

typedef __UINT16_TYPE__ nolibgcc_u16;
typedef __INT32_TYPE__ nolibgcc_s32;
typedef __UINT32_TYPE__ nolibgcc_u32;
typedef __INT64_TYPE__ nolibgcc_s64;
typedef __UINT64_TYPE__ nolibgcc_u64;

/* Hidden like libgcc's, so a shared object doesn't export them */
#define NOLIBGCC_LIB __attribute__((visibility("hidden")))
#define NOLIBGCC_INLINE static inline __attribute__((always_inline))

/* The magnitude of a signed value, unsigned so the most negative one fits */
#define nolibgcc__abs32(x) ((x) < 0 ? -(nolibgcc_u32)(x) : (nolibgcc_u32)(x))
#define nolibgcc__abs64(x) ((x) < 0 ? -(nolibgcc_u64)(x) : (nolibgcc_u64)(x))

/* What's in the library */
nolibgcc_u32 __mulsi3(nolibgcc_u32 a, nolibgcc_u32 b) NOLIBGCC_LIB;
nolibgcc_s32 __divsi3(nolibgcc_s32 a, nolibgcc_s32 b) NOLIBGCC_LIB;
nolibgcc_s32 __modsi3(nolibgcc_s32 a, nolibgcc_s32 b) NOLIBGCC_LIB;
nolibgcc_u32 __udivsi3(nolibgcc_u32 a, nolibgcc_u32 b) NOLIBGCC_LIB;
nolibgcc_u32 __umodsi3(nolibgcc_u32 a, nolibgcc_u32 b) NOLIBGCC_LIB;
nolibgcc_u32 __nolibgcc_udivmodsi4(nolibgcc_u32 n, nolibgcc_u32 d,
				   nolibgcc_u32 *rem) NOLIBGCC_LIB;
nolibgcc_s64 __muldi3(nolibgcc_s64 a, nolibgcc_s64 b) NOLIBGCC_LIB;
nolibgcc_s64 __divdi3(nolibgcc_s64 a, nolibgcc_s64 b) NOLIBGCC_LIB;
nolibgcc_s64 __moddi3(nolibgcc_s64 a, nolibgcc_s64 b) NOLIBGCC_LIB;
nolibgcc_u64 __udivdi3(nolibgcc_u64 a, nolibgcc_u64 b) NOLIBGCC_LIB;
nolibgcc_u64 __umoddi3(nolibgcc_u64 a, nolibgcc_u64 b) NOLIBGCC_LIB;
nolibgcc_u64 __nolibgcc_udivmoddi4(nolibgcc_u64 n, nolibgcc_u64 d,
				   nolibgcc_u64 *rem) NOLIBGCC_LIB;

/*
 * The building blocks. Nothing here may use the operation it implements or
 * the compiler turns it into a call to itself: multiplies are 16x16->32
 * (mulu.w) and divides 32/16->16 (divu.w).
 */

/* 16x16->32 of the low halves. A known zero half folds away */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc__mulu16(nolibgcc_u32 a, nolibgcc_u16 b)
{
	if ((__builtin_constant_p((nolibgcc_u16)a) && !(nolibgcc_u16)a) ||
	    (__builtin_constant_p(b) && !b))
		return 0;

	__asm__("mulu.w %1,%0" : "+d"(a) : "dmi"(b) : "cc");
	return a;
}

/* divu.w: quotient in the low half, remainder in the high. n / d must fit in 16 bits */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc__divu16(nolibgcc_u32 n, nolibgcc_u16 d)
{
	__asm__("divu.w %1,%0" : "+d"(n) : "dmi"(d) : "cc");
	return n;
}

/* Low 32 bits of a * b, the high halves multiplied only affect bits 32 and up */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc__mul32(nolibgcc_u32 a, nolibgcc_u32 b)
{
	nolibgcc_u16 mid = nolibgcc__mulu16(a >> 16, b) + nolibgcc__mulu16(a, b >> 16);

	return nolibgcc__mulu16(a, b) + ((nolibgcc_u32)mid << 16);
}

/* Full 64 bit product of two 32 bit values */
NOLIBGCC_INLINE nolibgcc_u64 nolibgcc__mul32x32(nolibgcc_u32 a, nolibgcc_u32 b)
{
	nolibgcc_u32 ll = nolibgcc__mulu16(a, b);
	nolibgcc_u32 lh = nolibgcc__mulu16(a, b >> 16);
	nolibgcc_u32 hl = nolibgcc__mulu16(a >> 16, b);
	nolibgcc_u32 hh = nolibgcc__mulu16(a >> 16, b >> 16);
	nolibgcc_u32 mid = lh + hl, lo, hi;

	/* mid carried out, that's worth 1 << 48 */
	if (mid < lh)
		hh += 0x10000;

	lo = ll + (mid << 16);
	hi = hh + (mid >> 16) + (lo < ll);

	return (nolibgcc_u64)hi << 32 | lo;
}

/* Low 64 bits of a * b, the high words multiplied only affect bits 64 and up */
NOLIBGCC_INLINE nolibgcc_u64 nolibgcc__mul64(nolibgcc_u64 a, nolibgcc_u64 b)
{
	nolibgcc_u32 al = a, ah = a >> 32, bl = b, bh = b >> 32;
	nolibgcc_u32 cross = nolibgcc__mul32(ah, bl) + nolibgcc__mul32(al, bh);

	return nolibgcc__mul32x32(al, bl) + ((nolibgcc_u64)cross << 32);
}

/* n / d for a 16 bit d, any n, a divu.w for each 16 bits of n */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc__udivmod32_16(nolibgcc_u32 n, nolibgcc_u16 d,
						    nolibgcc_u32 *rem)
{
	nolibgcc_u32 hi = nolibgcc__divu16(n >> 16, d);
	nolibgcc_u32 lo = nolibgcc__divu16((hi & 0xffff0000) | (n & 0xffff), d);

	if (rem)
		*rem = lo >> 16;

	return hi << 16 | (lo & 0xffff);
}

/* The same for a 64 bit n */
NOLIBGCC_INLINE nolibgcc_u64 nolibgcc__udivmod64_16(nolibgcc_u64 n, nolibgcc_u16 d,
						    nolibgcc_u64 *rem)
{
	nolibgcc_u32 r, qh, ql;

	qh = nolibgcc__udivmod32_16(n >> 32, d, &r);
	r = nolibgcc__divu16(r << 16 | (nolibgcc_u16)(n >> 16), d);
	ql = r << 16;
	r = nolibgcc__divu16((r & 0xffff0000) | (nolibgcc_u16)n, d);
	ql |= r & 0xffff;

	if (rem)
		*rem = r >> 16;

	return (nolibgcc_u64)qh << 32 | ql;
}

/* Full 32 bit division, the divisor must not be 0 */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc__udivmod32(nolibgcc_u32 n, nolibgcc_u32 d,
						 nolibgcc_u32 *rem)
{
	nolibgcc_u32 q, r, ds = d, ns = n;

	if (d <= 0xffff)
		return nolibgcc__udivmod32_16(n, d, rem);

	/*
	 * The quotient fits in 16 bits. Shift both down until the divisor is
	 * below 0xffff and divide by that plus one, which can only
	 * underestimate the quotient, so the multiply back can't overflow,
	 * and then fix it up from the remainder. The shifted divisor is at
	 * least 0x7fff so the estimate is only off by a few.
	 */
	do {
		ds >>= 1;
		ns >>= 1;
	} while (ds >= 0xffff);

	q = (nolibgcc_u16)nolibgcc__divu16(ns, ds + 1);
	r = n - (nolibgcc__mulu16(q, d) + (nolibgcc__mulu16(q, d >> 16) << 16));
	while (r >= d) {
		r -= d;
		q++;
	}

	if (rem)
		*rem = r;

	return q;
}

/* Full 64 bit division, the divisor must not be 0 */
NOLIBGCC_INLINE nolibgcc_u64 nolibgcc__udivmod64(nolibgcc_u64 n, nolibgcc_u64 d,
						 nolibgcc_u64 *rem)
{
	nolibgcc_u64 q = 0, bit = 1;

	if (!(n >> 32) && !(d >> 32)) {
		nolibgcc_u32 r32;

		q = nolibgcc__udivmod32(n, d, &r32);
		if (rem)
			*rem = r32;
		return q;
	}

	if (d <= 0xffff)
		return nolibgcc__udivmod64_16(n, d, rem);

	/* Restoring shift and subtract */
	while (d < n && !(d >> 63)) {
		d += d;
		bit += bit;
	}

	while (bit) {
		if (n >= d) {
			n -= d;
			q |= bit;
		}
		d >>= 1;
		bit >>= 1;
	}

	if (rem)
		*rem = n;

	return q;
}

#endif
