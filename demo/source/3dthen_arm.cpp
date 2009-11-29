#include "3dthen.h"
#include "exstructs.h"
#include <gba_dma.h>
#include <mappy.h>
#include <math.h>
#include "math.h"
#include "poly.h"

INLINE void scale8_vec3(s32& x, s32& y, s32& w, s32 scale) {
	x = ((long long)x * scale)>>8;
	y = ((long long)y * scale)>>8;
	w = ((long long)w * scale)>>8;
}


INLINE void transform_vec3(s32& x, s32& y, s32& z, s32& w, s32 sx, s32 sy, s32 sz, const RomMatrix4& m) {
	// dest  
	x = (((long long)(sx*m[0x0]))>>PREC_VEC) + (((long long)(sy*m[0x1]))>>PREC_VEC) + (((long long)(sz*m[0x2]))>>PREC_VEC) + m[0x3];
	y = (((long long)(sx*m[0x4]))>>PREC_VEC) + (((long long)(sy*m[0x5]))>>PREC_VEC) + (((long long)(sz*m[0x6]))>>PREC_VEC) + m[0x7];
	z = (((long long)(sx*m[0x8]))>>PREC_VEC) + (((long long)(sy*m[0x9]))>>PREC_VEC) + (((long long)(sz*m[0xA]))>>PREC_VEC) + m[0xB];
	w = (((long long)(sx*m[0xC]))>>PREC_VEC) + (((long long)(sy*m[0xD]))>>PREC_VEC) + (((long long)(sz*m[0xE]))>>PREC_VEC) + m[0xF];
}

INLINE void addbone_vec3(s32& x, s32& y, s32& w, const RomVertex& sv, const RomMatrix4& m, int scale) {
	const s32 sx = sv.x;
	const s32 sy = sv.y;
	const s32 sz = sv.z;
	// dest  
	s32 tx = (((long long)(sx*m[0x0]))>>PREC_VEC) + (((long long)(sy*m[0x1]))>>PREC_VEC) + (((long long)(sz*m[0x2]))>>PREC_VEC) + m[0x3];
	s32 ty = (((long long)(sx*m[0x4]))>>PREC_VEC) + (((long long)(sy*m[0x5]))>>PREC_VEC) + (((long long)(sz*m[0x6]))>>PREC_VEC) + m[0x7];
	s32 tw = (((long long)(sx*m[0xC]))>>PREC_VEC) + (((long long)(sy*m[0xD]))>>PREC_VEC) + (((long long)(sz*m[0xE]))>>PREC_VEC) + m[0xF];
	scale8_vec3(tx, ty, tw, scale);
	x += tx;
	y += ty;
	w += tw;
}
INLINE void transform_norm3(s32& x, s32& y, s32& z, const RomVertex& sv, const RomMatrix4& m) {
	const s32 sx = sv.x; 
	const s32 sy = sv.y;
	const s32 sz = sv.z;
	// dest  
	x = (((long long)(sx*m[0x0]))>>PREC_NORM) + (((long long)(sy*m[0x1]))>>PREC_NORM) + (((long long)(sz*m[0x2]))>>PREC_NORM);
	y = (((long long)(sx*m[0x4]))>>PREC_NORM) + (((long long)(sy*m[0x5]))>>PREC_NORM) + (((long long)(sz*m[0x6]))>>PREC_NORM);
	z = (((long long)(sx*m[0x8]))>>PREC_NORM) + (((long long)(sy*m[0x9]))>>PREC_NORM) + (((long long)(sz*m[0xA]))>>PREC_NORM);
}


INLINE void addbone_norm3(s32& x, s32& y, s32& z, const RomVertex& sv, const RomMatrix4& m, int scale) {
	const s32 sx = sv.x; 
	const s32 sy = sv.y;
	const s32 sz = sv.z; 
	// dest  
	s32 tx = (((long long)(sx*m[0x0]))>>PREC_NORM) + (((long long)(sy*m[0x1]))>>PREC_NORM) + (((long long)(sz*m[0x2]))>>PREC_NORM);
	s32 ty = (((long long)(sx*m[0x4]))>>PREC_NORM) + (((long long)(sy*m[0x5]))>>PREC_NORM) + (((long long)(sz*m[0x6]))>>PREC_NORM);
	s32 tz = (((long long)(sx*m[0x8]))>>PREC_NORM) + (((long long)(sy*m[0x9]))>>PREC_NORM) + (((long long)(sz*m[0xA]))>>PREC_NORM);
	scale8_vec3(tx, ty, tz, scale);
	x += tx;
	y += ty;
	z += tz;
}



INLINE void project_vec3(s32& dx, s32& dy, s32 x, s32 y, s32 w) {
	s32 w_inv = rcp(w);
	dx = ((s64(x)*120*w_inv)>>28) + (120<<4);
	dy = ((s64(y)*-80*w_inv)>>28) + (80<<4);
}


