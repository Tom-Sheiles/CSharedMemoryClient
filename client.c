#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "shared1.h"

struct Memory *shmptr;

void handleInput(){

	char buffer[1];
	while(shmptr->number != -2){

		if(shmptr->clientFlag == 0){
			fgets(buffer, 4, stdin);
			strtok(buffer, "\n");
			shmptr->number = atoi(buffer);
			shmptr->clientFlag = 1;
			bzero(buffer, sizeof(buffer));

			//Wait for server to accept request
			while(shmptr->clientFlag != 0);
			if(shmptr->number != -1){
				printf("Server Started request in slot: %d\n",shmptr->number);
			}else{
				printf("Server cannot handle more than 10 requests\n");
			}
		}
	}

}


int main(){
	
	key_t ShmKEY;
	int ShmID;
	
	ShmKEY = ftok(".", 'x');
	ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
	if(ShmID < 0){
		printf("*** shmget error(client) *** \n");
		int error = errno;
		printf("%d\n",error);
		exit(1);
	}
	
	shmptr = (struct Memory *) shmat(ShmID, NULL, 0);
	printf("Client Connected\n");

	handleInput();
	
	shmdt((void *) shmptr);
	printf("Client Complete\n");
	return 0;
	
}
