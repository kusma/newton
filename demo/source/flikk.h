#pragma once
#include "support.h"
#include "fjalloc.h"

//#define HUFF_SYMB_IWRAM
//#define HUFF_CACHE_IWRAM

class Flikk {
	int frameCount;
	int rleMaxBufferSize;
	int decodeMaxBufferSize;
	const u16* data;
	const u16* dataPtr;
	u16* decodeBuffer;
	u16* rleBuffer;
	int frame;
	
	bool huffmanActive;
	u8 huff_base[32];
	u8 huff_offs[32];
	#ifdef HUFF_SYMB_IWRAM
	u8 huff_symb[256];
	#else
	const u8* huff_symb;
	#endif
	
	#ifdef HUFF_CACHE_IWRAM
	u8 huff_cache[256];
	#else
	const u8* huff_cache;
	#endif
public:
	Flikk(const u16* data);
	~Flikk();
	void setFrame(int f);
	int  getFrame() { return frame; }
	void nextFrame();
	void drawMov(u8* fb, int color_key=-1);
	void drawAdd(u8* fb);
	void drawShift(u8* fb, int fade=0);


};
