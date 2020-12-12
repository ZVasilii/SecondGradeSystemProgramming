/*
	Communication between client and server through 
	Shared memory
	"0\n" - end of communication
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
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
enum {STD_MOD, FIL_MOD, MAXLEN = 1024};

void P(int sem_id, int n_sem);
void V(int sem_id, int n_sem);
void Z(int sem_id, int n_sem);



int main(int argc, char** argv)
{
	int mode = -1;
	int fd = -1;
	FILE* file = NULL;
	char buffer[MAXLEN] = "";
	char* newline = NULL;
	int errno = -1;
	if (argc == 1)
		mode = STD_MOD;
	else
		mode = FIL_MOD;

	//Generating key
	key_t key = ftok("/home", 'S');
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
	

	if (mode == STD_MOD)
	{
		printf("Client start working\n");
		file = fdopen(0, "r");			

		while ((newline = fgets(buffer, MAXLEN, file)) != NULL)
		{
			//****CRIT CECTION****//
			P(semid, SERVER);
			strcpy(sh_mem, newline);
			V(semid, CLIENT);
			//****CRIT CECTION****//
		}

		
	}

	if (mode == FIL_MOD)
	{
    printf("Client start working\n");
		for (int i = 1; i < argc; i++)
		{
			fd = open(argv[i], O_RDONLY);
			CHK(fd);
			file = fdopen(fd, "r");

			while ((newline = fgets(buffer, MAXLEN, file)) != NULL)
			{
				P(semid, SERVER);
				strcpy(sh_mem, newline);
				V(semid, CLIENT);
			}
		}
	}
	errno = fclose(file);
		CHK(errno);

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
