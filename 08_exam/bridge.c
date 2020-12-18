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
//bridge_status == 0 - DOWN
//bridge_status == 1 - UP
struct shm_unit
{
	unsigned car_waiting;
	unsigned ships_waiting;
};


struct shm_bridge
{
	unsigned bridge_status;
};


enum {BUFF_SIZE = 4096, SEM_QTY = 5, SHM_SIZE = 1024, PERMISS = 0777, MAX_SHIPS = 3, DELAY_T = 1000000};
enum sem{CAR, SHIP, BRIDGE, SHARED, SHARED_BR};

void car(int num, int sem_id, struct shm_unit* shar_mem, struct shm_bridge* shar_br);
void ship(int num, int sem_id, struct shm_unit* shar_mem, struct shm_bridge* shar_br);

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
  struct shm_unit* shar_mem = (struct shm_unit*) calloc (1, sizeof(struct shm_unit));
  shar_mem = (struct shm_unit*) shmat(shm_id, NULL, 0);
  if (shar_mem == NULL || (void*) shar_mem == (void*) -1)
  {
    perror("Something wrong with shmat\n");
    exit(EXIT_FAILURE);
  }


  //Creating shared structure number 2
	int shm_id_2 = shmget(IPC_PRIVATE, SHM_SIZE, PERMISS | IPC_CREAT);
	if (shm_id_2 < 0)
	{
		perror("something wrong with shmget");
		exit(EXIT_FAILURE);
	}
  struct shm_bridge* shar_br = (struct shm_bridge*) calloc (1, sizeof(struct shm_bridge));
  shar_br = (struct shm_bridge*) shmat(shm_id_2, NULL, 0);
  if (shar_br == NULL || (void*) shar_br == (void*) -1)
  {
    perror("Something wrong with shmat (Bridge)\n");
    exit(EXIT_FAILURE);
  }

  //Initializing
  shar_mem->car_waiting = 0;
  shar_mem->ships_waiting = 0;
  V(sem_id, SHARED);
  V(sem_id, BRIDGE);
  V(sem_id, SHARED_BR);
  printf("***STARTING***\n");
 
  
	V(sem_id, CAR);

	printf("Starting condition: BRIDGE FELL!\n");
	shar_br -> bridge_status = 0;
	usleep(DELAY_T);

  

  //Creating boat && car processes
	int fork_id = -1;
	for (int i = 0; i < n_ship + n_car; i++)
	{
		fork_id = fork();
		if (!fork_id)
		{
			if (i % 2)
				ship(i / 2, sem_id, shar_mem, shar_br);
			else
				car((i + 1) / 2, sem_id, shar_mem, shar_br);
		}
	}


	//Waiting and removing everything
	for (int j = 0; j < n_car + n_ship; j++)
		wait(NULL);

	if (semctl(sem_id, SEM_QTY, IPC_RMID) < 0)
	{
		perror("Something wrong with semctl\n");
		exit(EXIT_FAILURE);
	}
	if (shmctl(shm_id, IPC_RMID,  (struct shmid_ds*) shar_mem) < 0)
	{
		perror("Something wrong with shmctl\n");
		exit(EXIT_FAILURE);
	}

	if (shmctl(shm_id_2, IPC_RMID,  (struct shmid_ds*) shar_mem) < 0)
	{
		perror("Something wrong with shmctl\n");
		exit(EXIT_FAILURE);
	}

	printf("***FINISHING***\n");

	return 0;
}

void car(int num, int sem_id, struct shm_unit* shar_mem, struct shm_bridge* shar_br)
{

	#ifdef DELAY
	usleep(DELAY_T);
	#endif

	num++;

	P(sem_id, SHARED);
		shar_mem->car_waiting ++;
		printf("Car #%d!, waiting to enter the bridge!\n", num);
	V(sem_id, SHARED);

	P(sem_id, CAR);
	//******CRITICAL SECTION*****
	P(sem_id, BRIDGE);

	//RETURNING IF THERE ARE SHIPS IN FRONT OF THE BRIDGE
	P(sem_id, SHARED);
	if (shar_mem->ships_waiting > MAX_SHIPS)
	{
		V(sem_id, SHARED);
		P(sem_id, SHARED_BR);
		if (shar_br -> bridge_status != 1)
			  	printf("***Bridge: RISING!***\n");
					shar_br -> bridge_status = 1;
					#ifdef DELAY
					usleep(DELAY_T);
					#endif
					printf("***Bridge: RISED!***\n");
		V(sem_id, SHARED_BR);
		V(sem_id, SHIP);
		V(sem_id, BRIDGE);

	}
	else
	{
		V(sem_id, SHARED);
	}

	printf("Car #%d!, passing the brige!\n", num);

	#ifdef DELAY
	usleep(DELAY_T);
	#endif


	P(sem_id, SHARED);
		shar_mem->car_waiting --;
		printf("Car #%d!, passed the bridge!\n", num);

		if (shar_mem->ships_waiting > MAX_SHIPS || ((shar_mem->car_waiting == 0) && (shar_mem->ships_waiting != 0)))
		{
			V(sem_id, SHARED);
			P(sem_id, SHARED_BR);
			if (shar_br -> bridge_status != 1)
			  	printf("***Bridge: RISING!***\n");
					shar_br -> bridge_status = 1;
					#ifdef DELAY
					usleep(DELAY_T);
					#endif
					printf("***Bridge: RISED!***\n");
			V(sem_id, SHARED_BR);
			V(sem_id, SHIP);
		}
		else
		{
			V(sem_id, SHARED);
			V(sem_id, CAR);
		}
	V(sem_id, BRIDGE);
	//******CRITICAL SECTION*****
	exit(0);
}

void ship(int num, int sem_id, struct shm_unit* shar_mem, struct shm_bridge* shar_br)
{

	#ifdef DELAY
	usleep(DELAY_T);
	#endif
	
	num++;

	#ifdef DELAY
	usleep(DELAY_T);
	#endif

	if (num > 5)
	{
		#ifdef DELAY
		for (int j = 0; j < 10; j++)
			usleep(DELAY_T);
		#endif
	}

	P(sem_id, SHARED);
		shar_mem->ships_waiting ++;
		printf("Ship #%d!, waiting to enter the bridge!\n", num);
	V(sem_id, SHARED);

	P(sem_id, SHIP);
	//******CRITICAL SECTION*****
	P(sem_id, BRIDGE);
	printf("Ship #%d!, passing the brige!\n", num);

	#ifdef DELAY
	usleep(DELAY_T);
	#endif

	P(sem_id, SHARED);
		shar_mem->ships_waiting --;
		printf("Ship #%d!, passed the bridge!\n", num);

		if (shar_mem->ships_waiting != 0)
		{
			V(sem_id, SHARED);
			V(sem_id, SHIP);
		}
		else
		{
			V(sem_id, SHARED);
			V(sem_id, CAR);
			P(sem_id, SHARED_BR);
	  	printf("***Bridge: FALLING!***\n");
	  	shar_br -> bridge_status = 0;
	  	#ifdef DELAY
	  	usleep(DELAY_T);
	  	#endif
	  	printf("***Bridge: FELL!***\n");
	  	V(sem_id, SHARED_BR);

		}
	V(sem_id, BRIDGE);
	//******CRITICAL SECTION*****
	exit(0);
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