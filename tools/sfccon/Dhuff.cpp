/**************************************************************************
 *                                                                        *
 *  DHUFF.C:    Huffman Decompression Program.                            *
 *              14-August-1990    Bill Demas          Version 1.0         *
 *		               						  *
 *   This program decompresses a file previously compressed with the      *
 *   HUFF1 program.                                                       *
 *                                                                        *
 *           USAGE:   DHUFF <input file> <output file>                    *
 *		               						  *
 *   (DISK to DISK:  Input direct from disk, output direct to disk)       *
 **************************************************************************/


#include <vector>
#include <map>
#include "compress.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define    VERBOSE                          /* If defined, prints verbose
                                               program progress when it's
                                               running...                   */
namespace {

	short           decomp_tree[512];
	unsigned short  code[256];
//	unsigned long   file_size;
	unsigned char   code_length[256];




	/**************************************************************************

	MAIN ()

	This is the main program. It performs the Huffman decoding procedure in
	2 separate steps.

	I know that this program can be made more compact & faster, but I was more
	interested in UNDERSTANDABILITY !!!

	**************************************************************************/
	/*
	void main (argc, argv)
	int   argc;
	char  *argv[];
	{
	void  build_decomp_tree (), decompress_image ();


	if (argc == 3)
	{
		printf ("\nDHUFF:  Huffman Code Decompression Program.");
		printf ("\n        14-Aug-90  Bill Demas.  Version 1.0\n\n");


		if ((ifile = fopen (argv[1], "rb")) != NULL)
		{
			fread (&file_size, sizeof (file_size), 1, ifile);
			fread (code, 2, 256, ifile);
			fread (code_length, 1, 256, ifile);

			#ifdef VERBOSE
				printf ("(1) Building the tree.\n");
			#endif

			build_decomp_tree ();

			#ifdef VERBOSE
				printf ("(2) Decompressing & Creating the Output File.\n");
			#endif

			if ((ofile = fopen (argv[2], "wb")) != NULL)
			{
				decompress_image();
				fclose (ofile);
			}
			else
				printf ("\nERROR:  Couldn't create output file %s\n", argv[2]);

			fclose (ifile);
		}
		else
			printf ("\nERROR:  %s -- File not found!\n", argv[1]);
	}
	else
		printf ("Usage:  DHUFF <input filename> <output filename>\n\n");
	}
	*/

	/**************************************************************************

	BUILD_DECOMP_TREE ()

	This function builds the decompression tree.
	**************************************************************************/

	void  build_decomp_tree ()
	{
	register unsigned short  loop1;
	register unsigned short  current_index;

	unsigned short  loop;
	unsigned short  current_node = 1;


	decomp_tree[1] = 1;

	for (loop = 0; loop < 256; loop++)
	{
		if (code_length[loop])
		{
		current_index = 1;
		for (loop1 = code_length[loop] - 1; loop1 > 0; loop1--)
		{
			current_index = (decomp_tree[current_index] << 1) +
					((code[loop] >> loop1) & 1);
			if (!(decomp_tree[current_index]))
			decomp_tree[current_index] = ++current_node;
		}
		decomp_tree[(decomp_tree[current_index] << 1) +
		(code[loop] & 1)] = -loop;
		}
	}
	}



}; // namespace


namespace comp {
	/**************************************************************************

	DECOMPRESS_IMAGE ()

	This function decompresses the compressed image.
	**************************************************************************/

	unsigned char*  huff_decompress(unsigned char* dst, unsigned char* src, int file_size)
	{
		register unsigned short  cindex = 1;
		register short           bitshift;

		int charcount = 0;

		while (charcount < file_size) {
			unsigned char curchar = *src++;

			for (bitshift = 7; bitshift >= 0; --bitshift) {
				cindex = (cindex << 1) + ((curchar >> bitshift) & 1);

				if (decomp_tree[cindex] <= 0) {
					*dst++ = (int) (-decomp_tree[cindex]);

					if ((++charcount) == file_size)
						bitshift = 0;
					else
						cindex = 1;
				} else {
					cindex = decomp_tree[cindex];
				}
			}
		}
		return dst;
	}


	void huff_getDecompTree(short decompTree_out[512], const std::map<unsigned char, comp::HuffCode >& codeMap) {
		FORI(256) code[i]=0;
		FORI(256) code_length[i]=0;
		std::map<unsigned char, comp::HuffCode >::const_iterator iter;
		for (iter=codeMap.begin(); iter!=codeMap.end(); iter++) {
			int blah = iter->first;
			const HuffCode& c = iter->second;
			code[blah] = c.code;
			code_length[blah] = c.length;			
		}

		FORI(512) decomp_tree[i]=0;
		build_decomp_tree();

		FORI(512) decompTree_out[i] = decomp_tree[i];
	}

};