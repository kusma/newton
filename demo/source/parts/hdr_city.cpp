#include "exstructs.h"

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
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "3dthen.h"
#include "decomp.h"
#include "flikk.h"
#include "flikkfx.h"
#include "glow.h"
#include "exposure.h"
#include <pimpmobile.h>


extern const u16 hdr_city_xpal[];

extern const u8 normalmap_ebump[];
extern const u8 cityenv_tex[];
extern const u8 citysb_tex[];

namespace scene1 {
#include "scene_hdr_city_no_obj.h"
};

namespace scene2 {
#include "scene_hdr_city_obj.h"
};

static volatile int part_ = 0;
static int blanks = 0;
static bool glow_started=false;
void global_vblank();
static void vblank()
{
	pimp_vblank();
	if (glow_started) glow::vblank(); 

	static int fadeup=0;
	int val = pimp_get_row()-0x30;
	if (part_==1 && val>0) {
		fadeup++;
		if (fadeup>0xFF) fadeup=0xFF;
		BG_COLORS[1] = RGB8(fadeup, fadeup, fadeup);
	}

	pimp_frame(); 
	global_vblank();
	blanks++;
}



extern const u16 glowpal_yellow[];

namespace parts
{
	void hdr_city(int len, int part, int starttime)
	{
		part_ = part;
		Trede trede1(scene1::verts, scene1::vertcount, scene1::faces, scene1::facecount, scene1::uvs, scene1::materials, scene1::framestates, scene1::framecount, 0);
		Trede trede2(scene2::verts, scene2::vertcount, scene2::faces, scene2::facecount, scene2::uvs, scene2::materials, scene2::framestates, scene2::framecount, 0);


		irqSet(IRQ_VBLANK, vblank);
		irqEnable(IRQ_VBLANK);
		irqDisable(IRQ_HBLANK);		
		REG_DMA0CNT = 0;

		if (!glow_started) {
			glow::init(glowpal_yellow);
			int svart=0x01010101;
			CpuFastSet(&svart, fb::fb, FILL|COPY32|(240*160/4));
			CpuFastSet(&svart, fb::bb, FILL|COPY32|(240*160/4));
			glow_started=true;
		}

		int swaps=0;
		fixed16 time=starttime;
				
		
		trede1.letterbox = 30;
		trede2.letterbox = 30;


		int start_order = pimp_get_order();
		while ((pimp_get_order() - start_order < len) && (pimp_get_order() >= start_order))
		{
		
			if (part==0)
			{
				trede1.setTime(time);
				trede1.draw(fb::bb);
				time+=0.5;
			} else if (part==1) {
				trede2.setTime(time);
				trede2.draw(fb::bb);
				time+=2.0;
			}

 
			int lo = glow::intensity >> 5;
			int hi = glow::intensity + (glow::range*4);
			
			int val = pimp_get_row()-0x30;
			if (part==1 && val>0) {
				lo -= val<<10;
				hi -= val<<11;
			}
			
  			if (hi < lo+0x3000) hi = lo+0x3000;
			int glow_hi = hi;
			int glow_lo = lo + (glow_hi >> 1); // (lo+glow_hi*3)>>2;//(lo+glow_hi)>>1;

			if (pimp_get_order()>=start_order+len) break;
			
			if (swaps==0) VBlankIntrWait();
			fb::swap();
			exposure::update(lo, hi, glow_lo, glow_hi, hdr_city_xpal);
			glow::update(fb::bb, 0xA000);

			swaps++;
		}
//		{
//			int svart=0x01010101;
//			CpuFastSet(&svart, fb::fb, FILL|COPY32|(240*160/4));
//			CpuFastSet(&svart, fb::bb, FILL|COPY32|(240*160/4));
//		}		
	}

};
