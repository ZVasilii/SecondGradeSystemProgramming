/*
Program that doing some fun things with fork
*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

void fork_down(int num_proc);
void fork_linear(int num_proc);

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
		if (!strcmp(argv[2], "l"))
			fork_linear(num_proc);
		else if (!strcmp(argv[2], "d"))
			fork_down(num_proc);
		else
		{ 
			printf("Wrong argument\n Correct are \"l\" or \"d\"");
			exit(-1);
		}

		return 0;
	}

}


void fork_linear(int num_proc)
{
		int status = 0;
		int fork_flag = 100;
		printf("I'm parent, pid = %u\n", getpid());
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
			printf("I'm parent, waiting... pid = %u\n", getpid());
			wait(&status);
		}

}

void fork_down(int num_proc)
{
	printf("I'm parent, pid = %u\n", getpid());
	int fork_flag = 100;

	for (int i = 0; i < num_proc; ++i)
  {
    fork_flag = fork();
		if (!fork_flag)
    {
      int status = 0;
      wait(&status);
      return;
    }
    printf("I'm child #%d, pid = %u, par_pid = %u\n", i, getpid(), getppid());
    continue;
  }
}
