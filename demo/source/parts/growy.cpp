#include "../base.h"
#include "../exstructs.h"

#include <algorithm>
using std::min;
using std::max;

#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <stdio.h>
#include "../fb.h"
#include "../vcount.h"
#include "../palette.h"
#include "../3dthen.h"
#include "../part.h"
#include "../plant.h"
#include "../timer.h"
#include "../noise.h"
#include "../linearcurve.h"

static s32 targets[GBA_HEIGHT] EWRAM_DATA;
static s32 currents[GBA_HEIGHT] EWRAM_DATA;
static int deltas[GBA_HEIGHT] EWRAM_DATA;
static bool regen = true;

/*
extern const u8 growy_tex[];
extern const u8 flextextiles_tex[];
extern const u16 part_growy_pal[];
*/
extern const u8 crack_st[];
extern const u8 growy_tex[];
extern const u16 part_growy_pal[];
extern const u8 crack_tex[];
extern const u8 crack2_tex[];
extern const u8 crackfade2_tex[];
extern const u16 crack_pal[];

namespace{
namespace scene {
#include "../scenes/growy_plane.h"
};
namespace scene2 {
#include "../scenes/growy.h"
};
namespace scene3 {
#include "../scenes/growy_growing.h"
};
};

static bool enable_noise = false;


const LinearCurve::keyframe noise_keyframes[] = 
{
	{ 0.00, 0.00 },
	{ 4.00, 1.00 },
	{ 7.00, 2.00 },
	{ 8.00, 3.00 },
	{12.00, 4.00 },
	{15.00, 0.00 },
};

LinearCurve noise_curve = LINEARCURVE(noise_keyframes);

const LinearCurve::keyframe flash_keyframes[] = 
{
	{ 0.00, 0.50 },
	{ 3.99, 0.00 },
	{ 4.00, 0.50 },
	{ 7.99, 0.00 },
	{ 8.00, 0.50 },
	{11.99, 0.00 },
	{12.00, 0.50 },
	{14.99, 0.00 },
	{15.00, 0.50 },
	{19.00, 0.00 },
/*
	{31.99, 0.00 },
	{32.00, 1.00 },
	{35.00, 0.00 },
	
	{35.99, 0.00 },
	{36.00, 1.00 },
	{39.00, 0.00 },

	{39.99, 0.00 },
	{40.00, 1.00 },
	{43.00, 0.00 },
*/
};

LinearCurve flash_curve EWRAM_DATA = LINEARCURVE(flash_keyframes);

static volatile int flashmyass = 0;
static int blanks = 0;
static void vblank()
{
	blanks++;

	int v = flash_curve.getValue(timer::getBeat() - 162) * 32;
		
	if (flashmyass > v) v = flashmyass;
	flashmyass--;

	if (v > 31) v = 31;
		
		
	u16 add_col = RGB5(v,v,v);
	palette::add(BG_COLORS, crack_pal, 256, add_col);
//	CpuFastSet(crack_pal, BG_COLORS, (512) / 4);
	
	if (enable_noise == false) return;
	
	BG_COLORS[0] = add_col;
	noise::update();
	fixed16 a = fixed16(5) + (sin(fixed16(blanks) >> 8)) * 2;

	fixed16 beat = timer::getBeat() - 162;
	int alpha = noise_curve.getValue(beat);
//	int alpha = 2 + (a.get_val() >> 16);
	
	alpha += cheap_rand() & 3;
	if (alpha > 16) alpha = 16;
	if (alpha < 0) alpha = 0;
	REG_BLDALPHA = (alpha) | ((16 - alpha)<<8);

	s32 *dst  = currents;
	s32 *targ = targets;
	s32 offs = fixed_sin(blanks - 100) >>5;
	s32 rands = s32(cheap_rand()) >> 18;
	for (int i = GBA_HEIGHT; i; --i)
	{
//		rands += (s32(cheap_rand()) >> 22);
		rands = a.get_val() >> 10;
		*dst++ = (*targ * rands) >> 14; // *targ;
		targ++;
	}
	
//	REG_BG2X = blanks * 256;
	REG_DMA0CNT = 0;
	DMA0COPY(&currents[0], &REG_BG2X, DMA_HBLANK | DMA_REPEAT | DMA_DST_FIXED | DMA_SRC_INC | DMA32 | 1);
	
	if (0 == (blanks % 4)) regen = true;
	
	if (regen)
	{
		s32 *dst = targets;
		s32 offs = s32(cheap_rand()) >> 22;
		rands = 0;
		for (int i = GBA_HEIGHT; i; --i)
		{
			offs = fixed_fsin(i + blanks) >> 4;
			offs += fixed_fsin((i - blanks) * 5) >> 5;
			rands += ((s32(cheap_rand()) >> 22) * offs >> 11) ;
			
			int temp = (offs + rands);
			temp = (temp * temp) >> 10;
			temp += rands;
//			temp = (temp * temp) >> 12;
			*dst++ = -(1 << 11) + temp; // *targ;
		}
		regen = false;
	}
}


inline float randf()
{
	return float(rand() * (1.0 / RAND_MAX));
}

INLINE Vector3 get_spline_point(const s16 *spline, int i)
{
	return Vector3(
		 spline[1 + i * 3 + 0] >> 4,
		 spline[1 + i * 3 + 2] >> 4,
		 spline[1 + i * 3 + 1] >> 4
	);
}

