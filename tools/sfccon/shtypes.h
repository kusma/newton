/*
 *  shcodec ;) version 1.0.1 type definitions
 *  copyright (C) 1998-2002 Simakov Alexander
 *  march 2002
 *
 *  This software may be used freely for any purpose. However, when
 *  distributed, the original source must be clearly stated, and,
 *  when the source code is distributed, the copyright notice must
 *  be retained and any alterations in the code must be clearly marked.
 *  No warranty is given regarding the quality of this software.
 *
 *  internet: http://www.webcenter.ru/~xander
 *  e-mail: xander@online.ru
 */

#ifndef SHTYPES_INCLUDED
#define SHTYPES_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>

	typedef unsigned char uchar;


	typedef unsigned short ushort;

#if (INT_MAX==0x7fff) /* 16-bit machines */
typedef unsigned long uint32;
#elif (INT_MAX==0x7fffffff) /* 32/64-bit machines */
typedef unsigned int uint32;
#else
#error unsupported integer size
#endif

#ifdef __cplusplus
}
#endif

#endif /* SHTYPES_INCLUDED */