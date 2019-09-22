#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#include "shared1.h"
struct Memory *shmPTR;
int threadSlots[10];
pthread_mutex_t mutex;

// Struct used as function parameter for multithreaded functions
typedef struct {
    unsigned int nextNumber;
	int tNumber;
} threadInformation;


void *trialDivision(void *args){
	threadInformation *number = args;
	for(int i = 1; i <= number->nextNumber; i++){
		//sleep(1);
		if(number->nextNumber % i == 0){
			pthread_mutex_lock(&mutex);
			//printf("\n");
			//printf("%u: %u ",number->nextNumber,i);
			while(shmPTR->serverFlag[number->tNumber] != 0);
			shmPTR->serverFlag[number->tNumber] = 1;
			shmPTR->slot[number->tNumber] = i;
			pthread_mutex_unlock(&mutex);
		}
	}
}

// takes a 32 bit unsigned integer and circularly rotates it by the input n. Then returns this rotated number
unsigned int bitRotate(unsigned int number, int n){
	return ((unsigned int)number << n) | (number >> (32 - n));
}

// Rotates the input number 32 times and spawns a thread for each number to continue calculation
void *beginCalculation(void *args){
	
	threadInformation *argsStruct = args;
	pthread_mutex_init(&mutex, NULL);
	pthread_t rotationThreads[32];

	for(int i = 0; i < 32; i++){
		threadInformation *rotatedNumber = malloc(sizeof(rotatedNumber));
		rotatedNumber->nextNumber = bitRotate(argsStruct->nextNumber, i);
		pthread_create(&(rotationThreads[i]), NULL, &trialDivision, rotatedNumber);
	}

	for(int i = 0; i < 32; i++){
		pthread_join(rotationThreads[i], NULL);
		printf("\nThread %d done\n",i);
	}
	printf("Threads done\n");
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

// Creates a new thread for each new input that is recieved from the client and passes the number to this thread.
void createNewThread(pthread_t threadSlots[10], int threadSlot, unsigned int number){
	
	threadInformation *threadArgs = malloc(sizeof *threadArgs);
	threadArgs->nextNumber = number;
	threadArgs->tNumber = threadSlot;
	printf("slot %d\n",threadSlot);	

	pthread_create(&(threadSlots[threadSlot]), NULL, &beginCalculation, threadArgs);
	printf("ThreadCreated\n");

}

// Waits for the handshake flag to be set by the client and reads the value in number. 
void handleInput(){

	pthread_t threadSlots[10];
	unsigned int nextNumber;

	while(shmPTR->number != -2){
		if(shmPTR->clientFlag == 1){

			int nextSlot = findNextSlot();
			
			if(nextSlot >= 0){
				printf("number: %d\n",shmPTR->number);
				printf("nextSlot is %d\n",nextSlot);
				if(shmPTR->number == -2)
					return;

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

// main function handles connection to the shared memory and initializes its values.
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