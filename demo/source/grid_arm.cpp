#include "grid.h"
#include "vector3.h"

#define USE_SHIFT
// #define DITHER_SHIFT
// #define DITHER_UV

#define DITHER(interpolated, dither_mask) (interpolated + ((dither_mask) & 0xFF00))

#ifdef DITHER_SHIFT
#define S_FINAL(s_interpolated, s_dither_mask) DITHER(s_interpolated, s_dither_mask)
#else
#define S_FINAL(s_interpolated, s_dither_mask) (s_interpolated)
#endif

#define UV_FINAL(uv) ((uv & 0xFF00) | (uv >> 24))

static void draw_quad(register u16 *target, const grid_node *n, const u8 *const texture) __attribute__((section(".iwram1"), long_call));
void draw_grid(u8 *target, grid g, const u8 *const texture) __attribute__((section(".iwram1"), long_call));

static void draw_quad(register u16 *target, const grid_node *n, const u8 *const texture)
{
	// instead of interpolating left and right uvs, we rather interpolate left uv and uv-delta
	// both calculated at full prescision. left uv is concatinated. works fine.
	// uv-delta, however, can't be interpolated concatinated. fucking to lamer.
	// make sure texcoords and delta is in UUuuVVvv-format plz!
	// update: works now


	u32 l_uv = n->u << 16;
	l_uv |= n->v & 0xFFFF;
	
	u32 l_duvdy = (((n + GRID_WIDTH)->u - n->u) >> GRID_NODE_LOG2_HEIGHT) << 16; // left u-slope
	l_duvdy |= (((n + GRID_WIDTH)->v - n->v) >> GRID_NODE_LOG2_HEIGHT) & 0xFFFF; // left v-slope

	u32 du_t = (n + 1)->u - n->u;
	u32 du_b = (n + GRID_WIDTH + 1)->u - (n + GRID_WIDTH)->u;
	u32 ddu = (du_b - du_t) >> GRID_NODE_LOG2_HEIGHT;

	u32 dv_t = (n + 1)->v - n->v;
	u32 dv_b = (n + GRID_WIDTH + 1)->v - (n + GRID_WIDTH)->v;
	u32 ddv = (dv_b - dv_t) >> GRID_NODE_LOG2_HEIGHT;

	u32 test = ((du_t >> GRID_NODE_LOG2_WIDTH) << 16) | ((dv_t >> GRID_NODE_LOG2_WIDTH) & 0xFFFF);
	u32 test_d = ((ddu  >> GRID_NODE_LOG2_WIDTH) << 16) | ((ddv  >> GRID_NODE_LOG2_WIDTH) & 0xFFFF);

	/* color deltas */	
	u32 l_c = n->col;
	u32 l_dcdy = ((n + GRID_WIDTH)->col - n->col) >> GRID_NODE_LOG2_HEIGHT;

	u32 shift_t = (n + 1)->col - n->col;
	u32 shift_b = (n + GRID_WIDTH + 1)->col - (n + GRID_WIDTH)->col;
	u32 ddshift = (shift_b - shift_t) >> GRID_NODE_LOG2_HEIGHT;
	
	u32 shift_dx   = shift_t >> GRID_NODE_LOG2_WIDTH;
	u32 shift_dxdy = ddshift  >> GRID_NODE_LOG2_WIDTH;
/*	shift_dx += shift_dxdy; */


/*
1   9  3 11        These patterns (and their rotations
13  5 15  7        and reflections) are optimal for a
4  12  2 10        dispersed-pattern ordered dither.
16  8 14  6
*/
/*
	const u32 dithermask[4] = 
	{
		(0x11 << 24) | (0x99 << 16) | (0x33 << 8) | 0xbb,
		(0xdd << 24) | (0x55 << 16) | (0xff << 8) | 0x77,
		(0x44 << 24) | (0xcc << 16) | (0x22 << 8) | 0xaa,
		(0xff << 24) | (0x88 << 16) | (0xee << 8) | 0x66
	};
*/

	for (u32 y = GRID_NODE_HEIGHT; y; --y)
	{
		u32 dudx = du_t >> GRID_NODE_LOG2_WIDTH;
		u32 dvdx = dv_t >> GRID_NODE_LOG2_WIDTH;

		// texcoord-format : UUuuVVvv
		u32 uv = l_uv; // få vekk volatile, har faktisk en HEL DEL å si på ytelse...
//		u32 duvdx = test; // gir artifacts :/ ... hadde noe lignende tidligere, men husker ikke løsninga
		u32 duvdx = (dvdx & 0xFFFF) | (dudx << 16);
#if 0
		u32 sd = shift_dx;
		u32 s = l_c;
#else
		u32 sd = (shift_dx >> 3) & 0xFFFF;
		u32 s = (l_c >> 3) & 0xFFFF;
		s  |= (s + sd) << 16;
		sd |= sd << 16;
		sd <<= 1;
#endif

//		u32 dither = dithermask[y & 3];
#if defined(USE_SHIFT) && defined(DITHER_UV)
#define PIXEL(mask_shift1, mask_shift2) { \
			int s_dithered = s + ((mask_shift1) & 0xFF00); \
			register u32 pix = texture[((uv + ((mask_shift1 & 0xFF00) >> 8)) & 0xFF00) | ((uv +  ((mask_shift2 & 0xFF00) << 8)) >> 24)] >> (s_dithered >> 16); \
			uv += duvdx; \
			s += sd; \
			s_dithered = s + ((mask_shift2) & 0xFF00); \
			*target++ = (pix) | ((texture[((uv + ((mask_shift2 & 0xFF00) >> 8)) & 0xFF00) | ((uv + ((mask_shift1 & 0xFF00) << 8)) >> 24)] >> (s_dithered >> 16)) << 8); \
			uv += duvdx; \
			s += sd; \
		}
#elif defined(USE_SHIFT)
#define PIXEL(mask_shift1, mask_shift2) { \
			u32 s_final, uv_final, pix; \
			s_final = (s >> (16 - 3)) & ((1 << 3) - 1); \
			uv_final = UV_FINAL(uv); \
			pix = texture[uv_final] >> s_final; \
			uv += duvdx; \
			s_final = s >> (32 - 3); \
			uv_final = UV_FINAL(uv); \
			pix |= (texture[uv_final] >> s_final) << 8; \
			*target++ = pix; \
			uv += duvdx; \
			s += sd; \
		}
