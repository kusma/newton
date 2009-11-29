#include <gba_base.h>
#include <gba_systemcalls.h>
#include <gba_video.h>

#include "tunnel.h"
#include "config.h"
#include "math.h"
#include "vcount.h"

#define TEXTURE_LOG2_SIZE 8
#define TEXTURE_SIZE (1 << TEXTURE_LOG2_SIZE)

#define TUNNEL_TABLE_WIDTH  (WIDTH  / 2)
#define TUNNEL_TABLE_HEIGHT (HEIGHT / 2)

u16 tunnel[TUNNEL_TABLE_WIDTH * TUNNEL_TABLE_HEIGHT] EWRAM_DATA;



void unflip(int y, int param) {
	REG_BG2PA = 0x100;
	REG_BG2PB = 0x000;
	REG_BG2PC = 0x000;
	REG_BG2PD = 0x100;

	REG_BG2X = 0x00000;
	REG_BG2Y = 0x00000;
}

void flip(int y, int param) {
	BG_COLORS[0] = 0;
	while ((REG_DISPSTAT & 2) == 0);
	REG_BG2X = 0x100 * WIDTH - 1;
	REG_BG2Y = 0x100 * (HEIGHT / 2) - 1;
	REG_BG2PA = -0x100;
	REG_BG2PB = 0x000;
	REG_BG2PC = 0x000;
	REG_BG2PD = -0x100;
}

void setup_vcount_tunnel() {
	vcount::insert(79, flip);
	vcount::insert(210, unflip);
}

void init_tunnel()
{
	u16 *data = tunnel;
	for (u32 y = 0; y < TUNNEL_TABLE_HEIGHT; ++y)
	{
		for (u32 x = 0; x < TUNNEL_TABLE_WIDTH; ++x)
		{
			s32 ix = (s32)x - WIDTH / 2;
			s32 iy = (s32)y - HEIGHT / 2;

//			ix += fast_fsin(iy << 2).get_val() >> 12;
//			iy += fast_fsin(ix << 2).get_val() >> 12;

			u32 deg = ArcTan2(ix, iy);
			s32 dist = Sqrt(ix * ix + iy * iy);

#if 1
/*			dist *= (1 << 8) + (fixed_sin(128 + (deg >> 5)) >> 10);
			dist >>= 8; */

			u32 u = deg >> 7;
//			u32 v = (10000 / dist) >> 2;
			u32 v = (10000 / dist);
//			u32 v = DivArmAbs(10000, dist) >> 2;

#else

//			dist += fast_fsin(deg >> 4).get_val() >> 13;
			s32 z = 35 - dist;
/*
			if (z > 0) z += 1;
			else z -= 1;
*/

//			u32 u = DivArmAbs(ix * 25, z);
//			u32 v = DivArmAbs(iy * 25, z);
			u32 u = (ix * 25) / z;
			u32 v = (iy * 25) / z;

#endif


			u32 coord = ((v & (TEXTURE_SIZE - 1)) << TEXTURE_LOG2_SIZE) | (u & (TEXTURE_SIZE - 1));
			*data++ = coord;
		}
	}
}

IWRAM_CODE void draw_tunnel(u8 *target, u8 *texture, u32 offset) {
	register vu16 *data = tunnel;
	register u16 *dst = (u16*)target;
	u32 offs = offset & ((1 << (TEXTURE_LOG2_SIZE + TEXTURE_LOG2_SIZE)) - 1);
	register vu8 *tex = texture + offs;

	for (register u32 y = TUNNEL_TABLE_HEIGHT + 0; y; --y) {
		register u16 *dst2 = dst + (WIDTH / 2) - 1;

		for (register u32 x = WIDTH >> 2; x; --x) {
			register u32 p, uv1, uv2;

			uv1 = *data++;
			uv2 = *data++;
			p = tex[uv2 ^ (TEXTURE_SIZE - 1)];
			p |= tex[uv1 ^ (TEXTURE_SIZE - 1)] << 8;
			*dst2-- = p;

			p = tex[uv1];
			p |= tex[uv2] << 8;
			*dst++ = p;
		}
		dst += WIDTH >> 2;
	}
}


#if 0
IWRAM_CODE void draw_tunnel(u8 *target, u8 *texture, u32 offset) {
	register vu16 *data = tunnel;
	register u16 *dst = (u16*)target;
	u32 offs = offset & ((1 << (TEXTURE_LOG2_SIZE + TEXTURE_LOG2_SIZE)) - 1);
	register vu8 *tex = texture + offs;

	for (register u32 x = (WIDTH * HEIGHT) >> (1 + 2); x; --x) {
#if 0
		register u32 p0, p1, p2, p3;
		register u32 temp;

		p0 = tex[*data++];
		p0 |= tex[*data++] << 8;
		temp = tex[*data++];
		temp |= tex[*data++] << 8;
		p0 |= temp << 16;

		p1 = tex[*data++];
		p1 |= tex[*data++] << 8;
		temp = tex[*data++];
		temp |= tex[*data++] << 8;
		p1 |= temp << 16;
/*
		p2 = tex[*data++];
		p2 |= tex[*data++] << 8;
		temp = tex[*data++];
		temp |= tex[*data++] << 8;
		p2 |= temp << 16;

		p3 = tex[*data++];
		p3 |= tex[*data++] << 8;
		temp = tex[*data++];
		temp |= tex[*data++] << 8;
		p3 |= temp << 16;
*/
#if 1
		*dst++ = p0;
		*dst++ = p1;
/*		*dst++ = p2;
		*dst++ = p3; */
#else
		asm("stmia %0, {%1, %2}" : : "r"(dst), "r"(p0), "r"(p1));
		dst += 2;
//		asm("stmia %0, {%1, %2, %3, %4}" : : "r"(dst), "r"(p0), "r"(p1), "r"(p2), "r"(p3));
//		dst += 4;
#endif
#else
		register u32 p;
#define DATA ((*data++) ^ (TEXTURE_SIZE - 1))
// #define DATA (0)
		p = tex[DATA];
		p |= tex[DATA] << 8;
		*dst++ = p;

		p = tex[DATA];
		p |= tex[DATA] << 8;
		*dst++ = p;

		p = tex[DATA];
		p |= tex[DATA] << 8;
		*dst++ = p;

		p = tex[DATA];
		p |= tex[DATA] << 8;
		*dst++ = p;
#endif
	}
}
#endif

