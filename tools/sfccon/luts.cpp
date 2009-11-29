#include "main.h"

#define HEIGHT 160



namespace {



	int CLZ(int Input) {
		int C = 0;
		if(Input == 0) return 32;
		while( !(Input & 0x80000000)) { C++; Input <<= 1; }
		return C;
	}

	int rcp_lut[HEIGHT*2];
	int Rand_Table[257];
	int LZ8_Table[256];
	int Sinus_Table[256];
	void calc_diverse() {
		rcp_lut[0]=0;
		for (int i=1; i<HEIGHT*2; i++) {
			rcp_lut[i] = 0xFFFF / i;
		}
		for (int i=0; i<256; i++) {
			LZ8_Table[i] = CLZ(i)-24;
		}
		for (int i=0; i<256; i++) {
			double in = i*M_PI/256.0;
			Sinus_Table[i] = int(255.0*sin(in));
		}
		for (int i=0; i<257; i++) {
			int tall = 0;
			FORJ(24) {
				tall += rand();
				tall ^= tall>>8;
			}
			Rand_Table[i]=tall&0xFFFF;
		}
	}




	int RCP_Table[512];
	void prepare_fast_RCP() {
		int i, pex;
		int RCP_Table1[514], RCP_Table2[512];
		for(i=0;i<514;i++)
			RCP_Table1[i] = (int)(1.0/(i/256.0) * 0x40000000);

		pex = RCP_Table1[256];

		for(i=0;i<513;i++)
		{
			double ErrX = 1.0/((i+.5)/256.0) * 0x40000000;
			ErrX -= (pex + RCP_Table1[i+1])/2.0;
			pex = RCP_Table1[i+1];
			RCP_Table1[i] += (int)(ErrX * .25 + 0x2000);
			RCP_Table1[i+1] += (int)(ErrX * .25 + 0x2000);
		}

		for(i=256;i<513;i++)
			RCP_Table1[i] = (RCP_Table1[i] * 4) & 0xFFFF0000;
		RCP_Table1[256] = 0xFFFF0000;
		for(i=256;i<512;i++)
			RCP_Table2[i] = (((RCP_Table1[i+1] - RCP_Table1[i]) << 7) & 0xFFFF0000) >> 16;
		for(i=256;i<512;i++)
			RCP_Table[i] = RCP_Table1[i] | RCP_Table2[i];
	}

	
	int RCP8_Table[256];
	void prepare_fast_RCP8(int* RCP8_Table) {
		RCP8_Table[0]=255;
		for (int i=1; i<256; i++) {
			RCP8_Table[i] = 255/i;		
		}
	}




	void kommaliste(const char* start, int size, int* liste, int count) {
		char tallstr[10];
		sprintf(tallstr, "0x 0%ix, ", size*2);
		tallstr[2] = '%';
		int skift = 80/(size*2+2+2);

		printf("%s = {", start);
		for (int i=0; i<count; i++) {
			if (count>skift && (i%skift)==0) printf("\n");
			printf(tallstr, liste[i]);
		}

		printf("};\n\n");
	}



	void lag_luts() {
		// lag sakene
		calc_diverse();
		prepare_fast_RCP();

		// skriv fila
		//FILE* fil = fopen(lutsf, "wt");
		//if (!fil) error("Could not create \"%s\"", lutsf);
		printf("#include \"support.h\"\n\n");
		kommaliste("u16 IWRAM_DATA IntRcp_Table[]", 2, rcp_lut, 256);
		kommaliste("u8  IWRAM_DATA Sinus_Table[]", 1, Sinus_Table, 256);
		kommaliste("u16 IWRAM_DATA Rand_Table[]", 2, Rand_Table, 257);
	}

};

void task_luts() {
	lag_luts();

}