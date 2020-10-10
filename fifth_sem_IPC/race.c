#include <sys/msg.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>


struct msgbuf
{
	long mstype;
	int data;
};


enum{BUFF_SIZE = 4096, PERMISS = 0700};
enum status {WAIT_REGS = 100000000};

void judge(int N, int id);
void runner(int run_num, int id );


int main(int argc, char** argv)
{

	char buffer[BUFF_SIZE];
	setvbuf(stdout, buffer, _IOLBF, BUFF_SIZE);

	if (argc < 2) 
	{
		printf("Not enough arguments!\n");
		exit(0);
	}

	int N = atoi(argv[1]);
	///printf("%d\n", N);

	int id = msgget(IPC_PRIVATE, PERMISS);
	if (id < 0) 
	{
		perror("Something wrong with msgget\n");
		exit(0);
	}

	int fork_id = -1;
	int run_num = 0;



	for (int i = 0; i < N + 1; i++, run_num++)
	{

		fork_id = fork();

		if (!fork_id) 
		{
			///printf("My pid = %d, my parent is %d\n", getpid(), getppid());
			if (!run_num)
				judge(N, id);
				///printf("Hello, I'm judge!\n");
			else
				runner(run_num, id);
				///printf("Hello, I'm runner number %d\n", run_num);
			exit(0);
		}

	}

	for (int j = 1; j < N + 1; j++)
		wait(NULL);

	if (msgctl(id, IPC_RMID, 0) < 0)
	{
		perror("Something wrong with msgctl\n");
		exit(0);
	}

	return 0;

}

void judge(int N, int id)
{
	int msg_nums = 0;
	struct msgbuf message = {0,0};
	int error = -1;

	while (msg_nums != N)
	{
		///printf("Hello, i'm judge!\n");
		error = msgrcv(id, &message, sizeof(int), WAIT_REGS, 0);
		if (error < 0)
		{
			perror("Message not recieved(Judge)!!!\n");
			exit(0);
		}
		if (error > 0)
		{
			msg_nums ++;
			///printf("Message resieved from runner %d!\n", message.data);
		}
	}
	

	printf("JUDGE: everyone everyone is here!\n");
	printf("JUDGE: let the race starts!\n");




	message.mstype = 1;

	error = msgsnd(id, &message, sizeof(int), 0);
	if (error < 0)
	{
		perror("Message not sended(Judge)!!!\n");
		exit(0);
	}

	error = msgrcv(id, &message, sizeof(int), N + 1, 0);
		if (error < 0)
		{
			perror("Message not recieved(Judge)!!!\n");
			exit(0);
		}

	if (error > 0)
		{
			
			printf("JUDGE: race ended!!!\n");
		}


	for (int i = 1; i < N + 1; i++)
	{
		message.mstype = i;
		error = msgsnd(id, &message, sizeof(int), 0);
		if (error < 0)
		{
			perror("Message not sended(Judge)!!!\n");
			exit(0);
		}
	}

	printf("JUDGE: leaving the stadium!!!\n");




}

void runner(int run_num, int id)
{
	///printf("Hello, i'm runner number %d!\n", run_num);
	struct msgbuf message = {WAIT_REGS, run_num};
	int error = -1;
	error = msgsnd(id, &message, sizeof(int), 0);
	if (error < 0)
	{
		perror("Message not sended!!!(Runner)\n");
		exit(0);
	}
	if (!error)
		printf("Runner number %d is here!\n", run_num);

	error = msgrcv(id, &message, sizeof(int), run_num, 0);
		if (error < 0)
		{
			perror("Message not recieved!!!(Runner)\n");
			exit(0);
		}
		if (error > 0)
		{
			printf("Runner number  %d!, on the run!\n", run_num);
		}

	message.mstype = run_num + 1;
	error = msgsnd(id, &message, sizeof(int), 0);
	if (error < 0)
	{
		perror("Message not sended!!!(Runner)\n");
		exit(0);
	}
	
	error = msgrcv(id, &message, sizeof(int), run_num, 0);
		if (error < 0)
		{
			perror("Message not recieved!!!(Runner)\n");
			exit(0);
		}
		if (error > 0)
		{
			printf("Runner number  %d, leaving the stadium!\n", run_num);
		}


}