#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

///forkdown

void fork_linear(int num_proc)
{
		int status = 0;
		int fork_flag = 100;
		pid_t mypid = getpid();
		pid_t par_pid = 0;
		printf("I'm parent, pid = %u\n", mypid);
		for (int i = 0; i < num_proc; i++)
		{
			fork_flag = fork();
			if (!fork_flag)
			{
				printf("I'm child #%d, pid = %u, par_pid = %u\n", i, getpid(), getppid());
				return;
			}
		}
		for (int i = 0; i < num_proc; i++)
		{
			printf("I'm parent, waiting... pid = %u\n", mypid);
			wait(&status);
		}

}

void fork_down(int num_proc)
{
		int status = 0;
		int fork_flag = 100;
		printf("I'm parent, pid = %u\n", getpid());

		forkflag = fork();
}


int main(int argc, char** argv)
{
	if (argc == 1)
	{
		printf("Not enough arguments\n");
		exit(0);
	}
	else
	{
		int num_proc = atoi(argv[1]);
		fork_linear(num_proc);

		return 0;
	}

}