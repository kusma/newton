#pragma once

typedef void (*LutFunc)();

enum Input {
	INPUT_NONE,
	INPUT_BINARY,
	INPUT_IMAGE,
//	INPUT_FLC,
	INPUT_ACT,
	INPUT_ANIM
};

struct Task {
	LutFunc func;
	char* code;
	Input input;
	int argc;
};

// description of tasks
#define TASKCOUNT 19
extern Task tasks[TASKCOUNT];
extern const char* taskDescs[TASKCOUNT];