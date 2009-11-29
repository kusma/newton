#pragma once


struct Color8 {
	Color8(){}
	Color8(const Color8& op) {
		*this = op;
	}
	Color8(char r, char g, char b) {
		rgb[0] = r;
		rgb[1] = g;
		rgb[2] = b;
	}
	void operator=(const Color8& op) {
		rgb[0]=op.rgb[0];
		rgb[1]=op.rgb[1];
		rgb[2]=op.rgb[2];
	}
	unsigned char rgb[3];
};
