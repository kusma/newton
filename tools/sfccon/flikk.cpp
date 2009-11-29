#include "main.h"	
#include "topflc/topflc.h"
#include "compress.h"
#include <assert.h>
#include <map>
#include <math.h>
using namespace std;

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


void task_flikk() {
	FIMULTIBITMAP* anim = inputanim;

	float huff_thresh=0.5f;
//	int nevner=1;
//	int bias=0;
	sscanf(args[1].c_str(), "%f", &huff_thresh);
//	sscanf(args[2].c_str(), "%i", &nevner);
//	sscanf(args[3].c_str(), "%i", &bias);
	
	bool huffmanActive = huff_thresh>0.01f;
	int frameCount = FreeImage_GetPageCount(anim);
	int width=240;
	int height=160;
/*	{
		FIBITMAP* fiBitmap = FreeImage_LockPage(anim, 0);
		width  = FreeImage_GetWidth(fiBitmap);
		height = FreeImage_GetHeight(fiBitmap);
		FreeImage_UnlockPage(anim, fiBitmap, FALSE);
	}
*/	fprintf(stderr, "Loaded anim %ix%i, %i frames\n", width, height, frameCount);

	std::vector<unsigned short> rleCompFrame_prev;
	std::vector<unsigned short> rleCompFrame;
	
	u8* buff = new u8[width*height];

	vector< vector<unsigned short> > finalCompFrames;
	finalCompFrames.resize(frameCount);

	rleCompFrame.clear();
	int rleMaxBufferSize = 0;
	for (int fi=0; fi<frameCount; fi++) {
		// Load bitmap and convert to 8bpp
		FIBITMAP* fiBitmap_orig = FreeImage_LockPage(anim,  fi);
		FIBITMAP* fiBitmap = FreeImage_ConvertTo8Bits(fiBitmap_orig);
		FreeImage_UnlockPage(anim, fiBitmap_orig, FALSE);

		// WTF ???
	//	FIBITMAP* fiBitmap = FreeImage_ConvertTo8Bits(fiBitmap_orig);
	//	FreeImage_UnlockPage(anim, fiBitmap_orig, FALSE);
		
		int freeImageWidth = FreeImage_GetWidth(fiBitmap);
		int freeImageHeight = FreeImage_GetHeight(fiBitmap);

//		printf("%i %i\n", freeImageWidth , freeImageHeight );
	//	assert( FreeImage_GetWidth(fiBitmap) == width );
	//	assert( FreeImage_GetHeight(fiBitmap) == height );

		// copy to buff (do fancy stuff here then)
		memset(buff, 0, width*height); 
		for (int y=0; y<height && y<freeImageHeight; y++) {
			BYTE* src = FreeImage_GetScanLine(fiBitmap, y);
			for (int x=0; x<width && x<freeImageWidth; x++) {
				int v = src[x];///nevner + bias;
				//BYTE v;
				//FreeImage_GetPixelIndex(fiBitmap, x, y, &v);
				buff[x+y*width] = v;
			}
		}
	
		

		// make that buff lo-fo (last minute optimize)
//		for (int i=0; i<animinfo.Width*animinfo.Height; i+=2) {
//			u8& a = buff[i];
//			u8& b = buff[i+1];
//			a = (int(a)+int(b))>>1;
//			b = a;
//		}

		std::vector<unsigned short>& finalCompFrame = finalCompFrames[fi];
		rleCompFrame.clear();
		comp::rleImage(rleCompFrame, buff, buff+height*width, 4, true); // this is where i change color depth
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
	finalHuffmanFrames.resize(frameCount);

	// find "optimal" set of frames for huffman encoding
	vector< bool > framesUseHuffman;
	framesUseHuffman.resize(frameCount);
	FORI(frameCount) framesUseHuffman[i] = huffmanActive;
	if (huffmanActive) 
		for (int decideUseHuffmanCount=3; decideUseHuffmanCount; decideUseHuffmanCount--) {

		// get frequency counts
		bool thereAreSome=false;
		FORI(N) freq[i]=0;
		FORI(frameCount) {
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
		for (int framei=0; framei<frameCount; framei++) {
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
	for (int framei=0; framei<frameCount; framei++) {
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
	fwrite(&frameCount, 1, 2, stdout);
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


