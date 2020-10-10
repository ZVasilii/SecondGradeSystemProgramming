#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h> 

struct msgbuf
{
	long mstype; 
	int data;   //Number of runner
};


/*
	BUFFSIZE - size of buffer for setvbuf()
	PERMISS - const for permissions for msgget
	WAIT_REGS - signal for judge to wait for all the runners to come
	LEFT - signal for judge to wait for all the runners to left
*/
enum{BUFF_SIZE = 4096, PERMISS = 0700};
enum status {WAIT_REGS = 100000000, LEFT = 10000000001};

void judge(int N_runs, int id);
void runner(int run_num, int id );


int main(int argc, char** argv)
{
	//Setting buffer for line bufferisation
	char buffer[BUFF_SIZE];
	setvbuf(stdout, buffer, _IOLBF, BUFF_SIZE);

	if (argc < 2) 
	{
		printf("Not enough arguments!\n");
		exit(0);
	}

	//N_runs - number of runners
	int N_runs = atoi(argv[1]); 

	//id of the message query
	int id = msgget(IPC_PRIVATE, PERMISS);
	if (id < 0) 
	{
		perror("Something wrong with msgget\n");
		exit(0);
	}


	int fork_id = -1; //fork() return value
	int run_num = 0;	//Runner number



	for (int i = 0; i < N_runs + 1; i++, run_num++)
	{

		fork_id = fork();

		//CHILD proccess
		if (!fork_id) 
		{
			if (!run_num)
				judge(N_runs, id);
			else
				runner(run_num, id);
			exit(0);
		}

	}

	//Waiting for all child process to finish
	for (int j = 1; j < N_runs + 1; j++)
		wait(NULL);

	//Removing message query
	if (msgctl(id, IPC_RMID, 0) < 0)
	{
		perror("Something wrong with msgctl\n");
		exit(0);
	}

	return 0;
}


void judge(int N_runs, int id)
{
	#ifdef TIME
	struct timespec old = {0,0};
	struct timespec new = {0,0};
	#endif

	int msg_nums = 0; 							//Number of messages for the judge
	struct msgbuf message = {0,0};	//Initializing struct message
	int error = -1;

	//Waiting for all the runners to come
	while (msg_nums != N_runs)
	{
		error = msgrcv(id, &message, sizeof(int), WAIT_REGS, 0);
		if (error < 0)
		{
			perror("Message not recieved(Judge)!!!\n");
			exit(0);
		}
		if (error > 0)
		{
			msg_nums ++;
		}
	}
	

	printf("JUDGE: everyone is here!\n");
	printf("JUDGE: let the race starts!\n");

	//Start the clock
	#ifdef TIME
	clock_gettime(CLOCK_MONOTONIC, &old);
	#endif

	//Starting race by triggering first runner
	message.mstype = 1;
	error = msgsnd(id, &message, sizeof(int), 0);
	if (error < 0)
	{
		perror("Message not sended(Judge)!!!\n");
		exit(0);
	}

	//Finishing the race by recieving signal from the last runner
	error = msgrcv(id, &message, sizeof(int), N_runs + 1, 0);
	if (error < 0)
	{
		perror("Message not recieved(Judge)!!!\n");
		exit(0);
	}
	if (error > 0)
	{
		printf("JUDGE: race ended!!!\n");
	}

	//Stop the clock
	#ifdef TIME	
	clock_gettime(CLOCK_MONOTONIC, &new);
	#endif

	//Trigger all the runners to leave
	for (int i = 0; i < N_runs; i++)
	{
		message.mstype = i + 1;
		error = msgsnd(id, &message, sizeof(int), 0);
		if (error < 0)
		{
			perror("Message not sended(Judge)!!!\n");
			exit(0);
		}
	}

	//Recieving signals that all the runners left
	msg_nums = 0;
	while (msg_nums != N_runs)
	{
		error = msgrcv(id, &message, sizeof(int), LEFT, 0);
		if (error < 0)
		{
			perror("Message not recieved(Judge)!!!\n");
			exit(0);
		}
		if (error > 0)
		{
			msg_nums ++;
		}
	}

	//Printing time
	#ifdef TIME
	printf(
		"JUDGE: Time: %lg, millsec\n",
		((double) (new.tv_nsec - old.tv_nsec) /
		1000000 + (double) (new.tv_sec - old.tv_sec) * 1000)
		);
	#endif

	//Judge leaves the stadium
	printf("JUDGE: leaving the stadium!!!\n");
}


void runner(int run_num, int id)
{
	struct msgbuf message = {WAIT_REGS, run_num};	//Initializing struct message
	int error = -1;

	//Sending signals to judge that runner is ready
	error = msgsnd(id, &message, sizeof(int), 0);
	if (error < 0)
	{
		perror("Message not sended!!!(Runner)\n");
		exit(0);
	}
	if (!error)
	{
		printf("Runner number %d is here!\n", message.data);
	}

	//Starting the race for each runner
	error = msgrcv(id, &message, sizeof(int), run_num, 0);
	if (error < 0)
	{
		perror("Message not recieved!!!(Runner)\n");
		exit(0);
	}
	if (error > 0)
	{
		message.data = run_num;
		printf("Runner number  %d! on the run!\n", message.data);
	}

	//Triggering the next runner to race
	message.mstype = run_num + 1;
	error = msgsnd(id, &message, sizeof(int), 0);
	if (error < 0)
	{
		perror("Message not sended!!!(Runner)\n");
		exit(0);
	}
	
	//Recieving the signal to leave the stadium
	error = msgrcv(id, &message, sizeof(int), run_num, 0);
	if (error < 0)
	{
		perror("Message not recieved!!!(Runner)\n");
		exit(0);
	}
	if (error > 0)
	{
		message.data = run_num;
		printf("Runner number  %d, leaving the stadium!\n", message.data);
	}


	//Sending the feedback to judge of leaving the stadium
	message.mstype = LEFT;
	error = msgsnd(id, &message, sizeof(int), 0);
	if (error < 0)
	{
		perror("Message not sended!!!(Runner)\n");
		exit(0);
	}
}