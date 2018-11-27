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

/*******************************************************************************
 * main( )
 ******************************************************************************/
int main(int argc, char *argv[]) {
	int receivedID; // indicates who we should reply to
	name_attach_t *att;
	int status = 0;

	Display display;

	/* Configure as a server, register the name_space */
	if ((att = name_attach(NULL, "dev/punch-press-display", 0)) == NULL) {
		return EXIT_FAILURE;
	}

	while(1){
		/* Receive a package from controller */
		receivedID = MsgReceive(att->chid, &display, sizeof(display), NULL);
		if(receivedID == -1){
			perror("Cannot receive the message");
			exit (EXIT_FAILURE);
		}

		if(display.index != -1){
			printf("%s", outMessage[display.index]);
			printf("\n");
		}else{
			status = 1;
		}

		MsgReply(receivedID, EOK, &status, sizeof(status));

		/* Break the loop if the message is STOP_MSG */
		if(status == 1)
			break;

	}

	name_detach(att, 0);
	printf("\nExit display\n");
	return EXIT_SUCCESS;
}
