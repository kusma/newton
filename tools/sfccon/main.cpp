#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "tasks.h"
#include "fcntl.h"
#include "io.h"


void error(char* err, ...) {
	char str[512];
	va_list ap; 
	va_start (ap, err);
	vsprintf(str, err, ap); 
	va_end(ap); 
	fprintf(stderr, "Error: %s\n", str);
	exit(1);
}

void help(char* err) {
	if (err) {
		fprintf(stderr, "Error: %s\n\n", err);
	}
	printf("Available tasks:\n");
	for (int i=0; i<TASKCOUNT; i++) {
		Task& task = tasks[i];
		const char* taskDesc = taskDescs[i];
		printf("%s ", task.code);
		int argc = task.argc;
		int argi = 0;
		switch(task.input) {
			case INPUT_BINARY:
				printf("<file> ");
				argi++;
				break;
			case INPUT_IMAGE:
				printf("<image-file> ");
				argi++;
				break;
			case INPUT_ACT:
				printf("<Palette-file> ");
				argi++;
				break;
			case INPUT_ANIM:
				printf("<anim-file> ");
				argi++;
				break;
			default:;
		}
		for (; argi<argc; argi++) printf("<arg%i> ", argi+1);
		printf("\n - %s\n", taskDesc);
	}
	printf("\n");
	exit(1);
}

#include "compress.h"

int main(int argc, char* argv[]) {
	srand(13251365);

/*
	std::vector<unsigned char> compstr;

	{
		char* hei = "eivind lyngsnes liland";
		FORI(strlen(hei)+1) compstr.push_back(hei[i]);
	}

	compstr.resize('z'-'a'+1);
	FORI('z'-'a'+1) compstr[i]=i+'a';

	std::map<unsigned char, comp::HuffCode > codeMap;
	std::map<unsigned char, int> frequencyCount;
	comp::huff_getFrequencyCount(frequencyCount, &compstr[0], compstr.size());
	comp::huff_getCodeMap(codeMap, frequencyCount);
	std::map<unsigned char, comp::HuffCode >::iterator iter;
	for (iter = codeMap.begin(); iter!=codeMap.end(); iter++) {
		comp::HuffCode code = iter->second;
		char val = iter->first;
		printf("%c: ", (char)val);
		for (int i=code.length-1; i>=0; i--) {
			printf("%i", (code.code>>i)&1);
		}
		printf("\n");
	}
	std::vector<unsigned char> komprimert;
	comp::huff_compress(komprimert, codeMap, &compstr[0], compstr.size());
	printf("size: %i\n", komprimert.size());

	short tabbe[512];
	comp::huff_getDecompTree(tabbe, codeMap);

	unsigned char heheheh[1024];
	*comp::huff_decompress(heheheh, &komprimert[0], compstr.size() ) = 0;
	printf("der: %s\n", heheheh);
*/

	if (setmode(fileno(stdout), O_BINARY)==-1) fprintf(stderr, "Warning: Cannot set stdin to binary mode\n");
	if (argc<2) help();

	// find task
	Task* task=NULL;
	{
		string code = argv[1];
		for (int i=0; i<TASKCOUNT; i++) {
			if (code==tasks[i].code) {
				task = tasks+i;
				break;
			}
		}
	}
	if (!task) error("Invalid task code.");
	if (task->argc > 0 && argc-2 != task->argc) error("Wrong number of arguments to task.");
	if (task->argc < 0 && argc-2 < -task->argc) error("Wrong number of arguments to task. (should be at lease %i)", -task->argc);

	// store arguments
	for (int i=2; i<argc; i++) {
		::args.push_back( argv[i] );
	}

	// load arg 0
	if (task->input==INPUT_BINARY) loadBinary(args[0]);
	else if (task->input==INPUT_IMAGE) loadImage(args[0]);
	else if (task->input==INPUT_ACT) loadAct(args[0]);
	else if (task->input==INPUT_ANIM) loadAnim(args[0]);

	// call function
	if (task->func) {
		task->func();
	} else {
		error("Sorry kompis, task not implemented :-(");
	}

	return 0;
}