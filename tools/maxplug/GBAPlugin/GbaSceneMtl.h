#pragma once

#include "saker.h"
#include <imtl.h>


#define GBASCENEMATERIAL_CLASS_ID	Class_ID(0x83374921, 0x93658674)

namespace gbaSceneMtlStuff {

	enum {
		pbGeneral = 0
	};
	enum {
		texPrimary   = 0,
		texSecondary = 1,
	};
	enum { // params for PBLOCK_GENERAL
		genUvGen,
		genFiller,
		genTexPrimary, 
		genTexSecondary, 
		genBumpSrc,
		genOneone
	};
	
};