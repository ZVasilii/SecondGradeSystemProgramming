/*
Client-server communications through semaphores
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

 union semun 
 {
	 int val;
	 struct semid_ds *buf;
	 unsigned short  *array;
 };


enum {BUFF_SIZE = 4096, SEM_QTY = 100, ACSESS = 0777};
enum sem{TRAP_DOWN,  LAST_TRIP, MEN_ON_BOAT, TO_COAST, TRIP_END, TRIP_START, MEN_ON_TRAP};

void captain(int sem_id, int boat_cap, int trap_cap, int n_cycles);
void passanger(int sem_id, int pass_num);

void Z(int sem_id, int n_sem);
void P(int sem_id, int n_sem);
void V(int sem_id, int n_sem);


int main(int argc, char** argv)
{
	//Setting buffer for line bufferisation
	char buffer[BUFF_SIZE];
	setvbuf(stdout, buffer, _IOLBF, BUFF_SIZE);

	if (argc < 5) 
	{
		printf("Not enough arguments!\n");
		exit(0);
	}

	int n_man = atoi(argv[1]);
	int boat_cap = atoi(argv[2]);
	int trap_cap = atoi(argv[3]);
	int n_cycles = atoi(argv[4]);
	printf("Number of passangers: %d\n", n_man);
	printf("Boat capacity: %d\n", boat_cap);
	printf("Trap capacity: %d\n", trap_cap);
	printf("Number of voyages: %d\n", n_cycles);


	int sem_id = semget(IPC_PRIVATE, SEM_QTY, ACSESS);
	if (sem_id < 0)
		perror("something wrong with semget");

	int pass_num = 0;
	int fork_id = 0;

	union semun arg = {0};

  V(sem_id, TRAP_DOWN);
  V(sem_id, TRIP_END);
  V(sem_id, TRIP_START);

  arg.val = trap_cap;
  semctl(sem_id, MEN_ON_TRAP, SETVAL, arg);
  
  if (boat_cap < n_man)
  {
  	arg.val = boat_cap;
  	semctl(sem_id, MEN_ON_BOAT, SETVAL, arg);
  	semctl(sem_id, TO_COAST, SETVAL, arg);
  }
  else
  {
  	arg.val = n_man;
  	semctl(sem_id, MEN_ON_BOAT, SETVAL, arg);
  	semctl(sem_id, TO_COAST, SETVAL, arg);
  }


	for (int i = 0; i < n_man + 1; i++, pass_num++)
	{

		fork_id = fork();

		//CHILD proccess
		if (!fork_id) 
		{
			if (!pass_num)
				captain(sem_id, boat_cap, trap_cap, n_cycles);
			else
				passanger(sem_id, pass_num);
			exit(0);
		}

	}

	for (int j = 1; j < pass_num + 1; j++)
		wait(NULL);

	//Removing message query
	if (semctl(sem_id, IPC_RMID, 0) < 0)
	{
		perror("Something wrong with msgctl\n");
		exit(0);
	}

	return 0;
}


void captain(int sem_id, int boat_cap, int trap_cap, int n_cycles)
{
	union semun arg = {0};

	printf("Hello, i'm captain!\n");

	P(sem_id, TRAP_DOWN);
  P(sem_id, TRIP_START);
	for (int i = 0; i < n_cycles - 1; i++)
	{
    if (i == n_cycles - 1)
      V(sem_id, LAST_TRIP);

  	printf("Hello stranger, welcome to our board!\n");
    
    
    Z(sem_id, MEN_ON_BOAT);
    V(sem_id, TRAP_DOWN);

    printf("Waiting for trap to clear\n");

    arg.val = (short int)-trap_cap;
 	  semctl(sem_id, MEN_ON_TRAP, SETVAL, arg);
 	  arg.val = trap_cap;
 	  semctl(sem_id, MEN_ON_TRAP, SETVAL, arg);

    printf("STAAARRRRRTTTTTTT!!!\n"); 
    printf("EEEEENNNNNNDDDDDD!!!!\n");
    
    V(sem_id, TRIP_START);
    P(sem_id, TRIP_END);

    P(sem_id, TRAP_DOWN);
    
    Z(sem_id, TO_COAST);
    printf("Boat is empty now!\n");
    V(sem_id, TRIP_END);
    arg.val = boat_cap;
 	  semctl(sem_id, TO_COAST, SETVAL, arg);
    
    if (i == n_cycles - 1)
      break;
    
    P(sem_id, TRIP_START);
  }
  printf("Goodbye everyone!\n");
  return;


	
}

void passanger(int sem_id, int pass_num)
{
	printf("Hello, i'm passanger number %d!\n", pass_num);
  while (1)
  {
    Z(sem_id, TRAP_DOWN);
    if (semctl(sem_id, LAST_TRIP, GETVAL))
      break;

    P(sem_id, MEN_ON_BOAT);
    
    if (semctl(sem_id, LAST_TRIP, GETVAL))
      break;

    P(sem_id, MEN_ON_TRAP);
    printf("Passanger %d on trap\n", pass_num);
    V(sem_id, MEN_ON_TRAP);
    printf("Passangers %d on board\n", pass_num);
    
    Z(sem_id, TRIP_START);
    Z(sem_id, TRIP_END);

    Z(sem_id, TRAP_DOWN);
    P(sem_id, MEN_ON_TRAP);
    printf("Passanger %d on trap\n", pass_num);
    V(sem_id, MEN_ON_TRAP);
    printf("Passanger %d on board\n", pass_num);
    V(sem_id, MEN_ON_BOAT);
    P(sem_id, TO_COAST);
  }
  return;
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