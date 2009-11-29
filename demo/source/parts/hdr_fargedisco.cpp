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


extern const u16 hdr_disco_xpal[];
extern const u8 disco_bg[];
extern const u8 discoenv_tex[];




extern const u8 hdrbump_ebump[];
extern const u8 kulenm_ebump[];
extern const u8 alleyenvsb_tex[];
extern const u8 normalmap_ebump[];

namespace scene {
#include "scene_hdr_fargedisco2.h"
};

static volatile bool glow_enabled=false;
static int blanks = 0;
void global_vblank();
static void vblank()
{
	pimp_vblank();
	glow::vblank(); 
	pimp_frame(); 
	global_vblank();
	blanks++;
}

static volatile int boost=0x3000;

static void callback(int type, int data)
{
	if (data==1) boost = 0x3000;
}


extern const u16 glowpal_yellow[];

namespace parts
{
	void hdr_fargedisco(int len)
	{
//		FORI(100) VBlankIntrWait();
		irqSet(IRQ_VBLANK, vblank);
		irqEnable(IRQ_VBLANK);
		pimp_set_callback(callback);

		
		Trede trede(scene::verts, scene::vertcount, scene::faces, scene::facecount, scene::uvs, scene::materials, scene::framestates, scene::framecount);


		int img_height = 1000;
		int scrolly = 0;
		int swaps=0; 
		
		int intensity = 0x8000;

		fixed16 time = 0;

		int start_order = pimp_get_order();
		while ((pimp_get_order() - start_order < len) && (pimp_get_order() >= start_order))

		{
			scrolly += 4;
			if (scrolly>img_height) scrolly=0;
		
			int h0 = img_height - scrolly;
			if (h0>160) h0=160;
			int h1 = 160-h0;
			if (h1<0) h1=0;
			CpuFastSet(disco_bg+scrolly*240, fb::bb, COPY32|(h0*240/4));			
			if (h1) CpuFastSet(disco_bg, fb::bb+h0*240, COPY32|(h1*240/4));

			trede.setTime(time);
			trede.draw(fb::bb);
 
			//s32 exp = (fixed_sin(time.get_val())>>1) + 0x18000;
			int lo = glow::intensity - (glow::range);
			int hi = glow::intensity + (glow::range);
  
//			hi = (hi + (lo*3+0x2000))>>1;
//			if (hi > 0x10000) hi = 0x10000;
			
			if (lo < 0) lo = 0;


			lo -= boost>>1;
			hi -= boost;
			
			boost = (boost * 210) >> 8;

			if (hi < lo+0x3000) hi = lo+0x3000;
			
			int glow_hi = hi;// - (glow::range>>2);// - 0x2000;
//			if (glow_hi>0xA000) glow_hi=0xA000;
//			if (glow_lo > glow_hi-0x1000) glow_lo = glow_hi-0x1000;
			int glow_lo = (lo+glow_hi*3)>>2;//(lo+glow_hi)>>1;
		 	 
			if (swaps==0) VBlankIntrWait();
			fb::swap();
			if (swaps==0) glow::init(glowpal_yellow);
			exposure::update(lo, hi, glow_lo, glow_hi, hdr_disco_xpal);
			OBJ_COLORS[0] = RGB8(255,0,0);
			glow::update(fb::fb, 0xB000);

			time += 0.91;
			if (int(time)>=128) time=128;
			swaps++;
		}
	}

};
