/*
 ** Kha Mai
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <string.h>
#include <ctype.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/types.h>

#include "../../prog_inputs/src/mystruct.h"


/* Function declaration */
void state_start(State*);
void state_ready(Display*);
void state_left_down(Display*, State*);
void state_right_down(Display*, State*);
void state_armed(Display*, State*);
void state_punched(Display*, State*);
void state_exit(Display*, State*);
void state_stop(Display*);

void sendDisplay(int, Display);

/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(int argc, char* argv[] ) {
	int     rcvid;         // indicates who we should reply to
	int		server_coid;
	name_attach_t *att;
	State 	nextState = START_STATE;
	Display display;
	Operator op;

	/* Configure as a server, register the name_space */
	if ((att = name_attach(NULL, "dev/punch-press-controller", 0)) == NULL) {
		return EXIT_FAILURE;
	}

	if ((server_coid = name_open("dev/punch-press-display", 0)) == -1) {
		perror("name_open failed.");
		return EXIT_FAILURE;
	}

	/* Controller will move from state: START_STATE to state: READY_STATE
	 * so that user can input
	 */
	if(nextState == START_STATE){
		state_start(&nextState);
		state_ready(&display);
		sendDisplay(server_coid, display);
		sleep(3);
	}
	/* Start the loop */
	while (nextState != STOP_STATE) {

		/* Receive package from child */
		rcvid = MsgReceive (att->chid, &op, sizeof(op), NULL);
		if(rcvid == -1){
			perror("Cannot receive from child");
			exit (EXIT_FAILURE);
		}

		/* Check a current state of child */
		switch (op.curr) {
		case READY_STATE:
			nextState = READY_STATE;
			sleep(3);
			break;
		case LEFT_DOWN_STATE:
			state_left_down(&display, &nextState);
			sendDisplay(server_coid, display);
			break;
		case RIGHT_DOWN_STATE:
			state_right_down(&display, &nextState);
			sendDisplay(server_coid, display);
			break;
		case ARMED_STATE:
			state_armed(&display, &nextState);
			sendDisplay(server_coid, display);
			sleep(2);

			/* Controller goes to the next state: PUNCH_STATE */
			state_punched(&display, &nextState);
			sendDisplay(server_coid, display);
			sleep(1);
			break;
		case EXIT_STATE:
			state_exit(&display, &nextState);
			sendDisplay(server_coid, display);
			sleep(5);

			/* Controller goes to the next state: STOP_STATE */
			state_stop(&display);
			sendDisplay(server_coid, display);
			break;
		default:
			break;
		}

		if(nextState == READY_STATE){
			state_ready(&display);
			sendDisplay(server_coid, display);
		}

		//Reply to input and provide next state to handle next input.
		MsgReply (rcvid, EOK, &nextState, sizeof (State));
		//Send updated display object to display
	}

	// Close the connection
	name_close(server_coid);
	// Remove the name_space
	name_detach(att, 0);
	printf("\nExit controller\n");
	return EXIT_SUCCESS;
}
/*******************************************************************************
 * state_start( ): void
 ******************************************************************************/
void state_start(State *nextState){
	printf("Moving to state: Start\n");
	*nextState = READY_STATE;
}
/*******************************************************************************
 * state_ready( ): void
 ******************************************************************************/
void state_ready(Display *display){
	printf("Moving to state: Ready\n");
	display->index = READY_MSG;
}
/*******************************************************************************
 * state_left_down( ): void
 ******************************************************************************/
void state_left_down(Display *display, State *nextState){
	printf("Moving to state: LeftDown\n");
	*nextState = ARMED_STATE;
	display->index = LEFT_DOWN_MSG;
}
/*******************************************************************************
 * state_right_down( ): void
 ******************************************************************************/
void state_right_down(Display *display, State *nextState){
	printf("Moving to state: RightDown\n");
	*nextState = ARMED_STATE;
	display->index = RIGHT_DOWN_MSG;
}
/*******************************************************************************
 * state_armed( ): void
 ******************************************************************************/
void state_armed(Display *display, State *nextState){
	printf("Moving to state: Armed\n");
	*nextState = PUNCH_STATE;
	display->index = ARMED_MSG;
}
/*******************************************************************************
 * state_punched( ): void
 ******************************************************************************/
void state_punched(Display *display, State *nextState){
	printf("Moving to state: Punch\n");
	*nextState = READY_STATE;
	display->index = PUNCH_MSG;
}
/*******************************************************************************
 * state_exit( ): void
 ******************************************************************************/
void state_exit(Display *display, State *nextState){
	printf("Moving to state: Exit\n");
	*nextState = STOP_STATE;
	display->index = EXIT_MSG;
}
/*******************************************************************************
 * state_stop( ): void
 ******************************************************************************/
void state_stop(Display *display){
	printf("Moving to state: Stop\n");
	display->index = -1;
}
/*******************************************************************************
 * sendDisplay( ): int
 ******************************************************************************/
void sendDisplay(int server_coid, Display dspMsg){
	int status = 0;
	/* Send a package to display */
	if(MsgSend(server_coid, &dspMsg, sizeof(dspMsg), &status, sizeof(status)) == -1){
		fprintf (stderr, "Error during MsgSend\n");
		perror (NULL);
		exit (EXIT_FAILURE);
	}
}
