#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <pthread.h>

#include "shared1.h"

struct Memory *shmptr;
int inputNumbers[10];

typedef struct{
	int inputNumber;
}returnInfo;

typedef struct{
	unsigned int *array;
	size_t used;
	size_t size;
}Vector;

void CreateVector(Vector *a, size_t initSize){
	a->array = (unsigned int *)malloc(initSize * sizeof(unsigned int));
	a->used = 0;
	a->size = initSize;
}

void pushVector(Vector *a, unsigned int x){
	if(a->used == a->size){
		a->size *= 2;
		a->array = (unsigned int *)realloc(a->array, a->size * sizeof(unsigned int));
	}
	a->array[a->used++] = x;
}
Vector factors[10];

void *serverOutput(){
	while(1){
		for(int i = 0; i < 10; i++){
			
			if(shmptr->serverFlag[i] == 1){
				//printf("%u ", shmptr->slot[i]);
				//fflush(stdout);
				pushVector(&factors[i], shmptr->slot[i]);
				shmptr->serverFlag[i] = 0;
			}	
		}
		
	}
}


void handleInput(){

	pthread_t resultThread;
	pthread_create(&(resultThread), NULL, &serverOutput, NULL);
	

	char buffer[1];
	while(shmptr->number != -2){

	

		if(shmptr->clientFlag == 0){
			fgets(buffer, 4, stdin);
			strtok(buffer, "\n");
			int number = atoi(buffer);
			shmptr->number = number;
			shmptr->clientFlag = 1;

			// ########TODO FINISH PRINT STATEMENTS FROM HERE AND DO PROGRESS BAR
			if(atoi(buffer) == -3){
				
				for(int j = 0; j < 10; j++){
					printf("Slot %d\n",j);
					for(int k = 0; k < factors[j].used; k++){
						printf("%u ",factors[j].array[k]);
					}
					printf("\n");
				}
				shmptr->number = -2;
				return;
			}
			if(atoi(buffer) == -2)
				return;

			bzero(buffer, sizeof(buffer));

			//Wait for server to accept request
			while(shmptr->clientFlag != 0);
			if(shmptr->number != -1){
				printf("Server Started request in slot: %d\n",shmptr->number);
				inputNumbers[shmptr->number] = number;
				CreateVector(&factors[shmptr->number], 100);
				
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
