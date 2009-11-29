#ifndef PARTS_H
#define PARTS_H

#include "support.h"
#include "fixed.h"


namespace parts {

#if 0	
	void intro(int len, int startframe);
	void clownogroup();
	void end();
/*	void tunnel(int len);
	void kuber(int len);
	void sylinder(int len);
	void spike(int len);
	void parallax(int len);
	void glow(int len, int startglow);
	void skyboxting(bool intro, int startframe, int len);
	void skygge();
	void message(int len, int message);
	*/
#endif
	void radial(fixed16 start, fixed16 end);
	void zoomer(fixed16 start, fixed16 end);

	void test(int len);
	void cubegrid(fixed16 start, fixed16 end);
	void lattice(fixed16 start, fixed16 end);

	void growy(fixed16 start, fixed16 end);
	void grid(fixed16 start, fixed16 end);
	void sphere(fixed16 start, fixed16 end);
	
	void poelse(fixed16 start, fixed16 end);
	void parallax_scroll(fixed16 start, fixed16 end);
	void lockedloaded(fixed16 start, fixed16 end);

	
	void mercury(fixed16 start, fixed16 end);
	void skate(fixed16 start, fixed16 end, int flikkIndex, int offset);
	void telos(fixed16 start, fixed16 end);

	void plane(int len);
	void spheregrid(fixed16 start, fixed16 end, int seed);

	void bboy(int len);
	void mirror(fixed16 start, fixed16 end);
	void message(fixed16 start, fixed16 end, int index);


#if 0	
	void cube2(int len);
	
	void hdr_wall(int len);
	void hdr_fargedisco(int len);
	void hdr_city(int len, int part, int starttime);
//	void greeble2k(int len, int starttime=256);
	void cube2(int len);
	void greeble2k(int len);

	void anim(int len);
	void twister(int len);
	void bonespike(int len);
	void greeble(int len);
	void greeble_skybox(int len);
	void gridglow(int len);
	void projector(int len);
	void djdoc(int flikkvalg);
	void sphere2k(int len, int starttime);
	void wave(int len);
#endif

};

#endif /* PARTS_H */