void init_plant(Plant &plant, int spline_index)
{
	const s16 *spline = scene3::splines[spline_index];
	
	int segment_count = (spline[0] - 1) / 4;
	if (segment_count > PLANT_MAX_BRANCHES) segment_count = PLANT_MAX_BRANCHES;
	
	for (int i = 0; i < segment_count; ++i)
	{
		Branch &b = plant.branches[i];
		b.target    = get_spline_point(spline, (i + 1) * 4);
		b.origin    = get_spline_point(spline, i * 4);
		b.curr      = get_spline_point(spline, i * 4);
	}
	plant.branch_count = segment_count;
}

#define MAX_PLANTS 3
int plant_count = 0;

Plant plants[MAX_PLANTS] EWRAM_DATA;
fixed16 plant_amts[MAX_PLANTS] EWRAM_DATA;

Trede trede EWRAM_DATA;

static void init()
{
	VBlankIntrWait();
	REG_BLDCNT = 0;
	REG_BLDCNT = BIT(10) | (1 << 6) | BIT(4);

	fb::setMode(MODE_4 | BG2_ON | OBJ_ON | OBJ_1D_MAP);
	noise::setup();
	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (16) | (16<<8);
	
	CpuFastSet(crack_pal, BG_COLORS, (512) / 4);
	BG_COLORS[0] = 0;
}

namespace parts
{
	void growy(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		
		plant_count = 3;
		init_plant(plants[0], 0);
		init_plant(plants[1], 1);
		init_plant(plants[2], 2);
		
		for (int i = 0; i < plant_count; ++i)
		{
			plant_amts[i] = 0;
			plants[i].reset();
		}


		trede = Trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);
		//trede.shadeTable = crack_st;
		int swaps;
		swaps = 0;
		blanks = 0;
		
		while ((timer::getBeat() - start) < 16)
		{
			int svart = 0x80808080;
			CpuFastSet(&svart, fb::bb, ((GBA_WIDTH * GBA_HEIGHT) / 4) | FILL);

//			trede.setTime(fixed16(swaps) >> 3);
			fixed16 beat = timer::getBeat() - start;
//			trede.setFrame(beat, 0);
			trede.setTime(beat * 32);
			trede.draw(fb::bb);
			
			enable_noise = true;
			part::swap();
			swaps++;
		}
		
		swaps = 0;
		blanks = 0;

		enable_noise = false;
		REG_BLDALPHA = (16<<8); // quick hack noise away
		REG_DMA0CNT = 0;
		REG_BG2X = 0;
		
		trede = Trede(scene2::verts, scene2::vertcount, scene2::faces, scene2::facecount, scene2::uvs, scene2::materials, scene2::framestates, scene2::framecount);
		trede.shadeTable =crack_st;
		while (timer::getBeat() - start < 32)
		{
			int svart = 0x80808080;
			CpuFastSet(&svart, fb::bb, ((GBA_WIDTH * GBA_HEIGHT) / 4) | FILL);

			fixed16 beat = timer::getBeat() - start;
//			trede.setTime(beat * 7);

			int frame = (beat*16).get_val()>>16;
			frame+=256;
			trede.setFrame(frame,0);

//			trede.setTime(beat);
			trede.draw(fb::bb);

			part::swap();
			swaps++;
		}

		trede = Trede(scene3::verts, scene3::vertcount, scene3::faces, scene3::facecount, scene3::uvs, scene3::materials, scene3::framestates, scene3::framecount);

		while (timer::getBeat() < end)
		{
			int svart = 0x80808080;
			CpuFastSet(&svart, fb::bb, ((GBA_WIDTH * GBA_HEIGHT) / 4) | FILL);

//			trede.setTime(fixed16(swaps) >> 3);
			fixed16 beat = timer::getBeat() - start - 32;
//			trede.setFrame(swaps << 2, 0);

			static int last_cam = 0;
			int cam = int((beat) / 4) & 1;
			bool flash = false;
			
			if (last_cam != cam)
			{
				flash = true;
				last_cam = cam;
			}
			
			trede.setTime((beat * 16) + (256 * cam));
			
//			trede.setTime(beat);
			trede.draw(fb::bb);

			/* copy matrix */
			const RomMatrix4 *mat = trede.getRomFrame().mvps[0];
			Matrix4x4 mvp;
			for (int y = 0; y < 4; ++y)
			{
				for (int x = 0; x < 4; ++x)
				{
					fixed16 val = fixed16::make_raw(mat->m[x + y * 4]);
					mvp(y,x) = val;
				}
			}
			
			Matrix4x4 scale;
			const fixed16 fac = 1.f / (1 << 1);
			scale.scale(Vector3(fac, fac, fac));
			mvp = scale * mvp;
			
			for (int i = 0; i < plant_count; ++i)
			{
				Plant &plant = plants[i];

				plant_amts[i] += fixed16(0.03 * 16);
				plant.grow(plant_amts[i]);
				
				if (plant_amts[i] >= fixed16(1.2))
				{
					plant.advance();
					plant_amts[i] = 0;
					plant.grow(plant_amts[i]);
				}
				
				for (int i = 0; i < plant.visible_branch_count; ++i)
				{
					int frame = (16 - int(1 + plant.visible_branch_count - i));
					if (plant.visible_branch_count - 1 == i) frame = plant_amts[i].get_val() >> 13;
					plant.branches[i].frame = 15 - max(min(frame, 15), 0);
				}
				plant.draw(fb::bb, mvp, growy_tex);
			}

			if (flash) flashmyass = 16;
			part::swap();
			swaps++;
		}
		
	}
};
