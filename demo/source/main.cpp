#include <gba_video.h>
#include <gba_systemcalls.h>
#include <gba_interrupt.h>
#include <gba_sprites.h>
#include <gba_input.h>
#include <gba_console.h>
#include "tunnel.h"
#include "parts.h"
#include "grideffects.h"
#include "fb.h"

#include "support.h"
#include "palette.h"

// #define LAMER_VERSION

#include "part.h"
#include "adpcm.h"
#include "timer.h"
#include "noise.h"

extern u8 tune_adpcm[];
extern u8 tune2_adpcm[];

#ifndef REG_WAITCNT
#define REG_WAITCNT (*(vu16*)(REG_BASE + 0x0204))
#endif

extern const u8  start01_tex[];
extern const u16 start01_pal[];
extern const u8  start02_tex[];
extern const u16 start02_pal[];
extern const u8  start03_tex[];
extern const u16 start03_pal[];
extern const u8  start04_tex[];
extern const u16 start04_pal[];

static volatile bool enable_noise = true;

#include "linearcurve.h"
static const LinearCurve::keyframe flash_keyframes[] = 
{
	{ 0.00, 31.00 },
	{ 0.24, 0.00 },
	{ 0.25, 31.00 },
	{ 0.49, 0.00 },
	{ 0.50, 31.00 },
	{ 0.74, 0.00 },
	{ 0.75, 31.00 },
	{ 0.99, 0.00 },
	{ 1.00, 31.00 },
	{ 3.99, 0.00 },
	{ 4.00, 31.00 },
	{ 4.49, 0.00 },
	{ 4.50, 31.00 },
	{ 4.99, 0.00 },
	{ 5.00, 31.00 },
	{ 5.24, 0.00 },
	{ 5.25, 31.00 },
	{ 5.99, 0.00 },
	{ 6.00, 31.00 },
	{ 7.99, 0.00 },
	{ 8.00, 31.00 },
	{ 8.24, 0.00 },
	{ 8.25, 31.00 },
	{ 8.49, 0.00 },
	{ 8.50, 31.00 },
	{ 8.74, 0.00 },
	{ 8.75, 31.00 },
	{ 8.99, 0.00 },
	{ 9.00, 31.00 },
	{ 12.00, 0.00 },
};
static LinearCurve flash_curve = LINEARCURVE(flash_keyframes);

static void vblank()
{
	fixed16 beat = timer::getBeat();
	if (beat > fixed16(16))
	{
//		CpuFastSet(start04_pal, BG_COLORS, (512) / 4);
//		int fade = ((timer::getBeat() - 16) * 0.5).get_val() >> 8;
		fixed16 local_beat = beat - 16;
		int fade = (local_beat * 128);
		if (fade > 256) fade = 256;
		palette::boost(BG_COLORS, start04_pal, 256, -fade);
	}
	
	if (enable_noise)
	{
//		noise::update();
		int val = flash_curve.getValue(timer::getBeat());
		BG_COLORS[0] = RGB5(val, val,val);
		REG_BLDCNT = BIT(10) | BIT(6) | BIT(4);
		REG_BLDALPHA = (0) | (0 << 8);
		
//		palette::boost(BG_COLORS, start01_pal, 256, -100);
	}
}

static void init()
{
//	noise::setup();
	SetMode(MODE_4 | BG2_ON | OBJ_ENABLE | OBJ_1D_MAP);
 	REG_BLDCNT = BIT(12) | BIT(2) | BIT(13);
//	REG_BLDCNT = BIT(10) | BIT(6) | BIT(4) | BIT(13);
}

