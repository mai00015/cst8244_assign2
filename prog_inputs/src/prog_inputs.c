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
#include <sys/siginfo.h>
#include <time.h>


#include "mystruct.h"
#define PULSE_ES_ _PULSE_CODE_MAXAVAIL
#define PULSE_BTN_UP _PULSE_CODE_COIDDEATH
int checkInt (const char *operand);
/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(int argc, char *argv[]) {
	int msgSend = 0;
	Operator operator;
	State checkState = READY_STATE;

	int server_coid, checkValid = 0, status = 0, cancel = 0;
	char input[20], pause_time[20];

	if ((server_coid = name_open("dev/punch-press-controller", 0)) == -1) {
		perror("name_open failed.");
		return EXIT_FAILURE;
	}

	while(1){
		do{
			/* Prompt user to input */
			printf ("Enter the input (LD, LU, RD, RU, S, P <int>, ES): \n");
			scanf("%s", &input);

			if(checkState == PUNCH_STATE){
				if(strcmp(input,inMessage[LEFT_BUTTON_UP]) == 0 || strcmp(input,inMessage[RIGHT_BUTTON_UP]) == 0){
					operator.curr = READY_STATE;
					operator.direction = 0;
					MsgSend (server_coid, &operator, sizeof(operator), &checkState, (int)sizeof(checkState));
					msgSend++;
					cancel = 1;
					checkValid = 1;
					printf("Sent pulse: %d\n",msgSend);
				}else if(strcmp(input,inMessage[EMER_BUTTON]) == 0){
					printf("EMER_BUTTON PRESSED\n");
					MsgSendPulse(server_coid, SchedGet(0, 0, NULL), PULSE_ES_, 69);
					msgSend++;
					printf("Sent pulse: %d\n",msgSend);
					cancel = 1;
					//WRONG CANCELLATION INPUT DURING PUNCH STATE... WHAT SHOULD HAPPEN?
				}else{
					printf("Neither es or button up occured\n");
					checkState = READY_STATE;
				}
				printf("Abort section complete\n");
			}
			if(strcmp(input, "P") == 0){
				scanf("%s", &pause_time);

				// Checking the valid input
				if(checkInt(pause_time)){
					fprintf (stderr, "Sorry. It must be a number.\n\n");
					checkValid = 0;
					continue;
				}
				sleep(atoi(pause_time));
				checkValid = 0;
			}else{
				//strcpy(operator.input, input);

				// Check the state: READY_STATE and a button up cancellation didn't occur
				if(checkState == READY_STATE && cancel != 1){
					if(strcmp(input,inMessage[LEFT_BUTTON_DOWN]) == 0){
						operator.direction = LEFT;
						operator.curr = LEFT_DOWN_STATE;
						checkValid = 1;
					}
					else if(strcmp(input,inMessage[RIGHT_BUTTON_DOWN]) == 0){
						operator.direction = RIGHT;
						operator.curr = RIGHT_DOWN_STATE;
						checkValid = 1;

					}else if(strcmp(input, inMessage[STOP_BUTTON]) == 0){
						operator.curr = EXIT_STATE;
						checkValid = 1;
						status = 1;
						//Check if this if statement happened before sending message. Send a pulse.
					}else
						checkValid = 0;

					// Check the state: ARMED_STATE
					//FOR VALID EMERGENCY STOP:
					// send the message
					//We can check the reply to know if the controller is in armed state preparing to punch.
					//Go in loop to only take in valid input for the 2 seconds.
					//Button-up or Emergency Stop.
				}else if(checkState == ARMED_STATE){
					if(operator.direction == LEFT){
						if(strcmp(input, inMessage[RIGHT_BUTTON_DOWN]) == 0){
							checkValid = 1;
							operator.curr = ARMED_STATE;
							//Take input: ES or BUTTON UP during the 2 seconds.

						}else if(strcmp(input, inMessage[LEFT_BUTTON_UP]) == 0){
							checkValid = 1;
							operator.curr = READY_STATE;
						}else
							checkValid = 0;
					}else if(operator.direction == RIGHT){
						if(strcmp(input, inMessage[LEFT_BUTTON_DOWN]) == 0){
							checkValid = 1;
							operator.curr = ARMED_STATE;
							//Take input: ES or BUTTON UP during the 2 seconds.
						}else if(strcmp(input, inMessage[RIGHT_BUTTON_UP]) == 0){
							checkValid = 1;
							operator.curr = READY_STATE;
						}else
							checkValid = 0;
					}else
						checkValid = 0;

				}
				/* Wrong input? Output to the console */
				if(checkValid == 0)
					printf("\nYou entered a wrong input. Please try again\n");
			}

		}while(checkValid == 0);
		//Emergency stop button was pressed!
		if(cancel == 0){
			if (MsgSend (server_coid, &operator, sizeof(operator), &checkState, (int)sizeof(checkState)) == -1) {
				fprintf (stderr, "Error during MsgSend\n");
				perror (NULL);
				exit (EXIT_FAILURE);

			}
		}
		cancel = 0;
		msgSend++;
		printf("Sent message: %d time with %d\n",msgSend, operator.curr);
		// Break the loop if user enter "S"
		if(status == 1)
			break;
	}

	// Close the connection
	name_close(server_coid);
	return EXIT_SUCCESS;
}
/*******************************************************************************
 * checkInt( ): int
 ******************************************************************************/
int checkInt (const char *num){

	/* Validate input as a number */
	const char *check = num;
	while(*check != '\0'){
		if(!isdigit(*check)){
			return 1;
		}
		check++;
	}
	return 0;
}