INLINE void normalize_norm3(s32& x, s32& y, s32& z) {
	s32 len = fast_pythagoras(x, y, z);
	s32 len_rcp = rcp(len);
	x = MUL16(x, len_rcp);
	y = MUL16(y, len_rcp);
	z = MUL16(z, len_rcp);
}



#define MATRIXCACHE_SIZE 2

// Matrix cache
struct MatrixCache {
	struct StoredMatrix {
		StoredMatrix() : i(255) {}
		u8 i;
		u8 stamp;
		RomMatrix4 m;
	};

	const RomMatrix4* const* mvps0;
	const RomMatrix4* const* mvps1;
	int subframe;
	
	MatrixCache(const RomMatrix4* const* mvps0, const RomMatrix4* const* mvps1, int subframe) :
		mvps0(mvps0),
		mvps1(mvps1),
		subframe(subframe)
	{}
	
	StoredMatrix storedMats[MATRIXCACHE_SIZE];

	const RomMatrix4& getMatrix(int geti) __attribute__((noinline));
};



void antialias_edge(s32 x0, s32 y0, s32 x1, s32 y1, u8* target, const u8* shadeTable);
void antialias_vert(s32 x0, s32 y0,u8* target, const u8* shadeTable);



void Trede::draw(u8* dst) {
	// tegn polys
	for (u16* p=activefaces; p<activefaces_end; ) {
		int fi = *p++;
		RomFace f = rom_faces[fi];
		const Material& mtl = rom_mtls[f.mtl];

		PolyVertex vrpx[3];
		for (int i=0; i<3; i++) {
			const RamVertex& v = ram_verts[ f.v[i] ];
			vrpx[i].x = int(v.x)<<4;
			vrpx[i].y = int(v.y)<<4;
			// disse er 4:12.. hmm.. jeg vil ha 16:16 for tex256
			if (mtl.uvGen==UVGEN_UVS) {
				const RomUvs& uv = rom_uvs[ f.uv[i] ];
				vrpx[i].u = int(uv.u)<<4;
				vrpx[i].v = int(uv.v)<<4;
			} else if (mtl.uvGen==UVGEN_ENV) {
				const RamVertex& uv = ram_verts[ f.uv[i] ];
				vrpx[i].u = int(uv.x)<<4;
				vrpx[i].v = int(uv.y)<<4;
			}
		} 

		// init bump
		const s16* bumpVec=NULL;
		if (mtl.filler==FILLER_NMAP || mtl.filler==FILLER_EBUMP) {
			int mat = rom_verts[ f.v[0] ].matrix[0].i;
			bumpVec = fstate.bumpvecs[mat-1];
		}  
		polyfiller(dst, vrpx, mtl, bumpVec, letterbox, GBA_HEIGHT-letterbox); 

		// is there an antialias mask?
		if (p<activefaces_end && *p&0x4000) {
			int aaMask = *p++;
			
			if (shadeTable) {
				// aa edges
				for (int ei=0; ei<3; ei++) {
					int ei_next = (ei==2)?0:ei+1;
					PolyVertex& v0 = vrpx[ei];
					PolyVertex& v1 = vrpx[ei_next];
					if (aaMask&1) {
						antialias_edge(v0.x, v0.y, v1.x, v1.y, dst, shadeTable); 
					}
					aaMask>>=1;
				}
				// aa verts
				for (int vi=0; vi<3; vi++) {
					PolyVertex& v = vrpx[vi];
					if (aaMask&1) {
						antialias_vert(v.x, v.y, dst, shadeTable); 
					}
					aaMask>>=1;
				}
			}
		}

	}

	// tegn antialias
/*	if (shadeTable) { 
		const u16* activesilhouette=fstate.activesilhouette+1;
		const u16* activesilhouette_end=activesilhouette + 2*(*fstate.activesilhouette);
		for (const u16* p=activesilhouette; p<activesilhouette_end; p+=2) {
			u16 vi0 = p[0];
			u16 vi1 = p[1];
			const RamVertex& v0 = ram_verts[ vi0 ];
			const RamVertex& v1 = ram_verts[ vi1 ];

			int x0 = int(v0.x)<<4;
			int y0 = int(v0.y)<<4;
			int x1 = int(v1.x)<<4;
			int y1 = int(v1.y)<<4;
			
			antialias_edge(x0, y0, x1, y1, dst, shadeTable); 
			
		}
	}*/


}

