/*
	Simulating work of drawbridge
	Using shared mem && semaphores
	first arg - number of ships
	second arg - number of cars
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

//Shared structure
struct shm_unit
{
	unsigned car_waiting;
	unsigned ships_waiting;
};


enum {BUFF_SIZE = 4096, SEM_QTY = 4, SHM_SIZE = 1024, PERMISS = 0777, MAX_SHIPS = 3};
enum sem{CAR, SHIP, BRIDGE, SHARED};

void car(int num, int sem_id, struct shm_unit* shm);
void ship(int num, int sem_id, struct shm_unit* shm);

void Z(int sem_id, int n_sem);
void P(int sem_id, int n_sem);
void V(int sem_id, int n_sem);

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printf("Too few arguments!\n");
		printf("Correct: ./a.out <ships_num> <cars_num>\n");
		exit(0);
	}

	//Setting stdout mode
	char buffer[BUFF_SIZE];
	setvbuf(stdout, buffer, _IOLBF, BUFF_SIZE);

	int n_car = atoi(argv[1]);
	int n_ship = atoi(argv[2]);

	//Creating semaphore array
	int sem_id = semget(IPC_PRIVATE, SEM_QTY, PERMISS | IPC_CREAT);
	if (sem_id < 0)
	{
		perror("something wrong with semget");
		exit(EXIT_FAILURE);
	}

	//Creating shared structure
	int shm_id = shmget(IPC_PRIVATE, SHM_SIZE, PERMISS | IPC_CREAT);
	if (shm_id < 0)
	{
		perror("something wrong with shmget");
		exit(EXIT_FAILURE);
	}
  struct shm_unit* shm = (struct shm_unit*) calloc (1, sizeof(struct shm_unit));
  shm = (struct shm_unit*) shmat(shm_id, NULL, 0);
  if (shm == NULL || (void*) shm == (void*) -1)
  {
    perror("Something wrong with shmat\n");
    exit(EXIT_FAILURE);
  }

  //Initializing
  shm->car_waiting = 0;
  shm->ships_waiting = 0;
  V(sem_id, SHARED);
  V(sem_id, BRIDGE);
  printf("***STARTING***\n");
  if (n_ship >= n_car)
  {
  	V(sem_id, SHIP);
  	#ifdef PRINT
  	printf("***Bridge: UP!***\n");
  	#endif
  }
  else
  {
  	V(sem_id, CAR);
  	#ifdef PRINT
  	printf("***Bridge: DOWN!***\n");
  	#endif
  }

  //Creating boat && car processes
	int fork_id = -1;
	for (int i = 0; i < n_ship + n_car; i++)
	{
		fork_id = fork();
		if (!fork_id)
		{
			if (i < n_ship)
				ship(i, sem_id, shm);
			else
				car(i - n_ship, sem_id, shm);
			exit(0);
		}
	}

	#ifdef PRINT
	printf("Hello, I'm parent!\n");
	#endif

	//Waiting and removing everything
	for (int j = 1; j < n_car + n_ship; j++)
		wait(NULL);

	if (semctl(sem_id, SEM_QTY, IPC_RMID) < 0)
	{
		perror("Something wrong with semctl\n");
		exit(0);
	}
	if (shmctl(shm_id, IPC_RMID,  (struct shmid_ds*)shm) < 0)
	{
		perror("Something wrong with shmctl\n");
		exit(0);
	}

	printf("***FINISHING***\n");

	return 0;
}

void car(int num, int sem_id, struct shm_unit* shm)
{
	#ifdef PRINT
	printf("Hello, i'm car #%d!\n", num);
	#endif

	num++;

	P(sem_id, SHARED);
		shm->car_waiting ++;
		printf("Car #%d!, waiting to enter the bridge!\n", num);
	V(sem_id, SHARED);

	P(sem_id, CAR);
	//******CRITICAL SECTION*****
	P(sem_id, BRIDGE);

	printf("Car #%d!, passing the brige!\n", num);

	P(sem_id, SHARED);
		shm->car_waiting --;
		printf("Car #%d!, passed the bridge!\n", num);

		if (shm->ships_waiting > MAX_SHIPS || ((shm->car_waiting == 0) && (shm->ships_waiting != 0)))
		{
			#ifdef PRINT
			printf("***Bridge: UP!***\n");
			#endif
			V(sem_id, SHIP);
		}
		else
		{
			V(sem_id, CAR);
			#ifdef PRINT
			printf("***Bridge: DOWN!***\n");
			#endif
		}
	V(sem_id, SHARED);
	printf("Car #%d!, i'm dead now!\n", num);
	V(sem_id, BRIDGE);
	//******CRITICAL SECTION*****
	return;
}

void ship(int num, int sem_id, struct shm_unit* shm)
{
	#ifdef PRINT
	printf("Hello, i'm ship #%d!\n", num);
	#endif

	num++;

	P(sem_id, SHARED);
		shm->ships_waiting ++;
		printf("Ship #%d!, waiting to enter the bridge!\n", num);
	V(sem_id, SHARED);

	P(sem_id, SHIP);
	//******CRITICAL SECTION*****
	P(sem_id, BRIDGE);
	printf("Ship #%d!, passing the brige!\n", num);

	P(sem_id, SHARED);
		shm->ships_waiting --;
		printf("Ship #%d!, passed the bridge!\n", num);

		if (shm->ships_waiting > MAX_SHIPS || ((shm->car_waiting == 0) && (shm->ships_waiting != 0)))
		{
			V(sem_id, SHIP);
			#ifdef PRINT
			printf("***Bridge: UP!***\n");
			#endif
		}
		else
		{
			V(sem_id, CAR);
			#ifdef PRINT
			printf("***Bridge: DOWN!***\n");
			#endif
		}
	V(sem_id, SHARED);
	printf("Ship #%d!, i'm dead now!\n", num);
	V(sem_id, BRIDGE);
	//******CRITICAL SECTION*****
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
		exit(EXIT_FAILURE);
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