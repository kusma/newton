#include "main.h"	
#include "topflc/topflc.h"
#include "compress.h"
#include <assert.h>
#include <map>
#include <math.h>
using namespace std;

typedef unsigned int uint32;
typedef unsigned char uchar;

#include "shtypes.h"
#include "shc.h"




void  huff_decompress(unsigned char* dst, const short decomp_tree[512], const unsigned char* src, int file_size)
{
	register unsigned short  cindex = 1;
	register short           bitshift;

	int charcount = 0;
	while (charcount < file_size) {
		unsigned char curchar = *src++;

		for (bitshift = 7; bitshift >= 0; --bitshift) {
			cindex = (cindex << 1) + ((curchar >> bitshift) & 1);

			if (decomp_tree[cindex] <= 0) {
				*dst++ = (int) (-decomp_tree[cindex]);


				if ((++charcount) == file_size)
					bitshift = 0;
				else
					cindex = 1;
			} else {
				cindex = decomp_tree[cindex];
			}
		}
	}
}


u16* updateIndexList(u16* dst, const u16* data, const u16* prev)
{
	int data_size = data[0];
	data++;

	int i=0;
	int dsti=0;
	while (i<data_size) {
		if (data[i]&0x8000) {
			int match = data[i++]&0x7FFF;
			int count = s16(data[i++]);
			int direction = (count>0) ? 1 : -1;
			for (int j=match; j!=match+count; j+=direction) {
				assert(j>=dsti);
				dst[dsti++] = prev[j];
			}
		} else {
			dst[dsti++] = data[i++];
		}
	}
	return dst+dsti;
}

void task_flikk_priv(bool shadowflikk, float huff_thresh);

void task_flikk() {
	float huffthresh = 0.8f;
	sscanf(args[2].c_str(), "%f", &huffthresh);
	if (args[1]=="1") task_flikk_priv(true, huffthresh);
	else if (args[1]=="0") task_flikk_priv(false, huffthresh);
	else error("arg 2 må være 0 eller 1. 0 for vanlig flikk, 1 for shadowflikk (fargen invers og halvert");
}

void task_dblflikk() {
	float huffthresh = 0.8f;
	sscanf(args[4].c_str(), "%f", &huffthresh);
	if (args[3]=="1") task_flikk_priv(true, huffthresh);
	else if (args[3]=="0") task_flikk_priv(false, huffthresh);
	else error("arg 4 må være 0 eller 1. 0 for vanlig flikk, 1 for shadowflikk (fargen invers og halvert");
}

#define log2(x) (log10((double)(x))/log10(2.0))

double sh_CalcEntropy(uint32 *freq) {
	double ent, prob;
	uint32 totfreq;
	int i;

	for(totfreq=i=0; i<N; i++) totfreq+=freq[i];

	for(ent=0.0, i=0; i<N; i++)
		if(freq[i]){
			prob=(double)freq[i]/totfreq;
			ent+=-prob*log2(prob);
		}

		return ent;
}


