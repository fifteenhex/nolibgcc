/* SPDX-License-Identifier: MIT */
#ifndef NOLIBGCC_H
#define NOLIBGCC_H

/*
 * Code can use the operators and link libnolibgcc.a, or call the
 * nolibgcc_*() functions, which are inlined: on the 68000 and 68010 they take
 * a cheaper path when an operand is constant or known to be small, elsewhere
 * they're the operators.
 *
 * The library is built from this header. The nolibgcc__*() functions are
 * its implementations, not API. They mustn't use the operation they
 * implement, or the compiler turns it into a call to itself.
 */

typedef __UINT16_TYPE__ nolibgcc_u16;
typedef __INT32_TYPE__ nolibgcc_s32;
typedef __UINT32_TYPE__ nolibgcc_u32;
/* Not __INT64_TYPE__, long on 64 bit hosts, where long long * wouldn't match */
typedef long long nolibgcc_s64;
typedef unsigned long long nolibgcc_u64;

/* Hidden like libgcc's, so a shared object doesn't export them */
#define NOLIBGCC_LIB __attribute__((visibility("hidden")))
#define NOLIBGCC_INLINE static inline __attribute__((always_inline))

/* The magnitude of a signed value, unsigned so the most negative one fits */
#define nolibgcc__abs32(x) ((x) < 0 ? -(nolibgcc_u32)(x) : (nolibgcc_u32)(x))
#define nolibgcc__abs64(x) ((x) < 0 ? -(nolibgcc_u64)(x) : (nolibgcc_u64)(x))

/*
 * The 68000 and 68010 only: the 68020 up, CPU32 and ColdFire each define one
 * of these
 */
#if defined(__mc68000__) && !defined(__mc68020__) && !defined(__mc68030__) && \
    !defined(__mc68040__) && !defined(__mc68060__) && !defined(__mcoldfire__)
#include "m68k/arch.h"
#else
#include "generic.h"
#endif

#endif
