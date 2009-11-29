#ifndef MATH_H
#define MATH_H

#include "base.h"

#define SIN_TABLE_LOG2_SIZE 9
#define SIN_TABLE_SIZE (1 << SIN_TABLE_LOG2_SIZE)
extern const unsigned short sin_lut[(SIN_TABLE_SIZE / 4) + 1];

static inline int fixed_fsin(int i)
{
	int idx = i & ((SIN_TABLE_SIZE >> 2) - 1);
	switch ((i >> (SIN_TABLE_LOG2_SIZE - 2)) & 0x3)
	{
		case 0: return +int(sin_lut[idx]);
		case 1: return +int(sin_lut[(SIN_TABLE_SIZE / 4) - idx]);
		case 2: return -int(sin_lut[idx]);
		case 3: return -int(sin_lut[(SIN_TABLE_SIZE / 4) - idx]);
	}
	ASSERT(0); // should not happen. ever.
	return 0;
}

INLINE int fixed_fcos(int i)
{
	return fixed_fsin(i + (SIN_TABLE_SIZE / 4));
}

#define ASDSDF (16 - SIN_TABLE_LOG2_SIZE)
static inline int fixed_sin(int i)
{
	const unsigned index = unsigned(i) >> ASDSDF;
	const int v0 = fixed_fsin(index);
	const int v1 = fixed_fsin(index + 1);
	int diff = v1 - v0;
	int t = i & ((1 << ASDSDF) - 1);
	return v0 + ((diff * t) >> ASDSDF);
}

static inline int fixed_cos(int i)
{
	const unsigned index = unsigned(i) >> ASDSDF;
	const int v0 = fixed_fcos(index);
	const int v1 = fixed_fcos(index + 1);
	int diff = v1 - v0;
	int t = i & ((1 << ASDSDF) - 1);
	return v0 + ((diff * t) >> ASDSDF);
}
#undef ASDSDF


// Reading 32 bits unaligned produces garbage in 16 high bits.
// This probably reduces quality on the high 16 bits but is okay as we mostly use just the lowest bits.
extern const u16 Rand_Table[] IWRAM_DATA;
INLINE unsigned cheap_rand()
{
	static int seed = 0x87654321;
	static unsigned char iter=0;
	seed += *(unsigned*)(Rand_Table + iter++);
	//seed ^= (seed>>8);
	return seed;
}



extern const unsigned char clz_lut[256];
INLINE unsigned clz(unsigned input)
{
	/* 2 iterations of binary search */
	unsigned c = 0;
	if (input & 0xFFFF0000) input >>= 16;
	else c = 16;

	if (input & 0xFF00) input >>= 8;
	else c += 8;

	/* a 256 entries lut ain't too bad... */
	return clz_lut[input] + c;
}

INLINE int mul16x16(int Rm, int Rs)
{
	Rm = (Rm << 16) >> 16; /* 16bit sign-extend */
	Rs = (Rs << 16) >> 16; /* 16bit sign-extend */
	return Rm * Rs;
}

INLINE int mul16x32(int Rm, int Rs)
{
	Rm = (Rm << 16) >> 16; /* 16bit sign-extend */
	return Rm * Rs;
}

extern const int rcp_table[512];
static inline int rcp(int i)
{
	unsigned v, v1, v2, v3;
	int v4;
	int zero_bits;

	int neg = i < 0;
	if (neg) i = -i;

	zero_bits = clz(i);
	v = i << zero_bits;
	v1 = rcp_table[(v >> 23)];
	v2 = (v >> 14) & 511;

	v3 = mul16x32(v1, v2) + v1; // MLA in ARM :)
	v4 = v3 >> (31 - zero_bits);

	if (neg) return -v4;
	return v4;
}

extern const int rsq_table[512];
static inline int rsq(int f)
{
	int zero_bits = clz(f) & ~1;
	unsigned v = f << zero_bits;
	unsigned v1 = rsq_table[v >> 23];
	unsigned v2 = (v >> 14) & 511;

	unsigned v3 = mul16x32(v1, v2) + v1; // MLA in ARM
	unsigned v4 = v3 >> (22 - (zero_bits >> 1));

	return v4;
}

extern const u16 IntRcp_Table[160 * 2] IWRAM_DATA;
INLINE PURE u16 ircp(u32 x) {
	if (x<256) return IntRcp_Table[x];
	assert((x>>8)<256);
	return IntRcp_Table[x>>8]>>8;
}

INLINE PURE s32 fast_pythagoras(s32 x, s32 y) {
	x = x ^ (x>>31);
	y = y ^ (y>>31);
	int summ = x+y;
	int maxx = MAX(x, y);
	return (summ+maxx)>>1;
}

INLINE PURE s32 fast_pythagoras(s32 x, s32 y, s32 z) {
	x = x ^ (x>>31);
	y = y ^ (y>>31);
	z = z ^ (z>>31);
	int summ = x+y+z;
	int maxx = MAX(MAX(x,y),z);
	return (summ+maxx)>>1;
}

#endif /* _MATH_H_ */
