#include "support.h"

namespace decomp {
	void  huff_decompress(u8* obuf, int oSize, const u8* huff_base, const u8* huff_offs, const u8* huff_symb, const u8* huff_cache, const u32* ibuf);
	void rleImage4shift(u8* dst, u8* dst_end, const u16* rleData, int fade=0);
	void rleImage4add(u8* dst, u8* dst_end, const u16* rleData);
	void rleImage4mov(u8* dst, u8* dst_end, const u16* rleData, int color_key=-1);
	u16* updateIndexList(u16* dst, const u16* cmds, int cmds_size, const u16* prev);
};
