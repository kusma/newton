#include "exstructs.h"

#include <gba_interrupt.h>
#include <gba_dma.h>
#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_sprites.h>
#include <pimpmobile.h>
#include "fb.h"
#include "config.h"
#include "vcount.h"
#include "palette.h"
#include "glow.h"
#include "3dthen.h"
//#include "rle.h"
//#include "events.h"


extern u16 clownogroup_pal[];
extern u8  clownogroup_raw[];


static int boost=40;

static int blanks = 0;
static int bg_fade_speed=-400;
static int bg_fade=0xFFFF;
static int fuzz = 256; 
static void vblank_fra_newton() {
	boost += ((int(cheap_rand()>>16) & 0xFF) - 128)>>4;
	if (boost>86) boost=86;
	if (boost<-0) boost=-0;
	
	boost = (fixed_cos(blanks*0xB0)+0x14000)>>12;

	palette::color_boost(fb::temppal, clownogroup_pal, 256, boost);


	BG_COLORS[0]=0;

	int shade = (fixed_sin(blanks*0xD7))>>2;

	palette::boost(BG_COLORS+1, fb::temppal+1, 255, shade-(bg_fade)>>8);

	bg_fade += bg_fade_speed;
	if (bg_fade>(255<<8)) bg_fade=255<<8;
	if (bg_fade<0) bg_fade=0;

	static int tulling=0;
	tulling++;
	int hei = (-fixed_cos(tulling* 64)+0x10000) >> 10;
	fuzz = hei+128;
}

void global_vblank();
static void vblank()
{
	pimp_vblank();
	pimp_frame();
	global_vblank();
	vblank_fra_newton();
	blanks++;
}

static void callback(int type, int data)
{
}


namespace parts {
	void clownogroup()
	{
		pimp_set_callback(callback);
		SetInterrupt(IE_VBL, vblank);
		EnableInterrupt(IE_VBL);

		DisableInterrupt(IE_HBL);		
		REG_DMA0CNT = 0;

		int start=pimp_get_order();
		int scrollx=0;
		int swaps=0;
		int img_width=356/2;
		while (pimp_get_order()==start) {

			{			
				const u16* src_linep = (const u16*)clownogroup_raw;
				u16* dst_linep = (u16*)fb::bb;
				for (int y=0; y<160; y++, src_linep+=img_width, dst_linep+=240/2) {
					CpuFastSet(src_linep+(scrollx>>1), dst_linep, COPY16|(240>>1));			
				}
			}
							
			
			VBlankIntrWait();
			REG_BG2X  = ((scrollx&3) << 8) - 0x200;
			fb::swap();
			
	
//			if (swaps&3) scrollx++;
			scrollx++;
			swaps++;
		}
	}
};
