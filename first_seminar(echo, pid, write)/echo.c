#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
	int flag = 0;											//Flag that determines "-n" argument

	if (argc > 1)											//Case with any arguments
	{
    	flag = strcmp(argv[1], "-n");			

		if ((flag == 0) && (argc > 2))						//Cases with "-n"
		{
			for(int i = 2; i < argc - 1; i++)
			{
				if (strcmp(argv[i], "-n") != 0)
					printf("%s ", argv[i]);
			}
				if (strcmp(argv[argc - 1], "-n") != 0)
					printf("%s", argv[argc - 1]);			//Last elem without space
		}


		if ((flag != 0) && (argc >= 2))						//Cases without "-n"
		{
			for(int i = 1; i < argc - 1; i++)
				printf("%s ", argv[i]);

			printf("%s", argv[argc - 1]);					//Last elem without space
				printf("\n");
		}

	}	

	else 
		printf("\n");										//Case without any arguments
	return 0;
}