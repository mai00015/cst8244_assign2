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

#include "mystruct.h"

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL


int checkInt (const char *operand);
/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(int argc, char *argv[]) {

	Operator operator;
	State checkState = READY_STATE;

	int server_coid, checkValid = 0, status = 0;
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
			}else if(strcmp(input, inMessage[EMER_BUTTON]) == 0){
				MsgSendPulse(server_coid, SchedGet(0,0,NULL), _PULSE_CODE_MINAVAIL, 1);
				checkValid = 1;
				status = 1;
			}
			else{
				//strcpy(operator.input, input);

				// Check the state: READY_STATE
				if(checkState == READY_STATE){
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
					}
					else
						checkValid = 0;

					// Check the state: ARMED_STATE
				}else if(checkState == ARMED_STATE){
					if(operator.direction == LEFT){
						if(strcmp(input, inMessage[RIGHT_BUTTON_DOWN]) == 0){
							checkValid = 1;
							operator.curr = ARMED_STATE;
						}else if(strcmp(input, inMessage[LEFT_BUTTON_UP]) == 0){
							checkValid = 1;
							operator.curr = READY_STATE;
						}else
							checkValid = 0;
					}else if(operator.direction == RIGHT){
						if(strcmp(input, inMessage[LEFT_BUTTON_DOWN]) == 0){
							checkValid = 1;
							operator.curr = ARMED_STATE;
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

		// send the message
		if(strcmp(input, inMessage[EMER_BUTTON]) != 0){
			if (MsgSend (server_coid, &operator, sizeof(operator), &checkState, (int)sizeof(checkState)) == -1) {
				fprintf (stderr, "Error during MsgSend\n");
				perror (NULL);
				exit (EXIT_FAILURE);
			}
		}
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
