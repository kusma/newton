/**************************************************************************
 *                                                                        *
 *  HUFF1.C:    Huffman Compression Program.                              *
 *              14-August-1990    Bill Demas          Version 1.0         *
 *		               						  *
 *   This program compresses a file using the Huffman codes.              *
 *                                                                        *
 *           USAGE:   HUFF1 <input file> <output file>                    *
 *		               						  *
 *   (DISK to DISK:  Input direct from disk, output direct to disk)       *
 **************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#define    VERBOSE                          /* If defined, prints verbose
                                               program progress when it's
                                               running...                   */

short           father[512];
unsigned short  code[256], heap_length;
unsigned long   compress_charcount, file_size, heap[257];
unsigned char   code_length[256];
long            frequency_count[512];

FILE            *ifile, *ofile;



/**************************************************************************

 MAIN ()

 This is the main program. It performs the Huffman encoding procedure in
 5 separate steps.

 I know that this program can be made more compact & faster, but I was more
 interested in UNDERSTANDABILITY !!!

 **************************************************************************/
/*
void main (argc, argv)
int  argc;
char *argv[];
{
   unsigned short  generate_code_table ();
   void            build_code_tree (), build_initial_heap ();
   void            compress_image (), compression_report ();
   void            get_frequency_count ();


   if (argc == 3)
   {
      printf ("\nHUFF1:  Huffman Code Compression Program.");
      printf ("\n        14-Aug-90  Bill Demas.  Version 1.0\n\n");


      if ((ifile = fopen (argv[1], "rb")) != NULL)
      {
         fseek (ifile, 0L, 2);
         file_size = (unsigned long) ftell (ifile);

         #ifdef VERBOSE
            printf ("(1) Getting Frequency Counts.\n");
         #endif

         fseek (ifile, 0L, 0);
         get_frequency_count ();

         #ifdef VERBOSE
            printf ("(2) Building Initial Heap.\n");
         #endif

         build_initial_heap ();

         #ifdef VERBOSE
            printf ("(3) Building the Code Tree.\n");
         #endif

         build_code_tree ();

         #ifdef VERBOSE
            printf ("(4) Generating the Code Table.\n");
         #endif

         if (!generate_code_table ())
            printf ("ERROR!  Code Value Out of Range. Cannot Compress.\n");
         else
         {
            #ifdef VERBOSE
               printf ("(5) Compressing & Creating the Output File.\n");
            #endif

            if ((ofile = fopen (argv[2], "wb")) != NULL)
            {
               fwrite (&file_size, sizeof (file_size), 1, ofile);
               fwrite (code, 2, 256, ofile);
               fwrite (code_length, 1, 256, ofile);

               fseek (ifile, 0L, 0);
               compress_image ();

               fclose (ofile);
            }
            else
               printf("\nERROR: Couldn't create output file %s\n", argv[2]);

            #ifdef VERBOSE
               compression_report ();
            #endif
         }
         fclose (ifile);
      }
      else
         printf ("\nERROR:  %s -- File not found!\n", argv[1]);
   }
   else
      printf ("Usage:  HUFF1 <input filename> <output filename>\n\n");
}
*/

/**************************************************************************

 COMPRESS_IMAGE ()

 This function performs the actual data compression.
 **************************************************************************/

void compress_image ()
{
   register unsigned int    thebyte = 0;
   register short           loop1;
   register unsigned short  current_code;
   register unsigned long   loop;

   unsigned short  current_length, dvalue;
   unsigned long   curbyte = 0;
   short           curbit = 7;


   for (loop = 0L; loop < file_size; loop++)
   {
      dvalue         = (unsigned short) getc (ifile);
      current_code   = code[dvalue];
      current_length = (unsigned short) code_length[dvalue];

      for (loop1 = current_length-1; loop1 >= 0; --loop1)
      {
         if ((current_code >> loop1) & 1)
            thebyte |= (char) (1 << curbit);

         if (--curbit < 0)
         {
            putc (thebyte, ofile);
            thebyte = 0;
            curbyte++;
            curbit = 7;
         }
      }
   }
   putc (thebyte, ofile);
   compress_charcount = ++curbyte;
}


