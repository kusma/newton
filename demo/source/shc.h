/*
 *  shcodec ;) version 1.0.1 header file
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

#ifndef SHC_INCLUDED
#define SHC_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "shtypes.h"

#define N (1<<8)
#define SH_MAXLENGTH (31)
#define SH_CACHEBITS (8)
#define SH_CACHESIZE (1<<SH_CACHEBITS)

int sh_SortFreq(uint32 *freq, uchar *symb);

/*  Function: sh_SortFreq()
 *  Purpose: sort symbols by frequency
 *  Parameters: freq - symbol frequencies, symb - space for symbols
 *  Return: number of actual symbols, symb - sorted symbols
 *  Notes: none
 */

void sh_CalcLen(uint32 *freq, uchar *symb, uchar *len, int n, int maxlen);

/*  Function: sh_CalcLen()
 *  Purpose: calculate code lengths
 *  Parameters: freq - symbol frequencies, symb - sorted symbols,
 *  len - space for code lengths, n - number of actual symbols,
 *  maxlen - maximal allowed code length = [8..31] or -1 for
 *  mimimum-redundancy code lengths
 *  Return: len - code lengths
 *  Notes: len can be overlayed on (uchar *)freq
 */

void sh_SortLen(uchar *len, uchar *symb, int n);

/*  Function: sh_SortLen()
 *  Purpose: sort symbols by code length and actual value
 *  Parameters: len - code lengths, symb - sorted symbols,
 *  n - number of actual symbols
 *  Return: symb - sorted symbols
 *  Notes: none
 */

void sh_CalcCode(uchar *len, uchar *symb, uchar *code, int n);

/*  Function: sh_CalcCode()
 *  Purpose: calculate canonical huffman codes
 *  Parameters: len - code lengths, symb - sorted symbols,
 *  code - space for codes, n - number of actual symbols
 *  Return: code - symbol codes
 *  Notes: code can be overlayed on (uchar *)freq+256
 */

int sh_PackTree(uchar *len, uchar *symb, uchar *aux, uint32 *buf, int n);

/*  Function: sh_PackTree()
 *  Purpose: pack code tree
 *  Parameters: len - code lengths, symb - sorted symbols,
 *  aux - auxiliary space, buf - output buffer,
 *  n - number of actual symbols
 *  Return: packed tree size
 *  Notes: aux can be overlayed on (uchar *)freq+512
 */

int sh_ExpandTree(uchar *len, uchar *symb, uint32 *buf);

/*  Function: sh_ExpandTree()
 *  Purpose: expand code tree
 *  Parameters: len - space for code lengths, symb - space for
 *  symbols, buf - input buffer
 *  Return: number of actual symbols
 *  Notes: none
 */

void sh_CalcDecode(uchar *len, uchar *symb, uchar *base, uchar *offs, uchar *cache, int n);

/*  Function: sh_CalcDecode()
 *  Purpose: calculate decode tables
 *  Parameters: len - code lengths, symb - sorted symbols,
 *  base, offs and cache - space for decoding tables, n - number
 *  of actual symbols
 *  Return: base, offs and cache - ready for decoding
 *  Notes: cache can be overlayed on (uchar *)len
 */

/*  Macros: ENCODE_SYMB()
 *  Purpose: encode a symbol
 *  Parameters: len - code lengths, code - symbol codes,
 *  symbol - symbol to encode, buf - output buffer, bufpos - output buffer
 *  position, bits - number of free MS bits in bitbuf, bitbuf - bit buffer
 */

#define ENCODE_SYMB(len, code, symbol, buf, bufpos, bits, bitbuf) {\
    uint32 symblen, symbcode;\
    symblen=len[symbol];\
    symbcode=code[symbol];\
    if(symblen<=bits) {\
        bitbuf<<=symblen;\
        bitbuf|=symbcode;\
        bits-=symblen;\
    } else {\
        bitbuf<<=bits;\
        bitbuf|=(symbcode>>(symblen-bits));\
        buf[bufpos++]=bitbuf;\
        bitbuf=symbcode;\
        bits+=(32-symblen);\
    }\
}\

/*  Macros: DECODE_SYMB()
 *  Purpose: decode a symbol
 *  Parameters: base, offs, cache - decoding tables, symb - sorted symbols,
 *  buf - input buffer, bufpos - input buffer position, bits - number of
 *  unused LS bits in bitbuf, bitbuf - bit buffer, symbol - decoded symbol
 */

#define DECODE_SYMB(base, offs, cache, symb, buf, bufpos, bits, bitbuf, symbol)\
{   uint32 frame, codelen;\
    if(bits) frame=(bitbuf<<(32-bits))|(buf[bufpos]>>bits);\
    else frame=buf[bufpos]; codelen=cache[frame>>(32-SH_CACHEBITS)];\
    if(codelen>SH_CACHEBITS) while((frame>>(32-codelen))<base[codelen]) codelen++;\
    symbol=symb[(frame>>(32-codelen))-base[codelen]+offs[codelen]];\
    if(codelen<=bits) bits-=codelen;\
    else { bits+=32-codelen; bitbuf=buf[bufpos++];}\
}\

#ifdef __cplusplus
}
#endif

#endif /* SHC_INCLUDED */
