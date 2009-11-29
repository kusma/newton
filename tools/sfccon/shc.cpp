/*
 *  shcodec ;) version 1.0.1 source code
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

#include "shtypes.h"
#include "shc.h"

#define KEY(a)       (freq[symb[a]])
#define SWP(a, b)    (t=symb[a], symb[a]=symb[b], symb[b]=t)
#define CMP(a,b,c,d) (a<b ? 0 : (a>b ? 1 : (c<d ? 1 : 0)))

int sh_SortFreq(uint32 *freq, uchar *symb) {
    int ls[8], hs[8];
    int l, h, m, i, j, s, n;
    uint32 p; uchar t;

    /* mark & count actual symbols */
    for(i=n=0; i<N; i++)
    if(freq[i]) symb[n++]=i;

    ls[0]=0; hs[0]=n-1;

    for(s=0; s>=0; s--) {

        /* pop next one */
        l=ls[s]; h=hs[s];

        while(l<h) {

            /* choose pivot frequency */
            m=(l+h)>>1; p=KEY(m);
            SWP(l, m); i=l+1; j=h;

            /* partition loop */
            while(1) {
                while(i<=j && KEY(i)<=p) i++;
                while(p<KEY(j)) j--;
                if(i>=j) break;
                SWP(i, j); i++; j--;
            }

            SWP(l, j);

            /* push largest one */
            if(j-l<=h-j) {
                if(j+1<h) { ls[s]=j+1; hs[s++]=h; }
                h=j-1;
            } else {
                if(j-1>l) { ls[s]=l; hs[s++]=j-1; }
                l=j+1;
            }
        }
    }

    return n;
}

void sh_CalcLen(uint32 *freq, uchar *symb, uchar *len, int n, int maxlen) {
    int root, leaf, next, avbl, dpth;
    int removed, inserted, dpthins, first, last;

    /* check for pathological cases */
    if(n==1) {
        for(next=0; next<N; next++) len[next]=0;
        len[symb[0]]=1; return;
    }

    if(n==2) {
        for(next=0; next<N; next++) len[next]=0;
        len[symb[0]]=len[symb[1]]=1; return;
    }

    freq[symb[0]]+=freq[symb[1]]; root=0; leaf=2;

    /* first pass, left to right, setting parent pointers */
    for(next=1; next<n-1; next++) {

        if (leaf>=n || freq[symb[root]]<freq[symb[leaf]]) {
            freq[symb[next]]=freq[symb[root]];
            freq[symb[root++]]=next;
        } else 
            freq[symb[next]]=freq[symb[leaf++]];

        if(leaf>=n || (root<next && freq[symb[root]]<freq[symb[leaf]])) {
            freq[symb[next]]+=freq[symb[root]];
            freq[symb[root++]]=next+n;
        } else
            freq[symb[next]]+=freq[symb[leaf++]];
    }

    /* second pass, right to left, count internal nodes to be removed */
    if(maxlen==-1) { removed=0; dpthins=n+2; }
    else {
        first=last=n-3; dpth=1;

        while(dpth<maxlen-1) {

            while((freq[symb[first]]%n)>last) {
                root=first; first-=last-first+1;
                if(first<0) { first=-1; break; }
            }

            while(root-first>1) {
                if((freq[symb[(first+root)>>1]]%n)>last)
                root=(first+root)>>1;
                else first=(first+root)>>1;
            }

            last=first; dpth++;
            if(last<0) break;
        }
 
        removed=last+1;

        if((freq[symb[0]]%n)>=removed) {
            removed=0;
            dpthins=n+2;
        } else {
            for(dpthins=0; (1<<dpthins)<removed; dpthins++);
            dpthins=maxlen-dpthins;
        }
    }

    /* third pass, right to left, setting leaf depths */
    avbl=2; dpth=1; first=last=n-3;
    next=n-1; inserted=0;

    if(dpthins==1) {
        avbl=4; dpth=2;
        inserted=2;
    }

    while(avbl>0) {
        if(last<removed) first=last+1;
        else {

            while((freq[symb[first]]%n)>last) {
                root=first; first-=last-first+1;
                if(first<removed) { first=removed-1; break; }
            }

            while(root-first>1) {
                if((freq[symb[(first+root)>>1]]%n)>last)
                root=(first+root)>>1;
                else first=(first+root)>>1;
            }
   
            first=root;
        }

        avbl-=last-first+1; root=0;

        if(dpth>=dpthins) {
            root=1<<(dpth-dpthins);
            if((inserted+root)>removed)
            root=removed-inserted;
            inserted+=root; avbl-=root;
        }

        else if(dpth==(dpthins-1)) {
            if(freq[symb[first]]==(last+1)) first++;
            else avbl--; root=1; inserted++;
        }

        while(avbl>0) {
            freq[symb[next--]]=dpth;
            avbl--;
        }

        avbl=(last-first+1+root)<<1;
        dpth++; last=(--first);
    }

    for(next=0; next<N; next++) len[next]=freq[next];
}

