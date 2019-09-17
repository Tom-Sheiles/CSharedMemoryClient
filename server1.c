#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include "shared1.h"

int main(int argc, char *argv[]){
	
	key_t ShmKEY;
	int ShmID;
	struct Memory *shmPTR;

//-------------------------- Connect to shared memory
	ShmKEY = ftok(".", 'x');
	ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
	if(ShmID < 0){
		printf("*** shmget error (server) %d***\n",ShmID);
		int error = errno;
		printf("%d\n",error);
		exit(1);
	}
	
	shmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	if(shmPTR == NULL){
		printf("*** shmat error (server) ***\n");
		exit(1);
	}
	printf("Server has connected to shared memory\n");
	printf("Waiting For Client Connection...\n");

//--------------------------

	shmPTR->clientFlag = 0;
	shmPTR->number = 0;
	for(int i = 0; i < 10; i++){
		shmPTR->serverFlag[i] = 0;
		shmPTR->slot[i] = 0;
	}
	shmPTR->slot[10] = 1;

	printf("%d %d %d\n", shmPTR->serverFlag[10], shmPTR->slot[10], shmPTR->clientFlag);
	
	while(shmPTR->number != -1){
		sleep(1);
		printf("%d\n",shmPTR->number);
	}
	
	shmdt((void *) shmPTR);
	printf("Server Disconnected\n");
	shmctl(ShmID, IPC_RMID, NULL);
	return 0;	
	
}