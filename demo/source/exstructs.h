#pragma once

#define PREC_VEC	4
#define PREC_NORM	8
#define PREC_MVP	16



struct RomMatrix3x3 {
	signed short m[9];
	bool operator!=(const RomMatrix3x3& op) {
		for (int i=0; i<9; i++) if (m[i]!=op.m[i]) return true;
		return false;
	}
};
/*
struct RomMatrix3 {
	signed short m[12];
	bool operator!=(const RomMatrix3& op) {
		for (int i=0; i<12; i++) if (m[i]!=op.m[i]) return true;
		return false;
	}
};
*/
struct RomMatrix4 {
	signed int m[16];
	bool operator!=(const RomMatrix4& op) {
		for (int i=0; i<16; i++) if (m[i]!=op.m[i]) return true;
		return false;
	}
	const signed int& operator[](int i) const {
		return m[i];
	} 
};

struct RomVertex {
	unsigned char normal;
	struct RomMatrixRef {
		unsigned char i; // index
		unsigned char w; // weight
	};
	RomMatrixRef matrix[3];
	signed short x, y, z;
	bool operator<(const RomVertex& op) const {
		if (x!=op.x) return x<op.x;
		if (y!=op.y) return y<op.y;
		if (z!=op.z) return z<op.z;
		return matrix<op.matrix;
	}
};
struct RomFace {
	unsigned short mtl;
	unsigned short v[3];
	unsigned short uv[3]; 
};

struct RomUvs {
	signed short u, v;
	bool operator<(const RomUvs& op) const {
		unsigned int a = (int(v)<<16)|u;
		unsigned int b = (int(op.v)<<16)|op.u;
		return a<b;
	}
};

struct RomFrame {
	unsigned short pframe;
	const unsigned short* activeverts;
	const unsigned short* activefaces;
//	const unsigned short* activesilhouette;
	const RomMatrix4* const* mvps; 
	const signed short (*bumpvecs)[3];
	const signed short (*lights)[3];
	const unsigned short* shadow;
};


enum Filler {
	FILLER_TEX256,
	FILLER_EBUMP,
	FILLER_NMAP,
	FILLER_SHIFT,
};

enum UvGen {
	UVGEN_UVS,
	UVGEN_ENV,
};

struct Material {
	Filler filler;
	UvGen uvGen;
	const unsigned char* tex[2];
	unsigned char palOffs;
	unsigned char oneone;
};


/*
// cmd: keep, new, discard
struct RomFrame {
	RomMatrix4 mvps[];

	// active lists modifications
	unsigned short  avert_size;
	unsigned short  apoly_size;
	unsigned short* averti_mod;  // active verts indices
	unsigned short* avertcc_mod; // active verts clipcodes
	unsigned short* apolyi_mod;  // active polys - sorted ;)
};
*/




// unsigned char u, v; i stan
//typedef unsigned int RomUvs; // UUUUUUUuuuuuuu0000VVVVVVVvvvvvvv

