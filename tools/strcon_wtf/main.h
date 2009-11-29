#pragma once
#define _USE_MATH_DEFINES
#include "math.h"
#include <atlimage.h>
#include <vector>
#include <string>
#include "Color.h"
using namespace std;


#define FORI(a) for (int i=0; i<int(a); i++) 
#define FORJ(a) for (int j=0; j<int(a); j++) 
#define FORXY(a, b) for (int y=0; y<b; y++) for (int x=0; x<a; x++) 

#undef max
#undef min
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))


typedef	unsigned char			u8;
typedef	unsigned short int		u16;
typedef	unsigned int			u32;
typedef	unsigned long long		u64;
typedef	signed char				s8;
typedef	signed short int		s16;
typedef	signed int				s32;

void error(char* err, ...);
void help(char* err=NULL);


#include "loadsave.h"
