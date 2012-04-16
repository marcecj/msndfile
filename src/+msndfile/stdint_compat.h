#ifndef __STDINT_COMPAT_H__
#define __STDINT_COMPAT_H__

#if defined(_MSC_VER)

/* reuse equivalent types on Windows */
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;

#elif defined(HAVE_SYS_TYPES_H)

/* TODO: verify that this will work on platforms without glibc */

/* The POSIX header sys/types.h defines the types, but the unsigned version have
 * an underscore after the 'u'.  Note that we cannot just define them ourselves,
 * because the libsndfile header includes sys/types.h, leading to compile-time
 * errors. */
#include <sys/types.h>
typedef u_int8_t uint8_t;

#else

/* If all else fails, define everything ourselves.  I believe this won't help,
 * because Windows apparently has sys/types.h and sndfile.h includes it
 * unconditionally.  Leave it here anyway, because it might prevent unhelpful
 * error messages. */
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef long int32_t;

#endif

#endif