/**************************************************************************

 COMPRESSION_REPORT ()

 This function displays the results of the compression sequence.
 **************************************************************************/

void  compression_report ()
{
   float           savings;
   unsigned short  header_charcount;
   unsigned long   output_characters;


   header_charcount   = 768 + sizeof (file_size);
   output_characters  = (unsigned long) header_charcount +
			compress_charcount;

   printf ("\nRaw characters          : %ld\n", file_size);
   printf ("Header characters       : %d\n", header_charcount);
   printf ("Compressed characters   : %ld\n", compress_charcount);
   printf ("Total output characters : %ld\n", output_characters);

   savings = 100 - ((float) output_characters / (float) file_size) * 100;
   printf ("\nPercentage savings      : %3.2f%%\n", savings);
}


/**************************************************************************

 GENERATE_CODE_TABLE ()

 This function generates the compression code table.
 **************************************************************************/

unsigned short  generate_code_table ()
{
   register unsigned short  loop;
   register unsigned short  current_length;
   register unsigned short  current_bit;

   unsigned short  bitcode;
   short           parent;


   for (loop = 0; loop < 256; loop++)
      if (frequency_count[loop])
      {
         current_length = bitcode = 0;
         current_bit = 1;
         parent = father[loop];

         while (parent)
         {
            if (parent < 0)
            {
               bitcode += current_bit;
               parent = -parent;
            }
            parent = father[parent];
            current_bit <<= 1;
            current_length++;
         }

         code[loop] = bitcode;

         if (current_length > 16)
            return (0);
         else
            code_length[loop] = (unsigned char) current_length;
      }
      else
         code[loop] = code_length[loop] = 0;

   return (1);
}


/**************************************************************************

 BUILD_CODE_TREE ()

 This function builds the compression code tree.
 **************************************************************************/

void build_code_tree ()
{
   void    reheap ();

   register unsigned short  findex;
   register unsigned long   heap_value;


   while (heap_length != 1)
   {
      heap_value = heap[1];
      heap[1]    = heap[heap_length--];

      reheap (1);
      findex = heap_length + 255;

      frequency_count[findex] = frequency_count[heap[1]] +
                                frequency_count[heap_value];
      father[heap_value] =  findex;
      father[heap[1]]    = -findex;
      heap[1]            =  findex;

      reheap (1);
   }

   father[256] = 0;
}


/**************************************************************************

 REHEAP ()

 This function creates a "legal" heap from the current heap tree structure.
 **************************************************************************/

void reheap (heap_entry)
unsigned short  heap_entry;
{
   register unsigned short  index;
   register unsigned short  flag = 1;

   unsigned long   heap_value;


   heap_value = heap[heap_entry];

   while ((heap_entry <= (heap_length >> 1)) && (flag))
   {
      index = heap_entry << 1;

      if (index < heap_length)
         if (frequency_count[heap[index]] >= frequency_count[heap[index+1]])
            index++;

      if (frequency_count[heap_value] < frequency_count[heap[index]])
	 flag--;
      else
      {
         heap[heap_entry] = heap[index];
         heap_entry       = index;
      }
   }

   heap[heap_entry] = heap_value;
}


/**************************************************************************

 BUILD_INITIAL_HEAP ()

 This function builds a heap from the initial frequency count data.
 **************************************************************************/

void build_initial_heap ()
{
   void    reheap ();

   register unsigned short  loop;


   heap_length = 0;

   for (loop = 0; loop < 256; loop++)
      if (frequency_count[loop])
         heap[++heap_length] = (unsigned long) loop;

   for (loop = heap_length; loop > 0; loop--)
      reheap (loop);
}


/**************************************************************************

 GET_FREQUENCY_COUNT ()

 This function counts the number of occurrences of each byte in the data
 that are to be compressed.
 **************************************************************************/

void get_frequency_count ()
{
   register unsigned long  loop;


   for (loop = 0; loop < file_size; loop++)
      frequency_count[getc (ifile)]++;
}

