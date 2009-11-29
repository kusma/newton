#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_console.h>
#include <mappy.h>
#include "../base.h"
#include "../palette.h"
#include "../part.h"
#include "../fb.h"
#include "../math.h"
#include "../fixed.h"
#include "../infzoom.h"
#include "../timer.h"
#include "../linearcurve.h"
#include <math.h>

extern u8 zoom_map_raw[];
extern u16 zoom_map_pal[];

extern u8 zoomer_full_tex[];
extern u16 zoomer_full_pal[];

static volatile int blanks = 0;
static void vblank()
{
	blanks++;
}

static void init()
{
	VBlankIntrWait();
	
	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
	REG_BLDALPHA = (16 << 8);

	fb::setMode(MODE_4 | BG2_ON);
	CpuFastSet(zoom_map_pal, BG_COLORS, (512 / 4));
}

const LinearCurve::keyframe zoom_times_keyframes[] = 
{
	// this table maps from beats to zoom-steps (one zoom-step every integer). used to select when each zoom-step happens and their tempo.
	{ 0.00,  0.00 },
	{ 1.00,  1.00 },
	{ 4.00,  2.00 },
	{ 5.00,  3.00 },
	{ 9.00,  4.00 },
	{12.00,  5.00 },
	{13.00,  6.00 },
	{17.00,  7.00 },
	{20.00,  8.00 },
	{21.00,  9.00 },
	{24.00, 10.00 },
};
LinearCurve zoom_times_curve = LINEARCURVE(zoom_times_keyframes);


const LinearCurve::keyframe zoom_keyframes[] = 
{
	// this maps back from zoom-step space to actual zoom values. used to select where each step zooms at.
	{ 0.00, 0.09},
	{ 1.00, 0.20 },
	{ 2.00, 0.27 },
	{ 3.00, 0.31 },
	{ 4.00, 0.42 },
	{ 5.00, 0.52 },
	{ 6.00, 0.57 },
	{ 7.00, 0.65 },
	{ 8.00, 0.79 },
	{ 9.00, 0.84 },
	{10.00, 1.00 },
};
LinearCurve zoom_curve = LINEARCURVE(zoom_keyframes);

namespace parts
{
	void zoomer(fixed16 start, fixed16 end)
	{
		part::set_part(init, vblank);
		fixed16 start_beat = timer::getBeat();

		int swaps = 0;
		blanks = 0;
		while (timer::getBeat() < end)
		{
			fixed16 beat = timer::getBeat() - start;
			
			fixed16 zoom = zoom_times_curve.getValue(beat);
			
			int izoom = int(zoom);
			
			zoom &= 0xFFFF; /* get fractional part */
			fixed16 shake = fixed16(1) - zoom;
			
			/* linear is dull */
			zoom *= zoom;
			zoom *= zoom;
			zoom *= zoom;
			
			/* linear is dull */
			shake *= shake;
			shake *= shake;
			shake *= shake;
			
			/* reconstruct zoom */
			zoom += fixed16(izoom);
			zoom = zoom_curve.getValue(zoom);

			/* clamp zoom to the legal values */
			if (zoom <= fixed16(0)) zoom = fixed16::make_raw(0);
			if (beat >= fixed16(24))
			{
				CpuFastSet(zoomer_full_tex, fb::bb, (240*160/4));
				VBlankIntrWait();
				int col = 31;
				palette::add(BG_COLORS, zoomer_full_pal, 256, RGB5(col,col,col));
				part::swap();
				
				while (timer::getBeat() < end)
				{
					col -= 1;
					if (col < 0) col = 0;
					palette::add(BG_COLORS, zoomer_full_pal, 256, RGB5(col,col,col));
					VBlankIntrWait();				
				}
				return;
			}
			else
			{
				/* modulate zoom with a sine */
				fixed16 mod = sin(beat * 2) * (shake / 64) * zoom;
				zoom += mod;

				/* remap zoom from 0..1 linear range to 0..heigth exponential range */
				assert(zoom <= 1);
				zoom = zoom * zoom;
				zoom *= (1024 / 256);
	
				infzoom_draw(fb::bb, zoom_map_raw, zoom.get_val());
			}
			
			VBlankIntrWait();
			part::swap();
			swaps++;
		}
#if 0
		int blanks2 = blanks;
		consoleInit(0, 4, 0, NULL, 0, 15);
		BG_COLORS[0] = RGB5(0, 0, 0);
		BG_COLORS[241] = RGB5(31, 31, 31);
		REG_DISPCNT = MODE_0 | BG0_ON;
		printf("fps: %f\n", 60 * float(swaps) / blanks2);
		while (1);
#endif
	}
};
