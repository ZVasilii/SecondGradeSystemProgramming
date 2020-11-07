#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

enum {BUFF_SIZE = 4096, SEM_QTY = 4, ACSESS = 0777};
enum sem{TRAP, BOAT_IN, BOAT_OUT};

void captain(int sem_id, int n_man, int boat_cap, int trap_cap, int n_cycles);
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

	if (boat_cap == n_man)
		boat_cap = n_man;

	int sem_id = semget(IPC_PRIVATE, SEM_QTY, ACSESS);
	if (sem_id < 0)
		perror("something wrong with semget");

	int pass_num = 0;
	int fork_id = 0;

	for (int i = 0; i < n_man + 1; i++, pass_num++)
	{

		fork_id = fork();

		//CHILD proccess
		if (!fork_id) 
		{
			if (!pass_num)
				captain(sem_id, n_man, boat_cap, trap_cap, n_cycles);
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


void captain(int sem_id, int n_man, int boat_cap, int trap_cap, int n_cycles)
{
	printf("Hello, i'm captain!\n");
	for (int i = 0; i < n_cycles; i++)
	{

	}
}

void passanger(int sem_id, int pass_num)
{
	printf("Hello, i'm passanger number %d!\n", pass_num);
}

void Z(int sem_id, int n_sem)
{
	struct sembuf op = {sem, 0, 0};
	int err = semop(sem_id, &op, 1);
	if (err < 0)
		perror("Something wrong with changing the semaphores (Operation Z)\n");

}

void P(int sem_id, int n_sem)
{
	struct sembuf op = {sem, -1, 0};
	int err = semop(sem_id, &op, 1);
	if (err < 0)
		perror("Something wrong with changing the semaphores (Operation P)\n");

}

void V(int sem_id, int n_sem)
{
	struct sembuf op = {sem, 1, 0};
	int err = semop(sem_id, &op, 1);
	if (err < 0)
		perror("Something wrong with changing the semaphores (Operation V)\n");

}