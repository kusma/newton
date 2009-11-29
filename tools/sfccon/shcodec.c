/*
 *  shcodec ;) version 1.0.1 test program
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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include "shtypes.h"
#include "shc.h"

#if (INT_MAX == 0x7fff)
#define SH_BUFSIZE (0x4000)  /* 16 Kbyte for 16-bit machines */
#else
#define SH_BUFSIZE (0x10000) /* 64 Kbyte for 32/64-bit machines */
#endif

#if (SH_BUFSIZE & 3)
#error SH_BUFSIZE must be multiple of 4
#endif

#define SH_ENCODE  (0x65)    /* symbol 'e' */
#define SH_DECODE  (0x64)    /* symbol 'd' */

#define log2(x) (log10((double)(x))/log10(2.0))

double sh_CalcEntropy(uint32 *freq);
void sh_GetFreq(uint32 *freq, uchar *buf, FILE *f);
uint32 sh_FileSize(FILE *f);
uint32 sh_EncodeFile(FILE *ifile, FILE *ofile, uchar *ibuf, uint32 *obuf,
uint32 *freq, uchar *symb, uchar *len, uchar *code, uchar *aux);
uint32 sh_DecodeFile(FILE *ifile, FILE *ofile, uchar *len, uchar *symb,
uchar *base, uchar *offs, uchar *cache, uint32 *ibuf, uchar *obuf);

double sh_CalcEntropy(uint32 *freq) {
    double ent, prob;
    uint32 totfreq;
    int i;

    for(totfreq=i=0; i<N; i++) totfreq+=freq[i];

    for(ent=0.0, i=0; i<N; i++)
    if(freq[i]){
        prob=(double)freq[i]/totfreq;
        ent+=-prob*log2(prob);
    }

    return ent;
}

void sh_GetFreq(uint32 *freq, uchar *buf, FILE *f) {
    uint32 i, j;

    for(i=0; i<N; i++) freq[i]=0;
    while((j=fread(buf,1,SH_BUFSIZE,f))!=0)
    for(i=0; i<j; i++) freq[buf[i]]++;
    fseek(f,0,SEEK_SET);
}

uint32 sh_FileSize(FILE *f) {
    uint32 spos, fsize;
    spos=ftell(f);
    fseek(f,0L,SEEK_END);
    fsize=ftell(f);
    fseek(f,spos,SEEK_SET);
    return fsize;
}

uint32 sh_EncodeFile(FILE *ifile, FILE *ofile, uchar *ibuf, uint32 *obuf,
uint32 *freq, uchar *symb, uchar *len, uchar *code, uchar *aux) {
    int treesize, bits, n;
    uint32 isize, osize, bitbuf, i, j, r;

    /* save original file size */
    isize=sh_FileSize(ifile);
    fwrite(&isize,4,1,ofile);

    /* sort symbols by frequency */
    n=sh_SortFreq(freq, symb);

    /* calculate code lengths */
    sh_CalcLen(freq, symb, len, n, 31);

    /* sort symbols by code length & actual value */
    sh_SortLen(len, symb, n);

    /* calculate canonical huffman codes */
    sh_CalcCode(len, symb, code, n);

    /* pack code tree */
    treesize=sh_PackTree(len, symb, aux, obuf, n);
    fwrite(&treesize,1,1,ofile);

    /* buffer contain packed tree */
    j=treesize>>2;

    /* Important note: in order to avoid unnecessary
     * if() operator (due to bit shifting on 32) in
     * the decoder, we emit dummy bit and set bits
     * to 31 instead of 32
     */

    bits=31; bitbuf=0; osize=1;

    printf("Encoding...\n\n");

    /* encoding loop */
    while((r=fread(ibuf,1,SH_BUFSIZE,ifile))!=0)
    for(i=0; i<r; i++) {
        ENCODE_SYMB(len,code,ibuf[i],obuf,j,bits,bitbuf);
        if(j>=SH_BUFSIZE>>2) { fwrite(obuf,4,j,ofile); osize+=j; j=0; }
    }

    /* flush remaining data */
    fwrite(obuf,4,j,ofile); bitbuf<<=bits;
    fwrite(&bitbuf,4,1,ofile);
    osize=((osize+j+1)<<2)+1;

    /* report */
    printf("Input file size:        %lu bytes\n",isize);
    printf("Output file size:       %lu bytes\n",osize);
    printf("Packed tree size:       %d bytes\n",treesize);
    printf("Max code length:        %d bits\n",len[symb[0]]);

    return osize;
}

