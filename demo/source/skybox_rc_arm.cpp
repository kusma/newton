#include "skybox_rc.h"

#define TILESIZE_L2 4
#define TILESIZE (1<<TILESIZE_L2)

namespace skybox_rc {



	struct Node {
		Vector3 vec;
		fixed16 majorU, majorV;
		int major_axis;

		void raycastAxis(fixed16& u, fixed16& v, int axis) {
			fixed16 w;
			switch(axis&7) {
				case 0:
					w = vec.x;
					u = vec.y;
					v = vec.z;
					break;
				case 1:
					u = vec.x;
					w = vec.y;
					v = vec.z;
					break;
				case 2:
					u = vec.x;
					v = vec.y;
					w = vec.z;
				break;
			}
			fixed16 w_rcp = w.rcp();
			u *= w_rcp;
			v *= w_rcp;
		}
				
		void setVector(const Vector3& op) {	
			vec=op;
			int index=0;
			if (op.x.abs() > op.y.abs()) index |= 1;
			if (op.y.abs() > op.z.abs()) index |= 2;
			if (op.z.abs() > op.x.abs()) index |= 4;			
			switch(index) {
				case 1:
				case 1|4:
					// x is biggest
					major_axis = (op.x<fixed16(0)) ? 0 : 3;
					raycastAxis(majorU, majorV, 0);			
				break;
				case 2:
				case 2|1:
					// y is biggest
					major_axis = (op.y<fixed16(0)) ? 1 : 4;
					raycastAxis(majorU, majorV, 1);			
				break;
				case 4:
				case 4|2:
					// z is biggest
					major_axis = (op.z<fixed16(0)) ? 2 : 5;
					raycastAxis(majorU, majorV, 2);			
				break;
				default:
					assert(0);
			}
		}
		
	};

	void expand(u8* dst, int pitch, u8* tex, int texDim_l2, bool hq, bool clip, int u00, int v00, int u01, int v01, int u10, int v10, int u11, int v11)
	{
		int dudy_l = (u10 - u00) >> TILESIZE_L2;
		int dvdy_l = (v10 - v00) >> TILESIZE_L2;
		int dudy_r = (u11 - u01) >> TILESIZE_L2;
		int dvdy_r = (v11 - v01) >> TILESIZE_L2;
		int dudx_t = (u01 - u00) >> TILESIZE_L2;
		int dvdx_t = (v01 - v00) >> TILESIZE_L2;
		int dudx_b = (u11 - u10) >> TILESIZE_L2;
		int dvdx_b = (v11 - v10) >> TILESIZE_L2;
		int ddudxdy = (dudx_b-dudx_t) >> TILESIZE_L2;
		int ddvdxdy = (dvdx_b-dvdx_t) >> TILESIZE_L2;

		// ignore hq for now. do 2x2.
		if (!clip && false) {
			int uv = (u00<<16)|v00;
			
		
		} else {
			// this can be much slower...
			int y=0;
			int u_l = u00;
			int u_r = u01;
			int v_l = v00;
			int v_r = v01;
			
			// skip scanlines until something can be visible
			for (; y<TILESIZE; y++, dst+=pitch) {
				if ( (!(u_l&0xFFFF0000) || !(u_r&0xFFFF0000)) 
				  && (!(v_l&0xFFFF0000) || !(v_r&0xFFFF0000)) ) break;
				u_l+=dudy_l;
				v_l+=dvdy_l;
				u_r+=dudy_r;
				v_r+=dvdy_r;
			}
			
			// render scanlines 
			for (; y<TILESIZE; y++, dst+=pitch) {
				u8* dp = dst;
				int x=0;
				int dudx = (u_r-u_l)>>TILESIZE_L2;
				int dvdx = (v_r-v_l)>>TILESIZE_L2;
				int u=u_l;
				int v=v_l;
	
				// skip until you can draw
				for (; x<TILESIZE; x++, dp++) {
					if ( !(u&0xFFFF0000) && !(v&0xFFFF0000) ) break;
					u+=dudx;
					v+=dvdx;
				}
				for (; x<TILESIZE; x++, dp++) {
					// draw
					int index = (u>>(16-texDim_l2))<<(texDim_l2);
					index |= v>>(16-texDim_l2);
					*dp = tex[index];
					
					// increment
					u+=dudx;
					v+=dvdx;
					if ( (u&0xFFFF0000) || (v&0xFFFF0000) ) break;
				}
				
				// increment
				u_l+=dudy_l;
				v_l+=dvdy_l;
				u_r+=dudy_r;
				v_r+=dvdy_r;
				// quit when invisible again
				if ( ((u_l&0xFFFF0000) && (u_r&0xFFFF0000)) 
				  || ((v_l&0xFFFF0000) && (v_r&0xFFFF0000)) ) break;
			}
			
			
		}		
		
	}

	void draw(u8* dst, int dstWidth, int dstHeight, u8** cubemap, int texDim_l2, bool hq, Vector3 v, Vector3 dvdx, Vector3 dvdy) 
	{
		// find top-left view vector
		v = v - dvdx - dvdy;

		// scale vectors so that max length on screen is 1 (well)
		fixed16 mag_rcp = v.mag_rcp();
		v *= mag_rcp;
		dvdx *= mag_rcp;
		dvdy *= mag_rcp;

		// divide deltas by dimensions to get per-tile deltas
		dvdx *= fixed16(dstWidth >> TILESIZE_L2).rcp();
		dvdy *= fixed16(dstHeight >> TILESIZE_L2).rcp();

		// hmmmm static allocation ftw
		assert(dstWidth <= 256);
		Node nodeLines[257>>TILESIZE_L2][2];

		// one line of tiles is raytraced at a time		
		// need two buffers of node values 
		// - one for the top of the tile-line and one for the bottom..
		// they are double buffered:
		Node* highLine = nodeLines[0];
		Node* lowLine = nodeLines[1];
		Vector3 leftVecIter(v);
		for (int y=0; y<(dstHeight>>TILESIZE_L2)+1; y++, leftVecIter+=dvdy) {
			// swap those lines (dobbelbufferd)
			Node* tmp=highLine;
			highLine=lowLine;
			lowLine=tmp;

			// init node values from the vectors
			{	Vector3 vIter = leftVecIter;
				for (int x=0; x<(dstWidth>>TILESIZE_L2)+1; x++, vIter+=dvdx) {
					lowLine[x].setVector(vIter);
				}
			}

			// the first iteration is only for raytracing the first line of nodes
			if (y==0) continue;
			
			// iterate the line of tiles and render them
			Vector3 vIter = leftVecIter;
			for (int x=0; x<(dstWidth>>TILESIZE_L2); x++, vIter+=dvdx) {
				Node& n00 = highLine[x];
				Node& n01 = highLine[x+1];
				Node& n10 = lowLine[x];
				Node& n11 = lowLine[x+1];
				if (n00.major_axis==n01.major_axis && n10.major_axis==n11.major_axis && n00.major_axis==n10.major_axis) {
					// common case:
					// all 4 nodes have the same major axis: 
					// this is a job for the simple gridexpander
					// ...
					// ..
				} else {
					// general case: they dont all have the same major axis
					int m00 = (1<<n00.major_axis);
					int m01 = (1<<n01.major_axis);
					int m10 = (1<<n10.major_axis);
					int m11 = (1<<n11.major_axis);
					
					int total_mask = m00 | m01 | m10 | m11;
					for (int i=0; i<6; i++) {
						// only render axes that are in use
						if (((total_mask>>i)&1)==0) continue;
						
						// hehe lerp them
						// ...
						// ..
					
					}
					
				}					
			}
		}
	}

};