void sh_SortLen(uchar *len, uchar *symb, int n) {
    int ls[8], hs[8];
    int l, h, m, lm, sm, i, j, s;
    uchar t;

    ls[0]=0; hs[0]=n-1;

    for(s=0; s>=0; s--) {

        /* pop next one */
        l=ls[s]; h=hs[s];

        while(l<h) {

            /* choose pivot length & symbol */
            m=(l+h)>>1; lm=len[sm=symb[m]];
            SWP(l, m); i=l+1; j=h;

            /* partition loop */
            while(1) {
                while(i<=j && CMP(len[symb[i]],lm,symb[i],sm)) i++;
                while(CMP(lm,len[symb[j]],sm,symb[j])) j--;
                if(i>=j) break; SWP(i, j); i++; j--;
            }

            SWP(l, j);

            /* push largest one */
            if(j-l<=h-j) {
                if(j+1<h) { ls[s]=j+1; hs[s++]=h; }
                h=j-1;
            } else {
                if(j-1>l) { ls[s]=l; hs[s++]=j-1; }
                l=j+1;
            }
        }
    }
}

void sh_CalcCode(uchar *len, uchar *symb, uchar *code, int n) {
    int i, codeval, next, prev;

    for(prev=len[symb[0]], i=codeval=0; i<n; i++, codeval++) {
       next=len[symb[i]]; codeval>>=(prev-next);
       prev=next; code[symb[i]]=codeval;
    }
}

int sh_PackTree(uchar *len, uchar *symb, uchar *aux, uint32 *buf, int n) {
    int runlen, maxrun, maxlen, runbits, lenbits, flag, i, j, t;
    int treesize, bufpos, curval, curbits, bits;
    uint32 bitbuf;

    /* log(maxlen) base 2 */
    t=maxlen=len[symb[0]];
    for(lenbits=0; t>0; t>>=1, lenbits++);

    maxrun=runbits=flag=0;

    /* max 0-run length */
    if(n<N) {
        for(i=t=0; i<N; i++)

        /* 0-run detected */
        if(!len[i]) {
            for(j=i; (i<N) && (!len[i]); i++);
            maxrun=((i-j)>maxrun ? (i-j) : maxrun);
            aux[t++]=i-j;
        }

        /* log(maxrun) base 2 */
        for(runbits=0, t=maxrun; t>0; t>>=1, runbits++);

        /* 0-run flag, increase lenbits if necessary */
        flag=maxlen+1; if(!(maxlen&(maxlen+1))) lenbits++;
    }

    /* pack rle parameters */
    bitbuf=(uint32)((((lenbits<<4)|runbits)<<8)|flag);
    bits=16;

    /* pack tree */
    for(treesize=bufpos=i=j=0; i<N; i++) {
        curval=len[i]; curbits=lenbits;

        /* 0-run detected */
        if(!curval) {
            runlen=aux[j++]; i+=runlen-1;
            if(runbits<(runlen-1)*lenbits) {
                curval=((flag<<runbits)|runlen);
                curbits+=runbits;
            } else
                curbits+=(runlen-1)*lenbits;
        }

        /* pack bits */
        if(curbits<=bits) {
            bitbuf<<=curbits;
            bitbuf|=curval;
            bits-=curbits;
        } else {
            bitbuf<<=bits;
            bitbuf|=(curval>>(curbits-bits));
            buf[bufpos++]=bitbuf;
            bitbuf=curval; 
            bits+=(32-curbits);
            treesize++;
        }
    }

    /* flush remaining data */
    buf[bufpos]=(bitbuf<<bits);
    return (treesize+1)<<2;
}

int sh_ExpandTree(uchar *len, uchar *symb, uint32 *buf) {
    int curbits, curval, currun, lenbits, runbits, flag, bits, i, j, n;
    uint32 bitbuf, lenmask, runmask;

    /* extract rle parameters */
    bitbuf=buf[0];
    lenbits=bitbuf>>28;
    runbits=(bitbuf>>24)&0xf;
    flag=(bitbuf>>16)&0xff;

    for(i=0; i<N; i++) len[i]=0;

    lenmask=(1<<lenbits)-1;
    runmask=(1<<runbits)-1;

    /* expand tree */
    for(bits=16, i=n=0, j=1; i<N; i++) {

        /* extract value */
        if(lenbits<=bits) {
            curval=(bitbuf>>(bits-lenbits))&lenmask;
            bits-=lenbits;
        } else {
            curval=bitbuf;
            bitbuf=buf[j++];
            curval<<=(lenbits-bits);
            curval|=bitbuf>>(32-lenbits+bits);
            curval&=lenmask;
            bits+=32-lenbits;
        }

        /* extract run */
        if(curval!=flag) {
            len[i]=curval;
            if(curval) symb[n++]=i; /* curval may be zero */
        } else {
            if(runbits<=bits) {
                currun=(bitbuf>>(bits-runbits))&runmask;
                bits-=runbits;
            } else {
                currun=bitbuf;
                bitbuf=buf[j++];
                currun<<=(runbits-bits);
                currun|=bitbuf>>(32-runbits+bits);
                currun&=runmask;
                bits+=32-runbits;
            }

            i+=currun-1;
        }
    }

    return n;
}

void sh_CalcDecode(uchar *len, uchar *symb, uchar *base, uchar *offs, uchar *cache, int n) {
    int root, leaf, i, j;

    for(root=j=0, i=len[symb[0]]; i>=0; i--) {
        leaf=0;
        while((len[symb[j]]==i) && (j<n)) { leaf++; j++; }
        base[i]=root; if(leaf) offs[i]=j-leaf;
        root=(root+leaf)>>1;
    }

    for(i=0; i<SH_CACHESIZE; i++) {
        for(j=1; (j<=SH_CACHEBITS) && (i>>(SH_CACHEBITS-j)<base[j]); j++);
        cache[i]=j;
    }
}