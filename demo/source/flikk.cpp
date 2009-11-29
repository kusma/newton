#include "fb.h"
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
#include "shc.h"


// Originale: 5.286
// stuff i iwram: 5.286 
// med 16 cachebits: 5.405
// Stappa koden i ARM:
// Originale: 5.405
// stuff i iwram:  5.479
// med 16 cachebits: 5.556
// med maxlength på 32: 5.556
// bleee 5.439


// Hva er det som tar tid da??

#include "flikk.h"
#include "fjalloc.h"
#include "decomp.h"
#include "assert.h"

Flikk::Flikk(const u16* data) {
	frame = -1;

	huffmanActive = *(bool*)data;
	data += 2;
	
	if (huffmanActive) {
		// read decomp data
		FORI(31) {
			huff_base[i] = ((const u8*)data)[i];
			huff_offs[i] = (((const u8*)data) + 32)[i];
		}

		data = (const u16*)(((const u8*)data) + 64);
		#ifdef HUFF_SYMB_IWRAM
		FORI(256) huff_symb[i] = ((const u8*)data)[i];
		#else
		huff_symb = (const u8*)data;
		#endif
		data += N>>1;
		
		#ifdef HUFF_CACHE_IWRAM
		FORI(256) huff_cache[i] = ((const u8*)data)[i];
		#else
		huff_cache = (const u8*)data;
		#endif	
		data += SH_CACHESIZE>>1;
	}
	
	decodeMaxBufferSize = *data++;
	rleMaxBufferSize = *data++;
	frameCount = *data++;
	data++; // dummy for word-alignment
	this->dataPtr = this->data = data;
	decodeBuffer = (u16*)fjalloc::get(decodeMaxBufferSize+2);
	rleBuffer = (u16*)fjalloc::get(rleMaxBufferSize+2);
	decodeBuffer[decodeMaxBufferSize] = 0xBEEF;
	rleBuffer[rleMaxBufferSize] = 0xBEEF;

	nextFrame();
}	
Flikk::~Flikk() {
	fjalloc::release(decodeMaxBufferSize+2);
	fjalloc::release(rleMaxBufferSize+2);
}

void Flikk::setFrame(int f) {
	if (f>=frameCount) f %= frameCount;
	else if (f<0) f = (f % frameCount)+frameCount;
	
	if (frame>f) frame = -1;
	while (frame!=f) nextFrame();
}

void Flikk::nextFrame() {
	frame++;
	if (frame>=frameCount) frame = 0;
	if (frame==0) dataPtr = data;

	// these are now in bytes
	int huffmanSize = *dataPtr++;
	int decodeSize = *dataPtr++;

//	printf("decodeSize %x\n", *(u32*)dataPtr);
//	while(1);
///	printf("\n%i", huffmanSize);



	bool use_huffman = (huffmanSize != 0xFFFF);
	
	if (use_huffman) {
		u32* ibuf = (u32*)dataPtr;
		u8*  obuf = (u8*)decodeBuffer;
		decomp::huff_decompress(obuf, decodeSize, huff_base, huff_offs, huff_symb, huff_cache, ibuf);
		decomp::updateIndexList(rleBuffer, decodeBuffer, decodeSize>>1, rleBuffer); 
		dataPtr += ((huffmanSize+3)>>2)<<1;
	} else {
		decomp::updateIndexList(rleBuffer, dataPtr, decodeSize>>1, rleBuffer); 
		dataPtr += ((decodeSize+3)>>2)<<1;
	}

//	assert(decodeBuffer[decodeMaxBufferSize] == 0xBEEF);
//	assert(rleBuffer[rleMaxBufferSize] == 0xBEEF);

	// done? jupp
}

void Flikk::drawMov(u8* fb, int color_key) {
	decomp::rleImage4mov(fb, fb+240*160, rleBuffer, color_key);
}
void Flikk::drawAdd(u8* fb) {
//	decomp::rleImage4add(fb, fb+240*160, rleBuffer);
}
void Flikk::drawShift(u8* fb, int fade) {
	decomp::rleImage4shift(fb, fb+240*160, rleBuffer, fade);
}
