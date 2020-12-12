#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>



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

	int key = ftok("/home", 'S');
	CHK(key);

	//Creating semaphore set
	int semid = semget(key, SEM_QTY, PERMISS | IPC_CREAT);
	CHK(semid);

	union semun arg = {0};
  semctl(semid, SEMAPH, SETVAL, arg.val);

  V(semid, SEMAPH);

  int sem_val =  semctl(semid, SEMAPH , GETVAL);
  printf("arg = %d\n", sem_val);

	//******CRITICAL SECTION*****//
  if (sem_val == 1)
  	print_hello();
  else
  	print_goodbye();
  //******CRITICAL SECTION*****//
	P(semid, SEMAPH);
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