uint32 sh_DecodeFile(FILE *ifile, FILE *ofile, uchar *len, uchar *symb,
uchar *base, uchar *offs, uchar *cache, uint32 *ibuf, uchar *obuf) {
    int treesize=0, bits, symbol, n;
    uint32 bitbuf, osize, symbleft, i, j, r;

    /* read original file size */
    fread(&osize,4,1,ifile);

    /* read packed tree size */
    fread(&treesize,1,1,ifile);

    /* read packed tree */
    fread(ibuf,1,treesize,ifile);

    /* expand packed tree */
    n=sh_ExpandTree(len,symb,ibuf);

    /* sort symbols by code length & actual value */
    sh_SortLen(len,symb,n);

    /* calculate decode tables */
    sh_CalcDecode(len,symb,base,offs,cache,n);

    /* due to encoder convention, set bits to 31 instead of 32 */
    bits=31; i=j=0;

    /* read first portion of data, init bitbuf */
    r=fread(ibuf,1,SH_BUFSIZE,ifile); r>>=2;
    bitbuf=ibuf[i++];

    printf("Decoding...\n\n");

    /* decoding loop */
    symbleft=osize;
    while(symbleft--) {
        if(i>=r) { r=fread(ibuf,1,SH_BUFSIZE,ifile); r>>=2; i=0; }
        DECODE_SYMB(base,offs,cache,symb,ibuf,i,bits,bitbuf,symbol);
        obuf[j++]=symbol; if(j>=SH_BUFSIZE) { fwrite(obuf,1,j,ofile); j=0; }
    }

    /* flush remaining data */
    fwrite(obuf,1,j,ofile);
    return osize;
}

void mainn(int argc, char **argv) {
    FILE *ifile, *ofile;
    uint32 isize, osize;
    uchar *ibuf, *obuf;
    double sec, ent, bpb, ineff;
    uint32 *freq; uchar *symb;
    uchar *len, *base, *offs, *cache;
    clock_t t; int i;

    t=clock();

    /* print copyright information */
    printf("\nshcodec ;) 1.0.1, copyright (C) 1998-2002 Simakov Alexander\n\n");
    printf("internet: http://www.webcenter.ru/~xander\n");
    printf("e-mail: xander@online.ru\n\n");

    /* print usage information */
    if((argc!=4) || ((*argv[1]!=SH_ENCODE) && (*argv[1]!=SH_DECODE))) {
        printf("usage: shcodec <command> <infile> <outfile>\n");
        printf("<command> -> e - encode file <infile>  -> input file name\n");
        printf("<command> -> d - decode file <outfile> -> output file name\n");
        return;
    }

    /* open input file */
    if((ifile=fopen(argv[2],"rb"))==NULL) {
        printf("Can`t open file %s\n",argv[2]);
        return;
    }

    /* open output file */
    if((ofile=fopen(argv[3],"wb"))==NULL) {
        printf("Can`t create file %s\n",argv[3]);
        return;
    } 

    /* check if empty */
    if(!(isize=sh_FileSize(ifile))) {
        printf("File %s is empty\n",argv[2]);
        fclose(ofile); remove(argv[3]);
        return;
    } 

    /* allocate memory for buffers */
    ibuf=(uchar *)malloc(SH_BUFSIZE*sizeof(uchar));
    obuf=(uchar *)malloc(SH_BUFSIZE*sizeof(uchar));
    if(ibuf==NULL || obuf==NULL) {
        printf("Out of memory\n");
        return;
    }

    if(*argv[1]==SH_ENCODE) {

        /* allocate memory for encoder (1280 bytes) */
        freq=(uint32 *)malloc(N*sizeof(uint32));
        symb=(uchar *)malloc(N*sizeof(uchar));

        printf("Getting frequencies...\n");

        /* get symbol frequencies */
        sh_GetFreq(freq, ibuf, ifile);

        /* calculate entropy */
        ent=sh_CalcEntropy(freq);

        /* encode file */
        osize=sh_EncodeFile(ifile, ofile, ibuf, (uint32 *)obuf,
        freq, symb, (uchar *)freq, (uchar *)freq+256, (uchar *)freq+512);

        /* calculate average bpb value and inefficiency */
        bpb=osize; bpb=(bpb/isize)*8; ineff=100*(1-ent/bpb);

        /* get execution time */
        t=clock()-t; sec=(double)t/(double)CLOCKS_PER_SEC;
        if(sec==0.0) sec=0.001;

        /* report */
        printf("Average bpb:            %5.3f\n",bpb);
        printf("Entropy bpb:            %5.3f\n",ent);
        printf("Inefficiency:           %5.3f %%\n",ineff);
        printf("Execution time:         %5.3f seconds\n",sec);
        printf("Average encoding speed: %5.3f mb/sec\n",
        (double)(osize/sec)/0x100000);
        printf("\nWork complete ;)\n");

        free(freq);
        free(symb);
    } else {
        /* allocate memory for decoder (574 bytes) */
        len=(uchar *)malloc(N*sizeof(uchar));
        symb=(uchar *)malloc(N*sizeof(uchar));
        base=(uchar *)malloc(SH_MAXLENGTH*sizeof(uchar));
        offs=(uchar *)malloc(SH_MAXLENGTH*sizeof(uchar));

        /* decode file */
        osize=sh_DecodeFile(ifile,ofile,len,symb,base,offs,len,
        (uint32 *)ibuf,obuf);

        /* get execution time */
        t=clock()-t; sec=(double)t/(double)CLOCKS_PER_SEC;
        if(sec==0.0) sec=0.001;

        /* report */
        printf("Input file size:        %lu bytes\n",isize);
        printf("Output file size:       %lu bytes\n",osize);
        printf("Execution time:         %5.3f seconds\n",sec);
        printf("Average decoding speed: %5.3f mb/sec\n",
        (double)(osize/sec)/0x100000);
        printf("\nWork complete ;)\n");

        free(len); free(symb);
        free(base); free(offs);
    }

    free(ibuf);
    free(obuf);

    fclose(ifile);
    fclose(ofile);
}