#if 0
// new routines, not in use atm
static void transform_vertices(RamVertex* dst, const RomVertex* src, u32 vcount, const RomMatrix4 m)
{
	// fist pass, transform w and calculate 1 / w'
	{
		const int m30 = m[12];
		const int m31 = m[13];
		const int m32 = m[14];
		const int m33 = m[15];
		
		RamVertex *dst2 = dst;
		const RomVertex *src2 = src;
		for (int i = vcount; i; --i)
		{
			const int sx = src->x;
			const int sy = src->y;
			const int sz = src->z;
			const int w = 
				(((long long)sx * m30) >> PREC_VEC) +
				(((long long)sy * m31) >> PREC_VEC) +
				(((long long)sz * m32) >> PREC_VEC) +
				m33;
			dst2->w = rcp(w);
			src++;
			dst2++;
		}
	}

	// second pass, transform x and y, and calculate x' / w' and y' / w'
	{
		const int m00 = m[0];
		const int m01 = m[1];
		const int m02 = m[2];
		const int m03 = m[3];
		const int m10 = m[4];
		const int m11 = m[5];
		const int m12 = m[6];
		const int m13 = m[7];
		
		RamVertex *dst2 = dst;
		const RomVertex *src2 = src;
		for (int i = vcount; i; --i)
		{
			const int sx = src->x;
			const int sy = src->y;
			const int sz = src->z;
			const int x =
				(((long long)sx * m00) >> PREC_VEC) +
				(((long long)sy * m01) >> PREC_VEC) +
				(((long long)sz * m02) >> PREC_VEC) +
				m03;
			const int y =
				(((long long)sx * m10) >> PREC_VEC) +
				(((long long)sy * m11) >> PREC_VEC) +
				(((long long)sz * m12) >> PREC_VEC) +
				m13;
			const int w_inv = dst2->w;
			dst2->x = ((s64(x) * 120 * w_inv) >> 28) + (121 << 4);
			dst2->y = ((s64(y) * -80 * w_inv) >> 28) + (80 << 4);
			src++;
			dst2++;
		}
	}
}

static void transform_normals(RamVertex* dst, const RomVertex* src, u32 vcount, const RomMatrix4 m)
{
	// transform x, y and z, and normalize vector. store x and y
	const int m00 = m[0];
	const int m01 = m[1];
	const int m02 = m[2];
	const int m03 = m[3];
	const int m10 = m[4];
	const int m11 = m[5];
	const int m12 = m[6];
	const int m13 = m[7];
	const int m20 = m[8];
	const int m21 = m[9];
	const int m22 = m[10];
	const int m23 = m[11];
	
	RamVertex *dst2 = dst;
	const RomVertex *src2 = src;
	for (int i = vcount; i; --i)
	{
		const int sx = src->x;
		const int sy = src->y;
		const int sz = src->z;

		const int x =
			(((long long)sx * m00) >> PREC_VEC) +
			(((long long)sy * m01) >> PREC_VEC) +
			(((long long)sz * m02) >> PREC_VEC) +
			m03;
		const int y =
			(((long long)sx * m10) >> PREC_VEC) +
			(((long long)sy * m11) >> PREC_VEC) +
			(((long long)sz * m12) >> PREC_VEC) +
			m13;
		const int z =
			(((long long)sx * m20) >> PREC_VEC) +
			(((long long)sy * m21) >> PREC_VEC) +
			(((long long)sz * m22) >> PREC_VEC) +
			m23;
		
		const s32 len = fast_pythagoras(x, y, z);
		const s32 len_rcp = rcp(len);
		
		dst2->x = MUL16(x, len_rcp);
		dst2->y = MUL16(y, len_rcp);
		
		src++;
		dst2++;
	}
}
#endif

