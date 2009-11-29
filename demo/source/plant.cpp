#include "plant.h"
#include "poly.h"

/* uv coordinates for the different frames of texture-animation (8 128 x 64 sub-textures)*/
static const u16 uvs[16][3][2] =
{
	{
		(0 + 127), (0 + 31),
		(0 + 0  ), (0 +  0),
		(0 + 0  ), (0 + 63)
	},
	{
		(128 + 127), (0 + 31),
		(128 + 0  ), (0 +  0),
		(128 + 0  ), (0 + 63)
	},
	{
		(0 + 127), (64 + 31),
		(0 + 0  ), (64 +  0),
		(0 + 0  ), (64 + 63)
	},
	{
		(128 + 127), (64 + 31),
		(128 + 0  ), (64 +  0),
		(128 + 0  ), (64 + 63)
	},
	{
		(0 + 127), (128 + 31),
		(0 + 0  ), (128 +  0),
		(0 + 0  ), (128 + 63)
	},
	{
		(128 + 127), (128 + 31),
		(128 + 0  ), (128 +  0),
		(128 + 0  ), (128 + 63)
	},
	{
		(0 + 127), (192 + 31),
		(0 + 0  ), (192 +  0),
		(0 + 0  ), (192 + 63)
	},
	{
		(128 + 127), (192 + 31),
		(128 + 0  ), (192 +  0),
		(128 + 0  ), (192 + 63)
	},
	
/* flipped horisontally and offset 32 texels down */
	{
		(0 + 0  ), (0 + 31 + 32),
		(0 + 127), (0 +  0 + 32),
		(0 + 127), (0 + 63 + 32)
	},
	{
		(128 + 0  ), (0 + 31 + 32),
		(128 + 127), (0 +  0 + 32),
		(128 + 127), (0 + 63 + 32)
	},
	{
		(0 + 0  ), (64 + 31 + 32),
		(0 + 127), (64 +  0 + 32),
		(0 + 127), (64 + 63 + 32)
	},
	{
		(128 + 0  ), (64 + 31 + 32),
		(128 + 127), (64 +  0 + 32),
		(128 + 127), (64 + 63 + 32)
	},
	{
		(0 + 0  ), (128 + 31 + 32),
		(0 + 127), (128 +  0 + 32),
		(0 + 127), (128 + 63 + 32)
	},
	{
		(128 + 0  ), (128 + 31 + 32),
		(128 + 127), (128 +  0 + 32),
		(128 + 127), (128 + 63 + 32)
	},
	{
		(0 + 0  ), (192 + 31 + 32),
		(0 + 127), (192 +  0 + 32),
		(0 + 127), (192 + 63 + 32)
	},
	{
		(128 + 0  ), (192 + 31 + 32),
		(128 + 127), (192 +  0 + 32),
		(128 + 127), (192 + 63 + 32)
	},

};

static inline void plot_pixel(u8 *bb, int x, int y, int color)
{
	if (x < 0) return;
	if (x > GBA_WIDTH) return;
	if (y < 0) return;
	if (y > GBA_HEIGHT) return;
		
	bb[y * GBA_WIDTH + x] = color;
}

void Plant::draw(u8 *bb, const Matrix4x4 &mvp, const u8 *texture)
{
	Material m;

	m.filler = FILLER_SHIFT;
	m.oneone = true;
	
	m.uvGen = UVGEN_UVS;
	m.tex[0] = texture;
	
	PolyVertex polyverts[3];
	
	/* setup uv-coordinates (these are allways constant) */
	
	polyverts[0].u =   0 << 8;
	polyverts[0].v =  32 << 8;
	
	polyverts[1].u = 128 << 8;
	polyverts[1].v =   0 << 8;

	polyverts[2].u = 128 << 8;
	polyverts[2].v =  64 << 8;

	static int frame = 0;
	frame++;

	for (int i = 0; i < branch_count; ++i)
	{
		Branch &b = branches[i];
		
		if (b.frame == 15) continue;

		Vector4 p0 = mvp * b.origin;
		Vector4 p1 = mvp * b.curr;

		if (p0.w <= fixed16(0)) continue;
		if (p1.w <= fixed16(0)) continue;

		fixed16 w0_rcp = rcp(p0.w);
		p0.x = (p0.x * w0_rcp) * fixed16( GBA_WIDTH  / 2) + fixed16(GBA_WIDTH  / 2);
		p0.y = (p0.y * w0_rcp) * fixed16(-GBA_HEIGHT / 2) + fixed16(GBA_HEIGHT / 2);
		
		fixed16 w1_rcp = rcp(p1.w);
		p1.x = (p1.x * w1_rcp) * fixed16( GBA_WIDTH  / 2) + fixed16(GBA_WIDTH  / 2);
		p1.y = (p1.y * w1_rcp) * fixed16(-GBA_HEIGHT / 2) + fixed16(GBA_HEIGHT / 2);

		fixed16 tx = p1.x - p0.x;
		fixed16 ty = p1.y - p0.y;
/*
		while ((long long)tx.get_val() * tx.get_val() + (long long)ty.get_val() * ty.get_val() > INT_MAX)
		{
			tx >>= 1;
			ty >>= 1;
		}
*/

//			if ((tx * tx + ty * ty) < fixed16(0.1)) continue;
		
		/* 2d normal */
		fixed16 nx =  ty >> 2;
		fixed16 ny = -tx >> 2;
		
		fixed16 ns = fixed16(650) * w1_rcp * rsq(nx * nx + ny * ny);
		nx *= ns;
		ny *= ns;
		
		polyverts[0].x = p0.x.get_val() >> 8;
		polyverts[0].y = p0.y.get_val() >> 8;
		
		polyverts[1].x = (p1.x - nx).get_val() >> 8;
		polyverts[1].y = (p1.y - ny).get_val() >> 8;
		
		polyverts[2].x = (p1.x + nx).get_val() >> 8;
		polyverts[2].y = (p1.y + ny).get_val() >> 8;
		
		
		int texframe = b.frame;
		polyverts[0].u = uvs[texframe][0][0] << 8;
		polyverts[0].v = uvs[texframe][0][1] << 8;
		polyverts[1].u = uvs[texframe][1][0] << 8;
		polyverts[1].v = uvs[texframe][1][1] << 8;
		polyverts[2].u = uvs[texframe][2][0] << 8;
		polyverts[2].v = uvs[texframe][2][1] << 8;

		
		polyfiller(bb, polyverts, m, NULL, 0, GBA_HEIGHT);

/*
		// endpoints
		plot_pixel(bb, int(p0.x), int(p0.y), 0);
		plot_pixel(bb, int(p1.x), int(p1.y), 0);
*/

/*
		// vertices
		plot_pixel(bb, polyverts[0].x >> 8, polyverts[0].y >> 8, 0);
		plot_pixel(bb, polyverts[1].x >> 8, polyverts[1].y >> 8, 0);
		plot_pixel(bb, polyverts[2].x >> 8, polyverts[2].y >> 8, 0);
*/
	}
}
