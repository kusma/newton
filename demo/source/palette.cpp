#include "palette.h"
#include "math.h"
#include <gba_video.h>
#include "support.h"
namespace palette
{
	void bw(u16* dst, const u16* src, int count) {
		u16* dst_end = dst+count;
		for (; dst<dst_end; dst++, src++) {
			u32 r = *src&0x1F;
			u32 g = (*src>>5)&0x1F;
			u32 b = (*src>>10);
			u32 temp = ((r+g+b)*(256/3))>>8;
			*dst = RGB5(temp, temp, temp);
		}
	}

	void add(u16* dst, const u16* src, int count, u16 add) {
		u32 ra = add&0x1F;
		u32 ga = add&(0x1F<<5);
		u32 ba = add&(0x1F<<10);
		u16* dst_end = dst+count;
		for (; dst<dst_end; dst++, src++) {
			u32 col = *src;
			u32 r = col&0x1F;
			u32 g = col&(0x1F<<5);
			u32 b = col&(0x1F<<10);
			r += ra;
			g += ga;
			b += ba;
			if (r>0x1F) r=0x1F;
			if (g>(0x1F<<5)) g=0x1F<<5;
			if (b>(0x1F<<10)) b=0x1F<<10;
			*dst = r|g|b;
		}
	}

	void boost(u16 *dst, const u16 *src, u32 len, s32 boost_amount)
	{
		boost_amount += 256;
		for (u32 i = len; i; --i)
		{
			u16 col = *src++;
			s32 b = col & ((1 << 5) - 1);
			s32 g = ((col >>  5) & ((1 << 5) - 1));
			s32 r = col >> 10;

			r = ((r * boost_amount) >> 8);
			g = ((g * boost_amount) >> 8);
			b = ((b * boost_amount) >> 8);

			if (r > ((1 << 5) - 1)) r = ((1 << 5) - 1);
			if (g > ((1 << 5) - 1)) g = ((1 << 5) - 1);
			if (b > ((1 << 5) - 1)) b = ((1 << 5) - 1);

			*dst++ = (r << 10) | (g << 5) | b;
		}
	}

	void invert(u16 *dst, const u16 *src, u32 len, s32 invert_amount)
	{
		for (u32 i = len; i; --i)
		{
			u16 col = *src++;
			s32 b = col & ((1 << 5) - 1);
			s32 g = ((col >>  5) & ((1 << 5) - 1));
			s32 r = col >> 10;

			r += (((((1 << 5) - 1) - r) - r) * invert_amount) >> 5;
			g += (((((1 << 5) - 1) - g) - g) * invert_amount) >> 5;
			b += (((((1 << 5) - 1) - b) - b) * invert_amount) >> 5;

			if (r > ((1 << 5) - 1)) r = ((1 << 5) - 1);
			if (g > ((1 << 5) - 1)) g = ((1 << 5) - 1);
			if (b > ((1 << 5) - 1)) b = ((1 << 5) - 1);

			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;

			*dst++ = (r << 10) | (g << 5) | b;
		}
	}

	void xpal2pal(u16 *dst, const u16 *src, u32 len){
		FORI(len) {
			src++;
			int r = *src++;
			int g = *src++;
			int b = *src++;
			*dst++ = RGB8(r>>8, g>>8, b>>8);
		}
	}


	void color_boost(u16 *dst, const u16 *src, u32 len, s32 boost_amount)
	{
		for (u32 i = len; i; --i)
		{
			u16 col = *src++;
			s32 b = col & ((1 << 5) - 1);
			s32 g = ((col >>  5) & ((1 << 5) - 1));
			s32 r = col >> 10;

			s32 level = ((r + g + b) * ((1 << 16) / 3)) >> 16;

			s32 r_delta = r - level;
			s32 g_delta = g - level;
			s32 b_delta = b - level;

			r_delta = (r_delta * boost_amount) >> 5;
			g_delta = (g_delta * boost_amount) >> 5;
			b_delta = (b_delta * boost_amount) >> 5;

			r = level + r_delta;
			g = level + g_delta;
			b = level + b_delta;

			if (r > ((1 << 5) - 1)) r = ((1 << 5) - 1);
			if (g > ((1 << 5) - 1)) g = ((1 << 5) - 1);
			if (b > ((1 << 5) - 1)) b = ((1 << 5) - 1);

			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;

			*dst++ = (r << 10) | (g << 5) | b;
		}
	}

