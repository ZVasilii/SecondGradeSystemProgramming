#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>



#define CHK(cmd) \
	if((cmd) < 0) \
	{ \
		perror(#cmd "FAILURE!"); \
		exit(EXIT_FAILURE); \
	}

union semun 
{
   int val;
   struct semid_ds *buf;
   unsigned short  *array;
} arg;



const char hello[] = "Hello, world!\n";
const char goodbye[] = "Goodbye, world!\n";
const char SEM_QTY = 1;
enum {BUFF_SIZE = 1024, PERMISS = 0777, SEMAPH = 0};

void print_hello();
void print_goodbye();

void P(int sem_id, int n_sem);
void V(int sem_id, int n_sem);

int main()
{
	char buffer[BUFF_SIZE];
	setvbuf(stdout, buffer, _IONBF, BUFF_SIZE);
	int err = -1;

	int key = ftok("/home", 'G');
	CHK(key);

	//Creating semaphore set
	int semid = semget(key, SEM_QTY, PERMISS | IPC_CREAT);
	CHK(semid);

	struct sembuf op = {SEMAPH, 0, IPC_NOWAIT};
	int ret = semop(semid, &op, 1);

	op.sem_num = SEMAPH;  
  op.sem_op =  1;	
  op.sem_flg =  SEM_UNDO;  
	err = semop(semid, &op, 1);

	if (ret == -1 && errno == EAGAIN)
		print_goodbye();
	else if (ret != -1)
		print_hello();
	else
	{
		perror("Something wrong with changing the semaphores (Operation P)\n");
		exit(EXIT_FAILURE);
	}
	
	op.sem_num = SEMAPH;  
  op.sem_op =  1;	
  op.sem_flg =  SEM_UNDO;  
	err = semop(semid, &op, 1);

	if (err < 0)
	{
		perror("Something wrong with changing the semaphores (Operation V)\n");
		exit(EXIT_FAILURE);
	}

 
	return 0;
}

void print_hello()
{
	for (unsigned int i = 0; i < strlen(hello); i++)
	{
		putchar(hello[i]);
		sleep(1);
	}
}

void print_goodbye()
{
	for (unsigned int i = 0; i < strlen(goodbye); i++)
	{
		putchar(goodbye[i]);
		sleep(1);
	}

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
		exit(EXIT_FAILURE);
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
		exit(EXIT_FAILURE);
	}

}
