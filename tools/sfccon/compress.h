#pragma once
#include "main.h"
#include <map>
#include <vector>

namespace comp {
	void rleImage(vector<unsigned short>& dst, const u8* src, const u8* src_end, int bpp, bool reserveMSB);
	

	void updateList(vector<unsigned short>& dst, vector<unsigned short>& prev, vector<unsigned short>& curr);
	void updateListPFrame(vector<unsigned short>& dst, vector<unsigned short> b);

/*
	struct HuffCode {
		int  length;
		int	 code;
	};


	static inline void huff_getFrequencyCount (std::map<unsigned char, int>& frequency_count, unsigned char* data, int data_length)
	{
		FORI(data_length) {
			frequency_count[*data++]++;
		}
	}

	void   huff_getCodeMap(std::map<unsigned char, HuffCode >& codeMap, const std::map<unsigned char, int>& frequency_count_in);
	void   huff_compress(std::vector<unsigned char>& ofile, std::map<unsigned char, comp::HuffCode >& codeMap, const unsigned char* ifile, int ifile_size);

	void   huff_getDecompTree(short decompTree_out[512], const std::map<unsigned char, comp::HuffCode >& codeMap);
	unsigned char*  huff_decompress(unsigned char* dst, unsigned char* src, int file_size);
*/
};
