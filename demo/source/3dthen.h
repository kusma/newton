#include "support.h"
#include "math.h"
#include "exstructs.h"
#include "fixed.h"
//typedef fixed<s32, PREC_VEC> xvec;
//typedef fixed<s32, PREC_CMAT> xcmat;


#define MAX_ACTIVEFACES	4000
#define MAX_ACTIVEVERTS	4000

struct RamVertex {
	s16 x, y;
	//s16 depth;
	//u8 cc;
};


class Trede {
public:
	Trede() :shadeTable(NULL) {}
	Trede(const RomVertex* verts, int vcount, const RomFace* faces, int fcount, const RomUvs* uvs, const Material* mtls, const RomFrame* frames, int framecount, int lightcount=0);
	~Trede();
	void draw(u8* dst);
	//void shadow(u8* dst);
	void drawDots(u8* dst, u32 col);
	//Material materials[8];
	//RamBumpLight bumplights[8];
	int letterbox;
	void drawface(u8* dst, int fi);
	void setFrame(int frame, int subframe);
	void setTime(fixed16 time) {
		setFrame(int(time), (time.get_val()>>8)&0xFF);
	}
	int getFrame() { return frame; }
	const RomFrame& getRomFrame() { return rom_frames[frame]; }
//	void setframe(const RomFrame& fstate); // NB: dependent activelists
	s32* ram_lights;

	const u8* shadeTable;
	RamVertex* ram_verts;
private:
	int lightcount;
	int frame;
	int framecount;
	RomFrame fstate;
	int fcount;
	int vcount;
	const RomMatrix4* const* mvps_next; 
	const signed short (*lights_next)[3];
	
	const RomVertex* rom_verts;
	const RomFace* rom_faces;
	const RomUvs* rom_uvs;
	const Material* rom_mtls;
	const RomFrame* rom_frames;

	u16* activefaces_buff0;
	u16* activefaces_buff1;
	u16* activefaces;
	u16* activefaces_end;
	u16* activeverts_buff0; 
	u16* activeverts_buff1;
	u16* activeverts;
	u16* activeverts_end;
	void unpack_frame();
};


