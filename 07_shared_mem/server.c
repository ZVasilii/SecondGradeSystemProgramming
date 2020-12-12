/*
	Communication between client and server through 
	Shared memory
	"0\n" - end of communication
*/

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>
#include<errno.h>
#include <fcntl.h>
#include <string.h>

#define CHK(cmd) \
	if((cmd) < 0) \
	{ \
		perror("#cmd FAILURE!"); \
		exit(EXIT_FAILURE); \
	}
	
const int MEM_SIZE = 4096;
const int PERMISS = 0777;
const int SEM_QTY = 2;

enum sem {CLIENT = 0, SERVER = 1};



void P(int sem_id, int n_sem);
void V(int sem_id, int n_sem);
void Z(int sem_id, int n_sem);

int main()
{
	//Generating key
	int key = ftok("/home", 'S');
	CHK(key);
	
	//Creating shared memory object
	int id = shmget(key, MEM_SIZE, PERMISS | IPC_CREAT);
	CHK(id);

	char* sh_mem = (char*) shmat (id, NULL, 0);

	if(sh_mem == NULL || ((void*) sh_mem == (void*) -1))
	{
		perror("Something wrong with shmat");
		exit(-1);
	}
	
	//Creating semaphore set
	int semid = semget(key, SEM_QTY, PERMISS | IPC_CREAT);
	CHK(semid);
	unsigned short initial[2] = {0, 1};
  semctl(semid, 0, SETALL, initial);

	
	printf("Server start working\n");
	while(1)
	{
		//****CRIT CECTION****//
		P(semid, CLIENT);
		 if (sh_mem[0] != '\0')
    {
    	if(!strcmp(sh_mem, "0\n"))
    		break;
      printf("%s", sh_mem);
      sh_mem[0] = '\0';
    }
    V(semid, SERVER);
    //****CRIT CECTION****//
	}
	
	CHK(semctl(semid, SEM_QTY, IPC_RMID));
	shmdt(sh_mem);
	return 0;
}

void P(int sem_id, int n_sem)
{
	struct sembuf op = {0};
	  op.sem_num = n_sem;  
    op.sem_op =  -1;	
    op.sem_flg =  0;  
	int err = semop(sem_id, &op, 1);
	if (err < 0)
	{
		perror("Something wrong with changing the semaphores (Operation P)\n");
		exit(-1);
	}

}


void V(int sem_id, int n_sem)
{
		struct sembuf op = {0};
	  op.sem_num = n_sem;  
    op.sem_op =  1;	
    op.sem_flg =  0;  
	int err = semop(sem_id, &op, 1);
	if (err < 0)
	{
		perror("Something wrong with changing the semaphores (Operation V)\n");
		exit(-1);
	}

}

void Z(int sem_id, int n_sem)
{
	struct sembuf op = {0};
	  op.sem_num = n_sem;  
    op.sem_op =  0;	
    op.sem_flg =  0;  
	int err = semop(sem_id, &op, 1);
	if (err < 0)
	{
		perror("Something wrong with changing the semaphores (Operation Z)\n");
		exit(-1);
	}
}