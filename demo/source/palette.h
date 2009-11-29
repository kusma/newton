#ifndef _PALETTE_H_
#define _PALETTE_H_

#include <gba_base.h>

namespace palette
{
	// convert from xpal to pal
	void xpal2pal(u16 *dst, const u16 *src, u32 len);


	// works on xpal
	// ...


	// works on pal
	void boost(u16 *dst, const u16 *src, u32 len, s32 boost_amount);
	void invert(u16 *dst, const u16 *src, u32 len, s32 invert_amount);
	void color_boost(u16 *dst, const u16 *src, u32 len, s32 boost_amount);
	void colorize(u16 *dst, const u16 *src, u32 len, u32 color, s32 colorize_amount);
	void add(u16* dst, const u16* src, int count, u16 add);
	void bw(u16* dst, const u16* src, int count);
	void make_fadepal(u16 *dst, const u16 *src, const u16 dst_col);

}

#endif // _PALETTE_H_