	void colorize(u16 *dst, const u16 *src, u32 len, u32 color, s32 colorize_amount)
	{
		s32 color_b = color & ((1 << 5) - 1);
		s32 color_g = ((color >>  5) & ((1 << 5) - 1));
		s32 color_r = color >> 10;
	//	s32 color_level = (color_r + color_g + color_b) / 3;
		s32 color_level = ((color_r + color_g + color_b) * ((1 << 16) / 3)) >> 16;

		s32 color_r_delta = color_r - color_level;
		s32 color_g_delta = color_g - color_level;
		s32 color_b_delta = color_b - color_level;

		for (u32 i = 256; i; --i) {
			u16 col = *src++;
			s32 b = col & ((1 << 5) - 1);
			s32 g = ((col >>  5) & ((1 << 5) - 1));
			s32 r = col >> 10;

			s32 level = ((r + g + b) * ((1 << 16) / 3)) >> 16;

			s32 r_delta = r - level;
			s32 g_delta = g - level;
			s32 b_delta = b - level;

			r_delta += ((color_r_delta - r_delta) * colorize_amount) >> 5;
			g_delta += ((color_g_delta - g_delta) * colorize_amount) >> 5;
			b_delta += ((color_b_delta - b_delta) * colorize_amount) >> 5;

			r = level + r_delta;
			g = level + g_delta;
			b = level + b_delta;

			if (r > ((1 << 5) - 1)) r = ((1 << 5) - 1);
			if (g > ((1 << 5) - 1)) g = ((1 << 5) - 1);
			if (b > ((1 << 5) - 1)) b = ((1 << 5) - 1);

			if (r < 0) r = 0;
			if (g < 0) g = 0;
			if (b < 0) b = 0;

			*dst++ = (r << 10) | (g << 5) | b;
		}
	}
	
	#define GET_R(c) ((c >> 10) & ((1 << 5) - 1))
	#define GET_G(c) ((c >>  5) & ((1 << 5) - 1))
	#define GET_B(c) ((c >>  0) & ((1 << 5) - 1))
	
	#ifndef MIN
	#define MIN(x, y) ((x) > (y) ? (y) : (x))
	#endif
	
	inline int get_shiftpal_index(int sub_pal, int col)
	{
		return (128 >> sub_pal) + (col >> sub_pal);
	}
	
	void make_fadepal(u16 *dst, const u16 *src, const u16 dst_col)
	{
		/* copy "first" 128 colors */
		for (int i = 0; i < 128; ++i)
		{
			dst[255 - i] = src[255 - i];
		}
	
		/* down-sample two and two colors */
		int w = 64;
		for (int sub_pal = 1; sub_pal < 8; ++sub_pal)
		{
			for (int i = 0; i < w; ++i)
			{
				int base_color = i << sub_pal;
				
				int old_idx = get_shiftpal_index(sub_pal - 1, base_color);
				u32 col1 = dst[old_idx + 0];
				u32 col2 = dst[old_idx + 1];
				
				/* avg col1 and col2 */
	#if 1
				u32 r = MIN(GET_R(col1), GET_R(col2));
				u32 g = MIN(GET_G(col1), GET_G(col2));
				u32 b = MIN(GET_B(col1), GET_B(col2));
				u32 col  = (r << 10) | (g << 5) | b;
	#elif 0
				/* naive */
				u32 r = (GET_R(col1) + GET_R(col2)) >> 1;
				u32 g = (GET_G(col1) + GET_G(col2)) >> 1;
				u32 b = (GET_B(col1) + GET_B(col2)) >> 1;
				u32 col  = (r << 10) | (g << 5) | b;
	#elif 0
				/* looses carry if both lsbs are 1, ie a bit inaccurate */
				u32 col  = col1 & ~(1 | (1 << 5) | (1 << 10)); /* clear bottom bit */
				col     += col2 & ~(1 | (1 << 5) | (1 << 10)); /* clear bottom bit */
				col >>= 1;
	#else
				/* same as the first, but vectorized a tad by doing R and B components in one operation */
				u32 a1 = col1 & ((31 << 10) | 31);
				u32 a2 = col2 & ((31 << 10) | 31);
				u32 b1 = col1 & (31 << 5);
				u32 b2 = col1 & (31 << 5);
				u32 col  = ((a1 + a2) >> 1) & ((31 << 10) | 31);
				col     |= ((b1 + b2) >> 1) & (31 << 5);
	#endif
				dst[get_shiftpal_index(sub_pal, base_color)] = col;
			}
			w >>= 1;
		}
		
		/* shift towards dst_col */
		int dst_r = GET_R(dst_col);
		int dst_g = GET_G(dst_col);
		int dst_b = GET_B(dst_col);
	
		w = 64;
		for (int sub_pal = 1; sub_pal < 8; ++sub_pal)
		{
			for (int i = 0; i < w; ++i)
			{
				int base_color = i << sub_pal;
				int idx = get_shiftpal_index(sub_pal, base_color);
				u32 col = dst[idx];
				
				int alpha = sub_pal << (16 - 3);
				
				u32 r = GET_R(col);
				u32 g = GET_G(col);
				u32 b = GET_B(col);
				
				/* blend col towards dst_col */
				r = r + (((dst_r - r) * alpha) >> 16);
				g = g + (((dst_g - g) * alpha) >> 16);
				b = b + (((dst_b - b) * alpha) >> 16);
				
				col = ((r << 10) | (g << 5) | b);
				
				dst[idx] = col;
			}
			w >>= 1;
		}
	}

}
