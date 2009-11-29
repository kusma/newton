#include "3dthen.h"
#include "poly.h"
#include "decomp.h"
#include <gba_systemcalls.h>
#include "fb.h"	
#include "fjalloc.h"
void transform_verts(s32* ram_lights, const signed short (*lights0)[3], const signed short (*lights1)[3], int lightcount, RamVertex* ram_verts, const RomVertex* rom_verts, const RomMatrix4* const* mvps0, const RomMatrix4* const* mvps1, u32 vcount, const u16* list, int subframe) IWRAM_CODE;

void Trede::unpack_frame() {
	fstate = rom_frames[frame];
	if (frame+1==framecount) {
		mvps_next = rom_frames[0].mvps;
		lights_next = rom_frames[0].lights;
	} else {
		mvps_next = rom_frames[frame+1].mvps;
		lights_next = rom_frames[frame+1].lights;
	}

	u16* activefaces_bb;
	if (activefaces==activefaces_buff0) {
		activefaces_bb = activefaces_buff0;
		activefaces = activefaces_buff1;
	} else {
		activefaces_bb = activefaces_buff1;
		activefaces = activefaces_buff0;	
	}

	u16* activeverts_bb;
	if (activeverts==activeverts_buff0) {
		activeverts_bb = activeverts_buff0;
		activeverts = activeverts_buff1;
	} else {
		activeverts_bb = activeverts_buff1;
		activeverts = activeverts_buff0;
	}

	activeverts_end = decomp::updateIndexList(activeverts, fstate.activeverts+1, *fstate.activeverts, activeverts_bb);
	activefaces_end = decomp::updateIndexList(activefaces, fstate.activefaces+1, *fstate.activefaces, activefaces_bb);
}

void Trede::setFrame(int f, int subframe) {
	if (f<0 || f>=framecount) {
		f%=framecount;
		if (f<0) f+=framecount;
	}


	if (f!=frame) {	
		// go to closest pframe
		if (frame>f || rom_frames[f].pframe!=fstate.pframe) {
			// load pframe
			frame = rom_frames[f].pframe;
			unpack_frame();
		}

		// scan to correct frame
		while (frame<f) {
			frame++;
			unpack_frame();
		};
	}

	transform_verts(ram_lights, fstate.lights, lights_next, lightcount, ram_verts, rom_verts, fstate.mvps, mvps_next, activeverts_end-activeverts, activeverts, subframe);
}


Trede::Trede(const RomVertex* verts, int vcount, const RomFace* faces, int fcount, const RomUvs* uvs, const Material* mtls, const RomFrame* frames, int framecount, int lightcount) {
	shadeTable=NULL;
	this->lightcount=lightcount;
	this->fcount=fcount;
	this->vcount=vcount;
	ram_verts = (RamVertex*)fjalloc::get(vcount*sizeof(RamVertex));
	rom_verts=verts;
	rom_faces=faces;
	rom_uvs=uvs;
	rom_mtls=mtls;
	activeverts_buff0 = (u16*)fjalloc::get(MAX_ACTIVEVERTS*sizeof(u16));
	activeverts_buff1 = (u16*)fjalloc::get(MAX_ACTIVEVERTS*sizeof(u16));
	activefaces_buff0 = (u16*)fjalloc::get(MAX_ACTIVEFACES*sizeof(u16));
	activefaces_buff1 = (u16*)fjalloc::get(MAX_ACTIVEFACES*sizeof(u16));
	ram_lights = (s32*)fjalloc::get(3*lightcount);
	activeverts=NULL;
	activefaces=NULL;
	letterbox=0;
	rom_frames = frames;
	this->frame=0;
	this->framecount=framecount;
	unpack_frame();
	transform_verts(ram_lights, fstate.lights, lights_next, lightcount, ram_verts, rom_verts, fstate.mvps, mvps_next, activeverts_end-activeverts, activeverts, 0);
}
Trede::~Trede() {
	fjalloc::release(vcount*sizeof(RamVertex));
	fjalloc::release(MAX_ACTIVEVERTS*sizeof(u16));
	fjalloc::release(MAX_ACTIVEVERTS*sizeof(u16));
	fjalloc::release(MAX_ACTIVEFACES*sizeof(u16));
	fjalloc::release(MAX_ACTIVEFACES*sizeof(u16));
	fjalloc::release(3*lightcount);
}

/*
void Trede::shadow(u8* dst) {
	if (!fstate.shadow) return;
	decomp::rleImage4shift(dst, dst+GBA_WIDTH*GBA_HEIGHT, (const u16*)fstate.shadow);
}
*/


	INLINE u32 addShade32(u32 a, u32 b) {
		// get shade
		u32 shade = (a & 0x0F0F0F0F) + (b & 0x0F0F0F0F);
		
		// get clamp-mask
		u32 mask = shade & 0xF0F0F0F0;
		mask -= mask>>4;
		
		// clamp shade
		shade = (shade & 0x0F0F0F0F) | mask;
		
		// combine and return
		return shade;
	}
	
	// this throws away color
	INLINE u16 addShade16(u16 a, u16 b) {
		// get shade
		u32 shade = (a & 0x0F0F) + (b & 0x0F0F);
		
		// get clamp-mask
		u32 mask = shade & 0x1010;
		mask -= mask>>4;
		
		// combine and return
		return (shade|mask) & 0x0F0F;
	}
	
static void drawDotHorizAA(u8* fb, int x, int y, int scol) {
	u16* fb16 = (u16*)(fb + ((x>>4)&(~1)) + (y>>4)*240);
	int valx = x&15;
	int coll = (scol * (15-valx))>>4;
	int colr = (scol * valx)>>4;
	
	if (!((x>>4)&1)) {
		int col = coll | (colr<<8);	
		*fb16 = addShade16(*fb16, col);
	} else {
		fb16[0] = addShade16(fb16[0], coll<<8);
		fb16[1] = addShade16(fb16[1], colr);
	}
}


void Trede::drawDots(u8* fb, u32 scol) {
	for (u16* p = activeverts; p<activeverts_end; p++) {
		RamVertex vert = ram_verts[*p];
		if (vert.x<0 || vert.x>238*16 || vert.y<1 || vert.y>158*16) continue;


		int valy = vert.y&15;
		int colt = (15-valy);
		int colb = valy;
		drawDotHorizAA(fb-240, vert.x, vert.y, colt);
		drawDotHorizAA(fb, vert.x, vert.y, colb);
	}
	
}


