#include "timer.h"

fixed16 timer::time_scale;

void timer::setBpm(double bpm)
{
	time_scale = fixed16(bpm / (FRAMES_PR_SECOND * 60));
}
