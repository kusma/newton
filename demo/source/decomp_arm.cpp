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

#include "decomp.h"
#include <gba_systemcalls.h>
#include "assert.h"
#include "math.h"
namespace decomp {
	void  huff_decompress(u8* obuf, int decodeSize, const u8* huff_base, const u8* huff_offs, const u8* huff_symb, const u8* huff_cache, const u32* ibuf) __attribute__((section(".iwram0"), long_call));

	void  huff_decompress(u8* obuf, int decodeSize, const u8* huff_base, const u8* huff_offs, const u8* huff_symb, const u8* huff_cache, const u32* ibuf)
	{
	
		int bits=SH_MAXLENGTH;
		uint32 ibufpos=0;
		uint32 obufpos=0;

		/* read first portion of data, init bitbuf */
		uint32 bitbuf=ibuf[ibufpos++];

		/* decoding loop */
		uint32 symbleft=decodeSize;
		while(symbleft--) {
		
			// hent frame
			u32 frame;
			if (bits) {
				frame = (bitbuf<<(32-bits)) | (ibuf[ibufpos]>>bits);
			} else {
				frame = ibuf[ibufpos]; 
			}
			
			u32 codelen = huff_cache[frame>>(32-SH_CACHEBITS)];
			
			// oftest false
			if (codelen>SH_CACHEBITS) {
				while ((frame>>(32-codelen))<huff_base[codelen]) codelen++;
			}

			// lagre den tingen
			obuf[obufpos++] = huff_symb[ (frame>>(32-codelen)) - huff_base[codelen] + huff_offs[codelen] ]; 
			
			// hent nye bits fra 
			if (codelen<=bits) {
				bits -= codelen;
			} else { 
				bits += 32-codelen; 
				bitbuf = ibuf[ibufpos++];
			}
		}
	}

	u16* updateIndexList(u16* dst, const u16* data, int data_size, const u16* prev) __attribute__((section(".iwram0"), long_call));
	u16* updateIndexList(u16* dst, const u16* data, int data_size, const u16* prev)
	{
		int i=0;
		while (i<data_size) {
			if (data[i]&0x8000) {
				int match = data[i++]&0x7FFF;
				int count = s16(data[i++]);
				int direction = (count>0) ? 1 : -1;
				for (int j=match; j!=match+count; j+=direction) {
					*dst++ = prev[j];
				}
			} else {
				*dst++ = data[i++];
			}
		}
		return dst;
	}

	void rleImage4shift(u8* dst, u8* dst_end, const u16* src, int fade) __attribute__((section(".iwram0"), long_call));
	
