#pragma once

#define PREC_VEC	4 // should be 4
#define PREC_NORM	8
#define PREC_MVP	14

enum Filler {
	FILLER_TEX256,
	FILLER_EBUMP,
	FILLER_NMAP,
};

enum UvGen {
	UVGEN_UVS,
	UVGEN_ENV,
};



#define CMD_KEEP	(0<<14)
#define CMD_REPLACE (1<<14)
#define CMD_DELETE  (2<<14)
#define CMD_INSERT  (3<<14)
#define CMDMASK		(3<<14)

struct RomMatrix3 {
	signed short m[12];
	bool operator!=(const RomMatrix3& op) {
		for (int i=0; i<12; i++) if (m[i]!=op.m[i]) return true;
		return false;
	}
};

struct RomMatrix4 {
	signed int m[16];
	bool operator!=(const RomMatrix4& op) {
		for (int i=0; i<16; i++) if (m[i]!=op.m[i]) return true;
		return false;
	}
};


struct RomVertex {
	struct RomMatrixRef {
		unsigned char i; // index
		unsigned char w; // weight
	};
	unsigned char isNormal; // 0=vertex, 1=normal
	RomMatrixRef matrix[3];
	signed short x, y, z;
	bool operator<(const RomVertex& op) const {
		if (x!=op.x) return x<op.x;
		if (y!=op.y) return y<op.y;
		if (z!=op.z) return z<op.z;
		for (int i=0; i<3; i++) {
			if (matrix[i].i!=op.matrix[i].i) return matrix[i].i<op.matrix[i].i;
			if (matrix[i].w!=op.matrix[i].w) return matrix[i].w<op.matrix[i].w;
		}
		return false;
	}
};

struct RomFace {
	signed short mtl;
	unsigned short v[3];
	unsigned short uv[3]; 
	// some extra stuff
	int smGrp;
};

struct RomUvs {
	unsigned short u, v;
	bool operator<(const RomUvs& op) const {
		unsigned int a = (int(v)<<16)|u;
		unsigned int b = (int(op.v)<<16)|op.u;
		return a<b;
	}
};



struct RomFrame {
	int pframe;
	const unsigned short* activeverts;
	const unsigned short* activefaces;
	const RomMatrix4* const* mvp; 
	const signed short (*bumpvecs)[3];
	const signed short (*lights)[3];
	const unsigned short* shadow;
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

