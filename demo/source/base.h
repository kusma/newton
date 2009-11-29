#ifndef BASE_H
#define BASE_H

#ifdef WIN32

#include <windows.h>
#define IWRAM_DATA
#define IWRAM_CODE
#define EWRAM_DATA
#define EWRAM_CODE
typedef unsigned char      u8;
typedef   signed char      s8;
typedef unsigned short     u16;
typedef   signed short     s16;
typedef unsigned int       u32;
typedef   signed int       s32;

#define INLINE inline
#define OUTLINE
#define PURE

#else /* !defined(WIN32) */

#include <gba_base.h>

#define INLINE    inline __attribute__((always_inline))
#define OUTLINE   __attribute__((noinline))
#define PURE      __attribute__((pure))

#endif /* WIN32 */

typedef unsigned long long u64;
typedef   signed long long s64;

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

#ifndef GBA_WIDTH
#define GBA_WIDTH SCREEN_WIDTH
#endif
#ifndef GBA_HEIGHT
#define GBA_HEIGHT SCREEN_HEIGHT
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#include <assert.h>
#define ASSERT(expr) assert(expr)

#endif /* BASE_H */
