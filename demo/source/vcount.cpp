#include "vcount.h"
#include <gba_interrupt.h>
#include <assert.h>
namespace vcount {

	namespace {

		struct Slice {
			Slice() {}
			Slice(int y, VCountFn fn, int param=0) 
				: y(y)
				, fn(fn)
				, param(param)
			{}
			int y;
			VCountFn fn;
			int param;
		};

		Slice slices[25] EWRAM_DATA;
		Slice* slices_end = slices;
		Slice* stage = slices;

		INLINE void setup_stage(Slice* s) {
			stage = s;
			REG_DISPSTAT = VCOUNT(stage->y) | (REG_DISPSTAT&0xFF);	
		}

		void int_vcount();
	};

	void clear() {
		slices_end=slices;
	}
	void insert(int y, VCountFn fn, int param) {
		Slice* iter;
		for (iter=slices_end; (iter[-1].y>y)&&(iter>slices); iter--) {
			*iter = iter[-1];
		} 
		*iter = Slice(y, fn, param);
		slices_end++;
	}


	

	void vblank() {
		setup_stage(slices);
		SetInterrupt(IE_VCNT, int_vcount);
		EnableInterrupt(IE_VCNT);
	}



	namespace {
		
		void int_vcount() {
			assert(REG_VCOUNT==stage->y);
			stage->fn(stage->y, stage->param);
			
			Slice* nextStage = stage+1;
			if (nextStage==slices_end) DisableInterrupt(IE_VCNT);
			else {
				setup_stage(nextStage);
				assert(REG_VCOUNT<stage->y-1);
			}
		}
	};

};
