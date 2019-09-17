#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "shared1.h"

int main(){
	
	key_t ShmKEY;
	int ShmID;
	struct Memory *shmptr;
	
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

	char buffer[1];
	while(shmptr->number != -1){
		fgets(buffer, 4, stdin);
		strtok(buffer, "\n");
		shmptr->number = atoi(buffer);
		bzero(buffer, sizeof(buffer));
	}
	
	shmdt((void *) shmptr);
	printf("Client Complete\n");
	return 0;
	
}
