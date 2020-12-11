/*
Function that execute file and counting time
 */

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h> 

int main(int argc, char** argv)
{
	int fork_flag = 100;
	if (argc < 2)
	{
		printf("Not enough arguments\n");
		exit(0);
	}

	struct timespec old = {0,0};
	struct timespec new = {0,0};

	fork_flag = fork();
	if (fork_flag > 0)
		clock_gettime(CLOCK_MONOTONIC, &old);

	if (!fork_flag)
	{
		
		execvp(argv[1], (argv + 1));
		perror("Incorrect exec");
		exit(0);

	}

	wait(NULL);
	clock_gettime(CLOCK_MONOTONIC, &new);

	printf("Time: %lg, millsec\n", ((double) (new.tv_nsec - old.tv_nsec) / 1000000 + (double) (new.tv_sec - old.tv_sec) * 1000));
	
	return 0;
	

}