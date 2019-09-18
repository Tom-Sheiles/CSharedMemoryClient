#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <pthread.h>

#include "shared1.h"
struct Memory *shmPTR;
int threadSlots[10];

typedef struct {
    int nextNumber;
} threadInformation;


void *trialDivision(){
	//printf("Trial Div\n");
}


void *beginCalculation(void *args){
	
	threadInformation *argsStruct = args;
	pthread_t rotationThreads[32];

	for(int i = 0; i < 33; i++)
		pthread_create(&(rotationThreads[i]), NULL, &trialDivision, NULL);

	for(int i = 0; i < 33; i++)
		pthread_join(rotationThreads[i], NULL);

	/*while(1){
		printf("Thread Value: %d\n", argsStruct->nextNumber);
		sleep(1);
	}*/
}

// Finds the next avaliable thread slot, sets it to 1 and returns the index of its location or -1 if threads are full
int findNextSlot(){
	for(int i = 0; i < 11; i++){
		if(threadSlots[i] == 0){
			threadSlots[i] = 1;
			return i;
		}
	}
	return -1;
}

void createNewThread(pthread_t threadSlots[10], int threadSlot, int number){
	
	threadInformation *threadArgs = malloc(sizeof *threadArgs);
	threadArgs->nextNumber = number;
	printf("slot %d\n",threadSlot);	

	pthread_create(&(threadSlots[threadSlot]), NULL, &beginCalculation, threadArgs);
	printf("ThreadCreated\n");

}

void handleInput(){

	pthread_t threadSlots[10];
	int nextNumber;

	while(shmPTR->number != -2){
		if(shmPTR->clientFlag == 1){

			int nextSlot = findNextSlot();
			
			if(nextSlot >= 0){
				printf("number: %d\n",shmPTR->number);
				printf("nextSlot is %d\n",nextSlot);

				nextNumber = shmPTR->number;
				shmPTR->clientFlag = 0;
				shmPTR->number = nextSlot;
				printf("...CreatingSlot...\n");
				createNewThread(threadSlots, nextSlot, nextNumber);
			}else{
				shmPTR->clientFlag = 0;
				shmPTR->number = nextSlot;
			}
		}
	}

}

int main(int argc, char *argv[]){
	
	key_t ShmKEY;
	int ShmID;

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
		threadSlots[i] = 0;
	}
	
	handleInput();
	
	shmdt((void *) shmPTR);
	printf("Server Disconnected\n");
	shmctl(ShmID, IPC_RMID, NULL);
	return 0;	
	
}