void transform_verts(s32* ram_lights, const signed short (*lights0)[3], const signed short (*lights1)[3], int lightcount, RamVertex* ram_verts, const RomVertex* rom_verts, const RomMatrix4* const* mvps0, const RomMatrix4* const* mvps1, u32 vcount, const u16* list, int subframe) {
	MatrixCache matrixCache(mvps0, mvps1, subframe);

	// lights
	#if 0
	{
		const RomMatrix4& m0 = matrixCache.getMatrix(0);
		FORI(lightcount) {
			const s16* light0 = lights0[i];
			const s16* light1 = lights1[i];
			s32 sx = light0[0] + ((s64(light1[0]-light0[0])*subframe)>>8);
			s32 sy = light0[1] + ((s64(light1[1]-light0[1])*subframe)>>8);
			s32 sz = light0[2] + ((s64(light1[2]-light0[2])*subframe)>>8);
		
			sx = sx;
			sy = sy;
			sz = sz;

			s32 x, y, z, w;
			transform_vec3(x, y, z, w, sx, sy, sz, m0);
			int dx, dy;
			project_vec3(dx, dy, x, y, w);
			ram_lights[i*3+0] = dx>>4;
			ram_lights[i*3+1] = dy>>4;
			ram_lights[i*3+2] = z;
		}
	}
	#endif

	// verts/norms
	const u16* list_end = list+vcount;
	for(; list!=list_end; list++) {
		const RomVertex& sv = rom_verts[*list];
	
		if (sv.normal) {
			// NORMAL
			s32 x, y, z;
			{
				const RomMatrix4& m0 = matrixCache.getMatrix(sv.matrix[0].i);
				transform_norm3(x, y, z, sv, m0);
			}
			if (sv.matrix[1].w) {
				scale8_vec3(x, y, z, sv.matrix[0].w);
				// Apply bone 1
				{
					const RomMatrix4& m1 = matrixCache.getMatrix(sv.matrix[1].i);
					addbone_norm3(x, y, z, sv, m1, sv.matrix[1].w);
				}

				// Do we have a third bone?
				if (sv.matrix[2].w) {
					// Apply bone 2
					const RomMatrix4& m2 = matrixCache.getMatrix(sv.matrix[2].i);
					addbone_norm3(x, y, z, sv, m2, sv.matrix[2].w);
				}				

				// renormalize
				//normalize_norm3(x, y, z);			
			} else {
				// BLÆH: vi er off-by-one et sted!
				// (kjappfix)
				//x <<= 1;
				//y <<= 1;
				//z <<= 1;
			}
		

			// renormalize
			normalize_norm3(x, y, z);			

			
			// fra 16:16 til 4:12 
			s32 dx = (1<<11)+(x>>5);
			s32 dy = (1<<11)-(y>>5);
			ram_verts[*list].x = dx;
			ram_verts[*list].y = dy;

		} else {
			// VERTEX
			s32 x, y, w;

			// Apply first matrix.
			// This is all that happens in common case.
			// We assume for now that weight is 255.
			{
				const RomMatrix4& m0 = matrixCache.getMatrix(sv.matrix[0].i);
				s32 z_tmp;
				transform_vec3(x, y, z_tmp, w, sv.x, sv.y, sv.z, m0);
			}

			// Proceed with skinning?
			if (sv.matrix[1].w)
			{
				// We have skinning. This means matrix[0] had a weight != 255.
				scale8_vec3(x, y, w, sv.matrix[0].w);
				
				// Apply bone 1
				{
					const RomMatrix4& m1 = matrixCache.getMatrix(sv.matrix[1].i);
					addbone_vec3(x, y, w, sv, m1, sv.matrix[1].w);
				}

				// Do we have a third bone?
				if (sv.matrix[2].w) {
					// Apply bone 2
					const RomMatrix4& m2 = matrixCache.getMatrix(sv.matrix[2].i);
					addbone_vec3(x, y, w, sv, m2, sv.matrix[2].w);
				}
			}
			
			// project and we are done
			s32 dx, dy;
			project_vec3(dx, dy, x, y, w);
			ram_verts[*list].x = dx;
			ram_verts[*list].y = dy;
		}
	}
}

const RomMatrix4& MatrixCache::getMatrix(int geti)
{
	// decrease stamps
	if (MATRIXCACHE_SIZE>=1) storedMats[0].stamp >>= 1;
	if (MATRIXCACHE_SIZE>=2) storedMats[1].stamp >>= 1;
	if (MATRIXCACHE_SIZE>=3) storedMats[2].stamp >>= 1;
	if (MATRIXCACHE_SIZE>=4) storedMats[3].stamp >>= 1;
	if (MATRIXCACHE_SIZE>=5) storedMats[4].stamp >>= 1;
	if (MATRIXCACHE_SIZE>=6) storedMats[5].stamp >>= 1;

	// slå opp i le cache.
	FORI(MATRIXCACHE_SIZE) {
		if (storedMats[i].i==geti) {
			storedMats[i].stamp = 0xFF;
//			dprintf("h");
			return storedMats[i].m;
		}
	}
	
	// find lru index
	StoredMatrix* oldestMatrix = storedMats;
	for (int i=1; i<MATRIXCACHE_SIZE; i++) {
		if (storedMats[i].stamp < oldestMatrix->stamp) {
			oldestMatrix = &storedMats[i];
		}
	}

	// sett nytt stamp og index
	oldestMatrix->stamp = 0xFF;
	oldestMatrix->i = geti;

	// interpoler den matrisa
	s32* dval  = oldestMatrix->m.m; 
	const s32* sval0 = mvps0[geti]->m;
	const s32* sval1 = mvps1[geti]->m;
	
//	register int subframe = this->subframe;
	FORI(16) {
		s32 delta = *sval1 - *sval0;
		*dval++ = *sval0 + s32(((long long)delta*subframe) >> 8);
		sval0++;
		sval1++;
	}


	// returner.	
	return oldestMatrix->m;
}
