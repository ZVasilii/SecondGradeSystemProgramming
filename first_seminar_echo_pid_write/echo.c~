#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{

	//Flag that determines "-n" argument
	int new_line_mode = 0;	

	//Case with any arguments
	if (argc > 1)		 
	{
		new_line_mode = strcmp(argv[1], "-n");	

		//Cases with "-n"
		if (!new_line_mode && (argc > 2))						
		{
			for(int i = 2; i < argc; i++)
			{
				if (strcmp(argv[i], "-n") != 0)
				{
					if (i != (argc - 1))
						printf("%s ", argv[i]);
					else 
						printf("%s", argv[i]);	//Without space in the end
				}
			}
		}

		//Cases without "-n"
		if (new_line_mode && (argc >= 2))						
		{
			for(int i = 1; i < argc; i++)
			{
				if (i != (argc - 1))
					printf("%s ", argv[i]);
				else 
					printf("%s", argv[i]);	//Without space in the end
			}		
				printf("\n");
		}
	}	

	else 

		//Case without any arguments
		printf("\n");	

	return 0;
}
