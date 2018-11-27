#ifndef PROJ_H_
#define PROJ_H_

#define NUM_STATES 8
typedef enum {
	START_STATE = 0,
	READY_STATE = 1,
	LEFT_DOWN_STATE = 2,
	RIGHT_DOWN_STATE = 3,
	ARMED_STATE = 4,
	PUNCH_STATE = 5,
	EXIT_STATE = 6,
	STOP_STATE = 7
} State;

#define NUM_INPUTS 6
typedef enum {
	LEFT_BUTTON_DOWN = 0,
	LEFT_BUTTON_UP = 1,
	RIGHT_BUTTON_DOWN = 2,
	RIGHT_BUTTON_UP = 3,
	STOP_BUTTON = 4,
	EMER_BUTTON = 5
} Input;

#define NUM_OUTPUTS 7
typedef enum {
	READY_MSG = 0,
	LEFT_DOWN_MSG = 1,
	RIGHT_DOWN_MSG = 2,
	ARMED_MSG = 3,
	PUNCH_MSG = 4,
	EXIT_MSG = 5,
	EMER_MSG = 6
} Output;

const char *outMessage[NUM_OUTPUTS] = {
		"Ready...",
		"Left button down - press right button to arm press",
		"Right button down - press left button to arm press",
		"DANGER - Press is Armed! - Hold buttons for 2 seconds",
		"Press Cutting Now",
		"Powering down",
		"Emergency Stop!"
};

const char *inMessage[NUM_INPUTS] = {
		"LD",
		"LU",
		"RD",
		"RU",
		"S",
		"ES"
};

#define LEFT 0
#define RIGHT 1
typedef struct {
	int direction;
	State curr;
	//char input[3];
} Operator;

typedef struct {
	int index;
} Display;

#endif