#else
#define PIXEL(dummy1, dummy2) { \
			register u32 pix = texture[(uv & 0xFF00) | (uv >> 24)]; \
			uv += duvdx; \
			*target++ = pix | (texture[(uv & 0xFF00) | (uv >> 24)] << 8); \
			uv += duvdx; \
		}
#endif


#if 0
		for (register u32 x = GRID_NODE_WIDTH / 2; x; x--) PIXEL(dither >> 16, dither >> 8)
#elif 1

		PIXEL(dither >> 16, dither >> 8)
		PIXEL(dither, dither << 8)
#if GRID_NODE_WIDTH >= 8
		PIXEL(dither >> 16, dither >> 8) 
		PIXEL(dither, dither << 8)
#endif
#if GRID_NODE_WIDTH >= 16
		PIXEL(dither >> 16, dither >> 8)
		PIXEL(dither, dither << 8)
		PIXEL(dither >> 16, dither >> 8)
		PIXEL(dither, dither << 8)
#endif
#if GRID_NODE_WIDTH >= 32
		PIXEL(dither >> 16, dither >> 8)
		PIXEL(dither, dither << 8)
		PIXEL(dither >> 16, dither >> 8)
		PIXEL(dither, dither << 8)
		PIXEL(dither >> 16, dither >> 8)
		PIXEL(dither, dither << 8)
		PIXEL(dither >> 16, dither >> 8)
		PIXEL(dither, dither << 8)
#endif


#else
		asm("\
			.rep 1						\n\
			# -- superpixel r1			\n\
			# first pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r1, [%2, r0]			\n\
			add %1, %1, %3				\n\
										\n\
			# second pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r1, r1, r0, lsl #8		\n\
										\n\
			# third pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r1, r1, r0, lsl #16		\n\
										\n\
			# forth pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r1, r1, r0, lsl #24		\n\
										\n\
			# -- superpixel r2			\n\
			# first pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r2, [%2, r0]			\n\
			add %1, %1, %3				\n\
										\n\
			# second pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r2, r2, r0, lsl #8		\n\
										\n\
			# third pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r2, r2, r0, lsl #16		\n\
										\n\
			# forth pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r2, r2, r0, lsl #24		\n\
										\n\
			# -- superpixel r3			\n\
			# first pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r3, [%2, r0]			\n\
			add %1, %1, %3				\n\
										\n\
			# second pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r3, r3, r0, lsl #8		\n\
										\n\
			# third pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r3, r3, r0, lsl #16		\n\
										\n\
			# forth pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r3, r3, r0, lsl #24		\n\
										\n\
			# -- superpixel r4			\n\
			# first pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r4, [%2, r0]			\n\
			add %1, %1, %3				\n\
										\n\
			# second pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r4, r4, r0, lsl #8		\n\
										\n\
			# third pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r4, r4, r0, lsl #16		\n\
										\n\
			# forth pixel				\n\
			and r0, %1, #0xFF00			\n\
			orr r0, r0, %1, lsr #24		\n\
			ldrb r0, [%2, r0]			\n\
			add %1, %1, %3				\n\
			orr r4, r4, r0, lsl #24		\n\
										\n\
										\n\
			# store						\n\
			#str r1, [%0], #4			\n\
			#str r2, [%0], #4			\n\
			stmia %0!, {r1,r2,r3,r4}	\n\
			.endr						\n\
		"
		: 
		: "r"(target), "r"(uv), "r"(texture), "r"(duvdx)
		: "r1", "r2", "r3", "r4", "r0", "0", "1", "3");
//		target += GRID_NODE_WIDTH / 2;
#endif

#undef PIXEL

		l_uv += l_duvdy;
		test += test_d;
		
		l_c      += l_dcdy;
		shift_dx += shift_dxdy;

		du_t += ddu;
		dv_t += ddv;

		target += (GBA_WIDTH - GRID_NODE_WIDTH) >> 1;
	}
}

void draw_grid(u8 *target, grid g, const u8 *const texture)
{
	grid_node *n = &g[0];
	*target = 100; // no$gba error -> fps-counter :)

	for (u32 y = GRID_HEIGHT - 1; y; --y, ++n)
	{
		for (u32 x = GRID_WIDTH - 1; x; --x, ++n)
		{
			draw_quad((u16*)target, n, texture);
			target += GRID_NODE_WIDTH;
		}
		target += (GRID_NODE_HEIGHT * GBA_WIDTH) - GBA_WIDTH;
	}
}
