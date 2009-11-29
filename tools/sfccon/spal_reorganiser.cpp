#include "main.h"	
#include <vector>
#include <algorithm>
#include "Vector3.h"

namespace {

	std::vector<Vector3> testere;


	double spredning(Vector3* pal, int count) {
		// finn gjennomsnitt
		Vector3 avg(0,0,0);
		FORI(count) avg = avg + pal[i];
		avg = avg * (1.f/count);
		double ret=0;
		FORI(count) {
			Vector3 distvec = avg - pal[i];
			ret += distvec.magnitude_pow();
		}
		return ret;
	}

	struct SorterDings {
		SorterDings(){}
		Vector3 col;
		double val;
		bool operator<(const SorterDings& op) const {
			return val<op.val;
		}
	};
	void sorter(Vector3* pal, int count, Vector3 vec) {
		std::vector<SorterDings> sorterpal;
		sorterpal.resize(count);
		FORI(count) {
			sorterpal[i].col = pal[i];
			sorterpal[i].val = pal[i].dot(vec);
		}
		std::sort(sorterpal.begin(), sorterpal.end());
		FORI(count) pal[i] = sorterpal[i].col;
	}
};

void spal_reorganiser(Vector3* pal, int count, int reserved) {
	if (reserved<0) reserved=0;
	if (count<=2+reserved) return;
	int reservedL = max(0, reserved-count/2);
	int reservedR = min(reserved, count/2);

	int besti=-1;
	double bestv=0;
	// test alle testere
	FORI(testere.size()) {
		Vector3& vec = testere[i];
		sorter(pal, count-reserved, vec);
		double v = spredning(pal, count/2) + spredning(pal+count/2, count/2);
		if (v<bestv || i==0) {
			besti=i;
			bestv=v;
		}
	}
	// lag den paletten
	sorter(pal, count-reserved, testere[besti]);
	// na, rekurser
	spal_reorganiser(pal, count/2, reservedL);
	spal_reorganiser(pal+count/2, count/2, reservedR);
}


void spal_init() {
	// lag testere
	if (testere.empty()) {
		srand(245727);
		testere.push_back(  Vector3(0,0,1) );
		testere.push_back(  Vector3(0,1,0) );
		testere.push_back(  Vector3(1,0,0) );
		testere.push_back(  Vector3(0,1,1) );
		testere.push_back(  Vector3(1,0,1) );
		testere.push_back(  Vector3(1,1,0) );
		testere.push_back(  Vector3(1,1,1) );
		FORI(32) {
			testere.push_back( Vector3( rand()&255, rand()&255, rand()&255 ) );
		}
	}
}