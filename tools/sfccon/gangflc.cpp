#include "main.h"	
#include "topflc/topflc.h"
#include "compress.h"
#include <assert.h>
#include <map>
using namespace std;

void flic_handler(char *msg)
{
	error("topflc: %s\n", msg);
};


void task_gangflc() {
	TFErrorHandler_Set(flic_handler);

	TFAnimationInfo info_a;	
	TFAnimation* anim_a = TFAnimation_NewFile((char*)args[0].c_str());
	TFAnimation_GetInfo(anim_a, &info_a);
	TFAnimation_SetLooping(anim_a, TF_FALSE);
	TFBuffers_Alloc(anim_a);
	TFFrame_Seek(anim_a, 0);

	fprintf(stderr, "Loaded flic A %ix%i, %i frames\n", info_a.Width, info_a.Height, info_a.NumFrames);

	TFAnimationInfo info_b;	
	TFAnimation* anim_b = TFAnimation_NewFile((char*)args[1].c_str());
	TFAnimation_GetInfo(anim_b, &info_b);
	TFAnimation_SetLooping(anim_b, TF_FALSE);
	TFBuffers_Alloc(anim_b);
	TFFrame_Seek(anim_b, 0);

	fprintf(stderr, "Loaded flic B %ix%i, %i frames\n", info_b.Width, info_b.Height, info_b.NumFrames);




}