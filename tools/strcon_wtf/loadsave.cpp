#include "main.h"
#include "topflc/topflc.h"


vector<string> args;
FILE* inputbin;
int   inputbinSize;
CImage inputimage;
vector<Color8> palette;
vector<Vector3> palette_float;
TFAnimation* inputanim = NULL;

void saveAct(vector<Color8>& palette) {
	// lagre palett som ACT
	int i=0;
	for (i=0; i<min((int)palette.size(), 256); i++) {
		fwrite(palette[i].rgb, 1, 3, stdout);
	}
	if (i!=256) {
		for (; i<256; i++) {
			putchar(0);
			putchar(0);
			putchar(0);
		}
		putchar(0);
		putchar((int)palette.size());
		putchar(255);
		putchar(255);
	}
	fprintf(stderr, "Saved palette %i colors\n", int(palette.size()));
}

void loadBinary(std::string filename) {
	FILE* fil = fopen(filename.c_str(), "rb");
	if (!fil) {
		error("File not found: %s\n\n", args[0].c_str());
	}
	fseek(fil, 0, SEEK_END);
	int size = ftell(fil);
	rewind(fil);
	inputbin = fil;
	inputbinSize = size;
	fprintf(stderr, "Loaded binary %i bytes\n", inputbinSize);

}



void loadAct(std::string filnavn) {
	// mulighet for å koble ting sammen med +
	int nPos = filnavn.find_last_of("+");
	if (nPos>=0) {
		loadAct(filnavn.substr(0, nPos));
		loadAct(filnavn.substr(nPos+1));
		return;
	}

	bool isGbaPal = false;
	bool isXPal = false;
	if (filnavn.size() && filnavn[0] == '#') {
		isGbaPal=true;
		filnavn = filnavn.c_str()+1;
	} else if (filnavn.size() && filnavn[0] == '@') {
		isXPal=true;
		filnavn = filnavn.c_str()+1;
	}

	HRESULT ret = inputimage.Load(filnavn.c_str());

	if (!FAILED(ret)) {
		// decode palette
		if (inputimage.GetBPP()>=24) {
			fprintf(stderr, "Loading truecolor palette from pixel image..\n");
			FORI(inputimage.GetWidth()) {
				COLORREF c = inputimage.GetPixel(i, 0); 
				Color8  icol( GetRValue(c), GetGValue(c), GetBValue(c) );
				Vector3 fcol( GetRValue(c), GetGValue(c), GetBValue(c) );
				fcol *= 1.0/255;
				::palette.push_back(icol);
				::palette_float.push_back( fcol );
			}
		} else if (inputimage.GetMaxColorTableEntries()>0) {
			fprintf(stderr, "Getting palette from paletted image..\n");
			RGBQUAD pal[256];
			inputimage.GetColorTable(0, inputimage.GetMaxColorTableEntries(), pal);
			for (int i=0; i<inputimage.GetMaxColorTableEntries(); i++) {
				Color8  icol(pal[i].rgbRed, pal[i].rgbGreen, pal[i].rgbBlue);
				Vector3 fcol(pal[i].rgbRed, pal[i].rgbGreen, pal[i].rgbBlue);
				fcol *= 1.0/255;
				::palette.push_back(icol);
				::palette_float.push_back( fcol );
			}
		} else {
			error("Could not load palette");
		}
		inputimage.Destroy();
	} else if (isGbaPal) {
		fprintf(stderr, "Loading GBA palette..\n");
		FILE* fil = fopen(filnavn.c_str(), "rb");
		if (!fil) error("File not found: %s\n\n", filnavn.c_str());

		palette.clear();
		while (1) {
			unsigned short hei;
			size_t ret = fread(&hei, 1, 2, fil);
			if (!ret) break;

			int r = hei&0x1F;		
			int g = (hei>>5)&0x1F;		
			int b = (hei>>10)&0x1F;		
			r = (r<<3)|(r>>2);
			g = (g<<3)|(g>>2);
			b = (b<<3)|(b>>2);

			Color8  icol(r, g, b);
			Vector3 fcol(r, g, b);
			fcol *= (1.0/255);

			palette.push_back(icol);
			::palette_float.push_back( fcol );
		}
		fclose(fil);
	} else if (isXPal) {
		fprintf(stderr, "Loading HDR XPal palette..\n");
		FILE* fil = fopen(filnavn.c_str(), "rb");
		if (!fil) error("File not found: %s\n\n", filnavn.c_str());

		// read size
//		int count=0;
//		fread(&count, 2, 1, fil);
//		palette.resize(count);
//		FORI(count) {
		while (1) {
			unsigned short hei[4];
			size_t ret = fread(hei, 2, 4, fil);
			if (!ret) break;//error("File is too short!");

			int r = hei[1];
			int g = hei[2];
			int b = hei[3];

			Color8  icol( r>>8, g>>8, b>>8 );
			Vector3 fcol( r, g, b );
			fcol *= (1.0/0xFFFF);

			::palette.push_back( icol );
			::palette_float.push_back( fcol );
		}
		fclose(fil);
	} else {
		fprintf(stderr, "Loading 24 bit ACT palette..\n");
		FILE* fil = fopen(filnavn.c_str(), "rb");
		if (!fil) error("File not found: %s\n\n", filnavn.c_str());

		Color8 pal[256];
		int count=0;
		FORI(256) {
			Color8& col = pal[i];
			size_t ret = fread(col.rgb, 1, 3, fil);
			if (ret!=3) break;
			count++;
		}
		if (count==256 && getc(fil)!=EOF) {
			count = (unsigned char)getc(fil);
		}
		FORI(count) {
			Color8& icol = pal[i];
			int r = icol.rgb[0];
			int g = icol.rgb[1];
			int b = icol.rgb[2];
			Vector3 fcol(r, g, b);
			fcol *= (1.0/0xFF);
			::palette_float.push_back( fcol );
			::palette.push_back( icol );
		}
	}

	fprintf(stderr, "Loaded %i colors\n", palette.size());
}


void loadImage(std::string filnavn) {
	HRESULT ret = inputimage.Load(filnavn.c_str());
	if (FAILED(ret)) error("Can't load picture %s\n\n", filnavn.c_str());
	if (inputimage.GetMaxColorTableEntries()>0) {
		RGBQUAD pal[256];
		inputimage.GetColorTable(0, inputimage.GetMaxColorTableEntries(), pal);
		for (int i=0; i<inputimage.GetMaxColorTableEntries(); i++) {
			Color8 col(pal[i].rgbRed, pal[i].rgbGreen, pal[i].rgbBlue);
			::palette.push_back(col);
		}
	}
	fprintf(stderr, "Loaded picture %ix%i, %ibpp\n", inputimage.GetWidth(), inputimage.GetHeight(), inputimage.GetBPP());
}


void flic_handler(char *msg)
{
	error("topflc: %s\n", msg);
};

void loadFlc(std::string filnavn) {	
	TFErrorHandler_Set(flic_handler);
	inputanim = TFAnimation_NewFile((char*)filnavn.c_str());

	TFAnimation_SetLooping(inputanim, TF_FALSE);
	TFBuffers_Alloc(inputanim);
	TFFrame_Seek(inputanim, 0);

	TFAnimationInfo animinfo;
	TFAnimation_GetInfo(inputanim, &animinfo);
	fprintf(stderr, "Loaded flic %ix%i, %i frames\n", animinfo.Width, animinfo.Height, animinfo.NumFrames);
}

