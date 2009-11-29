#include "main.h"	
#include "spal_reorganiser.h"

namespace {
	Color8 col_merge(Color8 v0, Color8 v1) {
		Color8 ret;
		ret.rgb[0] = (int(v0.rgb[0]) + int(v1.rgb[0])) >> 1;
		ret.rgb[1] = (int(v0.rgb[1]) + int(v1.rgb[1])) >> 1;
		ret.rgb[2] = (int(v0.rgb[2]) + int(v1.rgb[2])) >> 1;
		return ret;
	}

	Color8 col_modde(Color8 v0, Color8 fogc, float fogv) {
		int r = v0.rgb[0]&0xFF;
		int g = v0.rgb[1]&0xFF;
		int b = v0.rgb[2]&0xFF;
		int fr = fogc.rgb[0]&0xFF;
		int fg = fogc.rgb[1]&0xFF;
		int fb = fogc.rgb[2]&0xFF;
		r = int(r*(1.f-fogv) + (fr*fogv));
		g = int(g*(1.f-fogv) + (fg*fogv));
		b = int(b*(1.f-fogv) + (fb*fogv));
		return Color8(r, g, b);
	}

}

void task_shiftprepal() {
	Color8 fogc;
	float fogv;
	{
		int r, g, b;
		sscanf(args[0].c_str(), "%2x%2x%2x", &r, &g, &b);
		fogc.rgb[0]=r;
		fogc.rgb[1]=g;
		fogc.rgb[2]=b;
		sscanf(args[1].c_str(), "%f", &fogv);
	}

	// concat
	vector<Color8> dst(8);

	// farg
	for (int i=0; i<8; i++) {
		int iter=i;
		float foggv = pow(iter/7.f, fogv);
		dst[i] = col_modde(Color8(255,255,255), fogc, foggv);
	}
	// lagre
	saveAct(dst);

}

void task_shiftpal() {
	Color8 fogc;
	float fogv;
	int reserved;
	{
		int r, g, b;
		sscanf(args[1].c_str(), "%2x%2x%2x", &r, &g, &b);
		fogc.rgb[0]=r;
		fogc.rgb[1]=g;
		fogc.rgb[2]=b;
		sscanf(args[2].c_str(), "%f", &fogv);
		sscanf(args[3].c_str(), "%d", &reserved);
	}

	if (palette.size()==256) {
		palette.resize(128);
	} else if (palette.size()!=128) {
		error("Palette must be of 128 colors!");
	}


	spal_init();
	std::vector<Vector3> spal;
	FORI(palette.size()) {
		unsigned char* rgb = palette[i].rgb;
		spal.push_back( Vector3(rgb[0], rgb[1], rgb[2]) );
	}

	// reorganiser
	spal_reorganiser(&spal[0], spal.size(), reserved);

	// concat
	vector<Color8> dst(256);
	for (int i=0; i<128; i++) {
		Color8& c = dst[i+128];
		c.rgb[0] = spal[i].x;
		c.rgb[1] = spal[i].y;
		c.rgb[2] = spal[i].z;
	}

	//sammensmelt
	{
		int l=128;
		int r=256;
		while (l>0) {
			for (int i=l; i<r; i+=2) {
				Color8 v0 = dst[i];
				Color8 v1 = dst[i+1];
				dst[i>>1] = col_merge(v0, v1);
			}
			l>>=1;
			r>>=1;
		}
	}
	// farg
	{
		int iter=1;
		int l=128;
		int r=256;
		while (l>0) {
			float foggv = pow(iter/7.f, fogv);
			if (foggv>1) foggv=1;
			for (int i=l; i<r; i+=2) {
				Color8& v0 = dst[i>>1];
				v0 = col_modde(v0, fogc, foggv);
			}
			iter++;
			l>>=1;
			r>>=1;
		}
	}
	// lagre
	saveAct(dst);
}