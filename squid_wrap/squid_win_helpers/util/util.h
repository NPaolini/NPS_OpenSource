/*
 * Copyright (C) 1996-2018 The Squid Software Foundation and contributors
 *
 * Squid software is distributed under GPLv2+ license and includes
 * contributions from numerous individuals and organizations.
 * Please see the COPYING and CONTRIBUTORS files for details.
 */

#ifndef SQUID_UTIL_H
#define SQUID_UTIL_H

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <winsock.h>
#include <process.h>

#ifdef __cplusplus
#define SQUIDCEXTERN extern "C"
#else
#define SQUIDCEXTERN extern
#endif

#if _USE_INLINE_
#define _SQUID_INLINE_ inline
#else
#define _SQUID_INLINE_
#endif
#define getpid _getpid

 /*
 * ISO C99 Standard printf() macros for 64 bit integers
 * On some 64 bit platform, HP Tru64 is one, for printf must be used
 * "%lx" instead of "%llx"
 */
#ifndef PRId64
#if _SQUID_WINDOWS_
#define PRId64 "I64d"
#elif SIZEOF_INT64_T > SIZEOF_LONG
#define PRId64 "lld"
#else
#define PRId64 "ld"
#endif
#endif

#ifndef PRIu64
#if _SQUID_WINDOWS_
#define PRIu64 "I64u"
#elif SIZEOF_INT64_T > SIZEOF_LONG
#define PRIu64 "llu"
#else
#define PRIu64 "lu"
#endif
#endif

#ifndef PRIX64
#if _SQUID_WINDOWS_
#define PRIX64 "I64X"
#elif SIZEOF_INT64_T > SIZEOF_LONG
#define PRIX64 "llX"
#else
#define PRIX64 "lX"
#endif
#endif

#ifndef PRIuSIZE
 // NP: configure checks for support of %zu and defines where possible
#if 1 //SIZEOF_SIZE_T == 4 && _SQUID_MINGW_
#define PRIuSIZE "I32u"
#elif SIZEOF_SIZE_T == 4
#define PRIuSIZE "u"
#elif SIZEOF_SIZE_T == 8 && _SQUID_MINGW_
#define PRIuSIZE "I64u"
#elif SIZEOF_SIZE_T == 8
#define PRIuSIZE "lu"
#else
#error size_t is not 32-bit or 64-bit
#endif
#endif /* PRIuSIZE */


SQUIDCEXTERN int tvSubUsec(struct timeval, struct timeval);
SQUIDCEXTERN double tvSubDsec(struct timeval, struct timeval);
SQUIDCEXTERN void Tolower(char *);

SQUIDCEXTERN time_t parse_iso3307_time(const char *buf);

SQUIDCEXTERN double xpercent(double part, double whole);
SQUIDCEXTERN int xpercentInt(double part, double whole);
SQUIDCEXTERN double xdiv(double nom, double denom);

SQUIDCEXTERN const char *xitoa(int num);
SQUIDCEXTERN const char *xint64toa(int64_t num);

typedef struct {
    size_t count;
    size_t bytes;
    size_t gb;
} gb_t;

/* gb_type operations */
#define gb_flush_limit (0x3FFFFFFF)
#define gb_inc(gb, delta) { if ((gb)->bytes > gb_flush_limit || delta > gb_flush_limit) gb_flush(gb); (gb)->bytes += delta; (gb)->count++; }
#define gb_incb(gb, delta) { if ((gb)->bytes > gb_flush_limit || delta > gb_flush_limit) gb_flush(gb); (gb)->bytes += delta; }
#define gb_incc(gb, delta) { if ((gb)->bytes > gb_flush_limit || delta > gb_flush_limit) gb_flush(gb); (gb)->count+= delta; }
extern double gb_to_double(const gb_t *);
SQUIDCEXTERN const char *double_to_str(char *buf, int buf_size, double value);
extern const char *gb_to_str(const gb_t *);
extern void gb_flush(gb_t *);  /* internal, do not use this */

/*
 * Returns the amount of known allocated memory
 */
int statMemoryAccounted(void);

SQUIDCEXTERN unsigned int RoundTo(const unsigned int num, const unsigned int what);

#endif /* SQUID_UTIL_H */

