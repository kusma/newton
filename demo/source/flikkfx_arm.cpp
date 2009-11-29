#include "flikkfx.h"
#include "math.h"
#include <gba_systemcalls.h>
namespace flikkfx {

/*
struct DblInt {
	u32 low;
	u32 high;
	DblInt() {}
	DblInt(u64 op) {
		low = op;
		high = op>>32;
	}
	DblInt INLINE PURE operator+(const DblInt& op) const {
		DblInt hei = *this;
		hei.low += op.low;
		hei.high += op.high;
		return hei;
	}
	DblInt INLINE operator+=(const DblInt& op)  {
		low += op.low;
		high += op.high;
	}
	DblInt INLINE operator-=(const DblInt& op)  {
		low -= op.low;
		high -= op.high;
	}
	u32 INLINE get32(int shift) {
		return (low>>shift) | (high << (32-shift));
	}
	
};
*/

	void cheap_blur(u8* dst_buff, u8* src_buff) {
		// clear bottom and top line
		{
			int hei=0;
			CpuFastSet(&hei, dst_buff, COPY32|FILL|(240/4));
			CpuFastSet(&hei, dst_buff+240*159, COPY32|FILL|(240/4));
			*(u32*)(src_buff+240*159) = 0;
		}

		for (int x=0; x<240; x+=4) {
			u8* dst = (dst_buff + x + 240);
			u8* src_lower = (src_buff + x);
			//src_lower += 240;
			u64 row0       = *(u64*)src_lower & 0x0F0F0F0F0F0F0F0FULL;	src_lower += 240;
			u64 row1       = *(u64*)src_lower & 0x0F0F0F0F0F0F0F0FULL;	src_lower += 240;
			u64 row2       = *(u64*)src_lower & 0x0F0F0F0F0F0F0F0FULL;	src_lower += 240;
			u64 vertSum = row0+row1+row2;
			for (int y=1; y<159; y++, src_lower+=240, dst+=240) {
				u32 heheSum = u32(vertSum) + u32(vertSum>>8) + u32(vertSum>>16);
	
#if 1			
				// Her skal vi dele på 9.. 
				// Jeg ganger i stedet på 7/64
				heheSum >>= 3;
				heheSum &= 0x1F1F1F1F;
				heheSum = (heheSum*7)>>3;
				
#else
				heheSum >>= 4;
#endif
				*(u32*)dst = heheSum;	

				vertSum -= row0;
				row0 = row1;
				row1 = row2;
				row2 = *(u64*)src_lower & 0x0F0F0F0F0F0F0F0FULL;
				vertSum += row2;
			}
		}
	}

	void overlay(u8* dst, u8* src, u32 scrolly, u8* img, int img_height) {
		if (scrolly>=img_height) scrolly %= img_height;
		
		u32* srcp = (u32*)src;
		u32* dstp = (u32*)dst;
		u32* imgp = (u32*)(img+scrolly*240);
		for (int y=0; y<160; y++) {
		
			for (int x=0; x<240; x+=16) {
				*dstp++ = (*srcp++ & 0x0F0F0F0F) | ((*imgp++)<<4);
				*dstp++ = (*srcp++ & 0x0F0F0F0F) | ((*imgp++)<<4);
				*dstp++ = (*srcp++ & 0x0F0F0F0F) | ((*imgp++)<<4);
				*dstp++ = (*srcp++ & 0x0F0F0F0F) | ((*imgp++)<<4);
			}
			

			scrolly++;
			if (scrolly==img_height) {
				imgp = (u32*)img;
				scrolly = 0;
			}
		}
	}
};
