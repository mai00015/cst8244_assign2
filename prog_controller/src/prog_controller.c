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
#define TIMER_PULSE_CODE _PULSE_CODE_MINAVAIL
#define PULSE_ES_ _PULSE_CODE_MAXAVAIL
#define PULSE_BTN_UP _PULSE_CODE_COIDDEATH
typedef union {
	struct _pulse   pulse;
	/* your other message structures would go
           here too */
	Operator op;
} my_message_t;
/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(int argc, char* argv[] ) {
	int     rcvid, rcvid2;         // indicates who we should reply to
	int		server_coid;
	name_attach_t *att;
	State 	nextState = START_STATE;
	Display display;
	//Operator op;

	struct sigevent         event;
	struct itimerspec       itime;
	timer_t                 timer_id;
	my_message_t            msg;
	int msgRcv = 0;




	/* Configure as a server, register the name_space */
	if ((att = name_attach(NULL, "dev/punch-press-controller", 0)) == NULL) {
		return EXIT_FAILURE;
	}

	if ((server_coid = name_open("dev/punch-press-display", 0)) == -1) {
		perror("name_open failed.");
		return EXIT_FAILURE;
	}
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
			att->chid,
			_NTO_SIDE_CHANNEL, 0);
	event.sigev_priority = SchedGet(0,0,NULL);
	event.sigev_code = TIMER_PULSE_CODE;
	timer_create(CLOCK_REALTIME, &event, &timer_id);


	itime.it_value.tv_sec = 5;
	itime.it_value.tv_nsec = 0;
	itime.it_interval.tv_sec = 0;
	itime.it_interval.tv_nsec = 0;
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
		printf("RCVID AT BEGINNING OF LOOP: %d\n",rcvid);
		/* Receive package from child
		 * TODO: after button up cancel, rcvid is reset to */
		rcvid = MsgReceive (att->chid, &msg, sizeof(msg), NULL);
		msgRcv++;
		printf("Msg Received: %d\n",msgRcv);
		/*Store the message send rcvid to reply*/
		printf("RCVID success: %d\n",rcvid);
		rcvid2 = rcvid;
		printf("%d\n", rcvid);
		if(rcvid == -1){
			perror("Cannot receive from child");
			exit (EXIT_FAILURE);
		}
		/* Check a current state of child */
		switch (msg.op.curr) {
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
			MsgReply (rcvid2, EOK, &nextState, sizeof (State));
			sleep(2);

			timer_settime(timer_id, 0, &itime, NULL);

			rcvid = MsgReceive(att->chid , &msg, sizeof(msg), NULL);
			printf("RCVID from ARMES STATE: %d\n",rcvid);
			if (rcvid == 0) { /* we got a pulse */
				/*We got a pulse from our timer*/
				if (msg.pulse.code == TIMER_PULSE_CODE) {
					printf("TIMER PULSE RECEIVED\n");
					/* Controller goes to the next state: PUNCH_STATE */
					state_punched(&display, &nextState);
					sendDisplay(server_coid, display);
					sleep(1);
				} /* else other pulses ... */
				else if(msg.pulse.code == PULSE_ES_ ){
					printf("EMERGENCY STOP PULSE RECEIVED\n");
					nextState = STOP_STATE;
					state_stop(&display);
					sendDisplay(server_coid, display);
				}
				/*This means we received a msgsend from input {LU, RU}*/
			}else if(msg.op.curr == READY_STATE){
				printf("MSGSEND BUTTON UP RECEIVED\n");
				printf("RCVID FROM BUTTON UP: %d\n",rcvid);
				nextState = READY_STATE;
				//Reset channel id to message send number.
				rcvid = rcvid2;
				printf("RCVID RESET FROM ARMED BUTTON UP: %d\n",rcvid);
			}


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
		/*ASK KHA: why doesn't state_ready function set nextstate?*/
		if(nextState == READY_STATE){
			state_ready(&display);
			sendDisplay(server_coid, display);
		}

		if(nextState != PUNCH_STATE)
			//Reply to input and provide next state to handle next input.
			printf("REPLY TO INPUT AT END OF LOOP: %d\n",nextState);
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