void task_flikk_priv(bool shadowflikk, float huff_thresh) {
	TFAnimationInfo animinfo;
	TFAnimation_GetInfo(inputanim, &animinfo);	

	bool huffmanActive = huff_thresh>0.01f;

	TFAnimationInfo info_a;	
	TFAnimation* anim_a = NULL;
	TFAnimationInfo info_b;	
	TFAnimation* anim_b = NULL;
	if (args.size()==5) {
		anim_a = TFAnimation_NewFile((char*)args[1].c_str());
		anim_b = TFAnimation_NewFile((char*)args[2].c_str());
	}
	if (anim_a) {
		TFAnimation_SetLooping(anim_a, TF_FALSE);
		TFBuffers_Alloc(anim_a);
		TFFrame_Seek(anim_a, 0);
		TFAnimation_GetInfo(anim_a, &info_a);
		fprintf(stderr, "Loaded flic A %ix%i, %i frames\n", info_a.Width, info_a.Height, info_a.NumFrames);
	}
	if (anim_b) {
		TFAnimation_SetLooping(anim_b, TF_FALSE);
		TFBuffers_Alloc(anim_b);
		TFFrame_Seek(anim_b, 0);
		TFAnimation_GetInfo(anim_b, &info_b);
		fprintf(stderr, "Loaded flic B %ix%i, %i frames\n", info_b.Width, info_b.Height, info_b.NumFrames);
	}

	std::vector<unsigned short> rleCompFrame_prev;
	std::vector<unsigned short> rleCompFrame;
	

	u8* buff = new u8[animinfo.Width*animinfo.Height];

	TFFrame_Seek(inputanim, 0);

	const int numFrames = animinfo.NumFrames;


	vector< vector<unsigned short> > finalCompFrames;
	finalCompFrames.resize(numFrames);

	rleCompFrame.clear();
	int rleMaxBufferSize = 0;
	for (int fi=0; fi<numFrames; fi++) {
		TFFrame_Decode(inputanim);
		if (anim_a) TFFrame_Decode(anim_a);
		if (anim_b) TFFrame_Decode(anim_b);
		fprintf(stderr, "%i\n", fi);

		std::vector<unsigned short>& finalCompFrame = finalCompFrames[fi];

		FORI(animinfo.Width*animinfo.Height) {
			u8 index = animinfo.Frame[i];
			assert(animinfo.Palette);
			u8* c = (*animinfo.Palette)[index];

			u8 val_a, val_b;
			u8 val = (u32(c[0])+u32(c[1])+u32(c[2])) / 3;
			val = (val<<2)|(val>>4);
	
			if (anim_a) {
				u8 index_a = info_a.Frame[i];
				assert(info_a.Palette);
				u8* c_a = (*info_a.Palette)[index_a];
				val_a = (u32(c_a[0])+u32(c_a[1])+u32(c_a[2])) / 3;
				val_a = (val_a<<2)|(val_a>>4);
			}

			if (anim_b) {
				u8 index_b = info_b.Frame[i];
				assert(info_b.Palette);
				u8* c_b = (*info_b.Palette)[index_b];
				val_b = (u32(c_b[0])+u32(c_b[1])+u32(c_b[2])) / 3;
				val_b = (val_b<<2)|(val_b>>4);
			}

			

			if (anim_a && anim_b) {
				if (shadowflikk) val = 255-val;
				// skygge og maske er hvit på svart
				int skyggeverdi = (val_a*val_b)>>8;
				
				// jaja
				val = max(s32(val),s32(skyggeverdi));
				//val = skyggeverdi;
			} else if (anim_a) {
				// bare multipliser :P
				val = (val * val_a)>>8;
				if (shadowflikk) val = 255-val;
			} else {
				if (shadowflikk) val = 255-val;
			}

			if (shadowflikk) {
				val = (val * 0xA0) >> 8;
				val >>= 1;
			}
			buff[i] = val;
		}
	
		// make that buff lo-fo
		for (int i=0; i<animinfo.Width*animinfo.Height; i+=2) {
			u8& a = buff[i];
			u8& b = buff[i+1];
			a = (int(a)+int(b))>>1;
			b = a;
		}

		rleCompFrame.clear();
		comp::rleImage(rleCompFrame, buff, buff+animinfo.Height*animinfo.Width, 4, true);
		FORI(rleCompFrame.size()) assert(!(rleCompFrame[i]>>15));

		if (fi==0) {
			comp::updateListPFrame(finalCompFrame, rleCompFrame);
		} else {
			comp::updateList(finalCompFrame, rleCompFrame_prev, rleCompFrame);
		}

		if (rleCompFrame.size()*2>rleMaxBufferSize) {
			rleMaxBufferSize=rleCompFrame.size()*2;
			fprintf(stderr, "RLE Buffer Size: %i\n", rleMaxBufferSize);
		}

		rleCompFrame_prev = rleCompFrame;
	}


	/* allocate memory for encoder (1280 bytes) */
	uint32* freq=(uint32 *)malloc(N*sizeof(uint32));
	uchar*  symb=(uchar *)malloc(N*sizeof(uchar));
	uchar*  len=(uchar *)malloc(N*sizeof(uchar));
	uchar*  base=(uchar *)malloc(32*sizeof(uchar));
	uchar*  offs=(uchar *)malloc(32*sizeof(uchar));
	uchar*  cache=(uchar *)malloc(SH_CACHESIZE*sizeof(uchar));
	uchar*  code = (uchar *)malloc(N*sizeof(uchar));


	// for huffman compressed frames
	std::vector< std::vector<uint32> > finalHuffmanFrames;
	finalHuffmanFrames.resize(numFrames);

	// find "optimal" set of frames for huffman encoding
	vector< bool > framesUseHuffman;
	framesUseHuffman.resize(numFrames);
	FORI(numFrames) framesUseHuffman[i] = huffmanActive;
	if (huffmanActive) 
		for (int decideUseHuffmanCount=3; decideUseHuffmanCount; decideUseHuffmanCount--) {

		// get frequency counts
		bool thereAreSome=false;
		FORI(N) freq[i]=0;
		FORI(numFrames) {
			if (!framesUseHuffman[i]) continue;
			thereAreSome=true;
			unsigned char* charstr = (unsigned char*)&(finalCompFrames[i])[0];
			FORJ(finalCompFrames[i].size()*2) {
				freq[*charstr++]++;
			}
		}
		if (!thereAreSome) break;

		/* calculate entropy */
		double ent = sh_CalcEntropy(freq);

		/* sort symbols by frequency */
		int n = sh_SortFreq(freq, symb);
		
		/* calculate code lengths */
		sh_CalcLen(freq, symb, len, n, 31);

		/* sort symbols by code length & actual value */
		sh_SortLen(len, symb, n);

		/* calculate canonical huffman codes */
		sh_CalcCode(len, symb, code, n);

		/* calculate decode tables */
		sh_CalcDecode(len,symb,base,offs,cache,n);

		// compress everything
		for (int framei=0; framei<numFrames; framei++) {
			std::vector<unsigned short>& finalCompFrame = finalCompFrames[framei];
			std::vector<uint32>&  finalHuffmanFrame = finalHuffmanFrames[framei];

			finalHuffmanFrame.resize(finalCompFrame.size()*7);
			uchar*  ibuf = (uchar*)&finalCompFrame[0];
			uint32* obuf = (uint32*)&finalHuffmanFrame[0];

			int bits = SH_MAXLENGTH; 
			uint32 bitbuf=0; 
			uint32 osize=1;
			uint32 obufpos = 0;

			/* encoding loop */
			for(int ibufpos=0; ibufpos<finalCompFrame.size()*2; ibufpos++) {
				ENCODE_SYMB(len, code, ibuf[ibufpos], obuf, obufpos, bits, bitbuf);
			}
			bitbuf<<=bits;
			obuf[obufpos++] = bitbuf;
			finalHuffmanFrame.resize(obufpos);

			unsigned short huffmanSize = finalHuffmanFrame.size()*4;
			unsigned short frameSize = finalCompFrame.size()*2;
			float bettersize = float(huffmanSize)/frameSize;
			framesUseHuffman[framei] = (finalCompFrame.size()>4) && (bettersize<huff_thresh);
		}
	}

	// now, find max buffer size for decompressed stuff from huffman stuff 
	int decodeMaxBufferSize = 0;
	for (int framei=0; framei<numFrames; framei++) {
		std::vector<unsigned short>& finalCompFrame = finalCompFrames[framei];

		if (framesUseHuffman[framei] && finalCompFrame.size()*2>decodeMaxBufferSize) {
			decodeMaxBufferSize=finalCompFrame.size()*2;
			fprintf(stderr, "Decode Buffer Size: %i\n", decodeMaxBufferSize);
		}
	}


	// write huffman active flag
	{
		int huffActiveInt = (huffmanActive)?1:0;
		fwrite(&huffActiveInt,  1, 4, stdout);
	}

	if (huffmanActive) {
		// write decomp data (print dem. finn feil.)
		fwrite(base,  32, 1, stdout);
		fwrite(offs,  32, 1, stdout);
		fwrite(symb, N, 1, stdout);
		fwrite(cache, SH_CACHESIZE, 1, stdout);
	}

	// for padding;
	int dummy = -1;

	// now write more stuff
	fwrite(&decodeMaxBufferSize, 1, 2, stdout);
	fwrite(&rleMaxBufferSize, 1, 2, stdout);
	fwrite(&numFrames, 1, 2, stdout);
	fwrite(&dummy, 1, 2, stdout); // bogus for word-alignment
	for (int framei=0; framei<finalCompFrames.size(); framei++) {
		std::vector<unsigned short>& finalCompFrame = finalCompFrames[framei];
		std::vector<uint32>& finalHuffmanFrame = finalHuffmanFrames[framei];

		unsigned short huffmanSize = finalHuffmanFrame.size()*4;
		unsigned short frameSize = finalCompFrame.size()*2;

		bool use_huffman = framesUseHuffman[framei];


		if (use_huffman) {
			fwrite(&huffmanSize, 1, 2, stdout);
		} else {
			fwrite(&dummy, 1, 2, stdout);
		}


		fwrite(&frameSize, 1, 2, stdout);
		fprintf(stderr, "huffman size: %i\n", huffmanSize);
		fprintf(stderr, "decode size: %i\n", frameSize);
		fprintf(stderr, "use_huffman: %i\n", use_huffman);

		if (use_huffman) {
			fwrite(&finalHuffmanFrame[0], huffmanSize, 1, stdout);
		} else {
			fwrite(&finalCompFrame[0], frameSize, 1, stdout);
			if (frameSize%4) fwrite(&dummy, 1, 2, stdout);
		}

		// test decode
		if (use_huffman) {

			unsigned short* testbuff = new unsigned short[frameSize*2];

			// init
			uint32* ibuf = &finalHuffmanFrame[0];
			uchar*  obuf = (uchar*)testbuff;


			/* due to encoder convention, set bits to 31 instead of 32 */
			int bits=SH_MAXLENGTH;
			uint32 ibufpos=0;
			uint32 obufpos=0;
			

			/* read first portion of data, init bitbuf */
			uint32 bitbuf=ibuf[ibufpos++];


			// decoding loop
			uint32 symbleft=frameSize;
			while(symbleft--) {
				int symbol;
				DECODE_SYMB(base,offs,cache,symb,ibuf,ibufpos,bits,bitbuf,symbol);
				obuf[obufpos++]=symbol; 
			}

			if (obufpos!=frameSize) {
					fprintf(stderr, "hm %i og %i\n", obufpos, frameSize);
			}
			
			FORI(finalCompFrame.size()) {
				if (testbuff[i]!=finalCompFrame[i]) {
					fprintf(stderr, "feil at %i\n", i);
				}
			}
		}
	}




//	FORI(fileData.size())  fwrite(&fileData[i], 1, 2, stdout);

}


