// #define FRAMES_PR_SECOND (60.0)
#define FRAMES_PR_SECOND (59.59)

#include "adpcm.h"
#include "fixed.h"

namespace timer
{
	extern fixed16 time_scale;
	
	void setBpm(double bpm);
	
	inline fixed16 getBeat()
	{
		return adpcm_gba_get_frame() * time_scale;
	}
};
