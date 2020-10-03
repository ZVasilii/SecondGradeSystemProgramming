#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h> 
#include <getopt.h>


enum {OPLEN = 10, BUFSIZE = 4096};
const char optstring[OPLEN] = "+ q e f";



int main(int argc, char** argv)
{
	struct option quiet = {"quiet", 0, NULL, 'q'};
	int fork_flag = 100;
	char q_flag = 0;
	char opt_flag = 0;

	if (argc < 2)
	{
		printf("Not enough arguments\n");
		exit(0);
	}


while((opt_flag = getopt_long(argc, argv, optstring, &quiet, NULL)) > 0)	
	if (opt_flag == 'q')
		q_flag = 1;


	char* buff = (char*) calloc(BUFSIZE, sizeof(char));

	size_t read_num = 0;
	size_t write_num = 0;
	int in_word = 0;


	size_t n_bytes = 0;
	size_t n_strings = 0;
	size_t n_words = 0;

	#ifdef TIME
	struct timespec old = {0,0};
	struct timespec new = {0,0};
	#endif

	int fd[2];

	if (pipe(fd) < 0)
		perror("Something wrong with pipe!\n");

	fork_flag = fork();

//PARENT
	if (fork_flag > 0)
	{

		#ifdef TIME
		clock_gettime(CLOCK_MONOTONIC, &old);
		#endif

		///printf("File descriptors: %d, %d\n", fd[0], fd[1]);   //fd[0] for read, fd[1] for write;

		if (close(fd[1]) < 0)
			perror("Something wrong closing fd[1]\n");

		wait(NULL);


		while ((read_num = read(fd[0] , buff, BUFSIZE)) > 0)
		{
			if(read_num < 0)
				perror("Something wrong with read\n");

			if (!q_flag)
			{ 
				write_num = write(1, buff, read_num);
				if(write_num < 0)
					perror("Something wrong with write\n");
			}	

			for(int i = 0; i < read_num + 1; i++)
			{

				if (isspace(buff[i]))
				{
					in_word = 0;
				}

				else if ((isspace(buff[i]) == 0) && (in_word == 0))
				{
					in_word = 1;
					n_words ++;
				}

				

				if (buff[i] == '\n')
					n_strings ++;
			}
			n_bytes += write_num;



		}

		if (n_strings == 0)
				n_words++;-

		fprintf(stderr, "\t%d\t%d\t%d\n",  n_strings, n_words - 1, n_bytes);


		close(fd[0]);

	}

//CHILD
	if (!fork_flag)
	{
		if (close(fd[0]) < 0)
			perror("Something wrong closing fd[0]\n");
		dup2(fd[1], 1);

		if(execvp(argv[optind ], (argv + optind)) < 0)
			perror("Incorrect exec\n");
		
	}


	#ifdef TIME
	wait(NULL);
	clock_gettime(CLOCK_MONOTONIC, &new);
	printf("Time: %lg, millsec\n", ((double) (new.tv_nsec - old.tv_nsec) / 1000000 + (double) (new.tv_sec - old.tv_sec) * 1000));
	#endif
	
	free(buff);

	return 0;
	

}