	void rleImage4shift(u8* dst, u8* dst_end, const u16* src, int fade)
	{
		do { 
			u16 tall = *src++;
			int skift = (tall & 15);
			int antall = tall >> 4;

			//if (!antall) continue;
			if (skift<=0) {
				dst += antall;
			} else if (antall==1) {
				u16& val = *(u16*)(u32(dst)&~1);
				if (!(u32(dst)&1)) {					
					u16 tall2 = *src;
					int skift2 = (tall2 & 15);
					int antall2 = tall2 >> 4;
					if (antall2!=1) {
						u32 col = (val&0xFF)>>skift;
						val = (val&0xFF00)|col;
						dst++;
					} else {
					 	val = ((val&0xFF)>>skift)|((val>>skift2)&0xFF00);
						src++;
						dst+=2;
					}
				} else {
					u32 col = (val>>skift)&0xFF00;
					val = (val&0x00FF)|(col);
					dst++;
				}
			} else if (antall<8) {
				int maske = ~(0x01010100 - (u32(0x80808080)>>(skift-1)));

				if (u32(dst)&1) {
					u16& val = *(u16*)(dst-1);
					u32 col = val>>(8+skift);
					val = (val&0x00FF)|(col<<8);
					dst++;
					antall--;
				}
				while (antall>=2) {
					u16& val = *(u16*)dst;
					val = (val>>skift)&maske;
					dst += 2;
					antall-=2;
				}
				if (antall) {
					u16& val = *(u16*)dst;
					u32 col = (val&0xFF)>>skift;
					val = (val&0xFF00)|col;
					dst++;
					//antall--;
				}
			} else {
				int maske = ~(0x01010100 - (u32(0x80808080)>>(skift-1)));


				if (u32(dst)&1) {
					u16& val = *(u16*)(dst-1);
					u32 col = val>>(8+skift);
					val = (val&0x00FF)|(col<<8);
					dst++;
					antall--;
				}

				if (u32(dst)&2 && antall>=2) {
					u16& val = *(u16*)dst;
					val = (val>>skift)&maske;
					dst += 2;
					antall-=2;
				}

				if (skift>=7) {
					int col = (1)|(1<<8)|(1<<16)|(1<<24);
					if (antall>=32) {
						int fastantall = antall & ~31;
						CpuSet(&col, dst, COPY32|FILL|(antall>>2));
						antall-=fastantall;
						dst+=fastantall;
					}
				} else {
					while (antall>=32) {
						u32& val0 = ((u32*)dst)[0];
						u32& val1 = ((u32*)dst)[1];
						u32& val2 = ((u32*)dst)[2];
						u32& val3 = ((u32*)dst)[3];
						u32& val4 = ((u32*)dst)[4];
						u32& val5 = ((u32*)dst)[5];
						u32& val6 = ((u32*)dst)[6];
						u32& val7 = ((u32*)dst)[7];
						val0 = (val0>>skift)&maske;
						val1 = (val1>>skift)&maske;
						val2 = (val2>>skift)&maske;
						val3 = (val3>>skift)&maske;
						val4 = (val4>>skift)&maske;
						val5 = (val5>>skift)&maske;
						val6 = (val6>>skift)&maske;
						val7 = (val7>>skift)&maske;
						dst += 32;
						antall-=32;
					}
				}

				while (antall>=2) {
					u16& val = *(u16*)dst;
					val = (val>>skift)&maske;
					dst += 2;
					antall-=2;
				}

				if (antall) {
					u16& val = *(u16*)dst;
					u32 col = (val&0xFF)>>skift;
					val = (val&0xFF00)|col;
					dst++;
					//antall--;
				}
			}
		} while (dst<dst_end); 
	}
	
	
	
	// NOTE: i think there is a bug with the color keying
	// shouldnt it be below the "alignment fix" ?
	void rleImage4mov(u8* dst, u8* dst_end, const u16* src, int color_key) __attribute__((section(".iwram0"), long_call));
	void rleImage4mov(u8* dst, u8* dst_end, const u16* src, int color_key)
	{
		u8 lastCol=0;
		while (dst<dst_end) {
			int col = (*src & 15);
			int count = *src >> 4;
			u32 colx2 = col | (col<<8);
			src++;

			// color keying
			if (col==color_key) {
				dst += count;
				lastCol=0;
				continue;
			}

			// fix alignment from last loop
			if (u32(dst)&1) {
				*(u16*)(dst-1) = lastCol | (col<<8);
				dst++;
				count--;
			}
			lastCol=col;
			
			// masse bytes om gangen
			// (...)
			
			// 2 bytes om gangen		
			for (int i=0; i<count-1; i+=2) {
				*(u16*)(dst+i) = colx2;
			}
			dst += count;
		}
	}
	
	// this throws away color
	INLINE u32 addShade32(u32 a, u32 b) __attribute__((section(".iwram"), long_call));
	INLINE u32 addShade32(u32 a, u32 b)
	{
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
	INLINE u16 addShade16(u16 a, u16 b)
	{
		// get shade
		u32 shade = (a & 0x0F0F) + (b & 0x0F0F);
		
		// get clamp-mask
		u32 mask = shade & 0x1010;
		mask -= mask>>4;
		
		// combine and return
		return (shade|mask) & 0x0F0F;
	}
	/*
	void rleImage4add(u8* dst, u8* dst_end, const u16* src) {
		u8 lastCol=0;
		while (dst<dst_end) {
//			int col = ((*src & 15) + (cheap_rand()&1))>>1;
			int col = (*src & 15);
			int count = *src >> 4;
			u32 colx2 = col | (col<<8);
			src++;

			// fix alignment from last loop
			if (u32(dst)&1) {
				u16& val = *(u16*)(dst-1);
				val = addShade16(val, lastCol | (col<<8));
				dst++;
				count--;
			}
			lastCol=col;
			
			// masse bytes om gangen
			// (...)
			
			// 2 bytes om gangen		
			for (int i=0; i<count-1; i+=2) {
				u16& val = *(u16*)(dst+i);
				val = addShade16(val, colx2);
			}
			dst += count;
		}
	}
*/


};
