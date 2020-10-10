#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

const size_t buffsize = 4096;

//mywrite

int main(int argc, int** argv)
{
	char* buffer = (char*) malloc (buffsize * sizeof(char));
	if (argc == 1) 
	{
		int rfl = 1;
		int wfl = 1;
		while (rfl != 0)
		{
			rfl = read(0, buffer, buffsize);
			if (rfl < 0) perror("Error with read!\n");

			wfl = write(1, buffer, buffsize);
		}
	}

	free(buffer);
	return 0;
}