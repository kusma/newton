#ifndef BASE_H
#define BASE_H

#ifdef WIN32
#include <windows.h>
#endif

#include <assert.h>
#define ASSERT(expr) assert(expr)

#ifndef INLINE
#define INLINE inline
#endif

#ifdef PURE
#undef PURE
#endif

#define PURE

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160
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
typedef unsigned long long u64;
typedef   signed long long s64;

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif
#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#endif /* BASE_H */
