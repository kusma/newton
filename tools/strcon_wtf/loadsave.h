#pragma once
#include "main.h"
#include "Vector3.h"
struct TFAnimation;

void saveAct(vector<Color8>& pal);



void loadAct(std::string filnavn);
void loadBinary(std::string filnavn);
void loadImage(std::string filnavn);
void loadFlc(std::string filnavn);

// this is where the loaders put their stuff
extern vector<string> args;
extern FILE* inputbin;
extern int   inputbinSize;
extern CImage inputimage;
extern vector<Color8> palette;
extern vector<Vector3> palette_float;
extern TFAnimation* inputanim;
