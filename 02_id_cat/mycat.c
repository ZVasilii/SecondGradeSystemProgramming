/*
Equivalent to "cat" UNIX function (print file into stdout)
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>




enum {STD_MOD, FIL_MOD, MAXLEN = 1024};


int main(int argc, char** argv)
{
	int mode = -1;
	int fd = -1;
	FILE* file = NULL;
	char buffer[MAXLEN] = "";
	char* newline = NULL;
	int errno = -1;

	if (argc == 1)
		mode = STD_MOD;
	else
		mode = FIL_MOD;


	if (mode == STD_MOD)
	{
		file = fdopen(0, "r");			

		while ((newline = fgets(buffer, MAXLEN, file)) != NULL)
			printf("%s\n", newline);

		errno = fclose(file);
		if (errno < 0)
			perror("Couldn't close the file\n");
		
	}

	if (mode == FIL_MOD)

		for (int i = 1; i < argc; i++)
		{
			fd = open(argv[i], O_RDONLY);
			if (fd < 0)
				perror("Couldn't open the file\n");
			file = fdopen(fd, "r");

			
			while ((newline = fgets(buffer, MAXLEN, file)) != NULL)
				printf("%s\n", newline);

			errno = fclose(file);
			if (errno < 0)
				perror("Couldn't close the file\n");
		}

	return 0;	
}