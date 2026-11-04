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

/* What's in the library */
nolibgcc_u32 __mulsi3(nolibgcc_u32 a, nolibgcc_u32 b) NOLIBGCC_LIB;

/*
 * The building blocks. Nothing here may use the operation it implements or
 * the compiler turns it into a call to itself: multiplies are 16x16->32
 * (mulu.w).
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

/* Low 32 bits of a * b, the high halves multiplied only affect bits 32 and up */
NOLIBGCC_INLINE nolibgcc_u32 nolibgcc__mul32(nolibgcc_u32 a, nolibgcc_u32 b)
{
	nolibgcc_u16 mid = nolibgcc__mulu16(a >> 16, b) + nolibgcc__mulu16(a, b >> 16);

	return nolibgcc__mulu16(a, b) + ((nolibgcc_u32)mid << 16);
}

#endif
