#include "grid.h"
#include "vector3.h"

extern Vector3 view_vector;

IWRAM_CODE static void draw_parallax_quad(register u16 *target, const grid_node *n, const u8 *const texture, const u8 *height)
{
	// instead of interpolating left and right uvs, we rather interpolate left uv and uv-delta
	// both calculated at full prescision. left uv is concatinated. works fine.
	// uv-delta, however, can't be interpolated concatinated. fucking to lamer.
	// make sure texcoords and delta is in UUuuVVvv-format plz!
	// update: works now

	register int vx = (view_vector.x.get_val() >> 11);
	register int vy = view_vector.y.get_val() >> 11;

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

	for (u32 y = GRID_NODE_HEIGHT; y; --y)
	{
		u32 dudx = du_t >> GRID_NODE_LOG2_WIDTH;
		u32 dvdx = dv_t >> GRID_NODE_LOG2_WIDTH;

		// texcoord-format : UUuuVVvv
		register u32 uv = l_uv; // få vekk volatile, har faktisk en HEL DEL å si på ytelse...
		register u32 duvdx = test; // gir artifacts :/ ... hadde noe lignende tidligere, men husker ikke løsninga
//		u32 duvdx = (dvdx & 0xFFFF) | (dudx << 16);

#define PIXEL { \
			register u32 h = height[(uv & 0xFF00) | (uv >> 24)]; \
			register u32 uv2 = uv; \
			uv2 += (vy * h); \
			uv2 += (vx * h) << 16; \
			register u32 pix = texture[(uv2 & 0xFF00) | (uv2 >> 24)]; \
			uv += duvdx; \
			uv2 += duvdx; \
			*target++ = pix | (texture[(uv2 & 0xFF00) | (uv2 >> 24)] << 8); \
			uv += duvdx; \
		}

#if 0
		for (register u32 x = GRID_NODE_WIDTH / 2; x; x--) PIXEL
#elif 1
		PIXEL
		PIXEL
		PIXEL
		PIXEL

		PIXEL
		PIXEL
		PIXEL
		PIXEL
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

		du_t += ddu;
		dv_t += ddv;

		target += (GBA_WIDTH - GRID_NODE_WIDTH) >> 1;
	}
}

IWRAM_CODE void draw_parallax_grid(u8 *target, grid g, const u8 *const texture, const u8 *height)
{

	grid_node *n = &g[0];
	*target = 100; // no$gba error -> fps-counter :)

	for (u32 y = GRID_HEIGHT - 1; y; --y, ++n)
	{
		for (u32 x = GRID_WIDTH - 1; x; --x, ++n)
		{
			draw_parallax_quad((u16*)target, n, texture, height);
			target += GRID_NODE_WIDTH;
		}
		target += (GRID_NODE_HEIGHT * GBA_WIDTH) - GBA_WIDTH;
	}
}
