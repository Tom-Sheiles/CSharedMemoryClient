#include <stdint.h>

struct Memory{
	
	unsigned int number;
	int clientFlag;
	int serverFlag[10];
	int slot[10];
	float complete[10];
};