void part1()
{
	// tune config
	const float bpm = 128.0;
	const float start_beat = 0; //  162 + 32;
	
	// don't touch ;)
	const int   start_frame = int(start_beat / (float(bpm) / (FRAMES_PR_SECOND * 60)));
	
	// init tune
	adpcm_gba_init(tune_adpcm);
	timer::setBpm(bpm);
	adpcm_gba_set_frame(start_frame);
	
	part::set_part(init, vblank);
	
	VBlankIntrWait();
	CpuFastSet(start01_pal, BG_COLORS, (512) / 4);
	CpuFastSet(start01_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	part::swap();
	while (timer::getBeat() < 12);
	enable_noise = false;

	VBlankIntrWait();
	CpuFastSet(start02_pal, BG_COLORS, (512) / 4);
	CpuFastSet(start02_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	part::swap();
	while (timer::getBeat() < fixed16(12.5));
	
	VBlankIntrWait();
	CpuFastSet(start03_pal, BG_COLORS, (512) / 4);
	CpuFastSet(start03_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	part::swap();
	while (timer::getBeat() < fixed16(13.25));
	
	VBlankIntrWait();
	CpuFastSet(start04_pal, BG_COLORS, (512) / 4);
	CpuFastSet(start04_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	part::swap();
	while (timer::getBeat() < 18);

	parts::radial(    18,  82);
//	parts::radial(    0,  82); // or: apheregrid preview and then mirror
//	parts::grid(     82,  98);
	parts::cubegrid( 82,  98);


//	parts::mercury(82, 146);

	//parts::skate(98, 118, 0); // ?? mirror thing?

	parts::mercury(98, 162);
//	parts::lattice(98, 162);


	parts::growy(   162, 226);


	parts::mirror(   226, 258); // placeholder for greets/mirror

	// corridor thing
	parts::zoomer(  258, 290);
	parts::sphere(  290, 320);

	parts::message(320, 336, 1);
	
	while (timer::getBeat() < 336);
}

extern const u16 sfc_bp_bkah_01_pal[];
extern const u16 sfc_bp_bkah_02_pal[];
extern const u16 sfc_bp_bkah_03_pal[];
extern const u16 sfc_bp_bkah_04_pal[];

extern const u16 sfc_bp_bkah_01_tex[];
extern const u16 sfc_bp_bkah_02_tex[];
extern const u16 sfc_bp_bkah_03_tex[];
extern const u16 sfc_bp_bkah_04_tex[];

void part2()
{
	// tune config
	const float bpm = 173.0;
	const float start_beat = 0;
	
	// don't touch ;)
	const int   start_frame = int(start_beat / (float(bpm) / (FRAMES_PR_SECOND * 60)));
	
	// init tune
	adpcm_gba_init(tune2_adpcm);
	timer::setBpm(bpm);
	adpcm_gba_set_frame(start_frame);
	
	VBlankIntrWait();
	BG_COLORS[0] = 0;
	int svart = 0;
	CpuFastSet(&svart, fb::bb, ((GBA_WIDTH * GBA_HEIGHT) / 4) | FILL);

	while (timer::getBeat() < 32);

	CpuFastSet(sfc_bp_bkah_01_pal, BG_COLORS, (512) / 4);
	CpuFastSet(sfc_bp_bkah_01_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	fb::swap();
	while (timer::getBeat() < 33);
		
	CpuFastSet(sfc_bp_bkah_02_pal, BG_COLORS, (512) / 4);
	CpuFastSet(sfc_bp_bkah_02_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	fb::swap();
	while (timer::getBeat() < 34);

	CpuFastSet(sfc_bp_bkah_03_pal, BG_COLORS, (512) / 4);
	CpuFastSet(sfc_bp_bkah_03_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	fb::swap();
	while (timer::getBeat() < fixed16(34.5));

	CpuFastSet(sfc_bp_bkah_04_pal, BG_COLORS, (512) / 4);
	CpuFastSet(sfc_bp_bkah_04_tex, fb::bb, (GBA_WIDTH * GBA_HEIGHT) / 4);
	fb::swap();
	while (timer::getBeat() < 36);
	
	parts::poelse(          36, 100);

	parts::lockedloaded(100, 108);
//	while (timer::getBeat() < 108);

	parts::skate(108, 109, 0, 0); // testing
	parts::spheregrid(109, 109+15,     500 << 8);
	parts::skate(109+15, 109+16, 0, 20); // testing
	parts::spheregrid(109+16, 109+31,  350003); // 2100032);
	parts::skate(109+31, 109+32, 0, 8); // testing
	parts::spheregrid(109+32, 109+47,  910012);
	parts::skate(109+47, 109+48, 0, 13); // testing
	parts::spheregrid(109+48, 109+63,  100002);
//	parts::skate(109+63, 109+64, 0, 0); // testing

	parts::skate(109+63, 180, 2, 0); // testing

//	parts::spheregrid(109, 180);


	parts::parallax_scroll(180, 306);
	parts::telos(        306, 338);
	parts::telos(        338, 370);
	parts::lattice(      370, 434);

	parts::skate(434, 434+12, 1, 0); // testing


	parts::message(434+12, 434+12+16, 0);


	//	parts::mercury(370, 434);

//	parts::cubegrid(1);
//	parts::grid(1);
//	parts::test(1);
//	parts::spheregrid(1);
//	parts::plane(1);

	while (timer::getBeat() < 450);
}

int main() {
	grideffects::init();
//	init_tunnel();
	
	/* setup default states */
	REG_WAITCNT = 0x46d6;
	
	/* clear bits to be conditionally set later on */
	REG_WAITCNT &= ~(((1 << 3) - 1) << 1);
	REG_WAITCNT &= ~(1 << 14);
	
	/* enable ROM prefetch */
	REG_WAITCNT = 1 << 14;

	/* setup wait states */	
#ifndef LAMER_VERSION
	REG_WAITCNT |= (1 << 4) | (2 << 2);
#else
	REG_WAITCNT |= (1 << 4) | (1 << 2);
#endif

	irqInit();
	fb::reset();
/*		
	consoleInit(0, 4, 0, NULL, 0, 15);

	BG_COLORS[0] = RGB5(0, 0, 0);
	BG_COLORS[241] = RGB5(31, 31, 31);
	REG_DISPCNT = MODE_0 | BG0_ON; */
	irqSet(IRQ_VBLANK, part::vblank);
	irqEnable(IRQ_VBLANK);

	part1();
	part2();
	
	adpcm_gba_close();

	irqDisable(IRQ_VBLANK);
	fb::reset();
	while(1) VBlankIntrWait();
}

